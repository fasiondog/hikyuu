/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Price serialization helpers: the text/xml archives cannot read back nan/inf (the C++ stream
 *  extraction rejects them), while the price fields legitimately carry Null<price_t> (nan) when
 *  no value is set, so they are written as a string and parsed back.
 */

#pragma once
#ifndef HKU_UTILITIES_PRICE_SERIALIZATION_H
#define HKU_UTILITIES_PRICE_SERIALIZATION_H

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace hku {

inline std::string priceToSerialString(double v) {
    if (std::isnan(v)) {
        return "nan";
    }
    if (std::isinf(v)) {
        return v > 0.0 ? "inf" : "-inf";
    }
    std::ostringstream ss;
    ss << std::setprecision(std::numeric_limits<double>::max_digits10) << v;
    return ss.str();
}

inline double priceFromSerialString(const std::string& s) {
    if (s == "nan" || s == "-nan") {
        return std::numeric_limits<double>::quiet_NaN();
    }
    if (s == "inf" || s == "+inf") {
        return std::numeric_limits<double>::infinity();
    }
    if (s == "-inf") {
        return -std::numeric_limits<double>::infinity();
    }
    return std::strtod(s.c_str(), nullptr);
}

}  // namespace hku

#endif  // HKU_UTILITIES_PRICE_SERIALIZATION_H
