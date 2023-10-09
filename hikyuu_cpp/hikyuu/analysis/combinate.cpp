/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-10-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/EXIST.h"
#include "combinate.h"

namespace hku {

std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs) {
    std::vector<Indicator> ret(inputs.size());
    auto indexs = combinateIndex(inputs);
    for (size_t i = 0, len = indexs.size(); i < len; i++) {
        size_t count = indexs[i].size();
        Indicator tmp = EXIST(inputs[indexs[i][0]]);
        for (size_t j = 0; j < count; j++) {
            tmp = tmp & EXIST(inputs[indexs[i][j]]);
        }
        ret.emplace_back(tmp);
    }
    return ret;
}

}  // namespace hku