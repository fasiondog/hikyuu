/*
 * TradeRecord.h
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADERECORD_H_
#define TRADERECORD_H_

#include "../StockManager.h"
#include "../trade_sys/system/SystemPart.h"
#include "CostRecord.h"

#include "../serialization/Datetime_serialization.h"
#include "../serialization/Stock_serialization.h"

namespace hku {

/**
 * 业务类型
 * @ingroup TradeManagerClass
 */
enum BUSINESS {
    BUSINESS_INIT = 0,            /**< 建立初始账户 */
    BUSINESS_BUY = 1,             /**< 买入 */
    BUSINESS_SELL = 2,            /**< 卖出 */
    BUSINESS_GIFT = 3,            /**< 送股 */
    BUSINESS_BONUS = 4,           /**< 分红 */
    BUSINESS_CHECKIN = 5,         /**< 存入现金 */
    BUSINESS_CHECKOUT = 6,        /**< 取出现金 */
    BUSINESS_MARGIN_CLOSEOUT = 7, /**< 交易所强平 */
    BUSINESS_INVALID = 8          /**< 无效类型 */
};

/**
 * 获取业务名称，用于打印输出
 * @ingroup TradeManagerClass
 */
string HKU_API getBusinessName(BUSINESS);

/**
 * 根据字符串获取相应的BUSINESS枚举值
 */
BUSINESS HKU_API getBusinessEnum(const string&);

/**
 * 交易记录
 * @ingroup TradeManagerClass
 */
class HKU_API TradeRecord {
public:
    TradeRecord() = default;
    TradeRecord(const Stock& stock, const Datetime& datetime, BUSINESS business, price_t planPrice,
                price_t realPrice, price_t goalPrice, double number, const CostRecord& cost,
                price_t stoploss, price_t cash, double margin_ratio, SystemPart from);

    /** 仅用于python的__str__ */
    string toString() const;

    bool isNull() const;

    Stock stock;                           ///< 交易对象
    Datetime datetime;                     ///< 交易日期
    BUSINESS business = BUSINESS_INVALID;  ///< 业务类型
    price_t planPrice = 0.0;               ///< 计划交易价格
    price_t realPrice = 0.0;               ///< 实际交易价格
    price_t goalPrice = 0.0;               ///< 目标价位，如果为0表示未限定目标
    double number = 0.0;                   ///< 成交数量
    CostRecord cost;                       ///< 交易成本
    price_t stoploss = 0.0;                ///< 止损价
    price_t cash = 0.0;                    ///< 现金余额
    double margin_ratio = 1.0;             ///< 保证金比例
    SystemPart from = PART_INVALID;  ///< 辅助记录交易系统部件，区别是哪个部件发出的指示

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        hku::uint64_t date_number = datetime.number();
        ar& bs::make_nvp("datetime", date_number);
        string business_name = getBusinessName(business);
        ar& bs::make_nvp<string>("business", business_name);
        ar& BOOST_SERIALIZATION_NVP(planPrice);
        ar& BOOST_SERIALIZATION_NVP(realPrice);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(cost);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(margin_ratio);
        string part_name(getSystemPartName(from));
        ar& bs::make_nvp<string>("from", part_name);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        hku::uint64_t date_number;
        ar& bs::make_nvp("datetime", date_number);
        datetime = Datetime(date_number);
        string business_name;
        ar& bs::make_nvp<string>("business", business_name);
        business = getBusinessEnum(business_name);
        ar& BOOST_SERIALIZATION_NVP(planPrice);
        ar& BOOST_SERIALIZATION_NVP(realPrice);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(cost);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(margin_ratio);
        string part_name;
        ar& bs::make_nvp<string>("from", part_name);
        from = getSystemPartEnum(part_name);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

/**
 * @ingroup TradeManagerClass
 */
typedef vector<TradeRecord> TradeRecordList;

/**
 * 输出TradeRecord信息
 * @ingroup TradeManagerClass
 */
inline std::ostream& operator<<(std::ostream& os, const TradeRecord& tr) {
    os << tr.toString();
    return os;
}

bool HKU_API operator==(const TradeRecord& d1, const TradeRecord& d2);

} /* namespace hku */
#endif /* TRADERECORD_H_ */
