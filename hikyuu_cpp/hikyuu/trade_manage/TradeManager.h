/*
 * TradeManager.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADEMANAGER_H_
#define TRADEMANAGER_H_

#include "TradeManagerBase.h"
#include "../utilities/Parameter.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "OrderBrokerBase.h"
#include "crt/TC_Zero.h"

namespace hku {

/**
 * Trade management module of the backtest simulated account; it manages the trade records and the
 * fund usage of the account
 * @details
 * <pre>
 * Default parameters:
 * precision(int): 2 calculation precision
 * support_borrow_cash(bool): false whether cash is borrowed automatically on a buy operation
 * support_borrow_stock(bool): false whether the stock is borrowed automatically when short selling
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManager : public TradeManagerBase {
    // PARAMETER_SUPPORT

public:
    explicit TradeManager(const Datetime& datetime = Datetime(199001010000LL),
                          price_t initcash = 100000.0, const TradeCostPtr& costfunc = TC_Zero(),
                          const string& name = "SYS");
    virtual ~TradeManager();

    /** Reset, clearing the trade and position records */
    virtual void _reset() override;

    virtual shared_ptr<TradeManagerBase> _clone() override;

    /**
     * Get the margin rate of the given security
     * @param datetime date
     * @param stock the given security
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) override;

    /** Initial cash */
    virtual price_t initCash() const override {
        return m_init_cash;
    }

    /** Account creation date */
    virtual Datetime initDatetime() const override {
        return m_init_datetime;
    }

    /** Date of the first buy trade; Null<Datetime>() is returned if no trade has happened */
    virtual Datetime firstDatetime() const override;

    /** Date of the last trade, regardless of the trade type; the account creation date is returned
     *  if no trade has happened */
    virtual Datetime lastDatetime() const override {
        return m_trade_list.empty() ? m_init_datetime : m_trade_list.back().datetime;
    }

    /**
     * Update the current positions and trades according to the weight (adjustment) information
     * @note It must be called in chronological order
     * @param datetime the current moment
     */
    virtual void updateWithWeight(const Datetime& datetime) override;

    /**
     * Return the current cash
     * @note Only the current information is returned, it is not adjusted according to the weight
     *       information
     */
    virtual price_t currentCash() const override {
        return m_cash;
    }

    /**
     * Get the cash of the given date
     * @note Without the date parameter the positions cannot be adjusted according to the weight
     *       information
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override;

    /**
     * Whether the given security is currently held
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool have(const Stock& stock) const override {
        return m_position.count(stock.id()) ? true : false;
    }

    /**
     * Whether the given security is currently held in the short position
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool haveShort(const Stock& stock) const override {
        return m_short_position.count(stock.id()) ? true : false;
    }

    /** Number of security types currently held */
    virtual size_t getStockNumber() const override {
        return m_position.size();
    }

    /** Number of security types currently held short */
    virtual size_t getShortStockNumber() const override {
        return m_short_position.size();
    }

    /** Get the held quantity of a security at the given moment */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) override;

    /** Get the short held quantity of a security at the given moment */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) override;

    /** Get the number of borrowed shares at the given moment */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) override;

    /** Get the amount of borrowed cash at the given moment */
    virtual price_t getDebtCash(const Datetime& datetime) override;

    /** Get all the trade records */
    virtual TradeRecordList getTradeList() const override {
        return m_trade_list;
    }

    /**
     * Get the trade records within the given date range [start, end)
     * @param start start date
     * @param end end date
     * @return trade record list
     */
    virtual TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const override;

    /** Get all the current position records */
    virtual PositionRecordList getPositionList() const override;

    /** Get all the historical position records, i.e. the closed position records */
    virtual PositionRecordList getHistoryPositionList() const override {
        return m_position_history;
    }

    /** Get all the current short position records */
    virtual PositionRecordList getShortPositionList() const override;

    /** Get all the historical short position records */
    virtual PositionRecordList getShortHistoryPositionList() const override {
        return m_short_position_history;
    }

    /**
     * Get the position record of the given security
     * @param date the given date
     * @param stock the given security
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) override;

    /** Get the current short position record of the given security; Null<PositionRecord>() is
     *  returned if the security is not currently held short */
    virtual PositionRecord getShortPosition(const Stock&) const override;

    /** Get the list of currently borrowed shares */
    virtual BorrowRecordList getBorrowStockList() const override;

    /**
     * Deposit funds
     * @param datetime deposit time
     * @param cash deposited amount
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) override;

    /**
     * Withdraw funds
     * @param datetime withdrawal time
     * @param cash withdrawn amount
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) override;

    /**
     * Deposit assets
     * @param datetime deposit date
     * @param stock the stock to deposit
     * @param price price per share of the deposited stock
     * @param number number of deposited shares
     * @return true | false
     */
    virtual bool checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                              double number) override;

    /**
     * Withdraw the current assets
     * @param datetime withdrawal date
     * @param stock the stock to withdraw
     * @param price withdrawal price per share
     * @param number withdrawn quantity
     * @return true | false
     * @note It should never be used
     */
    virtual bool checkoutStock(const Datetime& datetime, const Stock& stock, price_t price,
                               double number) override;

    /**
     * Buy operation
     * @param datetime buy time
     * @param stock the security to buy
     * @param realPrice actual buy price
     * @param number buy quantity
     * @param stoploss stop-loss price
     * @param goalPrice target price
     * @param planPrice planned buy price
     * @param from records which system part issued the buy instruction
     * @param remark buy remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                            double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                            price_t planPrice = 0.0, SystemPart from = PART_INVALID,
                            const string& remark = "") override;

    /**
     * Sell operation
     * @param datetime sell time
     * @param stock the security to sell
     * @param realPrice actual sell price
     * @param number sell quantity; MAX_DOUBLE means selling everything
     * @param stoploss new stop-loss price
     * @param goalPrice new target price
     * @param planPrice originally planned sell price
     * @param from records which system part issued the sell instruction
     * @param remark sell remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord sell(const Datetime& datetime, const Stock& stock, price_t realPrice,
                             double number = MAX_DOUBLE, price_t stoploss = 0.0,
                             price_t goalPrice = 0.0, price_t planPrice = 0.0,
                             SystemPart from = PART_INVALID, const string& remark = "") override;

    /**
     * Short sell
     * @param datetime short sell time
     * @param stock the security to short sell
     * @param realPrice actual short sell price
     * @param number sell quantity
     * @param stoploss stop-loss price
     * @param goalPrice target price
     * @param planPrice planned short sell price
     * @param from records which system part issued the buy instruction
     * @param remark remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                  double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                                  price_t planPrice = 0.0, SystemPart from = PART_INVALID,
                                  const string& remark = "") override;

    /**
     * Cover a short position
     * @param datetime buy time
     * @param stock the security to buy
     * @param realPrice actual buy price
     * @param number sell quantity; MAX_DOUBLE means selling everything
     * @param stoploss stop-loss price
     * @param goalPrice target price
     * @param planPrice planned buy price
     * @param from records which system part issued the sell instruction
     * @param remark remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                 double number = MAX_DOUBLE, price_t stoploss = 0.0,
                                 price_t goalPrice = 0.0, price_t planPrice = 0.0,
                                 SystemPart from = PART_INVALID,
                                 const string& remark = "") override;

    /**
     * Borrow funds, i.e. funds borrowed from another source, e.g. margin
     * @param datetime borrow time
     * @param cash borrowed cash
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) override;

    /**
     * Repay funds
     * @param datetime repayment date
     * @param cash repaid cash
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) override;

    /**
     * Borrow a security
     * @param datetime borrow time
     * @param stock the borrowed stock
     * @param price price per share when borrowing
     * @param number borrowed quantity
     * @return true | false
     */
    virtual bool borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override;

    /**
     * Return a borrowed security
     * @param datetime return time
     * @param stock the returned stock
     * @param price price per share when returning
     * @param number returned quantity
     * @return true | false
     */
    virtual bool returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override;

    /**
     * Get the asset details of the account at the current moment
     * @param ktype the type of the date
     * @return asset details
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const override;

    /**
     * Get the market value details of the assets at the given moment
     * @param datetime it must be later than the initial date of the account, or Null<Datetime>()
     * @param ktype the type of the date
     * @return asset details
     * @note When datetime equals Null<Datetime>(), it is the same as getFunds(KType)
     */
    virtual FundsRecord getFunds(const Datetime& datetime,
                                 KQuery::KType ktype = KQuery::DAY) override;

    /**
     * Add a trade record directly
     * @note If an account initialization record is added, all the existing trade and position
     *       records are cleared
     * @param tr the trade record to add
     * @return bool true on success | false on failure
     */
    virtual bool addTradeRecord(const TradeRecord& tr) override;

    /**
     * Add a position record directly
     * @note Special purpose: building the initial position, it may cause confusion
     * @param pr position record
     * @return true on success
     * @return false on failure
     */
    virtual bool addPosition(const PositionRecord& pr) override;

    /** String output */
    virtual string str() const override;

    /**
     * Export the trade records, open position records, closed position records and the net value
     * curve of the assets in csv format
     * @param path the directory of the output file
     */
    virtual void tocsv(const string& path) override;

private:
    // Save the trade actions in the form of a script, so that they can be corrected and calibrated
    void _saveAction(const TradeRecord&);

    bool _add_init_tr(const TradeRecord&);
    bool _add_buy_tr(const TradeRecord&);
    bool _add_sell_tr(const TradeRecord&);
    bool _add_checkin_tr(const TradeRecord&);
    bool _add_checkout_tr(const TradeRecord&);
    bool _add_checkin_stock_tr(const TradeRecord&);
    bool _add_checkout_stock_tr(const TradeRecord&);
    bool _add_borrow_cash_tr(const TradeRecord&);
    bool _add_return_cash_tr(const TradeRecord&);
    bool _add_borrow_stock_tr(const TradeRecord&);
    bool _add_return_stock_tr(const TradeRecord&);
    bool _add_sell_short_tr(const TradeRecord&);
    bool _add_buy_short_tr(const TradeRecord&);

private:
    Datetime m_init_datetime;         // Account creation date
    price_t m_init_cash;              // Initial cash
    Datetime m_last_update_datetime;  // The last moment when the positions and trade records were
                                      // adjusted according to the weight information

    price_t m_cash;            // Current cash
    price_t m_checkin_cash;    // Accumulated deposited funds, the initial cash counts as a deposit
    price_t m_checkout_cash;   // Accumulated withdrawn funds
    price_t m_checkin_stock;   // Accumulated value of deposited shares
    price_t m_checkout_stock;  // Accumulated value of withdrawn shares
    price_t m_borrow_cash;     // Currently borrowed funds, i.e. the debt

    list<LoanRecord> m_loan_list;  // Current margin financing status

    typedef map<uint64_t, BorrowRecord> borrow_stock_map_type;
    borrow_stock_map_type m_borrow_stock;  // Currently borrowed shares and their quantities

    TradeRecordList m_trade_list;  // Trade records

    typedef map<uint64_t, PositionRecord> position_map_type;
    position_map_type m_position;                 // Position record of the currently held security
                                                  // ["sh000001"-> ]
    PositionRecordList m_position_history;        // Historical position records
    position_map_type m_short_position;           // Short position records
    PositionRecordList m_short_position_history;  // Historical short position records

    // list<OrderBrokerPtr> m_broker_list;  // Order broker list
    // Datetime m_broker_last_datetime;     // The last moment an order broker performed an
    // operation

    list<string> m_actions;  // Records the trade actions, so that the trades of a live account can
                             // be modified or calibrated

//==================================================
// Serialization support
//==================================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeManagerBase);
        ar& BOOST_SERIALIZATION_NVP(m_init_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_init_cash);
        ar& BOOST_SERIALIZATION_NVP(m_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkin_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkout_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkin_stock);
        ar& BOOST_SERIALIZATION_NVP(m_checkout_stock);
        ar& BOOST_SERIALIZATION_NVP(m_borrow_cash);
        ar& BOOST_SERIALIZATION_NVP(m_loan_list);
        namespace bs = boost::serialization;
        BorrowRecordList borrow = getBorrowStockList();
        ar& bs::make_nvp<BorrowRecordList>("m_borrow_stock", borrow);
        PositionRecordList position = getPositionList();
        ar& bs::make_nvp<PositionRecordList>("m_position", position);
        ar& BOOST_SERIALIZATION_NVP(m_position_history);
        position = getShortPositionList();
        ar& bs::make_nvp<PositionRecordList>("m_short_position", position);
        ar& BOOST_SERIALIZATION_NVP(m_short_position_history);
        ar& BOOST_SERIALIZATION_NVP(m_trade_list);
        ar& BOOST_SERIALIZATION_NVP(m_actions);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeManagerBase);
        ar& BOOST_SERIALIZATION_NVP(m_init_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_init_cash);
        ar& BOOST_SERIALIZATION_NVP(m_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkin_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkout_cash);
        ar& BOOST_SERIALIZATION_NVP(m_checkin_stock);
        ar& BOOST_SERIALIZATION_NVP(m_checkout_stock);
        ar& BOOST_SERIALIZATION_NVP(m_borrow_cash);
        ar& BOOST_SERIALIZATION_NVP(m_loan_list);
        namespace bs = boost::serialization;
        BorrowRecordList borrow;
        ar& bs::make_nvp<BorrowRecordList>("m_borrow_stock", borrow);
        BorrowRecordList::const_iterator bor_iter = borrow.begin();
        for (; bor_iter != borrow.end(); ++bor_iter) {
            m_borrow_stock[bor_iter->stock.id()] = *bor_iter;
        }
        PositionRecordList position;
        ar& bs::make_nvp<PositionRecordList>("m_position", position);
        PositionRecordList::const_iterator iter = position.begin();
        for (; iter != position.end(); ++iter) {
            m_position[iter->stock.id()] = *iter;
        }
        ar& BOOST_SERIALIZATION_NVP(m_position_history);

        position.clear();
        ar& bs::make_nvp<PositionRecordList>("m_short_position", position);
        iter = position.begin();
        for (; iter != position.end(); ++iter) {
            m_short_position[iter->stock.id()] = *iter;
        }
        ar& BOOST_SERIALIZATION_NVP(m_short_position_history);
        ar& BOOST_SERIALIZATION_NVP(m_trade_list);
        ar& BOOST_SERIALIZATION_NVP(m_actions);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */
#endif /* TRADEMANAGER_H_ */
