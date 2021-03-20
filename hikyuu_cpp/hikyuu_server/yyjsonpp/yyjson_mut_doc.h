/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-15
 *     Author: fasiondog
 */

#pragma once

#include <memory>
#include <map>
#include <unordered_map>
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
        return mut_val_view(yyjson_mut_doc_get_root(m_doc), m_doc);
    }

    void root(mut_val_view val) {
        yyjson_mut_doc_set_root(m_doc, val.ptr());
    }

#if YYJSON_VERSION_HEX > 0x000200
    mut_val_view get_pointer(const char* pointer) const {
        return mut_val_view(yyjson_mut_doc_get_pointer(m_doc, pointer));
    }

    mut_val_view get_pointer(const std::string& pointer) const {
        return mut_val_view(yyjson_mut_doc_get_pointer(m_doc, pointer.c_str()));
    }
#endif

    mut_val_view copy_val(val_view val) {
        return mut_val_view(yyjson_val_mut_copy(m_doc, val.ptr()), m_doc);
    }

    mut_val_view mut_val(nullptr_t val) {
        return mut_val_view(yyjson_mut_null(m_doc), m_doc);
    }

    mut_val_view mut_val(bool val) {
        return mut_val_view(yyjson_mut_bool(m_doc, val), m_doc);
    }

    mut_val_view mut_val(uint64_t val) {
        return mut_val_view(yyjson_mut_uint(m_doc, val), m_doc);
    }

    mut_val_view mut_val(int64_t val) {
        return mut_val_view(yyjson_mut_sint(m_doc, val), m_doc);
    }

    mut_val_view mut_val(double val) {
        return mut_val_view(yyjson_mut_real(m_doc, val), m_doc);
    }

    mut_val_view mut_val(const char* val) {
        return mut_val_view(yyjson_mut_strcpy(m_doc, val), m_doc);
    }

    mut_val_view mut_val(const std::string& val) {
        return mut_val_view(yyjson_mut_strcpy(m_doc, val.c_str()), m_doc);
    }

    mut_val_view mut_val(const char* val, size_t len) {
        return mut_val_view(yyjson_mut_strncpy(m_doc, val, len), m_doc);
    }

    mut_val_view mut_arr() {
        return mut_val_view(yyjson_mut_arr(m_doc), m_doc);
    }

    mut_val_view mut_arr(bool* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_bool(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(float* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_float(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(double* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_real(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(int8_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint8(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(int16_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint16(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(int32_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint32(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(int64_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_sint(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(uint8_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint8(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(uint16_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint16(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(uint32_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint32(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(uint64_t* vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_uint(m_doc, vals, len), m_doc);
    }

    mut_val_view mut_arr(const char** vals, size_t len) {
        return mut_val_view(yyjson_mut_arr_with_strcpy(m_doc, vals, len), m_doc);
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

    /** Creates and returns a mutable object, returns NULL on error. */
    mut_val_view mut_obj() {
        return mut_val_view(yyjson_mut_obj(m_doc), m_doc);
    }

    /**
     * Creates and returns a mutable object with keys and values, returns NULL on error. The keys
     * and values are NOT copied. The strings should be encoded as UTF-8 with null-terminator.
     * <pre>
     * sample:
     * const char vkeys[] = {"name", "type", "id"};
     * const char *vals[] = {"Harry", "student", "888999"};
     * yyjson_mut_obj_with_str(doc, keys, vals, 3);
     * </pre>
     */
    mut_val_view mut_obj(const char** keys, const char** vals, size_t count) {
        return mut_val_view(yyjson_mut_obj_with_str(m_doc, keys, vals, count), m_doc);
    }

    template <typename KeyT, typename ValT>
    mut_val_view mut_obj(const std::map<KeyT, ValT>& kv) {
        mut_val_view result = mut_obj();
        for (auto iter = kv.begin(); iter != kv.end(); ++iter) {
            mut_val_view key = mut_val(iter->first);
            mut_val_view val = mut_val(iter->second);
            yyjson_mut_obj_add(result.ptr(), key.ptr(), val.ptr());
        }
        return result;
    }

    template <typename KeyT, typename ValT>
    mut_val_view mut_obj(const std::unordered_map<KeyT, ValT>& kv) {
        mut_val_view result = mut_obj();
        for (auto iter = kv.begin(); iter != kv.end(); ++iter) {
            mut_val_view key = mut_val(iter->first);
            mut_val_view val = mut_val(iter->second);
            yyjson_mut_obj_add(result.ptr(), key.ptr(), val.ptr());
        }
        return result;
    }

private:
    yyjson_mut_doc* m_doc{nullptr};
};

}  // namespace yyjson