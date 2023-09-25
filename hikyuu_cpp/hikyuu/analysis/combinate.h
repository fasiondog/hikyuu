/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#pragma once

#include <vector>
#include "../Log.h"

namespace hku {

/**
 * 获取指定序列索引的组合，返回由序列下标索引组成的组合集合
 * @note 组合的总数是 2**n - 1 个，所以inputs的长度需要控制
 * @exception 输入序列的最大长度仅支持15，超过时将抛出异常 HKUException
 * @param inputs 任意类型的序列
 * @return
 */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    size_t total = inputs.size();
    HKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");

    std::vector<std::vector<size_t>> result;
    std::vector<size_t> current;
    for (size_t i = 0; i < total; i++) {
        for (size_t j = 0, len = result.size(); j < len; j++) {
            current.resize(result[j].size());
            std::copy(result[j].cbegin(), result[j].cend(), current.begin());
            current.push_back(i);
            result.push_back(std::move(current));
        }
        current.clear();
        current.push_back(i);
        result.push_back(std::move(current));
    }

    return result;
}

}