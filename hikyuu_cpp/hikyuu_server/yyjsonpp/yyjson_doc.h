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
    doc() {}
    doc(const char *dat, size_t len, yyjson_read_flag flg = 0) {
        m_doc = yyjson_read(dat, len, flg);
    }

    doc(const std::string &dat, yyjson_read_flag flg = 0) {
        m_doc = yyjson_read(dat.c_str(), dat.size(), flg);
    }

    ~doc() {
        if (m_doc) {
            yyjson_doc_free(m_doc);
        }
    }

    operator bool() const {
        return m_doc != nullptr;
    }

    val_view get_root() const {
        return val_view(yyjson_doc_get_root(m_doc));
    }

    size_t get_read_size() const {
        return yyjson_doc_get_read_size(m_doc);
    }

    size_t get_val_count() const {
        return yyjson_doc_get_val_count(m_doc);
    }

private:
    yyjson_doc *m_doc{nullptr};
};

}  // namespace yyjson
