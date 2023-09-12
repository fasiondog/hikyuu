/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-10
 *      Author: fasiondog
 */

#pragma once

#include "../trade_sys/system/System.h"

namespace hku {

/**
 * 获取指定序列组合，返回由序列下标索引组成的组合集合
 * @param inputs 任意类型的序列
 * @return
 */
template <class T>
vector<vector<size_t>> generateCombinationsIndex(const vector<T>& inputs) {
    vector<vector<size_t>> result;

    vector<size_t> current;
    for (size_t i = 0, total = inputs.size(); i < total; i++) {
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

void HKU_API signalCombinationAnaysis(const vector<SGPtr>& sgs, const SYSPtr& sys, const TMPtr& tm);

}