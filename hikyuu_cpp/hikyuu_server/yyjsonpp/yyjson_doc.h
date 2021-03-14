/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#pragma once

#include "yyjson_val_view.h"

namespace yyjson {

class doc {
public:
    doc() = default;
    doc(const char *dat, size_t len, yyjson_read_flag flg = 0) {
        m_doc = yyjson_read(dat, len, flg);
        YYJSON_CHECK(m_doc, "Failed yyjson_read!");
    }

    doc(const std::string &dat, yyjson_read_flag flg = 0) {
        m_doc = yyjson_read(dat.c_str(), dat.size(), flg);
        YYJSON_CHECK(m_doc, "Failed yyjson_read!");
    }

    doc(const doc &) = delete;

    doc(doc &&rhs) noexcept : m_doc(rhs.m_doc) {
        rhs.m_doc = nullptr;
    }

    doc &operator=(const doc &rhs) = delete;

    doc &operator=(doc &&rhs) {
        if (this != &rhs) {
            if (m_doc) {
                yyjson_doc_free(m_doc);
            }
            m_doc = rhs.m_doc;
            rhs.m_doc = nullptr;
        }
        return *this;
    }

    ~doc() {
        if (m_doc) {
            yyjson_doc_free(m_doc);
        }
    }

    std::string json(yyjson_write_flag flg = YYJSON_WRITE_NOFLAG);

    val_view get_root() const {
        return val_view(yyjson_doc_get_root(m_doc));
    }

    val_view get_obj(const char *key) const {
        return get_root().get_obj(key);
    }

    val_view operator[](const char *key) const {
        return get_obj(key);
    }

    /* 0.2.0 版本尚不支持
    val_view get_obj_by_path(const char *path) {
        return val_view(yyjson_doc_get_pointer(m_doc, path));
    }*/

    size_t get_read_size() const {
        return yyjson_doc_get_read_size(m_doc);
    }

    size_t get_val_count() const {
        return yyjson_doc_get_val_count(m_doc);
    }

public:
    static doc read_file(const char *path, yyjson_read_flag flg) {
        doc d;
        yyjson_read_err err;
        d.m_doc = yyjson_read_file(path, flg, nullptr, &err);
        if (!d.m_doc) {
            YYJSON_THROW(fmt::format("{}", err.msg));
        }
        return d;
    }

private:
    yyjson_doc *m_doc{nullptr};
};

}  // namespace yyjson
