/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-02
 *     Author: fasiondog
 */

#include "mo.h"

namespace hku {

std::unordered_map<std::string, moFileLib::moFileReader> MOHelper::ms_dict;

void MOHelper::init() {
    ms_dict["zh_cn"] = moFileLib::moFileReader();
    ms_dict["zh_cn"].ReadFile("i8n/zh_CN.mo");
}

}  // namespace hku