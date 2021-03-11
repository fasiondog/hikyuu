/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <yyjson.h>
#include "exception.h"

namespace yyjson {

#define YY_VAL_IS(typ)                 \
    bool is_##typ() const {            \
        return yyjson_is_##typ(m_val); \
    }

#define YY_VAL_GET(rtyp, typ)           \
    rtyp get_##typ() const {            \
        return yyjson_get_##typ(m_val); \
    }

class val_view {
public:
    val_view(yyjson_val *val) : m_val(val) {}

    YY_VAL_IS(null)
    YY_VAL_IS(true)
    YY_VAL_IS(false)
    YY_VAL_IS(bool)
    YY_VAL_IS(uint)
    YY_VAL_IS(sint)
    YY_VAL_IS(int)
    YY_VAL_IS(real)
    YY_VAL_IS(num)
    YY_VAL_IS(str)
    YY_VAL_IS(arr)
    YY_VAL_IS(obj)
    YY_VAL_IS(ctn)

    YY_VAL_GET(bool, bool)
    YY_VAL_GET(uint64_t, uint)
    YY_VAL_GET(int64_t, sint)
    YY_VAL_GET(int, int)
    YY_VAL_GET(double, real)
    YY_VAL_GET(const char *, str)

    std::string get_string() const {
        return std::string(get_str());
    }

    size_t get_len() const {
        return yyjson_get_len(m_val);
    }

    size_t size() const {
        return yyjson_get_len(m_val);
    }

    bool equals_str(const char *str) const {
        return yyjson_equals_str(m_val, str);
    }

    bool equals_str(const std::string &str) const {
        return equals_str(str.c_str());
    }

    bool equals_strn(const char *str, size_t len) const {
        return yyjson_equals_strn(m_val, str, len);
    }

private:
    yyjson_val *m_val;
};

}  // namespace yyjson