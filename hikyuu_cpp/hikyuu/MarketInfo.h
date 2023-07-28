/*
 * MarketInfo.h
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#pragma once
#ifndef MARKETINFO_H_
#define MARKETINFO_H_

#include "DataType.h"

namespace hku {

/**
 * 市场信息记录
 * @ingroup StockManage
 */
class HKU_API MarketInfo {
public:
    /** 默认构造函数，返回Null<MarketInfo>() */
    MarketInfo();

    /**
     * @param market 市场简称
     * @param name 市场名称
     * @param description 市场描述
     * @param code 基本指数：用于读取该市场的交易日历
     * @param lastDate 市场当前最后日期
     * @param openTime1 开市时间段1起始时间
     * @param closeTime1 开市时间段1结束时间
     * @param openTime2 开市时间段2起始时间
     * @param closeTime2 开市时间段2结束时间
     */
    MarketInfo(const string& market, const string& name, const string& description,
               const string& code, const Datetime& lastDate, TimeDelta openTime1,
               TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2);

    /** 获取市场简称 */
    const string& market() const {
        return m_market;
    }

    /** 获取市场名称 */
    const string& name() const {
        return m_name;
    }

    /** 获取市场描述 */
    const string& description() const {
        return m_description;
    }

    /** 获取该市场对应的指数代码 */
    const string& code() const {
        return m_code;
    }

    /** 获取市场数据的最后更新日期 */
    Datetime lastDate() const {
        return m_lastDate;
    }

    /** 开市时间1 */
    TimeDelta openTime1() const {
        return m_openTime1;
    }

    /** 闭市时间1 */
    TimeDelta closeTime1() const {
        return m_closeTime1;
    }

    /** 开市时间2 */
    TimeDelta openTime2() const {
        return m_openTime2;
    }

    /** 闭市时间2 */
    TimeDelta closeTime2() const {
        return m_closeTime2;
    }

    /** 仅用于python的__str__ */
    string toString() const;

private:
    string m_market;         // 市场标识
    string m_name;           // 市场名称
    string m_description;    // 描述信息
    string m_code;           // 市场对应的指数代码，用于获取交易日历
    Datetime m_lastDate;     // 当前市场最后日期
    TimeDelta m_openTime1;   // 上午开市时间
    TimeDelta m_closeTime1;  // 上午闭市时间
    TimeDelta m_openTime2;   // 下午开市时间
    TimeDelta m_closeTime2;  // 下午闭市时间
};

/**
 * 输出市场信息，如：
 * MarketInfo(SH, 上海证券交易所, 上海市场, 000001, 2011-Dec-06 00:00:00)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const MarketInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// 关系比较函数
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const MarketInfo&, const MarketInfo&);
bool operator!=(const MarketInfo&, const MarketInfo&);

/** 相等比较 */
inline bool operator==(const MarketInfo& m1, const MarketInfo& m2) {
    return m1.market() == m2.market();
}

/** 不等比较 */
inline bool operator!=(const MarketInfo& m1, const MarketInfo& m2) {
    return m1.market() != m2.market();
}

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MarketInfo> : ostream_formatter {};
#endif

#endif /* MARKETINFO_H_ */
