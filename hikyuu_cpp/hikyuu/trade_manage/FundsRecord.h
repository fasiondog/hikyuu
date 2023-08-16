/*
 * FundsRecord.h
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef FUNDSRECORD_H_
#define FUNDSRECORD_H_

#include "../DataType.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#endif

namespace hku {

/**
 * 当前资产情况记录，由TradeManager::getFunds返回
 * @ingroup TradeManagerClass
 */
class HKU_API FundsRecord {
public:
    FundsRecord();
    FundsRecord(price_t cash, price_t market_value, price_t base_cash, price_t base_asset);

    price_t cash;         /**< 当前账户现金 */
    price_t market_value; /**< 当前持仓市值, 对于合约交易，则为占用保证金+盈利 */
    price_t base_cash;    /**< 当前投入本金principal */
    price_t base_asset; /**< 当前投入的自有资产价值 */

    // 当前总资产 = 现金 + 市值
    //          = cash + market_value
    // 当前投入本值资产 = 投入本金 + 投入资产价值
    //                = base_cash + base_asset
    // 当前收益 = 当前总资产 - 当前投入本值资产

    FundsRecord operator+(const FundsRecord other);

    FundsRecord& operator+=(const FundsRecord other);

    // 序列化支持
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(market_value);
        ar& BOOST_SERIALIZATION_NVP(base_cash);
        ar& BOOST_SERIALIZATION_NVP(base_asset);
    }
#endif
};

/**
 * 输出TradeRecord信息
 * @ingroup TradeManagerClass
 */
HKU_API std::ostream& operator<<(std::ostream&, const FundsRecord&);

bool HKU_API operator==(const FundsRecord& d1, const FundsRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::FundsRecord> : ostream_formatter {};
#endif

#endif /* FUNDSRECORD_H_ */
