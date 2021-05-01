/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-01
 *     Author: fasiondog
 */

#include "mo.h"

namespace hku {

moFileLib::moFileReader g_moFR;

void mo_init(const char *filename) {
    g_moFR.ReadFile(filename);
}

}  // namespace hku