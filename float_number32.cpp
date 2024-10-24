#include "float_number32.h"

std::string FloatNumber32::get_hexadecimal(RoundType rt) const {
    if (is_plus_zero()) {
        return "0x0.000000p+0";
    }
    if (is_minus_zero()) {
        return "-0x0.000000p+0";
    }
    if (is_plus_infinity()) {
        return "inf";
    }
    if (is_neg_infinity()) {
        return "-inf";
    }
    if (is_nan()) {
        return "nan";
    }

    std::string res;

    if (sign_ == 1) {
        res += "-";
    }
    res += "0x1.";

    int32_t ans_exponent;

    if (!is_denormalized()) {
        std::stringstream stream;
        stream << std::hex << std::setw(6) << std::setfill('0') << mantissa_ * 2;
        res += stream.str();
        ans_exponent = exponent_ - 127;
    } else {
        std::stringstream stream;
        stream << std::hex << std::setw(6) << std::setfill('0') <<
               get_normalized(mantissa_).second * 2;
        res += stream.str();

        ans_exponent = -126;
        ans_exponent -= get_normalized(mantissa_).first;
    }

    res += "p";

    if (ans_exponent >= 0) {
        res += "+";
    }

    res += std::to_string(ans_exponent);

    return res;
}

bool FloatNumber32::is_neg_infinity() const {
    return (sign_ == 1) && (exponent_ == 255) && (mantissa_ == 0);;
}

bool FloatNumber32::is_plus_infinity() const {
    return (sign_ == 0) && (exponent_ == 255) && (mantissa_ == 0);
}

bool FloatNumber32::is_plus_zero() const {
    return (sign_ == 0) && (exponent_ == 0) && (mantissa_ == 0);
}

bool FloatNumber32::is_minus_zero() const {
    return (sign_ == 1) && (exponent_ == 0) && (mantissa_ == 0);
}

bool FloatNumber32::is_denormalized() const {
    return (exponent_ == 0) && (mantissa_ != 0);
}

bool FloatNumber32::is_nan() const {
    return (exponent_ == 255) && (mantissa_ != 0);
}

FloatNumber32 FloatNumber32::multiply(const FloatNumber32& other, RoundType rt) const {
    check_multiply_special(other);

    FloatNumber32 res{};
    res.sign_ = (sign_ != other.sign_);
    res.mantissa_ = make_multiply_round(other, res.sign_, rt).first;
    res.exponent_ = make_multiply_round(other, res.sign_, rt).second;

    return res;
}

std::pair<uint32_t, uint32_t> FloatNumber32::make_multiply_round(const FloatNumber32& other,
                                                                 uint32_t sign, RoundType rt) const {
    uint64_t left = is_denormalized() ? mantissa_ : (1 << 23) + mantissa_;
    uint64_t right = other.is_denormalized() ? other.mantissa_ : (1 << 23) + other.mantissa_;
    uint64_t prod_mantissa = left * right;
    uint64_t shift_mantissa, rem_mantissa, half_rem_mantissa;
    int64_t prod_exponent;

    if (is_denormalized() || other.is_denormalized()) {
        return get_multiply_denormalized(other, sign, rt, prod_mantissa);
    }

    check_multiply_overflow(other, sign, rt, prod_mantissa);

    if (((prod_mantissa >> 47) == 0)) {
        shift_mantissa = (prod_mantissa >> 23) & ((1 << 23) - 1);
        rem_mantissa = prod_mantissa & ((1 << 23) - 1);
        half_rem_mantissa = 1 << 22;
        prod_exponent = exponent_ + other.exponent_ - 127;
    } else {
        shift_mantissa = (prod_mantissa >> 24) & ((1 << 23) - 1);
        rem_mantissa = prod_mantissa & ((1 << 24) - 1);
        half_rem_mantissa = 1 << 23;
        prod_exponent = exponent_ + other.exponent_ + 1 - 127;
    }

    if (prod_exponent < 0) {
        return get_multiply_denormalized(other, sign, rt, prod_mantissa);
    }

    if (rt == RoundType::toward_zero) {
        return std::make_pair(shift_mantissa, prod_exponent);
    }
    if (rt == RoundType::nearest_even) {
        if (rem_mantissa > half_rem_mantissa) {
            return std::make_pair(shift_mantissa + 1, prod_exponent);
        }
        if (rem_mantissa < half_rem_mantissa) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (shift_mantissa % 2 == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        return std::make_pair(shift_mantissa + 1, prod_exponent);
    }
    if (rt == RoundType::toward_infinity) {
        if (rem_mantissa == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (sign == 0) {
            return std::make_pair(shift_mantissa + 1, prod_exponent);
        }
        return std::make_pair(shift_mantissa, prod_exponent);
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (rem_mantissa == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (sign == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        return std::make_pair(shift_mantissa + 1, prod_exponent);
    }

    std::cerr << "Program error" << std::endl;
    exit(EXIT_FAILURE);
}

FloatNumber32 FloatNumber32::divide(const FloatNumber32& other, RoundType rt) const {
    check_division_special(other);

    uint32_t one = 1 << 23;
    uint64_t lma = is_denormalized() ? mantissa_ : mantissa_ + one;
    uint64_t rma = other.is_denormalized() ? other.mantissa_ : other.mantissa_ + one;

    uint64_t full_res;

    full_res = lma << (23 + 2);
    full_res = full_res / rma;

    uint32_t exp;
    exp = 127;
    exp += exponent_ - other.exponent_;

    if (full_res < one * 4) {
        int shift = 0;

        while (full_res < one * 4) {
            ++shift;
            full_res = lma << (23 + shift + 2);
            full_res = full_res / rma;
        }

        exp -= shift;

    } else if (full_res >= one * 2 * 4) {
        int shift = 0;

        while (full_res >= one * 2 * 4) {
            ++shift;
            full_res = lma << (23 + 2 - shift);
            full_res = full_res / rma;
        }

        exp += shift;
    }

    full_res = full_res & ((1 << 23) - 1);

    FloatNumber32 result{};
    result.sign_ = (sign_ == other.sign_) ? 0 : 1;
    result.exponent_ = exp;

    uint32_t rem = full_res % rma;

    if (rt == RoundType::toward_zero) {
        result.mantissa_ = full_res;
        return result;
    }
    if (rt == RoundType::nearest_even) {
        if (rem * 2 < rma) {
            result.mantissa_ = full_res;
            return result;
        }
        if (rem * 2 > rma) {
            result.mantissa_ = full_res + 1;
            return result;
        }
        if (rem % 2 == 0) {
            result.mantissa_ = full_res;
            return result;
        }
        result.mantissa_ = full_res + 1;
        return result;
    }
    if (rt == RoundType::toward_infinity) {
        if (rem == 0) {
            result.mantissa_ = full_res;
            return result;
        }
        if (sign_ == 0) {
            result.mantissa_ = full_res + 1;
            return result;
        }
        result.mantissa_ = full_res - 1;
        return result;
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (rem == 0) {
            result.mantissa_ = full_res;
            return result;
        }
        if (sign_ == 0) {
            result.mantissa_ = full_res - 1;
            return result;
        }
        result.mantissa_ = full_res + 1;
        return result;
    }

    return result;
}

FloatNumber32 FloatNumber32::minus(const FloatNumber32& other, RoundType rt) const {
    FloatNumber32 reversed = other;
    reversed.sign_ = 1 - other.sign_;

    return sum(reversed, rt);
}

void FloatNumber32::check_general_special(const FloatNumber32& other) const {
    if (is_nan() || other.is_nan()) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
}

void FloatNumber32::check_multiply_special(const FloatNumber32& other) const {
    check_general_special(other);

    if ((is_plus_infinity() || is_neg_infinity()) &&
        (other.is_plus_zero() || other.is_minus_zero())) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_zero() || is_minus_zero()) &&
        (other.is_plus_infinity() || other.is_neg_infinity())) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_zero() || is_minus_zero())
        || (other.is_plus_zero() || other.is_minus_zero())) {
        if (sign_ == other.sign_) {
            std::cout << "0x0.000000p+0";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x0.000000p+0";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_infinity() || is_neg_infinity())
        || (other.is_plus_infinity() || other.is_neg_infinity())) {
        if (sign_ == other.sign_) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
}

void FloatNumber32::check_sum_special(FloatNumber32 other, RoundType rt) const {
    if (is_plus_infinity() && other.is_neg_infinity()) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if (is_neg_infinity() && other.is_plus_infinity()) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if (is_plus_infinity()) {
        std::cout << "inf";
        exit(EXIT_SUCCESS);
    }
    if (is_neg_infinity()) {
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
    if (is_minus_zero() && other.is_minus_zero()) {
        std::cout << "-0x0.000000p+0";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_zero() || is_minus_zero())
        && (other.is_plus_zero() || other.is_minus_zero())) {
        if (rt == RoundType::toward_neg_infinity) {
            std::cout << "-0x0.000000p+0";
            exit(EXIT_SUCCESS);
        }
        std::cout << "0x0.000000p+0";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_zero() || is_minus_zero())) {
        std::cout << other.get_hexadecimal(rt);
        exit(EXIT_SUCCESS);
    }
    if ((other.is_plus_zero() || other.is_minus_zero())) { // TODO
        std::cout << (*this).get_hexadecimal(rt);
        exit(EXIT_SUCCESS);
    }
}

void FloatNumber32::check_division_special(const FloatNumber32& other) const {
    if ((is_plus_zero() || is_minus_zero())
        && (other.is_plus_zero() || other.is_minus_zero())) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_infinity() || is_neg_infinity())
        && (other.is_plus_infinity() || other.is_neg_infinity())) {
        std::cout << "nan";
        exit(EXIT_SUCCESS);
    }
    if (other.is_plus_zero() || other.is_minus_zero()) {
        if (sign_ == other.sign_) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
    if ((is_plus_zero() || is_minus_zero()) ||
        (other.is_plus_infinity() || other.is_neg_infinity())) {
        if (sign_ == other.sign_) {
            std::cout << "0x0.000000p+0";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x0.000000p+0";
        exit(EXIT_SUCCESS);
    }
}

void FloatNumber32::check_multiply_overflow(const FloatNumber32& other,
                                            uint32_t sign, RoundType rt, uint64_t prod_mantissa) const {
    int64_t prod_exponent;

    if (((prod_mantissa >> 47) == 0)) { // TODO what if round to +inf?
        prod_exponent = exponent_ + other.exponent_ - 127;
    } else {
        prod_exponent = exponent_ + other.exponent_ + 1 - 127;
    }

    if (prod_exponent < 255) {
        return;
    }

    if (sign != 0 && sign != 1) {
        std::cerr << "Program failed";
        exit(EXIT_FAILURE);
    }
    if (rt == RoundType::toward_zero) {
        if (sign == 0) {
            std::cout << "0x1.fffffep+127";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x1.fffffep+127";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::nearest_even) {
        if (sign == 0) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::toward_infinity) {
        if (sign == 0) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x1.fffffep+127";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (sign == 0) {
            std::cout << "0x1.fffffep+127";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
}

FloatNumber32 FloatNumber32::sum(const FloatNumber32& other, RoundType rt) const {
    if ((sign_ != other.sign_) && (exponent_ == other.exponent_) &&
        (mantissa_ == other.mantissa_)) {
        std::cout << "0x0.000000p+0";
        exit(EXIT_SUCCESS);
    }

    check_sum_special(other, rt);

    if (sign_ == other.sign_) {
        return basic_sum(other, rt);
    }
    return basic_substract(other, rt);
}

bool FloatNumber32::is_abs_bigger(const FloatNumber32& other) const {
    if (exponent_ > other.exponent_) {
        return true;
    }
    if (exponent_ < other.exponent_) {
        return false;
    }
    if (mantissa_ > other.mantissa_) {
        return true;
    }
    return false;
}

FloatNumber32 FloatNumber32::basic_sum(const FloatNumber32& other, RoundType rt) const {
    FloatNumber32 abs_big = is_abs_bigger(other) ? (*this) : other;
    FloatNumber32 abs_small = is_abs_bigger(other) ? other : (*this);

    if (exponent_ == other.exponent_) {
        return basic_sum_equal_exp(abs_small, abs_big, rt);
    }
    return basic_sum_inequal_exp(abs_small, abs_big, rt);
}

FloatNumber32 FloatNumber32::basic_substract(const FloatNumber32& other, RoundType rt) const {
    FloatNumber32 abs_big = is_abs_bigger(other) ? (*this) : other;
    FloatNumber32 abs_small = is_abs_bigger(other) ? other : (*this);

    if (abs_small.int_bits_ == 0) {
        return abs_big;
    }

    if (abs_big.exponent_ == abs_small.exponent_) {
        return basic_substract_equal_exp(abs_small, abs_big, rt);
    }
    return basic_substract_inequal_exp(abs_small, abs_big, rt);
}

std::pair<int32_t, uint32_t> FloatNumber32::get_normalized(uint32_t val) {
    for (uint64_t i = 1; i <= 23; ++i) {
        if (val & (1 << (23 - i))) {
            return std::make_pair(i, (val << i) & ((1 << 23) - 1));
        }
    }
    exit(EXIT_FAILURE);
}

std::pair<int32_t, uint64_t> FloatNumber32::get_long_normalized(uint64_t val) const {
    for (size_t i = 0; i <= 46; ++i) {
        if ((val >> (46 - i)) != 0) {
            return std::make_pair(i, val << i);
        }
    }
    exit(EXIT_FAILURE);
}

// one of two operands or both are denormalized
// or result is denormalized
std::pair<uint32_t, uint32_t> FloatNumber32::get_multiply_denormalized(const FloatNumber32& other, uint32_t sign,
                                                                       RoundType rt, uint64_t prod_mantissa) const {
    uint64_t shift_mantissa, rem_mantissa, half_rem_mantissa;
    int64_t prod_exponent = 0;
    prod_exponent += is_denormalized() ? -126 : (exponent_ - 127);
    prod_exponent += other.is_denormalized() ? -126 : (other.exponent_ - 127);

    if (((prod_mantissa >> 47) == 0)) {
        if (prod_exponent + 126 <= 0) {
            shift_mantissa = (prod_mantissa >> (23 + (-1) * (prod_exponent + 126))) & ((1 << 23) - 1);
            rem_mantissa = prod_mantissa & ((1 << (23 + (-1) * (prod_exponent + 126))) - 1);
            half_rem_mantissa = 1 << ((22 + (-1) * (prod_exponent + 126)) - 1);
            prod_exponent = 0;
        } else {
            prod_exponent -= get_long_normalized(prod_mantissa).first;
            prod_mantissa = get_long_normalized(prod_mantissa).second;
            shift_mantissa = (prod_mantissa >> 23) & ((1 << 23) - 1);
            rem_mantissa = prod_mantissa & ((1 << 23) - 1);
            half_rem_mantissa = 1 << 22;
            prod_exponent += 127;
        }
    } else {
        prod_exponent += 1;

        if (prod_exponent + 126 <= 0) { // result is denormalized
            shift_mantissa = (prod_mantissa >> (24 + (-1) * (prod_exponent + 126))) & ((1 << 23) - 1);
            rem_mantissa = prod_mantissa & ((1 << (24 + (-1) * (prod_exponent + 126))) - 1);
            half_rem_mantissa = 1 << ((23 + (-1) * (prod_exponent + 126)) - 1);
            prod_exponent = 0;
        } else {
            shift_mantissa = (prod_mantissa >> 24) & ((1 << 23) - 1);
            rem_mantissa = prod_mantissa & ((1 << 24) - 1);
            half_rem_mantissa = 1 << 23;
            prod_exponent += 127;
        }
    }

    if (rt == RoundType::toward_zero) {
        return std::make_pair(shift_mantissa, prod_exponent);
    }
    if (rt == RoundType::nearest_even) {
        if (rem_mantissa > half_rem_mantissa) {
            return std::make_pair(shift_mantissa + 1, prod_exponent);
        }
        if (rem_mantissa < half_rem_mantissa) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (shift_mantissa % 2 == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        return std::make_pair(shift_mantissa + 1, prod_exponent);
    }
    if (rt == RoundType::toward_infinity) {
        if (rem_mantissa == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (sign == 0) {
            return std::make_pair(shift_mantissa + 1, prod_exponent);
        }
        return std::make_pair(shift_mantissa, prod_exponent);
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (rem_mantissa == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        if (sign == 0) {
            return std::make_pair(shift_mantissa, prod_exponent);
        }
        return std::make_pair(shift_mantissa + 1, prod_exponent);
    }

    std::cerr << "Program error" << std::endl;
    exit(EXIT_FAILURE);
}

FloatNumber32
FloatNumber32::basic_sum_equal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big, RoundType rt) {
    FloatNumber32 res{};
    res.sign_ = abs_big.sign_;

    uint64_t prod_mantissa;
    uint64_t shift_mantissa, rem_mantissa;

    if ((abs_small.exponent_ != 0) && (abs_big.exponent_ != 0)) {
        prod_mantissa = (1 << 23) + abs_small.mantissa_ + (1 << 23) + abs_big.mantissa_;
    } else {
        prod_mantissa = abs_small.mantissa_ + abs_big.mantissa_;
    }

    uint64_t res_exponent = 0;

    if ((prod_mantissa >> 24) == 0) {
        shift_mantissa = prod_mantissa & ((1 << 23) - 1);
        rem_mantissa = 0;
    } else {
        shift_mantissa = (prod_mantissa >> 1) & ((1 << 23) - 1);
        rem_mantissa = prod_mantissa & 1;
        res_exponent += 1;
    }

    if (rt == RoundType::toward_zero) {
        res.mantissa_ = shift_mantissa;
        res_exponent += abs_big.exponent_;
    }
    if (rt == RoundType::nearest_even) {
        if (rem_mantissa == 0) {
            res.mantissa_ = shift_mantissa;
            res_exponent += abs_big.exponent_;
        } else if (shift_mantissa % 2 == 0) {
            res.mantissa_ = shift_mantissa;
            res_exponent += abs_big.exponent_;
        } else {
            if (shift_mantissa != (1 << 23) - 1) {
                res.mantissa_ = shift_mantissa + 1;
                res_exponent += abs_big.exponent_;
            } else {
                res.mantissa_ = 0;
                res_exponent += abs_big.exponent_ + 1;
            }
        }
    }
    if (rt == RoundType::toward_infinity) {
        if ((rem_mantissa == 1) && (abs_big.sign_ == 0)) {
            if (shift_mantissa != (1 << 23) - 1) {
                res.mantissa_ = shift_mantissa + 1;
                res_exponent += abs_big.exponent_;
            } else {
                res.mantissa_ = 0;
                res_exponent += abs_big.exponent_ + 1;
            }
        } else {
            res.mantissa_ = shift_mantissa;
            res_exponent += abs_big.exponent_;
        }
    }
    if (rt == RoundType::toward_neg_infinity) {
        if ((rem_mantissa == 1) && (abs_big.sign_ == 1)) {
            if (shift_mantissa != (1 << 23) - 1) {
                res.mantissa_ = shift_mantissa + 1;
                res_exponent += abs_big.exponent_;
            } else {
                res.mantissa_ = 0;
                res_exponent += abs_big.exponent_ + 1;
            }
        } else {
            res.mantissa_ = shift_mantissa;
            res_exponent += abs_big.exponent_;
        }
    }

    res.exponent_ = res_exponent;
    if (res_exponent < 255) {
        return res;
    }

    print_overflow(res, rt);
    exit(EXIT_SUCCESS);
}

void FloatNumber32::print_overflow(const FloatNumber32& value, RoundType rt) {
    if (value.sign_ != 0 && value.sign_ != 1) {
        std::cerr << "Program failed";
        exit(EXIT_FAILURE);
    }
    if (rt == RoundType::toward_zero) {
        if (value.sign_ == 0) {
            std::cout << "0x1.fffffep+127";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x1.fffffep+127";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::nearest_even) {
        if (value.sign_ == 0) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::toward_infinity) {
        if (value.sign_ == 0) {
            std::cout << "inf";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-0x1.fffffep+127";
        exit(EXIT_SUCCESS);
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (value.sign_ == 0) {
            std::cout << "0x1.fffffep+127";
            exit(EXIT_SUCCESS);
        }
        std::cout << "-inf";
        exit(EXIT_SUCCESS);
    }
}

FloatNumber32
FloatNumber32::basic_sum_inequal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big, RoundType rt) const {
    FloatNumber32 res{};
    res.sign_ = abs_big.sign_;
    res.exponent_ = abs_big.exponent_;

    uint32_t a = abs_small.is_denormalized() ? 0 : (1 << 23);
    uint32_t shifted_mantissa = (a + abs_small.mantissa_) >>
                                                          (abs_big.exponent_ - abs_small.exponent_);
    uint32_t shifted_rem = (a + abs_small.mantissa_) &
                           ((1 << (abs_big.exponent_ - abs_small.exponent_)) - 1);
    uint32_t half_rem = 1 << (abs_big.exponent_ - abs_small.exponent_ - 1);
    uint64_t prod_mantissa = shifted_mantissa + (1 << 23) + abs_big.mantissa_;

    if ((prod_mantissa >> 24) == 0) {
        if (rt == RoundType::toward_zero) {
            res.mantissa_ = prod_mantissa & ((1 << 23) - 1);
            return res;
        }
        if (rt == RoundType::nearest_even) {
            if (shifted_rem < half_rem) {
                res.mantissa_ = prod_mantissa & ((1 << 23) - 1);
                return res;
            }
            if (shifted_rem > half_rem) {
                if ((prod_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                    res.mantissa_ = (prod_mantissa & ((1 << 23) - 1)) + 1;
                    return res;
                }
                res.mantissa_ = 0;
                res.exponent_ += 1;
                return res;
            }
            if ((prod_mantissa & ((1 << 23) - 1)) % 2 == 0) {
                res.mantissa_ = prod_mantissa & ((1 << 23) - 1);
                return res;
            }
            if ((prod_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                res.mantissa_ = ((prod_mantissa >> 1) & ((1 << 23) - 1)) + 1;
                return res;
            }
            res.mantissa_ = 0;
            res.exponent_ += 1;
            return res;
        }
        if (rt == RoundType::toward_infinity) {
            if ((shifted_rem != 0) && (res.sign_ == 0)) {
                if ((prod_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                    res.mantissa_ = (prod_mantissa & ((1 << 23) - 1)) + 1;
                } else {
                    res.mantissa_ = 0;
                    res.exponent_ += 1;
                }
            } else {
                res.mantissa_ = prod_mantissa & ((1 << 23) - 1);
            }
            return res;
        }
        if (rt == RoundType::toward_neg_infinity) {
            if ((shifted_rem != 0) && (res.sign_ == 1)) {
                if ((prod_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                    res.mantissa_ = (prod_mantissa & ((1 << 23) - 1)) + 1;
                } else {
                    res.mantissa_ = 0;
                    res.exponent_ += 1;
                }
            } else {
                res.mantissa_ = prod_mantissa & ((1 << 23) - 1);
            }
            return res;
        }
    }

    res.exponent_ += 1;
    if (res.exponent_ > 255) {
        print_overflow(res, rt);
    }

    if (rt == RoundType::toward_zero) {
        res.mantissa_ = (prod_mantissa >> 1) & ((1 << 23) - 1);
    }
    if (rt == RoundType::nearest_even) {
        if (shifted_rem < half_rem) {
            res.mantissa_ = (prod_mantissa >> 1) & ((1 << 23) - 1);
        } else if (shifted_rem > half_rem) {
            if (((prod_mantissa >> 1) & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                res.mantissa_ = ((prod_mantissa >> 1) & ((1 << 23) - 1)) + 1;
            } else {
                res.mantissa_ = 0;
                res.exponent_ += 1;
            }
        } else if (((prod_mantissa >> 1) & ((1 << 23) - 1)) % 2 == 0) {
            res.mantissa_ = (prod_mantissa >> 1) & ((1 << 23) - 1);
        } else {
            if (((prod_mantissa >> 1) & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                res.mantissa_ = ((prod_mantissa >> 1) & ((1 << 23) - 1)) + 1;
            } else {
                res.mantissa_ = 0;
                res.exponent_ += 1;
            }
        }
    }
    if (rt == RoundType::toward_infinity) {
        if ((shifted_rem != 0) && (res.sign_ == 0)) {
            if (((prod_mantissa >> 1) & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                res.mantissa_ = ((prod_mantissa >> 1) & ((1 << 23) - 1)) + 1;
            } else {
                res.mantissa_ = 0;
                res.exponent_ += 1;
            }
        } else {
            res.mantissa_ = (prod_mantissa >> 1) & ((1 << 23) - 1);
        }
    }
    if (rt == RoundType::toward_neg_infinity) {
        if ((shifted_rem != 0) && (res.sign_ == 1)) {
            if (((prod_mantissa >> 1) & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                res.mantissa_ = ((prod_mantissa >> 1) & ((1 << 23) - 1)) + 1;
            } else {
                res.mantissa_ = 0;
                res.exponent_ += 1;
            }
        } else {
            res.mantissa_ = (prod_mantissa >> 1) & ((1 << 23) - 1);
        }
    }

    if (res.exponent_ > 255) {
        print_overflow(res, rt);
    }

    return res;
}

FloatNumber32
FloatNumber32::basic_substract_equal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big, RoundType rt) {
    FloatNumber32 res{};
    res.sign_ = abs_big.sign_;

    uint64_t prod_mantissa;
    uint64_t shift_mantissa;
    int64_t exp = 0;

    prod_mantissa = abs_big.mantissa_ - abs_small.mantissa_;
    exp += abs_big.exponent_;

    shift_mantissa = prod_mantissa & ((1 << 23) - 1);

    for (uint64_t i = 0; i <= 23; ++i) {
        if ((shift_mantissa >> (23 - i)) != 0) {
            exp -= i;
            shift_mantissa << i;
            break;
        }
    }

    if (exp > 0) {
        res.exponent_ = exp;
        res.mantissa_ = shift_mantissa;
        return res;
    }

    // check underflow
    exp = abs_big.exponent_;
    for (uint64_t i = 0; i <= 23; ++i) {
        if ((exp - i) != 0) {
            shift_mantissa << i;
            break;
        }
    }
    res.exponent_ = 0;
    res.mantissa_ = shift_mantissa;

    return res;
}

FloatNumber32
FloatNumber32::basic_substract_inequal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big,
                                           RoundType rt) {
    FloatNumber32 res{};
    res.sign_ = abs_big.sign_;

    int64_t exp = abs_big.exponent_;

    uint32_t a = abs_small.is_denormalized() ? 0 : (1 << 23);
    uint64_t prod_mantissa = (((1 << 23) + abs_big.mantissa_) << (abs_big.exponent_ - abs_small.exponent_))
                             - (a + abs_small.mantissa_);
    uint32_t shifted_mantissa = prod_mantissa >> (abs_big.exponent_ - abs_small.exponent_);
    uint32_t shifted_rem = prod_mantissa & ((1 << (abs_big.exponent_ - abs_small.exponent_)) - 1);
    uint32_t half_rem = 1 << (abs_big.exponent_ - abs_small.exponent_ - 1);

    if (rt == RoundType::toward_zero) {
        prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
    }
    if (rt == RoundType::nearest_even) {
        if (shifted_rem < half_rem) {
            prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
        } else if (shifted_rem < half_rem) {
            if ((shifted_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                prod_mantissa = (shifted_mantissa & ((1 << 23) - 1)) + 1;
            } else {
                prod_mantissa = 0;
                exp += 1;
            }
        } else if ((shifted_mantissa & ((1 << 23) - 1)) % 2 == 0) {
            prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
        } else {
            if ((shifted_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                prod_mantissa = (shifted_mantissa & ((1 << 23) - 1)) + 1;
            } else {
                prod_mantissa = 0;
                exp += 1;
            }
        }
    }
    if (rt == RoundType::toward_infinity) {
        if ((res.sign_ == 0) && (shifted_rem != 0)) {
            if ((shifted_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                prod_mantissa = (shifted_mantissa & ((1 << 23) - 1)) + 1;
            } else {
                prod_mantissa = 0;
                exp += 1;
            }
        } else {
            prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
        }
    }
    if (rt == RoundType::toward_neg_infinity) {
        if ((res.sign_ == 1) && (shifted_rem != 0)) {
            if ((shifted_mantissa & ((1 << 23) - 1)) != ((1 << 23) - 1)) {
                prod_mantissa = (shifted_mantissa & ((1 << 23) - 1)) + 1;
            } else {
                prod_mantissa = 0;
                exp += 1;
            }
        } else {
            prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
        }
    }

    if (exp > 0) {
        res.exponent_ = exp;
        res.mantissa_ = prod_mantissa;
        return res;
    }

    prod_mantissa = shifted_mantissa & ((1 << 23) - 1);
    exp = abs_big.exponent_;

    for (uint64_t i = 0; i <= 23; ++i) {
        if ((exp - i) != 0) {
            prod_mantissa << i;
            break;
        }
    }
    res.exponent_ = 0;
    res.mantissa_ = prod_mantissa;

    return res;
}
