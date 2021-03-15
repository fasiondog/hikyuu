/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <iterator>
#include <yyjson.h>
#include "exception.h"

namespace yyjson {

template <class T>
class ArrIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    ArrIterator(T start_val, size_t idx) : m_start_val(start_val), m_idx(idx) {
        if (!m_start_val.is_arr()) {
            YYJSON_THROW("This value is not array!");
        }
    }

    //赋值
    ArrIterator &operator=(const ArrIterator &iter) {
        m_idx = iter.m_idx;
    }

    //不等于
    bool operator!=(const ArrIterator &iter) {
        return m_idx != iter.m_idx;
    }

    //等于
    bool operator==(const ArrIterator &iter) {
        return m_idx == iter.m_idx;
    }

    //前缀自加
    ArrIterator &operator++() {
        m_idx++;
        return *this;
    }

    //后缀自加
    ArrIterator operator++(int) {
        ArrIterator tmp = *this;
        m_idx++;
        return tmp;
    }

    //取值
    T operator*() {
        return m_start_val.arr_get(m_idx);
    }

private:
    T m_start_val;
    size_t m_idx;
    size_t m_max;
    T *_ptr;  //实际的内容指针，通过该指针跟容器连接
};

#define YY_VAL_IS(typ)                 \
    bool is_##typ() const {            \
        return yyjson_is_##typ(m_val); \
    }

#define YY_VAL_GET(rtyp, typ)                                                      \
    rtyp get_##typ() const {                                                       \
        if (!yyjson_is_##typ(m_val)) {                                             \
            YYJSON_THROW("This value type is {}, not " #typ "!", get_type_desc()); \
        }                                                                          \
        return unsafe_yyjson_get_##typ(m_val);                                     \
    }

class val_view {
public:
    val_view(yyjson_val *val) : m_val(val) {}

    yyjson_val *const ptr() const {
        return m_val;
    }

    operator bool() const {
        return m_val != nullptr;
    }

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

    bool get_bool(bool fallback) const {
        return yyjson_is_bool(m_val) ? unsafe_yyjson_get_bool(m_val) : fallback;
    }

    bool get_bool() const {
        if (!yyjson_is_bool(m_val)) {
            YYJSON_THROW("This value type is {}, not bool!", get_type_desc());
        }
        return yyjson_get_bool(m_val);
    }

    uint64_t get_uint(uint64_t fallback) const {
        return yyjson_is_int(m_val) ? unsafe_yyjson_get_uint(m_val) : fallback;
    }

    uint64_t get_uint() const {
        if (!yyjson_is_int(m_val)) {
            YYJSON_THROW("This vale type is {}, not uint!", get_type_desc());
        }
        return unsafe_yyjson_get_uint(m_val);
    }

    int64_t get_int(int64_t fallback) const {
        return yyjson_is_int(m_val) ? unsafe_yyjson_get_sint(m_val) : fallback;
    }

    int64_t get_int() const {
        if (!yyjson_is_int(m_val)) {
            YYJSON_THROW("This value type is {}, not int!", get_type_desc());
        }
        return unsafe_yyjson_get_sint(m_val);
    }

    double get_double(double fallback) const {
        return yyjson_is_num(m_val) ? unsafe_yyjson_get_real(m_val) : fallback;
    }

    double get_double() const {
        if (!yyjson_is_num(m_val)) {
            YYJSON_THROW("This value type is {}, not double!", get_type_desc());
        }
        return unsafe_yyjson_get_real(m_val);
    }

    std::string get_str() const {
        if (!yyjson_is_str(m_val)) {
            YYJSON_THROW("This value type is {}, not string!", get_type_desc());
        }
        return std::string(unsafe_yyjson_get_str(m_val));
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

    typedef ArrIterator<val_view> iterator;
    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, arr_size());
    }

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