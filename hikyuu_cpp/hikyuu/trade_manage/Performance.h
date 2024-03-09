/*
 * Performance.h
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#pragma once
#ifndef PERFORMANCE_H_
#define PERFORMANCE_H_

#include <boost/function.hpp>
#include "TradeManager.h"

namespace hku {

/**
 * 简单绩效统计
 * @ingroup Performance
 */
class HKU_API Performance {
public:
    Performance();
    virtual ~Performance();

    Performance(const Performance& other) : m_result(other.m_result) {}
    Performance(Performance&& other) : m_result(std::move(other.m_result)) {}

    Performance& operator=(const Performance& other);
    Performance& operator=(Performance&& other);

    /** 复位，清除已计算的结果 */
    void reset();

    /** 按指标名称获取指标值，必须在运行 statistics 或 report 之后生效  */
    double get(const string& name) const;

    /** 同 get */
    double operator[](const string& name) const {
        return get(name);
    }

    /**
     * 简单的文本统计报告，用于直接输出打印
     * @param tm
     * @param datetime 指定的截止时刻
     * @return
     */
    string report(const TradeManagerPtr& tm, const Datetime& datetime = Datetime::now());

    /**
     * 根据交易记录，统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime，
     * 以便用于计算当前市值
     * @param tm 指定的交易管理实例
     * @param datetime 统计截止时刻
     */
    void statistics(const TradeManagerPtr& tm, const Datetime& datetime = Datetime::now());

    /** 获取所有统计项名称，顺序与 values 相同 */
    const StringList& names() const {
        return ms_keys;
    }

    /** 获取所有统计项值，顺序与 names 相同*/
    PriceList values() const;

    typedef std::map<string, double> map_type;
    typedef map_type::iterator iterator;
    typedef map_type::const_iterator const_iterator;

private:
    map_type m_result;
    static StringList ms_keys;  // 保存统计项顺序, map/unordered_map都不能保持按插入顺序遍历
};

} /* namespace hku */
#endif /* PERFORMANCE_H_ */
