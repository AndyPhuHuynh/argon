#pragma once

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>

inline auto is_number(std::string_view str) -> bool {
    if (str.empty()) return true;
    if (str[0] == '-') {
        if (str.size() <= 1) {
            return false;
        }
        str = str.substr(1, str.size() - 1);
    }
    return std::ranges::all_of(str, [](const char ch) {
        return std::isdigit(ch);
    });
}

inline auto add_one_to_positive(const std::string_view str) -> std::string {
    if (!is_number(str)) {
        throw std::invalid_argument("not a number");
    }
    if (str.empty()) {
        return "1";
    }
    if (str.starts_with('-')) {
        throw std::invalid_argument("not positive");
    }
    std::string result{str};

    int carry = 1;
    for (int i = static_cast<int>(result.size()) - 1; i >= 0 && carry; i--) {
        const int digit = (result[i] - '0') + carry;
        carry = digit / 10;
        result[i] = static_cast<char>((digit % 10) + '0');
    }
    if (carry > 0) {
        result.insert(result.begin(), '1');
    }
    return result;
}

inline auto subtract_one_from_negative(const std::string_view str) -> std::string {
    if (!is_number(str)) {
        throw std::invalid_argument("not a number");
    }
    if (str.empty() || str[0] == '0') {
        return "-1";
    }
    if (str[0] != '-') {
        throw std::invalid_argument("not negative");
    }
    std::string result = add_one_to_positive(str.substr(1));
    result.insert(result.begin(), '-');
    return result;
}

inline int div_mod(std::string& dec, const int base) {
    int carry = 0;

    for (char& c : dec) {
        const int cur = carry * 10 + (c - '0');
        c = static_cast<char>('0' + cur / base);
        carry = cur % base;
    }

    dec.erase(0, dec.find_first_not_of('0'));
    if (dec.empty())
        dec = "0";

    return carry;
}


inline std::string dec_to_hex(std::string dec) {
    bool negative = false;

    if (!dec.empty() && dec[0] == '-') {
        negative = true;
        dec.erase(0, 1);
    }

    if (dec == "0")
        return "0";

    std::string hex;
    while (dec != "0") {
        const int rem = div_mod(dec, 16);
        hex.push_back("0123456789abcdef"[rem]);
    }

    std::ranges::reverse(hex);
    if (negative) {
        hex.insert(0, "-0x");
    }  else {
        hex.insert(0, "0x");
    }

    return hex;
}

inline std::string dec_to_bin(std::string dec) {
    bool negative = false;

    if (!dec.empty() && dec[0] == '-') {
        negative = true;
        dec.erase(0, 1);
    }

    if (dec == "0")
        return "0";

    std::string hex;
    while (dec != "0") {
        const int rem = div_mod(dec, 2);
        hex.push_back("01"[rem]);
    }

    std::ranges::reverse(hex);
    if (negative) {
        hex.insert(0, "-0b");
    }  else {
        hex.insert(0, "0b");
    }

    return hex;
}