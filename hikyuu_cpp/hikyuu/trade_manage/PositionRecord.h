/*
 * PositionRecord.h
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef POSITIONRECORD_H_
#define POSITIONRECORD_H_

#include "../StockManager.h"
#include "../serialization/Stock_serialization.h"

namespace hku {

/**
 * 持仓记录
 * @ingroup TradeManagerClass
 */
class HKU_API PositionRecord {
public:
    PositionRecord();
    PositionRecord(const Stock& stock, const Datetime& takeDatetime, const Datetime& cleanDatetime,
                   double number, price_t stoploss, price_t goalPrice, double totalNumber,
                   price_t buyMoney, price_t totalCost, price_t totalRisk, price_t sellMoney);

    /** 仅用于python的__str__ */
    string str() const;

    Stock stock;              ///< 交易对象
    Datetime takeDatetime;    ///< 初次建仓日期
    Datetime cleanDatetime;   ///< 平仓日期，当前持仓记录中为Null<Datetime>()
    double number{0.0};       ///< 当前持仓数量
    price_t stoploss{0.0};    ///< 当前止损价
    price_t goalPrice{0.0};   ///< 当前的目标价格
    double totalNumber{0.0};  ///< 累计持仓数量
    price_t buyMoney{0.0};    ///< 累计买入资金
    price_t totalCost{0.0};   ///< 累计交易总成本
    price_t totalRisk{0.0};  ///< 累计交易风险 = 各次 （买入价格-止损)*买入数量, 不包含交易成本
    price_t sellMoney{0.0};  ///< 累计卖出资金

//===================
// 序列化支持
//===================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        uint64_t take = takeDatetime.number();
        uint64_t clean = cleanDatetime.number();
        ar& bs::make_nvp("takeDatetime", take);
        ar& bs::make_nvp("cleanDatetime", clean);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(stock);
        uint64_t take, clean;
        ar& bs::make_nvp("takeDatetime", take);
        ar& bs::make_nvp("cleanDatetime", clean);
        takeDatetime = Datetime(take);
        cleanDatetime = Datetime(clean);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goalPrice);
        ar& BOOST_SERIALIZATION_NVP(totalNumber);
        ar& BOOST_SERIALIZATION_NVP(buyMoney);
        ar& BOOST_SERIALIZATION_NVP(totalCost);
        ar& BOOST_SERIALIZATION_NVP(totalRisk);
        ar& BOOST_SERIALIZATION_NVP(sellMoney);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/** @ingroup TradeManagerClass */
typedef vector<PositionRecord> PositionRecordList;

/**
 * 输出持仓记录信息
 * @ingroup TradeManagerClass
 */
HKU_API std::ostream& operator<<(std::ostream&, const PositionRecord&);

bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::PositionRecord> : ostream_formatter {};
#endif

#endif /* POSITIONRECORD_H_ */
