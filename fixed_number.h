#ifndef ARCH_LAB1_FIXED_NUMBER_H
#define ARCH_LAB1_FIXED_NUMBER_H

#include <cstdint>
#include <string>
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "round.h"

class FixedNumber {
private:
    int a_size_;
    int b_size_;

    uint32_t actual_number_; // A + B bits

    uint32_t sign;

    int32_t integer_part_value_; // A bits

    uint32_t frac_part_value_; // B bits

    [[nodiscard]] int32_t get_signed_integer() const;

    [[nodiscard]] uint32_t get_signed_frac() const;

    static uint32_t make_round(int64_t product, uint64_t mod, RoundType rt, uint32_t res_sign) ;

    [[nodiscard]] std::pair<std::string, std::string> get_decimal_signs(RoundType rt);

    [[nodiscard]] std::pair<std::string, std::string> get_even_decimal(uint64_t thousands);

    static std::string get_three_signs(uint64_t value);

    [[nodiscard]] std::pair<std::string, std::string> get_up_rounded_abs(uint64_t thousands) const;

    [[nodiscard]] std::pair<std::string, std::string> get_down_rounded_abs(uint64_t thousands) const;

public:
    FixedNumber() : a_size_(1), b_size_(0), actual_number_(0),
                    integer_part_value_(0), frac_part_value_(0), sign(0) {};

    ~FixedNumber() = default;

    FixedNumber(const FixedNumber& other) = default;

    FixedNumber(const FixedNumber&& other);

    FixedNumber(uint32_t value, int aSize, int bSize);

    FixedNumber& operator=(const FixedNumber& other);

    void swap(FixedNumber& other) noexcept;

    std::string get_decimal(RoundType rt);

    [[nodiscard]] FixedNumber sum(const FixedNumber& right) const;

    [[nodiscard]] FixedNumber minus(const FixedNumber& right) const;

    [[nodiscard]] FixedNumber multiply(const FixedNumber& right, RoundType rt) const;

    [[nodiscard]] FixedNumber divide(const FixedNumber& right, RoundType rt) const;

    [[nodiscard]] uint32_t get_ab_bits() const;
};

#endif //ARCH_LAB1_FIXED_NUMBER_H
