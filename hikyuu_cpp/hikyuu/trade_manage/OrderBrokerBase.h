/*
 * OrderBrokerBase.h
 *
 *  Created on: 2017-6-28
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_ORDERBROKERBASE_H_
#define TRADE_MANAGE_ORDERBROKERBASE_H_

#include "../DataType.h"
#include "../utilities/Parameter.h"
#include "../trade_sys/system/SystemPart.h"

namespace hku {

struct HKU_API BrokerPositionRecord {
    Stock stock;
    price_t number{0.0};  // Quantity
    price_t money{0.0};   // Total funds spent on the buy

    BrokerPositionRecord() = default;
    BrokerPositionRecord(const Stock& stock, price_t number, price_t money);
    BrokerPositionRecord(const BrokerPositionRecord&) = default;
    BrokerPositionRecord(BrokerPositionRecord&& rv);

    BrokerPositionRecord& operator=(const BrokerPositionRecord&) = default;
    BrokerPositionRecord& operator=(BrokerPositionRecord&& rv);

    string str() const;
};

HKU_API std::ostream& operator<<(std::ostream& os, const BrokerPositionRecord&);

/**
 * Base class of the order broker, it implements the actual order operations and the programmatic
 * orders.
 * @details Multiple order broker instances can be registered into TradeManager through
 *          TradeManager.regBroker. These order brokers can perform extra buy / sell actions; for
 *          example an e-mail order broker can send an e-mail when TradeManager issues a buy / sell
 *          instruction.
 *
 *          By default TradeManager calls the order broker to perform the buy / sell action when it
 *          executes a buy / sell operation, but this causes problems in the live trading. The
 * reason is that the system needs to trace back the historical data to get the latest signal when
 *          it calculates the signal indication, so TradeManager would execute the buy / sell
 *          operation at a historical moment. At this time, if the order broker itself does not
 *          control the moment when the buy / sell instruction is issued, the broker would send a
 *          wrong instruction. Therefore it is necessary to specify that the buy / sell operation of
 *          the order broker is allowed only after a certain moment. The brokeLastDatetime attribute
 *          of TradeManager is used to specify that moment.
 * @ingroup OrderBroker
 */
class HKU_API OrderBrokerBase {
    PARAMETER_SUPPORT

public:
    OrderBrokerBase();
    OrderBrokerBase(const string& name);
    virtual ~OrderBrokerBase();

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /**
     * Execute the buy operation
     * @param datetime the time indicated by the strategy
     * @param market market identifier
     * @param code security code
     * @param price buy price
     * @param num buy quantity
     * @param stoploss expected stop-loss price
     * @param goalPrice expected target price
     * @param from the source system part
     * @param remark remark information
     */
    void buy(Datetime datetime, const string& market, const string& code, price_t price, double num,
             price_t stoploss, price_t goalPrice, SystemPart from, const string& remark) noexcept;

    /**
     * Execute the sell operation
     * @param datetime the time indicated by the strategy
     * @param market market identifier
     * @param code security code
     * @param price sell price
     * @param num sell quantity
     * @param stoploss new expected stop-loss price
     * @param goalPrice new expected target price
     * @param from the source system part
     * @param remark remark information
     */
    void sell(Datetime datetime, const string& market, const string& code, price_t price,
              double num, price_t stoploss, price_t goalPrice, SystemPart from,
              const string& remark) noexcept;

    /**
     * Get the current asset information
     * @return string a json string
     * <pre>
     * Interface specification:
     * {
     *   "datetime": "2001-01-01 18:00:00.12345",
     *   "cash": 0.0,
     *   "positions": [
     *       {"market": "SZ", "code": "000001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
     *        "cost_price": 0.0},
     *       {"market": "SH", "code": "600001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
     *        "cost_price": 0.0},
     *    ]
     * }
     *
     * Description:
     * cash: the currently available funds
     * number should be: the existing positions + the ones being bought - the ones being sold
     * cost_price: the buy cost price per share
     * </pre>
     */
    string getAssetInfo() noexcept;

    /**
     * Interface implemented by the subclass, it executes the actual buy operation
     * @param datetime the time indicated by the strategy
     * @param market market identifier
     * @param code security code
     * @param price buy price
     * @param num buy quantity
     * @param stoploss expected stop-loss price
     * @param goalPrice expected target price
     * @param from the source system part
     * @param remark remark information
     */
    virtual void _buy(Datetime datetime, const string& market, const string& code, price_t price,
                      double num, price_t stoploss, price_t goalPrice, SystemPart from,
                      const string& remark) = 0;

    /**
     * Interface implemented by the subclass, it executes the actual sell operation
     * @param datetime the time indicated by the strategy
     * @param market market identifier
     * @param code security code
     * @param price sell price
     * @param num sell quantity
     * @param stoploss new expected stop-loss price
     * @param goalPrice new expected target price
     * @param from the source system part
     * @param remark remark information
     */
    virtual void _sell(Datetime datetime, const string& market, const string& code, price_t price,
                       double num, price_t stoploss, price_t goalPrice, SystemPart from,
                       const string& remark) = 0;

    virtual string _getAssetInfo() {
        return string();
    }

protected:
    string m_name;

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(OrderBrokerBase)
#endif

/**
 * The client program should use this type for the actual operations
 * @ingroup OrderBroker
 */
typedef shared_ptr<OrderBrokerBase> OrderBrokerPtr;

/** @ingroup OrderBroker */
HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerBase&);

/** @ingroup OrderBroker */
HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerPtr&);

inline const string& OrderBrokerBase::name() const {
    return m_name;
}

inline void OrderBrokerBase::name(const string& name) {
    m_name = name;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::OrderBrokerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::OrderBrokerPtr> : ostream_formatter {};
#endif

#endif /* TRADE_MANAGE_ORDERBROKERBASE_H_ */
