/*
 * StockTypeInfo.h
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKTYPEINFO_H_
#define STOCKTYPEINFO_H_

#include "DataType.h"

namespace hku {

#define STOCKTYPE_BLOCK 0  /// 板块
#define STOCKTYPE_A 1      /// A股
#define STOCKTYPE_INDEX 2  /// 指数
#define STOCKTYPE_B 3      /// B股
#define STOCKTYPE_FUND 4   /// 基金
#define STOCKTYPE_ETF 5    /// ETF
#define STOCKTYPE_ND 6     /// 国债
#define STOCKTYPE_BOND 7   /// 债券
#define STOCKTYPE_GEM 8    /// 创业板
#define STOCKTYPE_START 9  /// 科创板
#define STOCKTYPE_CRYPTO 10 /// 数字货币

#define STOCKTYPE_TMP 999  /// 用于临时Stock

/**
 * 证券类型信息
 * @ingroup StockManage
 */
class HKU_API StockTypeInfo {
public:
    /** 默认构造函数，返回Null<StockTypeInfo>() */
    StockTypeInfo();
    StockTypeInfo(uint32_t, const string&, price_t, price_t, int, double, double);

    /** 获取证券类型 */
    uint32_t type() const {
        return m_type;
    }

    /** 获取证券类型描述信息 */
    const string& description() const {
        return m_description;
    }

    /** 获取最小跳动量 */
    price_t tick() const {
        return m_tick;
    }

    /** 每tick价格 */
    price_t tickValue() const {
        return m_tickValue;
    }

    /** 每单位价格 = tickValue / tick */
    price_t unit() const {
        return m_unit;
    }

    /** 获取价格精度 */
    int precision() const {
        return m_precision;
    }

    /** 获取每笔最小交易数量 */
    double minTradeNumber() const {
        return m_minTradeNumber;
    }

    /** 获取每笔最大交易数量 */
    double maxTradeNumber() const {
        return m_maxTradeNumber;
    }

    /** 仅用于python的__str__ */
    string toString() const;

private:
    uint32_t m_type;          // 证券类型
    string m_description;     // 描述信息
    price_t m_tick;           // 最小跳动量
    price_t m_tickValue;      // 每一个tick价格
    price_t m_unit;           // 每最小变动量价格，即单位价格 = tickValue/tick
    int m_precision;          // 价格精度
    double m_minTradeNumber;  // 每笔最小交易量
    double m_maxTradeNumber;  // 每笔最大交易量
};

/**
 * 输出证券类型信息，如：StockTypeInfo(type, description, tick, precision,
 * minTradeNumber, maxTradeNumber)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const StockTypeInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// 关系比较函数
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockTypeInfo&, const StockTypeInfo&);
bool operator!=(const StockTypeInfo&, const StockTypeInfo&);

/** 相等比较 */
inline bool operator==(const StockTypeInfo& m1, const StockTypeInfo& m2) {
    return m1.type() == m2.type();
}

/** 不等比较 */
inline bool operator!=(const StockTypeInfo& m1, const StockTypeInfo& m2) {
    return m1.type() != m2.type();
}

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::StockTypeInfo> : ostream_formatter {};
#endif

#endif /* STOCKTYPEINFO_H_ */
