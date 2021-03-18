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

struct iterator_uni : public yyjson_arr_iter, yyjson_obj_iter {};

template <class T>
class ValIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    ValIterator(T val) : m_val(val) {  //, m_idx(idx) {
        if (!val) {
            m_iter.idx = m_iter.max;
            return;
        }
        if (m_val.is_arr()) {
            yyjson_arr_iter_init(m_val.ptr(), &((yyjson_arr_iter)m_iter));
            m_is_arr = true;
        } else if (m_val.is_obj()) {
            yyjson_obj_iter_init(m_val.ptr(), &((yyjson_obj_iter)m_iter));
            m_is_arr = false;
        } else {
            YYJSON_THROW("This value is not array or object!");
        }
    }

    //赋值
    ValIterator &operator=(const ValIterator &iter) {
        m_iter = iter.m_iter;
        m_val = m_val;
    }

    //不等于
    bool operator!=(const ValIterator &iter) {
        return m_iter.idx != iter.m_iter.idx;
    }

    //等于
    bool operator==(const ValIterator &iter) {
        return m_iter.idx == iter.m_iter.idx;
    }

    //前缀自加
    ValIterator &operator++() {
        // m_idx++;
        m_val = T(yyjson_obj_iter_next(&m_iter));
        return *this;
    }

    //后缀自加
    ValIterator operator++(int) {
        ValIterator tmp = *this;
        m_val = T(yyjson_obj_iter_next(&m_iter));
        return tmp;
    }

    //取值
    T operator*() {
        return m_val;
    }

    T *operator->() {
        return &m_val;
    }

private:
    iterator_uni m_iter;
    T m_val;
    bool m_is_arr;
};

#define YY_TYPE_CHECK(typ, ...)                                                     \
    {                                                                               \
        if (!is_##typ()) {                                                          \
            throw yyjson::bad_cast(                                                 \
              fmt::format("This value type is {}, but expect " #typ, type_desc())); \
        }                                                                           \
    }

#define YY_VAL_IS(typ)                 \
    bool is_##typ() const {            \
        return yyjson_is_##typ(m_val); \
    }

#define YY_VAL_GET(rtyp, typ)                                                  \
    rtyp get_##typ() const {                                                   \
        if (!yyjson_is_##typ(m_val)) {                                         \
            YYJSON_THROW("This value type is {}, not " #typ "!", type_desc()); \
        }                                                                      \
        return unsafe_yyjson_get_##typ(m_val);                                 \
    }

class val_view {
public:
    val_view() = default;
    val_view(yyjson_val *val) : m_val(val) {}

    val_view(const val_view &) = default;
    val_view &operator=(const val_view &) = default;
    virtual ~val_view() = default;

    yyjson_val *const ptr() const {
        return m_val;
    }

    operator bool() {
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

    /** Returns value's type. */
    yyjson_type type() const {
        return yyjson_get_type(m_val);
    }

    /** Returns value's subtype. */
    yyjson_subtype subtype() const {
        return yyjson_get_subtype(m_val);
    }

    /** Returns value's tag. */
    uint8_t tag() const {
        return yyjson_get_tag(m_val);
    }

    /**
     * Returns type description, such as: "null", "string", "array", "object", "true", "false",
     * "uint", "sint", "real", "unknown"
     */
    const char *type_desc() const {
        return yyjson_get_type_desc(m_val);
    }

    bool value(bool fallback) const {
        return yyjson_is_bool(m_val) ? unsafe_yyjson_get_bool(m_val) : fallback;
    }

    uint64_t value(uint64_t fallback) const {
        return yyjson_is_int(m_val) ? unsafe_yyjson_get_uint(m_val) : fallback;
    }

    int64_t value(int64_t fallback) const {
        return yyjson_is_int(m_val) ? unsafe_yyjson_get_sint(m_val) : fallback;
    }

    int value(int fallback) const {
        int result = fallback;
        if (yyjson_is_int(m_val)) {
            try {
                result = static_cast<int>(unsafe_yyjson_get_sint(m_val));
            } catch (...) {
            }
        }
        return result;
    }

    double value(double fallback) const {
        return yyjson_is_num(m_val) ? unsafe_yyjson_get_real(m_val) : fallback;
    }

    std::string value(const char *fallback) {
        return yyjson_is_str(m_val) ? std::string(unsafe_yyjson_get_str(m_val))
                                    : std::string(fallback);
    }

    std::string value(const std::string &fallback) {
        return yyjson_is_str(m_val) ? std::string(unsafe_yyjson_get_str(m_val)) : fallback;
    }

    template <typename T>
    T value() const {
        YYJSON_THROW("Unsupport type!");
    }

    template <>
    bool value() const {
        YY_TYPE_CHECK(bool);
        return unsafe_yyjson_get_bool(m_val);
    }

    template <>
    uint64_t value() const {
        YY_TYPE_CHECK(int);
        return unsafe_yyjson_get_uint(m_val);
    }

    template <>
    int64_t value() const {
        YY_TYPE_CHECK(int);
        return unsafe_yyjson_get_sint(m_val);
    }

    template <>
    int value() const {
        YY_TYPE_CHECK(int);
        return static_cast<int>(unsafe_yyjson_get_sint(m_val));
    }

    template <>
    double value() const {
        YY_TYPE_CHECK(num);
        return unsafe_yyjson_get_real(m_val);
    }

    template <>
    std::string value() const {
        YY_TYPE_CHECK(str);
        return std::string(unsafe_yyjson_get_str(m_val));
    }

    val_view get(const char *key) const {
        return val_view(yyjson_obj_get(m_val, key));
    }

    val_view get(const std::string &key) const {
        return val_view(yyjson_obj_get(m_val, key.c_str()));
    }

    val_view get(size_t idx) const {
        return val_view(yyjson_arr_get(m_val, idx));
    }

    val_view operator[](const char *key) {
        return get(key);
    }

    val_view operator[](const std::string &key) {
        return get(key);
    }

    val_view operator[](size_t idx) {
        return get(idx);
    }

    /* 0.2.0 版本尚不支持
    val_view get_obj_by_path(const char *path) {
        return val_view(yyjson_get_pointer(m_val, path));
    }*/

    /*typedef ValIterator<val_view> iterator;
    iterator begin() {
        return iterator(*this);
    }

    iterator end() {
        return iterator(val_view());
    }*/

    size_t arr_size() const {
        return yyjson_arr_size(m_val);
    }

    /** Returns the number of key-value pairs in this object, or 0 if input is not an object. */
    size_t obj_size() const {
        return yyjson_obj_size(m_val);
    }

    /** Returns the number of key-value pairs in the object , or the number of arr, else 0. */
    size_t size() const {
        if (is_arr()) {
            return yyjson_arr_size(m_val);
        } else if (is_obj()) {
            return yyjson_obj_size(m_val);
        } else {
            return 0;
        }
    }

    void for_each(std::function<void(val_view)> func) const {
        size_t idx = 0, max = 0;
        yyjson_val *val = nullptr;
        yyjson_arr_foreach(m_val, idx, max, val) {
            func(val_view(val));
        }
    }

    void for_each(std::function<void(val_view, val_view)> func) const {
        size_t idx = 0, max = 0;
        yyjson_val *key = nullptr, *val = nullptr;
        yyjson_obj_foreach(m_val, idx, max, key, val) {
            func(val_view(key), val_view(val));
        }
    }

private:
    yyjson_val *m_val{nullptr};
};

}  // namespace yyjson