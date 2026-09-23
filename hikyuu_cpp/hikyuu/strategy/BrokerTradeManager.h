/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-16
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_manage/TradeManagerBase.h"

namespace hku {

class HKU_API BrokerTradeManager : public TradeManagerBase {
public:
    BrokerTradeManager() = default;
    explicit BrokerTradeManager(const OrderBrokerPtr& broker,
                                const TradeCostPtr& costfunc = TC_Zero(),
                                const string& name = "SYS");
    virtual ~BrokerTradeManager() {}

    virtual void _reset() override;

    virtual shared_ptr<TradeManagerBase> _clone() override;

    virtual void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker,
                                          const Datetime& datetime = Null<Datetime>()) override;

    /**
     * Update the current positions and trades according to the weight (adjustment) information
     * @note It must be called in chronological order
     * @param datetime the current moment
     */
    virtual void updateWithWeight(const Datetime& datetime) override {}

    /**
     * Get the margin rate of the given security
     * @param datetime date
     * @param stock the given security
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) override {
        HKU_WARN("The subclass does not implement a getMarginRate method");
        return 0.0;
    }

    /** Initial cash */
    virtual price_t initCash() const override {
        return m_cash;
    }

    /** Account creation date */
    virtual Datetime initDatetime() const override {
        return m_datetime;
    }

    /** Date of the first buy trade; Null<Datetime>() is returned if no trade has happened */
    virtual Datetime firstDatetime() const override {
        return m_datetime;
    }

    /** Date of the last trade, regardless of the trade type; the account creation date is returned
     *  if no trade has happened */
    virtual Datetime lastDatetime() const override {
        return m_datetime;
    }

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
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override {
        return m_cash;
    }

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
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** Number of security types currently held */
    virtual size_t getStockNumber() const override {
        return m_position.size();
    }

    /** Number of security types currently held short */
    virtual size_t getShortStockNumber() const override {
        HKU_WARN("The subclass does not implement this method");
        return 0;
    }

    /** Get the held quantity of a security at the given moment */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) override {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the short held quantity of a security at the given moment */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) override {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the number of borrowed shares at the given moment */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) override {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the amount of borrowed cash at the given moment */
    virtual price_t getDebtCash(const Datetime& datetime) override {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get all the trade records */
    virtual TradeRecordList getTradeList() const override {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /**
     * Get the trade records within the given date range [start, end)
     * @param start start date
     * @param end end date
     * @return trade record list
     */
    virtual TradeRecordList getTradeList(const Datetime& start,
                                         const Datetime& end) const override {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /** Get all the current position records */
    virtual PositionRecordList getPositionList() const override;

    /** Get all the historical position records, i.e. the closed position records */
    virtual PositionRecordList getHistoryPositionList() const override {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** Get all the current short position records */
    virtual PositionRecordList getShortPositionList() const override {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** Get all the historical short position records */
    virtual PositionRecordList getShortHistoryPositionList() const override {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /**
     * Get the position record of the given security
     * @param date the given date
     * @param stock the given security
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) override;

    /**
     * Get the short position record of the given security
     * @param stock the given security
     */
    virtual PositionRecord getShortPosition(const Stock& stock) const override {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecord();
    }

    /** Get the list of currently borrowed shares */
    virtual BorrowRecordList getBorrowStockList() const override {
        HKU_WARN("The subclass does not implement this method");
        return BorrowRecordList();
    }

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
    virtual bool checkout(const Datetime& datetime, price_t cash) override {
        m_cash = (cash > m_cash) ? 0.0 : m_cash - cash;
        return true;
    }

    /**
     * Deposit assets
     * @param datetime deposit date
     * @param stock the stock to deposit
     * @param price price per share of the deposited stock
     * @param number number of deposited shares
     * @return true | false
     */
    virtual bool checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                              double number) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

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
                               double number) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

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
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                  double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                                  price_t planPrice = 0.0, SystemPart from = PART_INVALID,
                                  const string& remark = "") override {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
    }

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
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                 double number = MAX_DOUBLE, price_t stoploss = 0.0,
                                 price_t goalPrice = 0.0, price_t planPrice = 0.0,
                                 SystemPart from = PART_INVALID,
                                 const string& remark = "") override {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
    }

    /**
     * Borrow funds, the funds borrowed from other sources, such as margin financing
     * @param datetime borrow time
     * @param cash borrowed cash
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Return the funds
     * @param datetime return date
     * @param cash returned cash
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Borrow securities
     * @param datetime borrow time
     * @param stock the borrowed stock
     * @param price price per share at borrowing
     * @param number quantity at borrowing
     * @return true | false
     */
    virtual bool borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Return the securities
     * @param datetime return time
     * @param stock the returned stock
     * @param price price per share at returning
     * @param number returned quantity
     * @return true | false
     */
    virtual bool returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Get the asset detail of the account at the current moment
     * @param ktype the type of the date
     * @return asset detail
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const override;

    /**
     * Get the asset market value detail at the given moment
     * @param datetime it must be greater than the account creation date, or Null<Datetime>()
     * @param ktype the type of the date
     * @return asset detail
     * @note When datetime equals Null<Datetime>() it is the same as getFunds(KType)
     */
    virtual FundsRecord getFunds(const Datetime& datetime,
                                 KQuery::KType ktype = KQuery::DAY) override;

    /**
     * Add a trade record directly
     * @note If an account initialization record is added, all the existing trade and position
     *       records are cleared
     * @param tr the trade record to add
     * @return bool true success | false failure
     */
    virtual bool addTradeRecord(const TradeRecord& tr) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Add a position record directly
     * @param pr position record
     * @return true success
     * @return false failure
     */
    virtual bool addPosition(const PositionRecord& pr) override {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** String output */
    virtual string str() const override;

    /**
     * Output the trade records, the open positions, the closed positions and the net asset value
     * curve in the csv format
     * @param path the directory of the output files
     */
    virtual void tocsv(const string& path) override {
        HKU_WARN("The subclass does not implement this method");
    }

private:
    Datetime m_datetime;  // Current date

    price_t m_cash{0.0};  // Current available cash

    typedef map<uint64_t, PositionRecord> position_map_type;
    position_map_type m_position;  // Position records of the currently held trading objects
};

TradeManagerPtr HKU_API crtBrokerTM(const OrderBrokerPtr& broker,
                                    const TradeCostPtr& costfunc = TC_Zero(),
                                    const string& name = "SYS",
                                    const std::vector<OrderBrokerPtr>& other_brokers = {});

}  // namespace hku