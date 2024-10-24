#ifndef ARCH_LAB1_FLOAT_NUMBER16_H
#define ARCH_LAB1_FLOAT_NUMBER16_H

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <sstream>
#include "round.h"

struct FloatNumber16 {
    union {
        uint16_t int_bits_;
        struct {
            uint16_t mantissa_: 10;
            uint16_t exponent_: 5;
            uint16_t sign_: 1;
        };
    };

    [[nodiscard]] bool is_denormalized() const;

    [[nodiscard]] bool is_plus_infinity() const;

    [[nodiscard]] bool is_neg_infinity() const;

    [[nodiscard]] bool is_plus_zero() const;

    [[nodiscard]] bool is_minus_zero() const;

    [[nodiscard]] bool is_nan() const;

    void check_general_special(const FloatNumber16& other) const;

    void check_sum_special(FloatNumber16 other, RoundType rt) const;

    void check_multiply_special(const FloatNumber16& other) const;

    void check_division_special(const FloatNumber16& other) const;

    void check_multiply_overflow(const FloatNumber16& other,
                                 uint16_t sign, RoundType rt, uint64_t prod_mantissa) const;

    [[nodiscard]] std::pair<uint16_t, uint16_t> get_multiply_denormalized(const FloatNumber16& other,
                                                                          uint16_t sign, RoundType rt,
                                                                          uint64_t prod_mantissa) const;

    [[nodiscard]] std::pair<uint16_t, uint16_t> make_multiply_round(const FloatNumber16& other,
                                                                    uint16_t sign, RoundType rt) const;

    [[nodiscard]] bool is_abs_bigger(const FloatNumber16& other) const;

    [[nodiscard]] FloatNumber16 basic_sum(const FloatNumber16& other, RoundType rt) const;

    [[nodiscard]] static FloatNumber16 basic_sum_equal_exp(const FloatNumber16& abs_small, const FloatNumber16& abs_big,
                                                           RoundType rt);

    [[nodiscard]] FloatNumber16 basic_sum_inequal_exp(const FloatNumber16& abs_small, const FloatNumber16& abs_big,
                                                      RoundType rt) const;

    [[nodiscard]] FloatNumber16 basic_substract(const FloatNumber16& other, RoundType rt) const;

    [[nodiscard]] static FloatNumber16
    basic_substract_equal_exp(const FloatNumber16& abs_small, const FloatNumber16& abs_big,
                              RoundType rt);

    [[nodiscard]] static FloatNumber16
    basic_substract_inequal_exp(const FloatNumber16& abs_small, const FloatNumber16& abs_big,
                                RoundType rt);

    [[nodiscard]] static std::pair<int16_t, uint16_t> get_normalized(uint16_t val);

    [[nodiscard]] std::pair<int16_t, uint64_t> get_long_normalized(uint64_t val) const;

    static void print_overflow(const FloatNumber16& value, RoundType rt);

    //..............................................

    [[nodiscard]] std::string get_hexadecimal(RoundType rt) const;

    [[nodiscard]] FloatNumber16 sum(const FloatNumber16& other, RoundType rt) const;

    [[nodiscard]] FloatNumber16 minus(const FloatNumber16& other, RoundType rt) const;

    [[nodiscard]] FloatNumber16 multiply(const FloatNumber16& other, RoundType rt) const;

    [[nodiscard]] FloatNumber16 divide(const FloatNumber16& other, RoundType rt) const;
};

#endif //ARCH_LAB1_FLOAT_NUMBER16_H
