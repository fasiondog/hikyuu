/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#pragma once

#include <type_traits>
#include <string>
#include <yyjson.h>
#include "exception.h"

namespace yyjson {

#define YY_VAL_IS(typ)                 \
    bool is_##typ() const {            \
        return yyjson_is_##typ(m_val); \
    }

#define YY_VAL_GET(rtyp, typ)                                      \
    rtyp get_##typ() const {                                       \
        if (!yyjson_is_##typ(m_val)) {                             \
            YYJSON_THROW("Value type mismatch, is not " #typ "!"); \
        }                                                          \
        return unsafe_yyjson_get_##typ(m_val);                     \
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
    YY_VAL_IS(real)  // double number
    YY_VAL_IS(num)   // integer or double number
    YY_VAL_IS(str)   // c字符串
    YY_VAL_IS(arr)   // array
    YY_VAL_IS(obj)   // object
    YY_VAL_IS(ctn)   // array or object

    YY_VAL_GET(bool, bool)
    YY_VAL_GET(uint64_t, uint)
    YY_VAL_GET(int64_t, sint)
    YY_VAL_GET(double, real)
    YY_VAL_GET(const char *, str)

    int get_int() const {
        if (!yyjson_is_int(m_val)) {
            YYJSON_THROW("Value type mismatch, is not int!");
        }
        return (int)unsafe_yyjson_get_sint(m_val);
    }

    std::string get_string() const {
        return std::string(get_str());
    }

    val_view get_obj(const char *key) const {
        return val_view(yyjson_obj_get(m_val, key));
    }

    val_view operator[](const char *key) {
        return get_obj(key);
    }

    /* 0.2.0 版本尚不支持
    val_view get_obj_by_path(const char *path) {
        return val_view(yyjson_get_pointer(m_val, path));
    }*/

    size_t arr_size() const {
        return yyjson_arr_size(m_val);
    }

    val_view arr_get(size_t idx) const {
        return val_view(yyjson_arr_get(m_val, idx));
    }

    val_view arr_get_first() const {
        return val_view(yyjson_arr_get_first(m_val));
    }

    val_view arr_get_last() const {
        return val_view(yyjson_arr_get_last(m_val));
    }

    void for_each(std::function<void(val_view)> func) const {
        size_t idx, max;
        yyjson_val *val;
        yyjson_arr_foreach(m_val, idx, max, val) {
            func(val_view(val));
        }
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

    const char *get_type_desc() const {
        return yyjson_get_type_desc(m_val);
    }

private:
    yyjson_val *m_val;
};

}  // namespace yyjson