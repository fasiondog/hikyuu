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

namespace hku {

/**
 * 当前资产情况记录，由TradeManager::getFunds返回
 * @ingroup TradeManagerClass
 */
class HKU_API FundsRecord {
public:
    FundsRecord() = default;
    FundsRecord(price_t cash, price_t market_value, price_t short_market_value, price_t base_cash,
                price_t base_asset, price_t borrow_cash, price_t borrow_asset);

    price_t cash{0.0};               /**< 当前现金 */
    price_t market_value{0.0};       /**< 当前多头市值 */
    price_t short_market_value{0.0}; /**< 当前空头仓位市值 */
    price_t base_cash{0.0};          /**< 当前投入本金principal */
    price_t base_asset{0.0};         /**< 当前投入的资产价值 */
    price_t borrow_cash{0.0};        /**< 当前借入的资金，即负债 */
    price_t borrow_asset{0.0};       /**< 当前借入证券资产价值 */

    // 当前总资产 = 现金 + 多头市值 + 空头数量×（借入价格 - 当前价格)
    //          = cash + market_value + borrow_asset - short_market_value
    // 当前负债 = 借入的资金 + 借入的资产价值
    //         = borrow_cash + borrow_asset
    // 当前净资产 = 总资产 - 负债
    //          = cash + market_value - short_market_value - borrow_cash
    // 当前投入本值资产 = 投入本金 + 投入资产价值
    //                = base_cash + base_asset
    // 当前收益 = 当前净资产 - 当前投入本值资产
    //         = cash + market_value - short_market_value - borrow_cash - base_cash - base_asset

    // 当前总资产
    price_t total_assets() const {
        return cash + market_value + borrow_asset - short_market_value;
    }

    // 当前净资产
    price_t net_assets() const {
        return cash + market_value - short_market_value - borrow_cash;
    }

    // 总负债
    price_t total_borrow() const {
        return borrow_cash + borrow_asset;
    }

    // 当前投入本值资产
    price_t total_base() const {
        return base_cash + base_asset;
    }

    // 当前收益
    price_t profit() const {
        return cash + market_value - short_market_value - borrow_cash - base_cash - base_asset;
    }

    FundsRecord operator+(const FundsRecord& other) const;

    FundsRecord& operator+=(const FundsRecord& other);

    // 序列化支持
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(cash);
        ar& BOOST_SERIALIZATION_NVP(market_value);
        ar& BOOST_SERIALIZATION_NVP(short_market_value);
        ar& BOOST_SERIALIZATION_NVP(base_cash);
        ar& BOOST_SERIALIZATION_NVP(base_asset);
        ar& BOOST_SERIALIZATION_NVP(borrow_cash);
        ar& BOOST_SERIALIZATION_NVP(borrow_asset);
    }
#endif
};

typedef vector<FundsRecord> FundsList;
typedef vector<FundsRecord> FundsRecordList;

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
