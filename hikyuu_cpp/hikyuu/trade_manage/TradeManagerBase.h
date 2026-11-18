/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#pragma once

#include <cmath>
#include <mutex>
#include <set>

#include "RateTable.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "PositionExtInfo.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "tradecost/TradeCostBase.h"
#include "OrderBrokerBase.h"
#include "margin/MarginBase.h"
#include "margin/crt/MG_None.h"
#include "tradecost/crt/TC_Zero.h"
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
 * margin_accounting(bool): false book margin-security openings by margin only
 * settle_with_close(bool): true use the close as the daily settlement price; off disables daily
 *                           settlement (no other price source yet)
 * force_liquidate(bool): false only warn on a maintenance shortfall; on queues a forced liquidation
 *                         for the trading system at the next open
 * annual_days(int): 365 days per year, the denominator prorating margin interest over elapsed
 *                   calendar days
 * currency_rates(string): "" fixed rate of each foreign market: 1 unit of the market's currency is
 *                         worth rate units of the account currency; an unlisted market is billed at
 *                         1.0 with a warning, an empty table treats all markets as the account
 *                         currency
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManagerBase : public enable_shared_from_this<TradeManagerBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    TradeManagerBase() : TradeManagerBase("", TC_Zero()) {}

    explicit TradeManagerBase(const string& name) : TradeManagerBase(name, TC_Zero()) {}

    TradeManagerBase(const string& name, const TradeCostPtr& costFunc);

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

    /** Get the margin model; null when the account has none */
    virtual MarginPtr margin() const {
        return m_margin;
    }

    /** Set the margin model; null falls back to the plain cash model */
    virtual void margin(const MarginPtr& m) {
        m_margin = m ? m : MG_None();
    }

    /** Margin occupied by an opening, in the security's own currency; 0 when no model is set */
    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const {
        return m_margin ? m_margin->getOpenMargin(datetime, stock, price, num, side) : 0.0;
    }

    /**
     * Margin an opening really locks: the model requirement on the account rounding grid
     * @details A requirement below one monetary unit keeps one unit instead of rounding to zero;
     *          a zero lock would clear the holding's margin marker, demote it to cash and count the
     *          full notional as equity. Returns 0 when the model reports no requirement, so the
     *          caller falls back to cash.
     */
    price_t getOpenMarginToLock(const Datetime& datetime, const Stock& stock, price_t price,
                                double num, MarginSide side) const;

    /** Maintain margin of an opening, in the security's own currency; 0 when no model is set */
    virtual price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const {
        return m_margin ? m_margin->getMaintainMargin(datetime, stock, price, num, side) : 0.0;
    }

    /** Maximum openable quantity under the given funds; 0 when no margin model is set.
     *  Funds are in the account currency, converted for the model, which is pure in the security's
     *  own currency */
    virtual double getMaxOpenNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                    price_t available, MarginSide side) const {
        return m_margin ? m_margin->getMaxOpenNumber(datetime, stock, price,
                                                     _fromAccount(stock, available), side)
                        : 0.0;
    }

    /** Convert an amount from the security's own currency into the account currency via
     *  currency_rates; an empty table is the identity */
    price_t toAccount(const Stock& stock, price_t amount) const {
        return amount * _currencyRate(stock);
    }

    /**
     * Whether an opening is booked by margin accounting instead of full cash
     * @details Requires margin_accounting on; model-covered securities that are auto-financed or
     *          borrow-free (futures, crypto) use margin, the rest use cash
     */
    virtual bool isMarginAccounted(const Stock& stock) const;

    /**
     * Margin occupied by the current long and short positions at the given moment, in the account
     * currency
     * @details Uncovered securities take nothing, so a mixed account excludes its plain cash
     *          holdings
     * @note This is the requirement recomputed at the given price for every model-covered holding,
     *       cash-accounted holdings of a margin security included (brokers count the whole
     *       collateral); FundsRecord::occupied_margin is the amount already locked, so the two
     *       differ by mark-to-market drift and by holdings opened on the plain cash books
     * @note Positions are priced exactly like getFunds, so both halves of getAvailableMargin use
     *       one snapshot
     * @note Measured on current positions, so only current or future moments are meaningful
     */
    virtual price_t getOccupiedMargin(const Datetime& datetime,
                                      KQuery::KType ktype = KQuery::DAY) const;

    /**
     * Available funds for a new opening: net assets minus collateral haircut and occupied margin
     * @note Net assets, not cash, are used: a short adds proceeds to cash, which would otherwise
     *       allow unlimited shorts
     * @note Not const only because getFunds lazily applies ex-rights weights; read it before
     *       mutating the account in a trade flow
     * @note The operands are sequenced: getFunds applies pending weights and changes positions, so
     *       it must run before getOccupiedMargin reads them (operator- order is unspecified)
     * @note Only current or future moments: on a past date getFunds replays historical holdings
     *       while getOccupiedMargin stays on current ones, subtracting today's occupation from a
     *       past net value
     */
    virtual price_t getAvailableMargin(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY);

    /**
     * Settle the margin holdings of the account as of the given moment
     * @details Mark each margin-accounted holding to the settlement price (the close of the given
     *          K type), post the mark-to-market profit to cash, recompute the locked margin at that
     *          price and post the difference, then check the whole book's maintenance requirement
     *          against equity (skipped holdings still count on both sides). A shortfall only warns
     *          by default; force_liquidate queues flattening for the trading system. Then bill
     *          interest on outstanding liabilities into the debt itself (cost model's
     *          getBorrowCashRate / getBorrowStockRate, 0.0 bills nothing), prorated over calendar
     *          days since the last billing and posted before the check, so a billed day is also a
     *          called day. Plain cash books have nothing to settle and keep their live valuation,
     *          so margin_accounting off is never touched.
     * @param datetime the settlement moment, must be later than or equal to lastDatetime()
     * @param ktype the K type the settlement price is taken from
     * @return true when posted (by this call or an earlier one the same calendar day); false when
     *         the account never settles (plain cash books or switch off) or there was nothing to
     *         book (flat and unborrowed, or all holdings skipped with no liability billed)
     * @note Both halves are gated per calendar day, so a second settlement that day posts nothing;
     *       isSettled() only probes that a batch was entered, not that posting ran. The system's K
     *       type does not change this: a finer one simply settles at its first bar
     * @note Only the current moment can be settled: posting is a real cash flow, so past dates are
     *       served by getFunds / getFundsList replay
     */
    virtual bool settlement(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) {
        HKU_IF_RETURN(!tryGetParam<bool>("margin_accounting", false), false);
        HKU_WARN(
          "Settlement is not supported by the account {}, the margin holdings stay on "
          "their opening basis!",
          m_name);
        return false;
    }

    /**
     * Whether the calendar day of the given moment has entered a settlement batch
     * @note Posting and billing are gated apart, so true does not mean holdings were marked: a day
     *       that only billed interest also answers true
     * @param datetime the moment to probe
     */
    virtual bool isSettled(const Datetime&) const {
        return false;
    }

    /**
     * Whether a forced liquidation is queued as of the given moment
     * @details Armed by settlement() when equity falls below the maintenance requirement with
     *          force_liquidate on; the trading system flattens at the next open. Each settlement
     *          rebuilds the queue, so a recovered account clears it; with nothing left to close the
     *          answer is false even while the armed date is recorded. The probe means "armed and
     *          still holding" and consumes nothing.
     * @param datetime the moment to probe
     */
    virtual bool isForcedLiquidationQueued(const Datetime&) const {
        return false;
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
    shared_ptr<TradeManagerBase> clone();

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
     * @note The dates must be in ascending order; the internal parallel calculation advances the
     *       ex-rights ratchet via the last date first, so it is thread-safe by itself
     * @param dates date list
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return daily asset record list
     */
    virtual FundsList getFundsList(const DatetimeList& dates,
                                   const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Get the net value curve of the assets, including the borrowed assets
     * @param dates date list, the net value curve is derived from it
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return net asset value list
     */
    PriceList getFundsCurve(const DatetimeList& dates, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Get the profit curve, i.e. the net asset value curve after deducting every deposit
     * @param dates date list, the profit curve is derived from it; it should be in ascending order
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return profit curve
     */
    PriceList getProfitCurve(const DatetimeList& dates, const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Get the cumulative return rate curve
     * @note The value is total_assets / total_base rounded by the account precision; when the
     *       invested base is 0 (nothing was ever invested) the ratio is undefined and Null
     *       (NaN) is returned instead of a fake 0 (which would read as a wiped-out account)
     * @param dates date list
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return return rate curve
     */
    PriceList getProfitCumChangeCurve(const DatetimeList& dates,
                                      const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Get the invested base asset curve
     * @param dates date list, the curve is derived from it; it should be in ascending order
     * @param ktype K-line type, it must match the date list, KQuery::DAY by default
     * @return price curve
     */
    PriceList getBaseAssetsCurve(const DatetimeList& dates,
                                 const KQuery::KType& ktype = KQuery::DAY);

    /**
     * Update the current positions and trades according to the weight (adjustment) information
     * @note It must be called in chronological order; not thread-safe, it mutates the internal
     *       state (positions, cash and the trade list), concurrent access to the same instance
     *       must be serialized externally
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
     * Get the reference of all the trade records, avoiding copying the whole list for read-only
     * access. The returned reference is only valid before the next modification of the
     * TradeManager. The default implementation returns an empty list; subclasses holding a
     * persistent trade list should override it.
     */
    virtual const TradeRecordList& getRefTradeList() const {
        HKU_WARN("The subclass does not implement this method");
        static const TradeRecordList g_empty_trade_list;
        return g_empty_trade_list;
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
     * @note Only number is the exact value on the given date; the other economic fields (e.g.
     *       takeDatetime, buyMoney, totalCost, sellMoney) are taken from the most recent closed
     *       position record of the security, for reference only
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
     * @param realPrice actual buy price; 0 is a legal market-order placeholder (the dealing
     *        price is decided by the broker)
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
     * @param realPrice actual sell price; 0 is a legal market-order placeholder (the dealing
     *        price is decided by the broker)
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
     * @param realPrice actual short sell price; 0 is a legal market-order placeholder (the dealing
     *        price is decided by the broker)
     * @param number sell quantity
     * @param stoploss stop-loss price
     * @param goalPrice target price
     * @param planPrice planned short sell price
     * @param from records which system part issued the sell instruction
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
     * @param realPrice actual covering price; 0 is a legal market-order placeholder (the dealing
     *        price is decided by the broker)
     * @param number buy quantity; MAX_DOUBLE means covering the entire short position
     * @param stoploss stop-loss price
     * @param goalPrice target price
     * @param planPrice planned buy price
     * @param from records which system part issued the buy instruction
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

    /** Whether this trade manager, its cost function or its margin model is defined in Python */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object || (m_costfunc && m_costfunc->hasPythonPart()) ||
               (m_margin && m_margin->hasPythonPart());
    }

    // These interfaces are declared last: inserting a virtual mid-class shifts the vtable slots of
    // all later ones, so an out-of-tree plugin built against the old header would dispatch wrongly.

    /**
     * Collateral conversion rate of the security, answered by the margin model
     * @details 1.0 when the account has no margin model, i.e. the holding counts in full
     * @param datetime the queried moment
     * @param stock the held security
     * @param side long or short
     */
    virtual double getCollateralRate(const Datetime& datetime, const Stock& stock,
                                     MarginSide side) const {
        return m_margin ? m_margin->getCollateralRate(datetime, stock, side) : 1.0;
    }

    /**
     * Value the collateral table discounts away, in the account currency: Σ collateral base ×
     * (1 - conversion rate) over the long holdings
     * @details The rate is the share of a holding the credit account counts as collateral; cash and
     *          liabilities always count in full, shorts contribute no asset. An unconfigured table
     *          answers 1.0, so the result is 0 and available funds are unchanged
     * @note A margin-accounted holding is discounted on its net-asset contribution (locked margin
     *       plus unrealized profit), not full value: the financed part is already a liability, so
     *       discounting the whole value would remove it twice
     * @note Covers every security getOccupiedMargin requires margin for, but only its long side (a
     *       short holds no asset); securities outside the model are plain cash holdings, neither
     *       margin-posting nor credit collateral
     * @note Holdings are priced exactly like getOccupiedMargin, for the same reason
     */
    virtual price_t getCollateralHaircut(const Datetime& datetime,
                                         KQuery::KType ktype = KQuery::DAY) const;

    /**
     * Annualized interest rate of the borrowed cash, answered by the cost model
     * @details 0.0 when the model has no such rate, i.e. nothing is billed
     * @param datetime the day being settled
     */
    virtual double getBorrowCashRate(const Datetime& datetime) const {
        return m_costfunc ? m_costfunc->getBorrowCashRate(datetime) : 0.0;
    }

    /**
     * Annualized interest rate of a borrowed security, answered by the cost model
     * @details 0.0 when the model has no such rate, i.e. nothing is billed
     * @param datetime the day being settled
     * @param stock the borrowed security
     */
    virtual double getBorrowStockRate(const Datetime& datetime, const Stock& stock) const {
        return m_costfunc ? m_costfunc->getBorrowStockRate(datetime, stock) : 0.0;
    }

protected:
    /**
     * The moment the holdings of a date are valued at, mirroring getFunds
     * @details An unset date or the last trade date values at the last trade moment; any other date
     *          at that day's close
     */
    Datetime _valuationDatetime(const Datetime& datetime) const {
        return (datetime == Null<Datetime>() || datetime == lastDatetime())
                 ? lastDatetime()
                 : Datetime(datetime.year(), datetime.month(), datetime.day(), 23, 59);
    }

    string m_name;            // Account name
    TradeCostPtr m_costfunc;  // Cost algorithm
    MarginPtr m_margin;       // Margin model, null when the account has none

    Datetime m_broker_last_datetime;     // The last moment an order broker performed an operation,
                                         // i.e. the current startup time
    list<OrderBrokerPtr> m_broker_list;  // Order broker list

    bool m_is_python_object{false};

protected:
    /** Fixed rate of the security's market: 1.0 when the table is empty or the market is unlisted
     *  (warned once); the table is parsed by baseCheckParam */
    price_t _currencyRate(const Stock& stock) const;

    /** Convert an account-currency amount into the security's currency */
    price_t _fromAccount(const Stock& stock, price_t amount) const {
        return amount / _currencyRate(stock);
    }

protected:
    mutable RateTable m_currency_rates;    // Parsed currency_rates, empty: all 1.0
    mutable std::set<string> m_fx_warned;  // Markets already warned as having no rate
    // Guards the one-off warning above: getFundsList values the dates in parallel
    mutable std::mutex m_fx_warned_mutex;

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
        if (version >= 1) {
            ar& BOOST_SERIALIZATION_NVP(m_margin);
        }
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_broker_list);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_params);
        // The parsed caches derived from the params are not archived: rebuild them here
        baseCheckParam("currency_rates");
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_costfunc);
        // A version 0 archive has no margin model: the value set by the constructor is kept
        if (version >= 1) {
            ar& BOOST_SERIALIZATION_NVP(m_margin);
        }
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
    } else if ("currency_rates" == name) {
        // A rate is a positive, unbounded factor: 0 would wipe amounts out. Parsed here rather than
        // in _checkParam so every account flavour, including forwarding views, parses it
        m_currency_rates.parse(tryGetParam<string>("currency_rates", ""), "TM currency_rates",
                               false, false, true);
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

#if HKU_SUPPORT_SERIALIZATION
// 0 = no margin model in the archive, 1 = m_margin added
BOOST_CLASS_VERSION(::hku::TradeManagerBase, 1)
#endif

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::TradeManagerPtr> : ostream_formatter {};
#endif