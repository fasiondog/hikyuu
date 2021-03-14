/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-11
 *     Author: fasiondog
 */

#include "yyjson_doc.h"

namespace yyjson {

std::string doc::json(yyjson_write_flag flg) {
    char *cstr = yyjson_write(m_doc, flg, nullptr);
    YYJSON_CHECK(cstr, "Failed yyjson_write");
    std::string result(cstr);
    free(cstr);
    return result;
}

}  // namespace yyjson