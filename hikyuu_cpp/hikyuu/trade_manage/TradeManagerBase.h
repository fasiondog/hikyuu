/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#pragma once

#include "TradeRecord.h"
#include "PositionRecord.h"
#include "PositionExtInfo.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "TradeCostBase.h"
#include "OrderBrokerBase.h"
#include "crt/TC_Zero.h"
#include "../utilities/Parameter.h"

namespace hku {

class HKU_API Performance;

/**
 * Base class of the account trade management; it manages the trade records and the fund usage of
 * the account
 * @details
 * <pre>
 * Default parameters:
 * precision(int): 2 calculation precision
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManagerBase : public enable_shared_from_this<TradeManagerBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    TradeManagerBase() : TradeManagerBase("", TC_Zero()) {}

    explicit TradeManagerBase(const string& name) : TradeManagerBase(name, TC_Zero()) {}

    TradeManagerBase(const string& name, const TradeCostPtr& costFunc)
    : m_name(name), m_costfunc(costFunc), m_broker_last_datetime(Datetime::now()) {
        setParam<int>("precision", 2);  // Calculation precision
    }

    virtual ~TradeManagerBase() {}

    /** Account name */
    const string& name() const {
        return m_name;
    }

    /** Set the account name */
    void name(const string& name) {
        m_name = name;
    }

    /** Trade precision */
    int precision() const {
        return getParam<int>("precision");
    }

    /** Get the trade cost algorithm pointer */
    TradeCostPtr costFunc() const {
        return m_costfunc;
    }

    /** Set the trade cost algorithm pointer */
    void costFunc(const TradeCostPtr& func) {
        m_costfunc = func;
    }

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock traded security
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord trade cost record
     */
    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                          double num) const {
        return m_costfunc ? m_costfunc->getBuyCost(datetime, stock, price, num) : CostRecord();
    }

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock traded security
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord trade cost record
     */
    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const {
        return m_costfunc ? m_costfunc->getSellCost(datetime, stock, price, num) : CostRecord();
    }

    /**
     * Calculate the cost charged when the cash is credited
     * @param datetime borrow date
     * @param cash cash amount
     */
    CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) {
        return m_costfunc ? m_costfunc->getBorrowCashCost(datetime, cash) : CostRecord();
    }

    /**
     * Calculate the cost of repaying the margin financing
     * @param borrow_datetime borrow date
     * @param return_datetime repayment date
     * @param cash repaid amount
     */
    CostRecord getReturnCashCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                 price_t cash) {
        return m_costfunc ? m_costfunc->getReturnCashCost(borrow_datetime, return_datetime, cash)
                          : CostRecord();
    }

    /**
     * Calculate the cost of borrowing through securities lending
     * @param datetime securities lending date
     * @param stock borrowed security
     * @param price price per share
     * @param num borrowed quantity
     */
    CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) {
        return m_costfunc ? m_costfunc->getBorrowStockCost(datetime, stock, price, num)
                          : CostRecord();
    }

    /**
     * Calculate the cost of returning through securities lending
     * @param borrow_datetime borrow date
     * @param return_datetime return date
     * @param stock returned security
     * @param price price per share when returning
     * @param num returned quantity
     */
    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) {
        return m_costfunc ? m_costfunc->getReturnStockCost(borrow_datetime, return_datetime, stock,
                                                           price, num)
                          : CostRecord();
    }

    /** From which moment the order broker is started to place orders */
    Datetime getBrokerLastDatetime() const {
        return m_broker_last_datetime;
    }

    /** Set the moment from which the order broker starts operating */
    void setBrokerLastDatetime(const Datetime& date) {
        m_broker_last_datetime = date;
    }

    /** Reset, clearing the trade and position records */
    void reset() {
        _reset();
    }

    virtual void _reset() {
        HKU_WARN("The subclass does not implement a reset method");
    }

    /** Perform the clone operation */
    shared_ptr<TradeManagerBase> clone() {
        shared_ptr<TradeManagerBase> p = _clone();
        HKU_CHECK(p, "Invalid ptr from _clone!");
        p->m_params = m_params;
        p->m_name = m_name;
        p->m_is_python_object = m_is_python_object;
        p->m_broker_last_datetime = m_broker_last_datetime;
        p->m_costfunc = m_costfunc;
        p->m_broker_list = m_broker_list;
        return p;
    }

    virtual shared_ptr<TradeManagerBase> _clone() {
        HKU_WARN("The subclass does not implement a reset method");
        return shared_ptr<TradeManagerBase>();
    }

    /**
     * Register an order broker
     * @param broker order broker instance
     */
    void regBroker(const OrderBrokerPtr& broker) {
        m_broker_list.push_back(broker);
    }

    /**
     * Clear the registered order brokers
     */
    void clearBroker() {
        m_broker_list.clear();
    }

    /**
     * Get all the daily asset records of the given date list
     * @param dates date list
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return daily asset record list
     */
    FundsList getFundsList(const DatetimeList& dates, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Get the net value curve of the assets, including the borrowed assets
     * @param dates date list, the net value curve is derived from it
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return net asset value list
     */
    PriceList getFundsCurve(const DatetimeList& dates, const KQuery::KType& ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        int precision = getParam<int>("precision");
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = roundEx(funds_list[i].total_assets(), precision);
        }
        return ret;
    }

    /**
     * Get the profit curve, i.e. the net asset value curve after deducting every deposit
     * @param dates date list, the profit curve is derived from it; it should be in ascending order
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return profit curve
     */
    PriceList getProfitCurve(const DatetimeList& dates, const KQuery::KType& ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        int precision = getParam<int>("precision");
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = roundEx(funds_list[i].profit(), precision);
        }
        return ret;
    }

    /**
     * Get the cumulative return rate curve
     * @param dates date list
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return return rate curve
     */
    PriceList getProfitCumChangeCurve(const DatetimeList& dates,
                                      const KQuery::KType& ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = funds_list[i].total_assets() / funds_list[i].total_base();
        }
        return ret;
    }

    /**
     * Get the invested base asset curve
     * @param dates date list, the curve is derived from it; it should be in ascending order
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return price curve
     */
    PriceList getBaseAssetsCurve(const DatetimeList& dates,
                                 const KQuery::KType& ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = funds_list[i].total_base();
        }
        return ret;
    }

    /**
     * Update the current positions and trades according to the weight (adjustment) information
     * @note It must be called in chronological order
     * @param datetime the current moment
     */
    virtual void updateWithWeight(const Datetime& datetime) {
        HKU_WARN("The subclass does not implement a updateWithWeight method");
    }

    /**
     * Get the margin rate of the given security
     * @param datetime date
     * @param stock the given security
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement a getMarginRate method");
        return 0.0;
    }

    /** Initial cash */
    virtual price_t initCash() const {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Account creation date */
    virtual Datetime initDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** Date of the first buy trade; Null<Datetime>() is returned if no trade has happened */
    virtual Datetime firstDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** Date of the last trade, regardless of the trade type; the account creation date is returned
     *  if no trade has happened */
    virtual Datetime lastDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /**
     * Return the current cash
     * @note Only the current information is returned, it is not adjusted according to the weight
     *       information
     */
    virtual price_t currentCash() const {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /**
     * Get the cash of the given date
     * @note Without the date parameter the positions cannot be adjusted according to the weight
     *       information
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /**
     * Whether the given security is currently held
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool have(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Whether the given security is currently held in the short position
     * @note The date parameter is not used here, so execution in chronological order is required
     * @param stock the given security
     * @return true yes | false no
     */
    virtual bool haveShort(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** Number of security types currently held */
    virtual size_t getStockNumber() const {
        HKU_WARN("The subclass does not implement this method");
        return 0;
    }

    /** Number of security types currently held short */
    virtual size_t getShortStockNumber() const {
        HKU_WARN("The subclass does not implement this method");
        return 0;
    }

    /** Get the held quantity of a security at the given moment */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the short held quantity of a security at the given moment */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the number of borrowed shares at the given moment */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get the amount of borrowed cash at the given moment */
    virtual price_t getDebtCash(const Datetime& datetime) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** Get all the trade records */
    virtual TradeRecordList getTradeList() const {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /**
     * Get the trade records within the given date range [start, end)
     * @param start start date
     * @param end end date
     * @return trade record list
     */
    virtual TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /** Get all the current position records */
    virtual PositionRecordList getPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** Get the dictionary of all the current position records */
    std::unordered_map<Stock, PositionRecord> getPositionDict() const {
        std::unordered_map<Stock, PositionRecord> ret;
        for (const auto& pos : getPositionList()) {
            ret[pos.stock] = pos;
        }
        return ret;
    }

    /** Get all the historical position records, i.e. the closed position records */
    virtual PositionRecordList getHistoryPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** Get all the current short position records */
    virtual PositionRecordList getShortPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** Get all the historical short position records */
    virtual PositionRecordList getShortHistoryPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /**
     * Get the position record of the given security
     * @param date the given date
     * @param stock the given security
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecord();
    }

    /**
     * Get the short position record of the given security
     * @param stock the given security
     */
    virtual PositionRecord getShortPosition(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecord();
    }

    /** Get the list of currently borrowed shares */
    virtual BorrowRecordList getBorrowStockList() const {
        HKU_WARN("The subclass does not implement this method");
        return BorrowRecordList();
    }

    /**
     * Deposit funds
     * @param datetime deposit time
     * @param cash deposited amount
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Withdraw funds
     * @param datetime withdrawal time
     * @param cash withdrawn amount
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                              double number) {
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
                               double number) {
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
     * @param remark remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                            double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                            price_t planPrice = 0.0, SystemPart from = PART_INVALID,
                            const string& remark = "") {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
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
     * @param remark sell remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord sell(const Datetime& datetime, const Stock& stock, price_t realPrice,
                             double number = MAX_DOUBLE, price_t stoploss = 0.0,
                             price_t goalPrice = 0.0, price_t planPrice = 0.0,
                             SystemPart from = PART_INVALID, const string& remark = "") {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
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
     * @param remark remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                  double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                                  price_t planPrice = 0.0, SystemPart from = PART_INVALID,
                                  const string& remark = "") {
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
     * @param remark remark
     * @return the corresponding trade record; business equals BUSINESS_INVALID if the operation
     *         failed
     */
    virtual TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                 double number = MAX_DOUBLE, price_t stoploss = 0.0,
                                 price_t goalPrice = 0.0, price_t planPrice = 0.0,
                                 SystemPart from = PART_INVALID, const string& remark = "") {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
    }

    /**
     * Borrow funds, i.e. funds borrowed from another source, e.g. margin
     * @param datetime borrow time
     * @param cash borrowed cash
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Repay funds
     * @param datetime repayment date
     * @param cash repaid cash
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Borrow a security
     * @param datetime borrow time
     * @param stock the borrowed stock
     * @param price price per share when borrowing
     * @param number borrowed quantity
     * @return true | false
     */
    virtual bool borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Return a borrowed security
     * @param datetime return time
     * @param stock the returned stock
     * @param price price per share when returning
     * @param number returned quantity
     * @return true | false
     */
    virtual bool returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Get the asset details of the account at the current moment
     * @param ktype the type of the date
     * @return asset details
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const {
        HKU_WARN("The subclass does not implement this method");
        return FundsRecord();
    }

    /**
     * Get the market value details of the assets at the given moment
     * @param datetime it must be later than the initial date of the account, or Null<Datetime>()
     * @param ktype the type of the date
     * @return asset details
     * @note When datetime equals Null<Datetime>(), it is the same as getFunds(KType)
     */
    virtual FundsRecord getFunds(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) {
        HKU_WARN("The subclass does not implement this method");
        return FundsRecord();
    }

    /**
     * Add a trade record directly
     * @note If an account initialization record is added, all the existing trade and position
     *       records are cleared
     * @param tr the trade record to add
     * @return bool true on success | false on failure
     */
    virtual bool addTradeRecord(const TradeRecord& tr) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * Add a position record directly
     * @param pr position record
     * @return true on success
     * @return false on failure
     */
    virtual bool addPosition(const PositionRecord& pr) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** String output */
    virtual string str() const {
        HKU_WARN("The subclass does not implement this method");
        return string();
    }

    /**
     * Export the trade records, open position records, closed position records and the net value
     * curve of the assets in csv format
     * @param path the directory of the output file
     */
    virtual void tocsv(const string& path) {
        HKU_WARN("The subclass does not implement this method");
    }

    /**
     * Synchronize the current account asset information (funds, positions, ...) from the order
     * broker instance
     * @param broker order broker instance
     * @param datetime the synchronization time; usually the current time (Null), it can also be
     *        forced to a given moment
     */
    virtual void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker,
                                          const Datetime& datetime = Null<Datetime>()) {
        HKU_WARN("The subclass does not implement this method");
    }

    //-------------------------------------------------------------
    // The following are donation features
    //-------------------------------------------------------------
    /**
     * Calculate the system performance up to a given moment; datetime must be no earlier than
     * lastDatetime, so that the current market value can be calculated
     * @param datetime the end moment of the statistics
     * @param ktype K-line type
     * @param ext whether the extended statistics are needed (donating users)
     */
    Performance getPerformance(const Datetime& datetime = Datetime::now(),
                               const KQuery::KType& ktype = KQuery::DAY, bool ext = true);

    /**
     * @brief Get the maximum drawdown percentage (a negative number) of the account at the given
     *        moment (calculated from the close price only)
     * @param date the given date (inclusive)
     * @param ktype K-line type
     * @return price_t
     */
    price_t getMaxPullBack(const Datetime& date, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * @brief Get the extended details of the historical positions of the account
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics: 0 - trade at the close,
     *        1 - trade at the next open
     * @return std::vector<PositionExtInfo>
     */
    std::vector<PositionExtInfo> getHistoryPositionExtInfoList(
      const KQuery::KType& ktype = KQuery::DAY, int trade_mode = 0);

    /**
     * @brief Get the position details after the last trade moment of the account
     * @param current_time the current moment (it must be no earlier than the last trade moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics: 0 - trade at the close,
     *        1 - trade at the next open
     * @return std::vector<PositionExtInfo>
     */
    std::vector<PositionExtInfo> getPositionExtInfoList(
      const Datetime& current_time = Datetime::now(), const KQuery::KType& ktype = KQuery::DAY,
      int trade_mode = 0);

    /**
     * Get the position details after the last trade moment of the account, with Stock as the key
     * and PositionExtInfo as the value
     * @param current_time the current moment (it must be no earlier than the last trade moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics: 0 - trade at the close,
     *        1 - trade at the next open
     */
    std::unordered_map<Stock, PositionExtInfo> getPositionExtInfoDict(
      const Datetime& datetime = Datetime::now(), const KQuery::KType& ktype = KQuery::DAY,
      int trade_mode = 0);

    /**
     * Get the position details after the last trade moment of the account
     */
    PositionExtInfo getPositionExtInfo(const Stock& stock,
                                       const Datetime& current_time = Datetime::now(),
                                       const KQuery::KType& ktype = KQuery::DAY,
                                       int trade_mode = 0);

    /**
     * @brief Get the profit percentage of each month before the given end time
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    std::vector<std::pair<Datetime, double>> getProfitPercentMonthly(
      const Datetime& datetime = Datetime::now());

    /**
     * @brief Get the profit percentage of each year before the given end time
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    std::vector<std::pair<Datetime, double>> getProfitPercentYearly(
      const Datetime& datetime = Datetime::now());

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;            // Account name
    TradeCostPtr m_costfunc;  // Cost algorithm

    Datetime m_broker_last_datetime;  // The last moment an order broker performed an operation,
                                      // i.e. the current startup time
    list<OrderBrokerPtr> m_broker_list;  // Order broker list

    bool m_is_python_object{false};

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_costfunc);
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_broker_list);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_costfunc);
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_broker_list);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

inline void TradeManagerBase::baseCheckParam(const string& name) const {
    if ("precision" == name) {
        int precision = getParam<int>("precision");
        HKU_ASSERT(precision > 0);
    }
}

inline void TradeManagerBase::paramChanged() {}

/**
 * Client programs should use this type for the actual operations
 * @ingroup TradeManagerClass
 */
typedef shared_ptr<TradeManagerBase> TradeManagerPtr;
typedef shared_ptr<TradeManagerBase> TMPtr;

inline std::ostream& operator<<(std::ostream& os, const TradeManagerBase& tm) {
    os << tm.str();
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const TradeManagerPtr& ptm) {
    if (ptm) {
        os << ptm->str();
    } else {
        os << "TradeManager(NULL)";
    }
    return os;
}

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::TradeManagerPtr> : ostream_formatter {};
#endif