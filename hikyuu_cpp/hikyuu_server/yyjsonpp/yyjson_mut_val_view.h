/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-15
 *     Author: fasiondog
 */

#pragma once

#include <type_traits>
#include <string>
#include <vector>
#include <iterator>
#include <yyjson.h>
#include "yyjson_val_view.h"
#include "exception.h"

namespace yyjson {

template <class T>
class MutValArrIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    MutValArrIterator(T val) {
        if (val && val.is_arr()) {
            m_idx = 0;
            m_max = unsafe_yyjson_get_len(val.ptr());
            if (m_max) {
                m_cur.ptr((yyjson_mut_val *)val.ptr()->uni.ptr);
                m_cur.ptr(m_cur.ptr()->next);
            }
        }
    }

    //赋值
    MutValArrIterator &operator=(const MutValArrIterator &iter) {
        m_idx = iter.m_idx;
        m_max = iter.m_max;
        m_cur = iter.m_cur;
    }

    //不等于
    bool operator!=(const MutValArrIterator &iter) {
        return m_cur != iter.m_cur;
    }

    //等于
    bool operator==(const MutValArrIterator &iter) {
        return m_cur == iter.m_cur;
    }

    //前缀自加
    MutValArrIterator &operator++() {
        if (m_idx < m_max) {
            m_idx++;
            if (m_idx < m_max) {
                m_cur.ptr(m_cur.ptr()->next);
            } else {
                m_cur.ptr(nullptr);
            }
        }
        return *this;
    }

    //后缀自加
    MutValArrIterator operator++(int) {
        MutValArrIterator tmp = *this;
        if (m_idx < m_max) {
            m_idx++;
            if (m_idx < m_max) {
                m_cur.ptr(m_cur.ptr()->next);
            } else {
                m_cur.ptr(nullptr);
            }
        }
        return tmp;
    }

    //取值
    T operator*() {
        return m_cur;
    }

    T *operator->() {
        return &m_cur;
    }

private:
    size_t m_idx{0};
    size_t m_max{0};
    T m_cur;
};

#define YY_MUT_VAL_IS(typ)                 \
    bool is_##typ() const {                \
        return yyjson_mut_is_##typ(m_val); \
    }

class mut_val_view {
public:
    mut_val_view() = default;
    mut_val_view(yyjson_mut_val *val, yyjson_mut_doc *doc) : m_val(val), m_doc(doc) {}
    mut_val_view(const mut_val_view &) = default;
    mut_val_view &operator=(const mut_val_view &) = default;

    virtual ~mut_val_view() = default;

    yyjson_mut_val *ptr() const {
        return m_val;
    }

    void ptr(yyjson_mut_val *val) {
        m_val = val;
    }

    operator bool() const {
        return m_val != nullptr;
    }

    bool operator==(const mut_val_view &other) {
        return m_val == other.m_val;
    }

    bool operator!=(const mut_val_view &other) {
        return m_val != other.m_val;
    }

    YY_MUT_VAL_IS(null)
    YY_MUT_VAL_IS(true)
    YY_MUT_VAL_IS(false)
    YY_MUT_VAL_IS(bool)
    YY_MUT_VAL_IS(uint)
    YY_MUT_VAL_IS(sint)
    YY_MUT_VAL_IS(int)
    YY_MUT_VAL_IS(real)  // double number
    YY_MUT_VAL_IS(num)   // integer or double number
    YY_MUT_VAL_IS(str)   // c字符串
    YY_MUT_VAL_IS(arr)   // array
    YY_MUT_VAL_IS(obj)   // object
    YY_MUT_VAL_IS(ctn)   // array or object

    /** Returns value's type. */
    yyjson_type type() const {
        return yyjson_mut_get_type(m_val);
    }

    /** Returns value's subtype. */
    yyjson_subtype subtype() const {
        return yyjson_mut_get_subtype(m_val);
    }

    /** Returns value's tag. */
    uint8_t tag() const {
        return yyjson_mut_get_tag(m_val);
    }

    /**
     * Returns type description, such as: "null", "string", "array", "object", "true", "false",
     * "uint", "sint", "real", "unknown"
     */
    const char *type_desc() const {
        return yyjson_mut_get_type_desc(m_val);
    }

    bool value(bool fallback) const {
        return yyjson_mut_is_bool(m_val) ? unsafe_yyjson_get_bool((yyjson_val *)m_val) : fallback;
    }

    uint64_t value(uint64_t fallback) const {
        return yyjson_mut_is_int(m_val) ? unsafe_yyjson_get_uint((yyjson_val *)m_val) : fallback;
    }

    int64_t value(int64_t fallback) const {
        return yyjson_mut_is_int(m_val) ? unsafe_yyjson_get_sint((yyjson_val *)m_val) : fallback;
    }

    int value(int fallback) const {
        int result = fallback;
        if (yyjson_mut_is_int(m_val)) {
            try {
                result = static_cast<int>(unsafe_yyjson_get_sint((yyjson_val *)m_val));
            } catch (...) {
            }
        }
        return result;
    }

    double value(double fallback) const {
        return yyjson_mut_is_num(m_val) ? unsafe_yyjson_get_real((yyjson_val *)m_val) : fallback;
    }

    float value(float fallback) const {
        return yyjson_mut_is_num(m_val) ? (float)unsafe_yyjson_get_real((yyjson_val *)m_val)
                                        : fallback;
    }

    std::string value(const char *fallback) const {
        return yyjson_mut_is_str(m_val) ? std::string(unsafe_yyjson_get_str((yyjson_val *)m_val))
                                        : std::string(fallback);
    }

    std::string value(const std::string &fallback) const {
        return yyjson_mut_is_str(m_val) ? std::string(unsafe_yyjson_get_str((yyjson_val *)m_val))
                                        : fallback;
    }

    template <typename T>
    T value() const {
        YYJSON_THROW("Unsupport type!");
    }

    template <>
    bool value() const {
        YY_TYPE_CHECK(bool);
        return unsafe_yyjson_get_bool((yyjson_val *)m_val);
    }

    template <>
    uint64_t value() const {
        YY_TYPE_CHECK(int);
        return unsafe_yyjson_get_uint((yyjson_val *)m_val);
    }

    template <>
    int64_t value() const {
        YY_TYPE_CHECK(int);
        return unsafe_yyjson_get_sint((yyjson_val *)m_val);
    }

    template <>
    int value() const {
        YY_TYPE_CHECK(int);
        return static_cast<int>(unsafe_yyjson_get_sint((yyjson_val *)m_val));
    }

    template <>
    double value() const {
        YY_TYPE_CHECK(num);
        return unsafe_yyjson_get_real((yyjson_val *)m_val);
    }

    template <>
    float value() const {
        YY_TYPE_CHECK(num);
        return static_cast<float>(unsafe_yyjson_get_real((yyjson_val *)m_val));
    }

    template <>
    std::string value() const {
        YY_TYPE_CHECK(str);
        return std::string(unsafe_yyjson_get_str((yyjson_val *)m_val));
    }

    mut_val_view get(const char *key) const {
        return mut_val_view(yyjson_mut_obj_get(m_val, key), m_doc);
    }

    mut_val_view operator[](const char *key) {
        return get(key);
    }

    mut_val_view get_no_mapping() const {
        return mut_val_view(yyjson_mut_obj_get(m_val, NULL), m_doc);
    }

#if YYJSON_VERSION_HEX > 0x000200
    mut_val_view get_pointer(const char *pointer) const {
        return mut_val_view(yyjson_mut_get_pointer(m_val, pointer));
    }

    mut_val_view get_pointer(const std::string &pointer) const {
        return mut_val_view(yyjson_mut_get_pointer(m_val, pointer.c_str()));
    }
#endif

    size_t arr_size() const {
        return yyjson_mut_arr_size(m_val);
    }

    /** Returns the number of key-value pairs in this object, or 0 if input is not an object. */
    size_t obj_size() const {
        return yyjson_mut_obj_size(m_val);
    }

    /** Returns the number of key-value pairs in the object , or the number of arr, else 0. */
    size_t size() const {
        if (is_arr()) {
            return yyjson_mut_arr_size(m_val);
        } else if (is_obj()) {
            return yyjson_mut_obj_size(m_val);
        } else {
            return 0;
        }
    }

    MutValArrIterator<mut_val_view> arr_begin() {
        return MutValArrIterator<mut_val_view>(*this);
    }

    MutValArrIterator<mut_val_view> arr_end() {
        return MutValArrIterator<mut_val_view>(mut_val_view());
    }

    /*typedef ArrIterator<mut_val_view> iterator;
    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, arr_size());
    }*/

    void for_each(std::function<void(mut_val_view)> func) const {
        size_t idx, max;
        yyjson_mut_val *val;
        yyjson_mut_arr_foreach(m_val, idx, max, val) {
            func(mut_val_view(val, m_doc));
        }
    }

    void for_each(std::function<void(mut_val_view, mut_val_view)> func) {
        size_t idx = 0, max = 0;
        yyjson_mut_val *key = nullptr;
        yyjson_mut_val *val = nullptr;
        for (idx = 0, max = yyjson_mut_obj_size(m_val),
            key = max ? ((yyjson_mut_val *)m_val->uni.ptr)->next->next : NULL,
            val = key ? key->next : NULL;
             idx < max; idx++, key = val->next, val = key->next) {
            func(mut_val_view(key, m_doc), mut_val_view(val, m_doc));
        }
    }

    mut_val_view arr_get(size_t idx) const {
        return mut_val_view(yyjson_mut_arr_get(m_val, idx), m_doc);
    }

    mut_val_view arr_get_first() const {
        return mut_val_view(yyjson_mut_arr_get_first(m_val), m_doc);
    }

    mut_val_view arr_get_last() const {
        return mut_val_view(yyjson_mut_arr_get_last(m_val), m_doc);
    }

    /**
     * Inserts a value into an array at a given index, returns false on error.Note that Tthis
     * function takes a linear search time.
     */
    bool arr_insert(size_t idx, mut_val_view val) {
        return yyjson_mut_arr_insert(m_val, val.ptr(), idx);
    }

    /** Inserts a val at the end of the array, returns false on error. */
    bool arr_append(mut_val_view val) {
        return yyjson_mut_arr_append(m_val, val.ptr());
    }

    /** Inserts a val at the head of the array, returns false on error. */
    bool arr_prepend(mut_val_view val) {
        return yyjson_mut_arr_prepend(m_val, val.ptr());
    }

    /**
     * Replaces a value at index and returns old value, returns NULL on error.
     * @note Note that his function takes a linear search time.
     */
    mut_val_view arr_replace(size_t idx, mut_val_view val) {
        return mut_val_view(yyjson_mut_arr_replace(m_val, idx, val.ptr()), m_doc);
    }

    /**
     * Removes and returns a value at index, returns NULL on error.
     * @note Note that this function takes a linear search time.
     */
    mut_val_view arr_remove(size_t idx) {
        return mut_val_view(yyjson_mut_arr_remove(m_val, idx), m_doc);
    }

    /** Returns and returns the first value in this array, returns NULL on error. */
    mut_val_view arr_remove_first() {
        return mut_val_view(yyjson_mut_arr_remove_first(m_val), m_doc);
    }

    /** Returns and returns the last value in this array, returns NULL on error. */
    mut_val_view arr_remove_last() {
        return mut_val_view(yyjson_mut_arr_remove_last(m_val), m_doc);
    }

    /**
     * Removes all values within a specified range in the array.
     * @note Note that this function takes a linear search time.
     */
    bool arr_remove_range(size_t idx, size_t len) {
        return yyjson_mut_arr_remove_range(m_val, idx, len);
    }

    /** Removes all values in this array. */
    bool arr_clear() {
        return yyjson_mut_arr_clear(m_val);
    }

    bool arr_add(mut_val_view val) {
        return yyjson_mut_arr_add_val(m_val, val.ptr());
    }

    bool arr_add(nullptr_t val) {
        return yyjson_mut_arr_add_null(m_doc, m_val);
    }

    bool arr_add(bool val) {
        return yyjson_mut_arr_add_bool(m_doc, m_val, val);
    }

    bool arr_add(uint64_t val) {
        return yyjson_mut_arr_add_uint(m_doc, m_val, val);
    }

    bool arr_add(int64_t val) {
        return yyjson_mut_arr_add_sint(m_doc, m_val, val);
    }

    bool arr_add(int val) {
        return yyjson_mut_arr_add_int(m_doc, m_val, val);
    }

    bool arr_add(double val) {
        return yyjson_mut_arr_add_real(m_doc, m_val, val);
    }

    bool arr_add(float val) {
        return yyjson_mut_arr_add_real(m_doc, m_val, val);
    }

    bool arr_add(const char *val) {
        return yyjson_mut_arr_add_strcpy(m_doc, m_val, val);
    }

    bool arr_add(const std::string &val) {
        return yyjson_mut_arr_add_strcpy(m_doc, m_val, val.c_str());
    }

    /**
     * Removes key-value pair from the object with given key.
     * @note Note that this function takes a linear search time.
     */
    bool obj_remove(mut_val_view key) {
        return yyjson_mut_obj_remove(m_val, key.ptr());
    }

    /**
     * Removes all key-value pairs for the given key.
     * Note that this function takes a linear search time.
     */
    bool obj_remove(const char *key) {
        return yyjson_mut_obj_remove_str(m_val, key);
    }

    /** Removes all key-value pairs in this object. */
    bool obj_clear() {
        return yyjson_mut_obj_clear(m_val);
    }

    /**
     * Adds a key-value pair at the end of the object. The key must be a string.
     * This function allows duplicated key in one object.
     */
    bool obj_add(mut_val_view key, mut_val_view val) {
        return yyjson_mut_obj_add(m_val, key.ptr(), val.ptr());
    }

    /**
     * Adds a key-value pair to the object, The key must be a string.
     * This function may remove all key-value pairs for the given key before add.
     * @note Note that this function takes a linear search time.
     */
    bool obj_put(mut_val_view key, mut_val_view val) {
        return yyjson_mut_obj_put(m_val, key.ptr(), val.ptr());
    }

    bool obj_add(const char *key, nullptr_t val) {
        return yyjson_mut_obj_add_null(m_doc, m_val, key);
    }

    bool obj_add(const char *key, bool val) {
        return yyjson_mut_obj_add_bool(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, uint64_t val) {
        return yyjson_mut_obj_add_uint(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, int64_t val) {
        return yyjson_mut_obj_add_sint(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, int val) {
        return yyjson_mut_obj_add_int(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, double val) {
        return yyjson_mut_obj_add_real(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, float val) {
        return yyjson_mut_obj_add_real(m_doc, m_val, key, val);
    }

    bool obj_add(const char *key, const char *val) {
        return yyjson_mut_obj_add_strcpy(m_doc, m_val, key, val);
    }

private:
    yyjson_mut_val *m_val{nullptr};
    yyjson_mut_doc *m_doc{nullptr};
};

inline std::ostream &operator<<(std::ostream &os, mut_val_view val) {
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
        /*} else if (val.is_obj()) {
            os << "{";
            for (auto iter = val.begin(); iter != val.end(); ++iter) {
                os << iter->first << ": " << iter->second << ", ";
            }
            os << "}";*/
    } else {
        os << val.type_desc();
    }
    return os;
}

}  // namespace yyjson