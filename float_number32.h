#ifndef ARCH_LAB1_FLOAT_NUMBER32_H
#define ARCH_LAB1_FLOAT_NUMBER32_H

#include <cstdint>
#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "round.h"

struct FloatNumber32 {
    union {
        float a_float_;
        uint32_t int_bits_;
        struct {
            uint32_t mantissa_: 23;
            uint32_t exponent_: 8;
            uint32_t sign_: 1;
        };
    };

    [[nodiscard]] bool is_denormalized() const;

    [[nodiscard]] bool is_plus_infinity() const;

    [[nodiscard]] bool is_neg_infinity() const;

    [[nodiscard]] bool is_plus_zero() const;

    [[nodiscard]] bool is_minus_zero() const;

    [[nodiscard]] bool is_nan() const;

    void check_general_special(const FloatNumber32& other) const;

    void check_sum_special(FloatNumber32 other, RoundType rt) const;

    void check_multiply_special(const FloatNumber32& other) const;

    void check_division_special(const FloatNumber32& other) const;

    void check_multiply_overflow(const FloatNumber32& other,
                                 uint32_t sign, RoundType rt, uint64_t prod_mantissa) const;

    [[nodiscard]] std::pair<uint32_t, uint32_t> get_multiply_denormalized(const FloatNumber32& other,
                                                                          uint32_t sign, RoundType rt,
                                                                          uint64_t prod_mantissa) const;

    [[nodiscard]] std::pair<uint32_t, uint32_t> make_multiply_round(const FloatNumber32& other,
                                                                    uint32_t sign, RoundType rt) const;

    [[nodiscard]] bool is_abs_bigger(const FloatNumber32& other) const;

    [[nodiscard]] FloatNumber32 basic_sum(const FloatNumber32& other, RoundType rt) const;

    [[nodiscard]] static FloatNumber32 basic_sum_equal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big,
                                                           RoundType rt);

    [[nodiscard]] FloatNumber32 basic_sum_inequal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big,
                                                      RoundType rt) const;

    [[nodiscard]] FloatNumber32 basic_substract(const FloatNumber32& other, RoundType rt) const;

    [[nodiscard]] static FloatNumber32
    basic_substract_equal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big,
                              RoundType rt);

    [[nodiscard]] static FloatNumber32
    basic_substract_inequal_exp(const FloatNumber32& abs_small, const FloatNumber32& abs_big,
                              RoundType rt);

    [[nodiscard]] static std::pair<int32_t, uint32_t> get_normalized(uint32_t val);

    [[nodiscard]] std::pair<int32_t, uint64_t> get_long_normalized(uint64_t val) const;

    static void print_overflow(const FloatNumber32& value, RoundType rt);

    //..............................................

    [[nodiscard]] std::string get_hexadecimal(RoundType rt) const;

    [[nodiscard]] FloatNumber32 sum(const FloatNumber32& other, RoundType rt) const;

    [[nodiscard]] FloatNumber32 minus(const FloatNumber32& other, RoundType rt) const;

    [[nodiscard]] FloatNumber32 multiply(const FloatNumber32& other, RoundType rt) const;

    [[nodiscard]] FloatNumber32 divide(const FloatNumber32& other, RoundType rt) const;
};

#endif //ARCH_LAB1_FLOAT_NUMBER32_H
