#include "fixed_number.h"

FixedNumber::FixedNumber(uint32_t value, int aSize, int bSize) :
        a_size_(aSize), b_size_(bSize), actual_number_(value),
        integer_part_value_(0), frac_part_value_(0), sign(0) {
    sign = (actual_number_ >> (a_size_ + b_size_ - 1)) & 1;
    integer_part_value_ = get_signed_integer();
    frac_part_value_ = get_signed_frac();
}

int32_t FixedNumber::get_signed_integer() const {
    uint32_t cut_number = get_ab_bits();

    if (sign == 0) {
        return cut_number >> b_size_;
    }

    uint64_t abs_number;
    if (a_size_ + b_size_ != 32) {
        abs_number = (1 << (a_size_ + b_size_)) - cut_number;
    } else {
        abs_number = ((1 << (a_size_ + b_size_ - 1)) - (cut_number >> 1)) << 1;
    }
    return (-1) * (int32_t) (abs_number >> b_size_);
}

uint32_t FixedNumber::get_ab_bits() const {
    if (a_size_ + b_size_ < 32) {
        return actual_number_ & ((1 << (a_size_ + b_size_)) - 1);
    }
    return actual_number_;
}

uint32_t FixedNumber::get_signed_frac() const {
    uint64_t cut_number = get_ab_bits();

    if (sign == 0) {
        return cut_number & ((1 << b_size_) - 1);
    }

    uint64_t abs_number;
    if (a_size_ + b_size_ != 32) {
        abs_number = (1 << (a_size_ + b_size_)) - cut_number;
    } else {
        abs_number = ((1 << (a_size_ + b_size_ - 1)) - (cut_number >> 1)) << 1;
    }

    return abs_number & ((1 << b_size_) - 1);
}

FixedNumber& FixedNumber::operator=(const FixedNumber& other) {
    auto tmp = other;
    swap(tmp);
    return *this;
}

void FixedNumber::swap(FixedNumber& other) noexcept {
    std::swap(a_size_, other.a_size_);
    std::swap(b_size_, other.b_size_);
    std::swap(actual_number_, other.actual_number_);
    std::swap(integer_part_value_, other.integer_part_value_);
    std::swap(frac_part_value_, other.frac_part_value_);
    std::swap(sign, other.sign);
}

std::string FixedNumber::get_decimal(RoundType rt) {
    if (b_size_ == 0) {
        return std::to_string(integer_part_value_) + ".000";
    }

    std::string decimal_integer = get_decimal_signs(rt).first;
    std::string decimal_frac = get_decimal_signs(rt).second;

    if ((decimal_integer) == "0" && (sign == 1) && (decimal_frac) != "000") {
        return "-" + decimal_integer + "." + decimal_frac;
    }

    if (decimal_integer[0] != '-' && sign == 1 && (decimal_integer) != "0" && (decimal_frac) != "000") {
        return "-" + decimal_integer + "." + decimal_frac;
    }
    return decimal_integer + "." + decimal_frac;
}

FixedNumber FixedNumber::sum(const FixedNumber& right) const {
    return {actual_number_ + right.actual_number_, a_size_, b_size_};
}

FixedNumber FixedNumber::minus(const FixedNumber& right) const {
    return {actual_number_ - right.actual_number_, a_size_, b_size_};
}

FixedNumber FixedNumber::multiply(const FixedNumber& right, RoundType rt) const {
    if (actual_number_ == 0 || right.actual_number_ == 0) {
        std::cout << "0.000";
        exit(EXIT_SUCCESS);
    }

    uint32_t temp1, temp2;
    if (sign == 0) {
        if (a_size_ + b_size_ != 32) {
            temp1 = actual_number_ & ((1 << (a_size_ + b_size_)) - 1);
        } else {
            temp1 = actual_number_;
        }
    } else {
        if (a_size_ + b_size_ != 32) {
            temp1 = (1 << (a_size_ + b_size_)) - (actual_number_ & ((1 << (a_size_ + b_size_)) - 1));
        } else {
            temp1 = (uint32_t) (((uint64_t) (1 << 31) - ((actual_number_ & ((uint64_t) (1 << 32) - 1)) >> 1)) << 1);
        }
    }

    if (right.sign == 0) {
        if (a_size_ + b_size_ != 32) {
            temp2 = right.actual_number_ & ((1 << (a_size_ + b_size_)) - 1);
        } else {
            temp2 = right.actual_number_;
        }
    } else {
        if (a_size_ + b_size_ != 32) {
            temp2 = (1 << (a_size_ + b_size_)) - (right.actual_number_ & ((1 << (a_size_ + b_size_)) - 1));
        } else {
            temp2 = (uint32_t) (((uint64_t) (1 << 31) - ((right.actual_number_ & ((uint64_t) (1 << 32) - 1)) >> 1))
                    << 1);
        }
    }

    FixedNumber result;
    int64_t product = (int64_t) temp1 * temp2;
    uint64_t mod = (1 << b_size_);
    auto actual_res = make_round(product, mod, rt, (sign == right.sign) ? 0 : 1);

    if (a_size_ + b_size_ != 32) {
        result = {actual_res & ((1 << (a_size_ + b_size_)) - 1), a_size_, b_size_};
    } else {
        result = {actual_res, a_size_, b_size_};
    }
    result.sign = (sign == right.sign) ? 0 : 1;

    return result;
}

FixedNumber FixedNumber::divide(const FixedNumber& right, RoundType rt) const {
    uint32_t temp1, temp2;
    if (sign == 0) {
        if (a_size_ + b_size_ != 32) {
            temp1 = actual_number_ & ((1 << (a_size_ + b_size_)) - 1);
        } else {
            temp1 = actual_number_;
        }
    } else {
        if (a_size_ + b_size_ != 32) {
            temp1 = (1 << (a_size_ + b_size_)) - (actual_number_ & ((1 << (a_size_ + b_size_)) - 1));
        } else {
            temp1 = ((1 << 31) - ((actual_number_ & ((uint32_t) (1 << 31) - 1)) >> 1)) << 1;
        }
    }

    if (right.sign == 0) {
        if (a_size_ + b_size_ != 32) {
            temp2 = right.actual_number_ & ((1 << (a_size_ + b_size_)) - 1);
        } else {
            temp2 = right.actual_number_;
        }
    } else {
        if (a_size_ + b_size_ != 32) {
            temp2 = (1 << (a_size_ + b_size_)) - (right.actual_number_ & ((1 << (a_size_ + b_size_)) - 1));
        } else {
            temp2 = ((1 << 31) - ((right.actual_number_ & ((uint32_t) (1 << 31) - 1)) >> 1)) << 1;
        }
    }

    if (actual_number_ == 0) {
        std::cout << "0.000";
        exit(EXIT_SUCCESS);
    }
    if (a_size_ == 1 && b_size_ == 0) {
        std::cout << "0.000";
        exit(EXIT_SUCCESS);
    }
    if (temp2 == 0) {
        std::cout << "division by zero";
        exit(EXIT_SUCCESS);
    }

    FixedNumber result;

    int64_t product = (int64_t) temp1 * (1 << b_size_);
    uint64_t mod;
    if (a_size_ + b_size_ != 32) {
        mod = temp2 & ((1 << (a_size_ + b_size_)) - 1);
    } else {
        mod = temp2;
    }

    auto actual_res = make_round(product, mod, rt, (sign == right.sign) ? 0 : 1);
    result = {actual_res, a_size_, b_size_};
    result.sign = (sign == right.sign) ? 0 : 1;

    return result;
}

std::pair<std::string, std::string> FixedNumber::get_decimal_signs(RoundType rt) {
    uint64_t thousands = ((uint64_t) 1e3 * frac_part_value_) >> b_size_;

    if (frac_part_value_ == 0) {
        return std::make_pair(std::to_string(integer_part_value_), get_three_signs(0));
    }
    if (thousands != 0) {
        // 12.23900000
        uint64_t nok = thousands * frac_part_value_;
        if (1000 * nok / thousands == (uint64_t) (1 << b_size_) * nok / frac_part_value_) {
            return std::make_pair(std::to_string(integer_part_value_), get_three_signs(thousands));
        }
    }

    // 12.23930303
    if (rt == RoundType::toward_zero) {
        return get_down_rounded_abs(thousands);
    }
    if (rt == RoundType::nearest_even) {
        return get_even_decimal(thousands);
    }
    if (rt == RoundType::toward_infinity) {
        if (sign == 0) {
            return get_up_rounded_abs(thousands);
        }
        return get_down_rounded_abs(thousands);
    }

    if (rt == RoundType::toward_neg_infinity) {
        if (sign == 0) {
            return get_down_rounded_abs(thousands);
        }
        return get_up_rounded_abs(thousands);
    }

    std::cerr << "Program error" << std::endl;
    exit(EXIT_FAILURE);
}

std::pair<std::string, std::string> FixedNumber::get_up_rounded_abs(uint64_t thousands) const {
    if (thousands + 1 == 1000) {
        if (sign == 0) {
            return std::make_pair(std::to_string(integer_part_value_ + 1), get_three_signs(0));
        }
        return std::make_pair(std::to_string(integer_part_value_ - 1), get_three_signs(0));
    }
    return std::make_pair(std::to_string(integer_part_value_), get_three_signs(thousands + 1));
}

std::pair<std::string, std::string> FixedNumber::get_even_decimal(uint64_t thousands) {
    uint64_t x = frac_part_value_ & ((1 << b_size_) - 1);
    uint64_t y = ((uint64_t) 1e9 * x) >> b_size_;
    uint64_t middle = (uint64_t) 1e5 * (((1000 * x) >> b_size_) * 10 + 5);

    if (y < middle) {
        return get_down_rounded_abs(thousands);
    } else if (y > middle) {
        return get_up_rounded_abs(thousands);
    }

    if (thousands % 2 == 0) {
        return get_down_rounded_abs(thousands);
    }
    return get_up_rounded_abs(thousands);
}

std::string FixedNumber::get_three_signs(uint64_t value) {
    if (value >= 1000) {
        std::cerr << "Program error" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (value >= 100) {
        return std::to_string(value);
    }
    if (value >= 10) {
        return "0" + std::to_string(value);
    }
    return "00" + std::to_string(value);
}

std::pair<std::string, std::string> FixedNumber::get_down_rounded_abs(uint64_t thousands) const {
    return std::make_pair(std::to_string(integer_part_value_), get_three_signs(thousands));
}

uint32_t FixedNumber::make_round(int64_t product, uint64_t mod, RoundType rt, uint32_t res_sign) {
    if (rt == RoundType::toward_zero) {
        return product / mod;
    }
    if (rt == RoundType::nearest_even) {
        if ((product % mod) * 2 < mod) {
            return product / mod;
        }
        if ((product % mod) * 2 > mod) {
            return product / mod + 1;
        }
        if ((product / mod) % 2 == 0) {
            return product / mod;
        }
        return product / mod + 1;
    }
    if (rt == RoundType::toward_infinity) {
        if (product % mod == 0) {
            return product / mod;
        }
        if (!res_sign) {
            return product / mod + 1;
        }
        return product / mod - 1;
    }
    if (rt == RoundType::toward_neg_infinity) {
        if (product % mod == 0) {
            return product / mod;
        }
        if (!res_sign) {
            return product / mod - 1;
        }
        return product / mod + 1;
    }

    std::cerr << "Program error" << std::endl;
    exit(EXIT_FAILURE);
}
