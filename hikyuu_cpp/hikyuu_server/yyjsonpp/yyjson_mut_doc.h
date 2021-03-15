/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-15
 *     Author: fasiondog
 */

#pragma once

#include <memory>
#include "yyjson_doc.h"
#include "yyjson_val_view.h"
#include "yyjson_mut_val_view.h"

namespace yyjson {

class mut_doc {
public:
    mut_doc(yyjson_alc* alc = nullptr) : m_doc(yyjson_mut_doc_new(alc)) {}

    mut_doc(yyjson_mut_doc* doc) : m_doc(doc) {}

    mut_doc(const mut_doc&) = delete;

    mut_doc(mut_doc&& rhs) : m_doc(rhs.m_doc) {
        rhs.m_doc = nullptr;
    }

    mut_doc& operator=(const mut_doc&) = delete;

    mut_doc& operator=(mut_doc&& rhs) {
        if (this != &rhs) {
            m_doc = rhs.m_doc;
            rhs.m_doc = nullptr;
        }
        return *this;
    }

    explicit mut_doc(const doc& doc) {
        if (doc) {
            m_doc = yyjson_doc_mut_copy(doc.ptr(), nullptr);
        }
    }

    mut_doc(const doc& doc, yyjson_alc* alc) {
        if (doc) {
            m_doc = yyjson_doc_mut_copy(doc.ptr(), alc);
        }
    }

    virtual ~mut_doc() {
        if (m_doc) {
            yyjson_mut_doc_free(m_doc);
        }
    }

    operator bool() const {
        return m_doc != nullptr;
    }

    std::string json(yyjson_write_flag flg = YYJSON_WRITE_NOFLAG) const {
        char* cstr = yyjson_mut_write(m_doc, flg, nullptr);
        YYJSON_CHECK(cstr, "Failed yyjson_write");
        std::string result(cstr);
        free(cstr);
        return result;
    }

    mut_val_view root() const {
        return mut_val_view(yyjson_mut_doc_get_root(m_doc));
    }

    void root(mut_val_view val) {
        yyjson_mut_doc_set_root(m_doc, val.ptr());
    }

    mut_val_view copy_val(val_view val) {
        return mut_val_view(yyjson_val_mut_copy(m_doc, val.ptr()));
    }

    mut_val_view mut_null() {
        return mut_val_view(yyjson_mut_null(m_doc));
    }

    mut_val_view mut_true() {
        return mut_val_view(yyjson_mut_true(m_doc));
    }

    mut_val_view mut_false() {
        return mut_val_view(yyjson_mut_false(m_doc));
    }

    mut_val_view mut_bool(bool val) {
        return mut_val_view(yyjson_mut_bool(m_doc, val));
    }

    mut_val_view mut_uint(uint64_t val) {
        return mut_val_view(yyjson_mut_uint(m_doc, val));
    }

    mut_val_view mut_sint(int64_t val) {
        return mut_val_view(yyjson_mut_sint(m_doc, val));
    }

    mut_val_view mut_int(int64_t val) {
        return mut_val_view(yyjson_mut_int(m_doc, val));
    }

    mut_val_view mut_real(double val) {
        return mut_val_view(yyjson_mut_real(m_doc, val));
    }

    mut_val_view mut_str(const char* val) {
        return mut_val_view(yyjson_mut_strcpy(m_doc, val));
    }

    mut_val_view mut_str(const std::string& val) {
        return mut_val_view(yyjson_mut_strcpy(m_doc, val.c_str()));
    }

    mut_val_view mut_strn(const char* val, size_t len) {
        return mut_val_view(yyjson_mut_strncpy(m_doc, val, len));
    }

    mut_val_view mut_arr() {
        return mut_val_view(yyjson_mut_arr(m_doc));
    }

    template <typename T>
    mut_val_view mut_arr(T*, size_t len) {
        YYJSON_THROW("Unsupported type!");
        return mut_val_view(nullptr);
    }

    template <>
    mut_val_view mut_arr(bool* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_bool(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(float* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_float(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(double* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_real(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(int8_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint8(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(int16_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint16(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(int32_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint32(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(int64_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(uint8_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint8(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(uint16_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint16(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(uint32_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint32(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(uint64_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint(m_doc, vals, len));
    }

    template <>
    mut_val_view mut_arr(const char** vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_strcpy(m_doc, vals, len));
    }

    template <typename T>
    mut_val_view mut_arr(const T& vals) {
        std::unique_ptr<T::value_type[]> ptr(new T::value_type[vals.size()]);
        std::copy(vals.begin(), vals.end(), ptr);
        return mut_arr(ptr.get(), vals.size());
    }

    template <typename T>
    mut_val_view mut_arr(const std::vector<T>& vals) {
        return mut_arr(vals.data(), vals.size());
    }

    template <>
    mut_val_view mut_arr(const std::vector<bool>& vals) {
        bool* ptr = new bool[vals.size()];
        std::copy(vals.begin(), vals.end(), ptr);
        mut_val_view result(mut_arr(ptr, vals.size()));
        delete[] ptr;
        return result;
    }

private:
    yyjson_mut_doc* m_doc;
};

}  // namespace yyjson