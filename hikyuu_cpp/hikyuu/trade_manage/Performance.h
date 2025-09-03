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
#include "TradeManagerBase.h"

namespace hku {

/**
 * 简单绩效统计
 * @ingroup Performance
 */
class HKU_API Performance {
public:
    Performance();
    virtual ~Performance();

    Performance(const Performance& other) = default;
    Performance(Performance&& other)
    : m_result(std::move(other.m_result)), m_keys(std::move(other.m_keys)) {}

    Performance& operator=(const Performance& other);
    Performance& operator=(Performance&& other);

    /** 是否为合法的统计项 */
    bool exist(const string& key);

    /** 复位，清除已计算的结果 */
    void reset();

    /** 按指标名称获取指标值，必须在运行 statistics 或 report 之后生效  */
    double get(const string& name) const;

    /** 同 get */
    double operator[](const string& name) const {
        return get(name);
    }

    /**
     * 简单的文本统计报告，用于直接输出打印。
     * @note 只有运行 statistics 后或 Performance 本身为从 TM 获取的结果时才生效
     * @return
     */
    string report();

    /**
     * 根据交易记录，统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime，
     * 以便用于计算当前市值
     * @param tm 指定的交易管理实例
     * @param datetime 统计截止时刻
     */
    void statistics(const TradeManagerPtr& tm, const Datetime& datetime = Datetime::now());

    /** 获取所有统计项名称，顺序与 values 相同 */
    const StringList& names() const {
        return m_keys;
    }

    /** 获取所有统计项值，顺序与 names 相同*/
    PriceList values() const;

    typedef std::map<string, double> map_type;
    typedef map_type::iterator iterator;
    typedef map_type::const_iterator const_iterator;

    const map_type& getAll() const {
        return m_result;
    }

    void addKey(const string& key);
    void setValue(const string& key, double value);

private:
    map_type m_result;
    StringList m_keys;  // 保存统计项顺序, map/unordered_map都不能保持按插入顺序遍历
};

} /* namespace hku */
#endif /* PERFORMANCE_H_ */
