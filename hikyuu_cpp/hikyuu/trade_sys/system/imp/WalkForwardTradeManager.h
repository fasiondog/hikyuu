/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-19
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_manage/TradeManagerBase.h"

namespace hku {

class WalkForwardTradeManager : public TradeManagerBase {
public:
    WalkForwardTradeManager() : TradeManagerBase("", TC_Zero()) {}

    // Only the actual trades with a date >= run_start are allowed
    WalkForwardTradeManager(const TMPtr& tm, const Datetime& run_start) : m_run_start(run_start) {
        HKU_ASSERT(tm);
        m_tm = tm;

        m_name = tm->name();
        m_costfunc = m_tm->costFunc();

        // Mirror the wrapped account's accounting params on the view: getParam/setParam are
        // non-virtual, so the System pre-checks (the deposit sizing, the delayed-buy re-size,
        // readyForRun) read the view while the real booking reads the wrapped account. A stale
        // default would round on another precision grid or warn about a switch the account never
        // lost. The mirror is read-only: a write through the view never reaches the shared account
        // (the host WalkForwardSystem writes the merged switches straight onto the real book)
        _mirrorParams();
    }

    virtual ~WalkForwardTradeManager() {}

    // The view must never reset the wrapped real account: it filters by run_start, it does not
    // own the account (the host WalkForwardSystem resets through its own m_tm)
    virtual void _reset() override {}

    virtual shared_ptr<TradeManagerBase> _clone() override {
        WalkForwardTradeManager* p = new WalkForwardTradeManager();
        p->m_tm = m_tm->clone();
        p->m_run_start = m_run_start;
        p->_mirrorParams();
        return shared_ptr<TradeManagerBase>(p);
    }

    /** Copy the wrapped account's accounting params onto the view (see the constructor) */
    void _mirrorParams() {
        setParam<int>("precision", m_tm->precision());
        setParam<bool>("margin_accounting", m_tm->tryGetParam<bool>("margin_accounting", false));
        setParam<bool>("settle_with_close", m_tm->tryGetParam<bool>("settle_with_close", true));
        setParam<bool>("force_liquidate", m_tm->tryGetParam<bool>("force_liquidate", false));
        setParam<bool>("support_borrow_cash",
                       m_tm->tryGetParam<bool>("support_borrow_cash", false));
        setParam<bool>("support_borrow_stock",
                       m_tm->tryGetParam<bool>("support_borrow_stock", false));
        setParam<int>("annual_days", m_tm->tryGetParam<int>("annual_days", 365));
        // The System pre-checks convert with the view, so a missing mirror would value the need of
        // a foreign market at 1.0 while the real account books it at its rate
        setParam<string>("currency_rates", m_tm->tryGetParam<string>("currency_rates", ""));
    }

    /**
     * Update the current positions and trades according to the weight (adjustment) information
     * @note It must be called in chronological order
     * @param datetime the current moment
     */
    virtual void updateWithWeight(const Datetime& datetime) override {
        m_tm->updateWithWeight(datetime);
    }

    /**
     * Get the margin rate of the given security
     * @param datetime date
     * @param stock the given security
     * @note Before the window the rate is 1.0, i.e. the plain no-leverage semantics; 0 would read
     * as an infinite buying power
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) override {
        return (datetime >= m_run_start) ? m_tm->getMarginRate(datetime, stock) : 1.0;
    }

    /**
     * The margin model of the wrapped account
     * @note Forwarded instead of copied: the base accessors are virtual, so a snapshot taken at
     *       construction would go stale as soon as the host system swaps its own model
     */
    virtual MarginPtr margin() const override {
        return m_tm->margin();
    }

    virtual void margin(const MarginPtr& m) override {
        m_tm->margin(m);
    }

    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const override {
        return m_tm->getOpenMargin(datetime, stock, price, num, side);
    }

    /** The accounting switch lives on the wrapped account, so the eligibility follows it */
    virtual bool isMarginAccounted(const Stock& stock) const override {
        return m_tm->isMarginAccounted(stock);
    }

    virtual price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const override {
        return m_tm->getMaintainMargin(datetime, stock, price, num, side);
    }

    /** Before the window nothing may be opened, so the quota stays 0 */
    virtual double getMaxOpenNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                    price_t available, MarginSide side) const override {
        return (datetime >= m_run_start)
                 ? m_tm->getMaxOpenNumber(datetime, stock, price, available, side)
                 : 0.0;
    }

    virtual price_t getOccupiedMargin(const Datetime& datetime,
                                      KQuery::KType ktype = KQuery::DAY) const override {
        return m_tm->getOccupiedMargin(datetime, ktype);
    }

    virtual price_t getAvailableMargin(const Datetime& datetime,
                                       KQuery::KType ktype = KQuery::DAY) override {
        return m_tm->getAvailableMargin(datetime, ktype);
    }

    /** The view has no margin model of its own, so the collateral table follows the real account */
    virtual double getCollateralRate(const Datetime& datetime, const Stock& stock,
                                     MarginSide side) const override {
        return m_tm->getCollateralRate(datetime, stock, side);
    }

    virtual price_t getCollateralHaircut(const Datetime& datetime,
                                         KQuery::KType ktype = KQuery::DAY) const override {
        return m_tm->getCollateralHaircut(datetime, ktype);
    }

    /**
     * Settle the wrapped account
     * @note Forwarded to the real book (the settlement is a real cash posting) but only inside
     *       the window: before run_start the real account holds nothing of this run
     */
    virtual bool settlement(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override {
        return (datetime >= m_run_start) ? m_tm->settlement(datetime, ktype) : false;
    }

    virtual bool isSettled(const Datetime& datetime) const override {
        return m_tm->isSettled(datetime);
    }

    virtual bool isForcedLiquidationQueued(const Datetime& datetime) const override {
        return m_tm->isForcedLiquidationQueued(datetime);
    }

    /** A Python defined cost or margin model of the wrapped account decides the pickling too */
    virtual bool hasPythonPart() const noexcept override {
        return m_tm->hasPythonPart();
    }

    /** Initial cash */
    virtual price_t initCash() const override {
        return m_tm->initCash();
    }

    /** Account creation date */
    virtual Datetime initDatetime() const override {
        return m_tm->initDatetime();
    }

    /** Date of the first buy trade; Null<Datetime>() is returned if no trade has happened */
    virtual Datetime firstDatetime() const override {
        return m_tm->firstDatetime();
    }

    /** Date of the last trade, regardless of the trade type; the account creation date is returned
     *  if no trade has happened */
    virtual Datetime lastDatetime() const override {
        return m_tm->lastDatetime();
    }

    /**
     * Return the current cash
     * @note Only the current information is returned, it is not adjusted according to the weight
     *       information
     */
    virtual price_t currentCash() const override {
        return m_tm->currentCash();
    }

    /**
     * Get the cash of the given date
     * @note Without the date parameter the positions cannot be adjusted according to the weight
     *       information
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override {
        return m_tm->cash(datetime, ktype);
    }

    /**
     * Whether the given security is currently held
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool have(const Stock& stock) const override {
        return m_tm->have(stock);
    }

    /**
     * Whether the given security is currently held in the short position
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool haveShort(const Stock& stock) const override {
        return m_tm->haveShort(stock);
    }

    /** Number of security types currently held */
    virtual size_t getStockNumber() const override {
        return m_tm->getStockNumber();
    }

    /** Number of security types currently held short */
    virtual size_t getShortStockNumber() const override {
        return m_tm->getShortStockNumber();
    }

    /** Get the held quantity of a security at the given moment */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getHoldNumber(datetime, stock);
    }

    /** Get the short held quantity of a security at the given moment */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getShortHoldNumber(datetime, stock);
    }

    /** Get the number of borrowed shares at the given moment */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getDebtNumber(datetime, stock);
    }

    /** Get the amount of borrowed cash at the given moment */
    virtual price_t getDebtCash(const Datetime& datetime) override {
        return m_tm->getDebtCash(datetime);
    }

    /** Get all the trade records */
    virtual TradeRecordList getTradeList() const override {
        return m_tm->getTradeList();
    }

    /** Get the reference of all the trade records, only for read-only access (no copy) */
    virtual const TradeRecordList& getRefTradeList() const override {
        return m_tm->getRefTradeList();
    }

    /**
     * Get the trade records within the given date range [start, end)
     * @param start start date
     * @param end end date
     * @return trade record list
     */
    virtual TradeRecordList getTradeList(const Datetime& start,
                                         const Datetime& end) const override {
        return m_tm->getTradeList(start, end);
    }

    /** Get all the current position records */
    virtual PositionRecordList getPositionList() const override {
        return m_tm->getPositionList();
    }

    /** Get all the historical position records, i.e. the closed position records */
    virtual PositionRecordList getHistoryPositionList() const override {
        return m_tm->getHistoryPositionList();
    }

    /** Get all the current short position records */
    virtual PositionRecordList getShortPositionList() const override {
        return m_tm->getShortPositionList();
    }

    /** Get all the historical short position records */
    virtual PositionRecordList getShortHistoryPositionList() const override {
        return m_tm->getShortHistoryPositionList();
    }

    /**
     * Get the position record of the given security
     * @param date the given date
     * @param stock the given security
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) override {
        return m_tm->getPosition(date, stock);
    }

    /**
     * Get the short position record of the given security
     * @param stock the given security
     */
    virtual PositionRecord getShortPosition(const Stock& stock) const override {
        return m_tm->getShortPosition(stock);
    }

    /** Get the list of currently borrowed shares */
    virtual BorrowRecordList getBorrowStockList() const override {
        return m_tm->getBorrowStockList();
    }

    /**
     * Deposit funds
     * @param datetime deposit time
     * @param cash deposited amount
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->checkin(datetime, cash) : false;
    }

    /**
     * Withdraw funds
     * @param datetime withdrawal time
     * @param cash withdrawn amount
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->checkout(datetime, cash) : false;
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
        return (datetime >= m_run_start) ? m_tm->checkinStock(datetime, stock, price, number)
                                         : false;
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
        return (datetime >= m_run_start) ? m_tm->checkoutStock(datetime, stock, price, number)
                                         : false;
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
                            const string& remark = "") override {
        return (datetime >= m_run_start) ? m_tm->buy(datetime, stock, realPrice, number, stoploss,
                                                     goalPrice, planPrice, from, remark)
                                         : TradeRecord();
    }

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
                             SystemPart from = PART_INVALID, const string& remark = "") override {
        return (datetime >= m_run_start) ? m_tm->sell(datetime, stock, realPrice, number, stoploss,
                                                      goalPrice, planPrice, from, remark)
                                         : TradeRecord();
    }

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
        return (datetime >= m_run_start)
                 ? m_tm->sellShort(datetime, stock, realPrice, number, stoploss, goalPrice,
                                   planPrice, from, remark)
                 : TradeRecord();
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
        return (datetime >= m_run_start)
                 ? m_tm->buyShort(datetime, stock, realPrice, number, stoploss, goalPrice,
                                  planPrice, from, remark)
                 : TradeRecord();
    }

    /**
     * Borrow funds, the funds borrowed from other sources, such as margin financing
     * @param datetime borrow time
     * @param cash borrowed cash
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->borrowCash(datetime, cash) : false;
    }

    /**
     * Return the funds
     * @param datetime return date
     * @param cash returned cash
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->returnCash(datetime, cash) : false;
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
        return (datetime >= m_run_start) ? m_tm->borrowStock(datetime, stock, price, number)
                                         : false;
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
        return (datetime >= m_run_start) ? m_tm->returnStock(datetime, stock, price, number)
                                         : false;
    }

    /**
     * Get the asset detail of the account at the current moment
     * @param ktype the type of the date
     * @return asset detail
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const override {
        return m_tm->getFunds(ktype);
    }

    /**
     * Get the asset market value detail at the given moment
     * @param datetime it must be greater than the account creation date, or Null<Datetime>()
     * @param ktype the type of the date
     * @return asset detail
     * @note When datetime equals Null<Datetime>() it is the same as getFunds(KType)
     */
    virtual FundsRecord getFunds(const Datetime& datetime,
                                 KQuery::KType ktype = KQuery::DAY) override {
        return m_tm->getFunds(datetime, ktype);
    }

    /**
     * Add a trade record directly
     * @note If an account initialization record is added, all the existing trade and position
     *       records are cleared
     * @param tr the trade record to add
     * @return bool true success | false failure
     */
    virtual bool addTradeRecord(const TradeRecord& tr) override {
        return (tr.datetime >= m_run_start) ? m_tm->addTradeRecord(tr) : false;
    }

    /**
     * Add a position record directly
     * @param pr position record
     * @return true success
     * @return false failure
     */
    virtual bool addPosition(const PositionRecord& pr) override {
        return m_tm->addPosition(pr);
    }

    /** String output */
    virtual string str() const override {
        return m_tm->str();
    }

    /**
     * Output the trade records, the open positions, the closed positions and the net asset value
     * curve in the csv format
     * @param path the directory of the output files
     */
    virtual void tocsv(const string& path) override {
        m_tm->tocsv(path);
    }

    /**
     * Synchronize the current account asset information (including the funds, the positions, etc.)
     * from the order broker instance
     * @param broker the order broker instance
     */
    virtual void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker,
                                          const Datetime& datetime = Null<Datetime>()) override {
        m_tm->fetchAssetInfoFromBroker(broker, datetime);
    }

private:
    TMPtr m_tm;
    Datetime m_run_start;

//============================================
// Serialization support
//============================================
// A runtime-only view over the host account, rebuilt by WalkForwardSystem on every run; it is
// never part of an archive. Reaching the throw means a view instance leaked into a serialized
// object graph (a bug), and loading such an archive could silently break the run_start filter
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        (void)ar;
        (void)version;
        HKU_THROW("WalkForwardTradeManager is a runtime-only view and cannot be serialized");
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

inline TMPtr crtWalkForwardTM(const TMPtr& tm, const Datetime& run_start) {
    return std::make_shared<WalkForwardTradeManager>(tm, run_start);
}

}  // namespace hku
