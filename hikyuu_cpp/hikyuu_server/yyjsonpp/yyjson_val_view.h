/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#pragma once

#include <iostream>
#include <type_traits>
#include <string>
#include <vector>
#include <iterator>
#include <utility>
#include <yyjson.h>
#include "exception.h"

namespace yyjson {

template <class T>
class ValArrIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    ValArrIterator(T val) : m_val(val) {
        if (m_val && m_val.is_arr()) {
            m_idx = 0;
            m_max = unsafe_yyjson_get_len(m_val.ptr());
            m_val.ptr(unsafe_yyjson_get_first(m_val.ptr()));
        }
    }

    //赋值
    ValArrIterator &operator=(const ValArrIterator &iter) {
        m_val = iter.m_val;
        m_idx = iter.m_idx;
        m_max = iter.m_max;
    }

    //不等于
    bool operator!=(const ValArrIterator &iter) {
        return m_val != iter.m_val;
    }

    //等于
    bool operator==(const ValArrIterator &iter) {
        return m_val == iter.m_val;
    }

    //前缀自加
    ValArrIterator &operator++() {
        if (m_val && m_idx < m_max) {
            m_idx++;
            if (m_idx < m_max) {
                m_val.ptr(unsafe_yyjson_get_next(m_val.ptr()));
            } else {
                m_val.ptr(nullptr);
            }
        }
        return *this;
    }

    //后缀自加
    ValArrIterator operator++(int) {
        ValArrIterator tmp = *this;
        if (m_val && m_idx < m_max) {
            m_idx++;
            if (m_idx < m_max) {
                m_val.ptr(unsafe_yyjson_get_next(m_val.ptr()));
            } else {
                m_val.ptr(nullptr);
            }
        }
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
    T m_val;
    size_t m_idx{0};
    size_t m_max{0};
};

template <class T>
class ValObjIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    ValObjIterator(T val) {
        if (val && val.is_obj()) {
            m_idx = 0;
            m_max = unsafe_yyjson_get_len(val.ptr());
            yyjson_val *cur = unsafe_yyjson_get_first(val.ptr());
            m_cur = std::make_pair(cur, cur + 1);
            m_pre = m_cur;
        }
    }

    //赋值
    ValObjIterator &operator=(const ValObjIterator &iter) {
        m_cur = iter.m_cur;
        m_pre = iter.m_pre;
        m_idx = iter.m_idx;
        m_max = iter.m_max;
    }

    //不等于
    bool operator!=(const ValObjIterator &iter) {
        return m_cur.first != iter.m_cur.first;
    }

    //等于
    bool operator==(const ValObjIterator &iter) {
        return m_cur.first == iter.m_cur.first;
    }

    //前缀自加
    ValObjIterator &operator++() {
        if (m_cur.first && m_idx < m_max) {
            m_idx++;
            m_pre = m_cur;
            yyjson_val *cur = nullptr;
            if (m_idx < m_max) {
                yyjson_val *cur = unsafe_yyjson_get_next(m_cur.second.ptr());
                m_cur.first = val_view(cur);
                m_cur.second = val_view(cur + 1);
            } else {
                m_cur.first = val_view();
                m_cur.second = val_view();
            }
        }
        return *this;
    }

    //后缀自加
    ValObjIterator operator++(int) {
        ValObjIterator tmp = *this;
        if (m_cur.first && m_idx < m_max) {
            m_idx++;
            m_pre = m_cur;
            yyjson_val *cur = nullptr;
            if (m_idx < m_max) {
                yyjson_val *cur = unsafe_yyjson_get_next(m_cur.second.ptr());
                m_cur.first = val_view(cur);
                m_cur.second = val_view(cur + 1);
            } else {
                m_cur.first = val_view();
                m_cur.second = val_view();
            }
        }
        return tmp;
    }

    //取值
    std::pair<T, T> operator*() {
        return m_cur;
    }

    std::pair<T, T> *operator->() {
        return &m_cur;
    }

private:
    std::pair<T, T> m_cur;
    std::pair<T, T> m_pre;
    size_t m_idx{0};
    size_t m_max{0};
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

    void ptr(yyjson_val *val) {
        m_val = val;
    }

    operator bool() {
        return m_val != nullptr;
    }

    bool operator==(const val_view &other) {
        return m_val == other.m_val;
    }

    bool operator!=(const val_view &other) {
        return m_val != other.m_val;
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

    float value(float fallback) const {
        return yyjson_is_num(m_val) ? (float)unsafe_yyjson_get_real(m_val) : fallback;
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
    float value() const {
        YY_TYPE_CHECK(num);
        return static_cast<float>(unsafe_yyjson_get_real(m_val));
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

#if YYJSON_VERSION_HEX > 0x000200
    val_view get_pointer(const char *pointer) const {
        return val_view(yyjson_get_pointer(m_val, pointer));
    }

    val_view get_pointer(const std::string& pointer) const {
        return val_view(yyjson_get_pointer(m_val, pointer.c_str()));
    }
#endif

    ValArrIterator<val_view> arr_begin() const {
        return ValArrIterator<val_view>(*this);
    }

    ValArrIterator<val_view> arr_end() const {
        return ValArrIterator<val_view>(val_view());
    }

    ValObjIterator<val_view> begin() const {
        return ValObjIterator<val_view>(*this);
    }

    ValObjIterator<val_view> end() const {
        return ValObjIterator<val_view>(val_view());
    }

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

inline std::ostream &operator<<(std::ostream &os, val_view val) {
    if (val.is_bool()) {
        if (val.value<bool>()) {
            os << "true";
        } else {
            os << "false";
        }
    } else if (val.is_uint()) {
        os << val.value<uint64_t>();
    } else if (val.is_sint()) {
        os << val.value<int64_t>();
    } else if (val.is_real()) {
        os << val.value<double>();
    } else if (val.is_str()) {
        os << "\"" << val.value<std::string>() << "\"";
    } else if (val.is_null()) {
        os << "null";
    } else if (val.is_arr()) {
        os << "[";
        for (auto iter = val.arr_begin(); iter != val.arr_end(); ++iter) {
            os << *iter << ", ";
        }
        os << "]";
    } else if (val.is_obj()) {
        os << "{";
        for (auto iter = val.begin(); iter != val.end(); ++iter) {
            os << iter->first << ": " << iter->second << ", ";
        }
        os << "}";
    } else {
        os << val.type_desc();
    }
    return os;
}

}  // namespace yyjson