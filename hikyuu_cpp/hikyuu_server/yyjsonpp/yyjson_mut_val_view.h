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

#define YY_MUT_VAL_IS(typ)                 \
    bool is_##typ() const {                \
        return yyjson_mut_is_##typ(m_val); \
    }

class mut_val_view {
public:
    mut_val_view(yyjson_mut_val *val) : m_val(val) {}

    yyjson_mut_val *ptr() {
        return m_val;
    }

    operator bool() const {
        return m_val != nullptr;
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
    yyjson_type get_type() const {
        return yyjson_mut_get_type(m_val);
    }

    /** Returns value's subtype. */
    yyjson_subtype get_subtype() const {
        return yyjson_mut_get_subtype(m_val);
    }

    /** Returns value's tag. */
    uint8_t get_tag() const {
        return yyjson_mut_get_tag(m_val);
    }

    /**
     * Returns type description, such as: "null", "string", "array", "object", "true", "false",
     * "uint", "sint", "real", "unknown"
     */
    const char *get_type_desc() const {
        return yyjson_mut_get_type_desc(m_val);
    }

    bool get_bool(bool fallback) const {
        return yyjson_mut_is_bool(m_val) ? unsafe_yyjson_get_bool((yyjson_val *)m_val) : fallback;
    }

    bool get_bool() const {
        if (!yyjson_mut_is_bool(m_val)) {
            YYJSON_THROW("This value type is {}, not bool!", get_type_desc());
        }
        return unsafe_yyjson_get_bool((yyjson_val *)m_val);
    }

    uint64_t get_uint(uint64_t fallback) const {
        return yyjson_mut_is_int(m_val) ? unsafe_yyjson_get_uint((yyjson_val *)m_val) : fallback;
    }

    uint64_t get_uint() const {
        if (!yyjson_mut_is_int(m_val)) {
            YYJSON_THROW("This vale type is {}, not uint!", get_type_desc());
        }
        return unsafe_yyjson_get_uint((yyjson_val *)m_val);
    }

    int64_t get_int(int64_t fallback) const {
        return yyjson_mut_is_int(m_val) ? unsafe_yyjson_get_sint((yyjson_val *)m_val) : fallback;
    }

    int64_t get_int() const {
        if (!yyjson_mut_is_int(m_val)) {
            YYJSON_THROW("This value type is {}, not int!", get_type_desc());
        }
        return unsafe_yyjson_get_sint((yyjson_val *)m_val);
    }

    double get_double(double fallback) const {
        return yyjson_mut_is_num(m_val) ? unsafe_yyjson_get_real((yyjson_val *)m_val) : fallback;
    }

    double get_double() const {
        if (!yyjson_mut_is_num(m_val)) {
            YYJSON_THROW("This value type is {}, not double!", get_type_desc());
        }
        return unsafe_yyjson_get_real((yyjson_val *)m_val);
    }

    std::string get_str() const {
        if (!yyjson_mut_is_str(m_val)) {
            YYJSON_THROW("This value type is {}, not string!", get_type_desc());
        }
        return std::string(unsafe_yyjson_get_str((yyjson_val *)m_val));
    }

    mut_val_view get(const char *key) const {
        return mut_val_view(yyjson_mut_obj_get(m_val, key));
    }

    mut_val_view operator[](const char *key) {
        return get(key);
    }

    mut_val_view get_no_mapping() const {
        return mut_val_view(yyjson_mut_obj_get(m_val, NULL));
    }

    /** Returns the number of key-value pairs in this object, or 0 if input is not an object. */
    size_t size() const {
        return yyjson_mut_obj_size(m_val);
    }

    bool equals_str(const char *str) const {
        return yyjson_mut_equals_str(m_val, str);
    }

    bool equals_str(const std::string &str) const {
        return equals_str(str.c_str());
    }

    bool equals_strn(const char *str, size_t len) const {
        return yyjson_mut_equals_strn(m_val, str, len);
    }

    typedef ArrIterator<mut_val_view> iterator;
    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, arr_size());
    }

    void for_each(std::function<void(mut_val_view)> func) const {
        size_t idx, max;
        yyjson_mut_val *val;
        yyjson_mut_arr_foreach(m_val, idx, max, val) {
            func(mut_val_view(val));
        }
    }

    size_t arr_size() const {
        return yyjson_mut_arr_size(m_val);
    }

    mut_val_view arr_get(size_t idx) const {
        return mut_val_view(yyjson_mut_arr_get(m_val, idx));
    }

    mut_val_view arr_get_first() const {
        return mut_val_view(yyjson_mut_arr_get_first(m_val));
    }

    mut_val_view arr_get_last() const {
        return mut_val_view(yyjson_mut_arr_get_last(m_val));
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
        return mut_val_view(yyjson_mut_arr_replace(m_val, idx, val.ptr()));
    }

    /**
     * Removes and returns a value at index, returns NULL on error.
     * @note Note that this function takes a linear search time.
     */
    mut_val_view arr_remove(size_t idx) {
        return mut_val_view(yyjson_mut_arr_remove(m_val, idx));
    }

    /** Returns and returns the first value in this array, returns NULL on error. */
    mut_val_view arr_remove_first() {
        return mut_val_view(yyjson_mut_arr_remove_first(m_val));
    }

    /** Returns and returns the last value in this array, returns NULL on error. */
    mut_val_view arr_remove_last() {
        return mut_val_view(yyjson_mut_arr_remove_last(m_val));
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

    bool arr_add_val(mut_val_view val) {
        return yyjson_mut_arr_add_val(m_val, val.ptr());
    }

private:
    yyjson_mut_val *m_val;
};

}  // namespace yyjson