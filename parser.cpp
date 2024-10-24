#include "parser.h"

void parse(int argc, const char** argv) {
    check_input_valid(argc, argv);

    auto rt = static_cast<RoundType>(argv[2][0] - '0');
    uint32_t number1 = get_number(std::string(argv[3]));
    uint32_t number2;
    if (argc == 6) {
        number2 = get_number(std::string(argv[5]));
    }

    if (std::string(argv[1]) == "f") {
        FloatNumber32 fn1{};
        fn1.int_bits_ = number1;
        if (argc == 4) {
            std::cout << fn1.get_hexadecimal(rt) << std::endl;
            return;
        }
        FloatNumber32 fn2{};
        fn2.int_bits_ = number2;
        FloatNumber32 res = product(fn1, fn2, std::string(argv[4]), rt);
        std::cout << res.get_hexadecimal(rt) << std::endl;
        return;
    }
    if (std::string(argv[1]) == "h") {
        FloatNumber16 fn1{};
        fn1.int_bits_ = number1;
        if (argc == 4) {
            std::cout << fn1.get_hexadecimal(rt) << std::endl;
            return;
        }
        FloatNumber16 fn2{};
        fn2.int_bits_ = number2;
        FloatNumber16 res = product(fn1, fn2, std::string(argv[4]), rt);
        std::cout << res.get_hexadecimal(rt) << std::endl;
        return;
    }

    std::pair<int, int> form_pair = get_form(std::string(argv[1]));
    FixedNumber fn1(number1, form_pair.first, form_pair.second);

    if (argc == 4) {
        std::cout << fn1.get_decimal(rt) << std::endl;
        return;
    }
    FixedNumber fn2(number2, form_pair.first, form_pair.second);
    FixedNumber res = product(fn1, fn2, std::string(argv[4]), rt);

    std::cout << res.get_decimal(rt) << std::endl;
}

void check_input_valid(int argc, const char** argv) {
    if (!(argc == 4 || argc == 6)) {
        std::cerr << "Invalid number of arguments" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string form = std::string(argv[1]);
    std::string type = std::string(argv[2]);
    std::string number1 = std::string(argv[3]);

    check_form(form);

    std::vector<std::string> valid_type = {"0", "1", "2", "3"};

    if (std::find(valid_type.begin(), valid_type.end(), type) == valid_type.end()) {
        std::cerr << "Invalid type of rounding" << std::endl;
        exit(EXIT_FAILURE);
    }

    check_number(number1);

    if (argc != 6) {
        return;
    }

    std::string ar_operation = std::string(argv[4]);

    std::vector<std::string> valid_operation = {"+", "-", "/", "*"};

    if (std::find(valid_operation.begin(), valid_operation.end(), ar_operation)
        == valid_operation.end()) {
        std::cerr << "Invalid arithmetical operation" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string number2 = std::string(argv[5]);
    check_number(number2);
}

FixedNumber product(const FixedNumber& left, const FixedNumber& right,
                    const std::string& ar_operation, RoundType rt) {
    if (ar_operation == "+") {
        return left.sum(right);
    }
    if (ar_operation == "-") {
        return left.minus(right);
    }
    if (ar_operation == "*") {
        return left.multiply(right, rt);
    }
    if (ar_operation == "/") {
        return left.divide(right, rt);
    }

    exit(EXIT_FAILURE);
}

std::pair<int, int> get_form(const std::string& str) {
    return std::make_pair(std::stoi(str.substr(0, str.find('.'))),
                          std::stoi(str.substr(str.find('.') + 1)));
}

bool is_decimal_number(const std::string& str) {
    for (auto ch: str) {
        if (!(ch >= '0' && ch <= '9')) {
            return false;
        }
    }

    return true;
}

void check_form(const std::string& str) {
    if (str == "f" || str == "h") {
        return;
    }

    if (str.find('.') == std::string::npos) {
        std::cerr << "Invalid form" << std::endl;
        exit(EXIT_FAILURE);
    }

    // A part
    std::string A_string = str.substr(0, str.find('.'));
    if (A_string.empty() || A_string.size() > 2) {
        std::cerr << A_string << std::endl;
        std::cerr << "Invalid size of A part" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!is_decimal_number(A_string)) {
        std::cerr << "Invalid A part" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (std::stoi(A_string) < 1 || std::stoi(A_string) > 32) {
        std::cerr << "Invalid value of A part" << std::endl;
        exit(EXIT_FAILURE);
    }

    // B part
    std::string B_string = str.substr(str.find('.') + 1);

    if (B_string.empty() || B_string.size() > 2) {
        std::cerr << "Invalid size of B part" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (!is_decimal_number(B_string)) {
        std::cerr << "Invalid B part" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (std::stoi(B_string) < 0 || std::stoi(B_string) > 31) {
        std::cerr << "Invalid value of B part" << std::endl;
        exit(EXIT_FAILURE);
    }

    // A.B
    if (std::stoi(A_string) + std::stoi(B_string) > 32) {
        std::cerr << "Too much A.B bits, only 32 provided" << std::endl;
        exit(EXIT_FAILURE);
    }
}

uint32_t get_number(const std::string& str) {
    return std::stoll(str.substr(2), nullptr, 16);
}

void check_number(const std::string& str) {
    if (str.size() < 3) {
        std::cerr << "Invalid size of a number" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (str.substr(0, 2) != "0x") {
        std::cerr << "Invalid notation" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (auto ch: str.substr(2)) {
        if (!(ch >= '0' && ch <= '9') && !(ch >= 'a' && ch <= 'f') &&
            !(ch >= 'A' && ch <= 'F')) {
            std::cerr << "Invalid hexadecimal number" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

FloatNumber32 product(const FloatNumber32& left, const FloatNumber32& right,
                      const std::string& ar_operation, RoundType rt) {
    if (ar_operation == "+") {
        return left.sum(right, rt);
    }
    if (ar_operation == "-") {
        return left.minus(right, rt);
    }
    if (ar_operation == "*") {
        return left.multiply(right, rt);
    }
    if (ar_operation == "/") {
        return left.divide(right, rt);
    }

    exit(EXIT_FAILURE);
}

FloatNumber16 product(const FloatNumber16& left, const FloatNumber16& right,
                      const std::string& ar_operation, RoundType rt) {
    if (ar_operation == "+") {
        return left.sum(right, rt);
    }
    if (ar_operation == "-") {
        return left.minus(right, rt);
    }
    if (ar_operation == "*") {
        return left.multiply(right, rt);
    }
    if (ar_operation == "/") {
        return left.divide(right, rt);
    }

    exit(EXIT_FAILURE);
}
