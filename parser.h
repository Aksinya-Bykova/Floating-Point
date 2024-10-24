#ifndef ARCHNUMBERSSE_PARSER_H
#define ARCHNUMBERSSE_PARSER_H

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include "fixed_number.h"
#include "float_number32.h"
#include "float_number16.h"

void parse(int argc, const char** argv);

void check_input_valid(int argc, const char** argv);

void check_number(const std::string& str);

void check_form(const std::string& str);

std::pair<int, int> get_form(const std::string& str);

uint32_t get_number(const std::string& str);

FixedNumber product(const FixedNumber& left, const FixedNumber& right,
                    const std::string& ar_operation, RoundType rt);

/**std::stoi can catch exception,
 * but we always need to exit with non-zero code
 * and print a message with std::cerr
 **/
bool is_decimal_number(const std::string& str);

FloatNumber32 product(const FloatNumber32& left, const FloatNumber32& right,
                      const std::string& ar_operation, RoundType rt);

FloatNumber16 product(const FloatNumber16& left, const FloatNumber16& right,
                      const std::string& ar_operation, RoundType rt);

#endif //ARCHNUMBERSSE_PARSER_H
