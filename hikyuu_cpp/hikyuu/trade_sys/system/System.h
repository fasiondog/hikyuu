/*
 * SystemBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#pragma once
#ifndef SYSTEMBASE_H_
#define SYSTEMBASE_H_

#include <nlohmann/json.hpp>
// #include "../../KData.h"
// #include "../../utilities/Parameter.h"
// #include "../../trade_manage/TradeManager.h"
#include "../environment/EnvironmentBase.h"
#include "../condition/ConditionBase.h"
#include "../moneymanager/MoneyManagerBase.h"
#include "../signal/SignalBase.h"
#include "../stoploss/StoplossBase.h"
#include "../profitgoal/ProfitGoalBase.h"
#include "../slippage/SlippageBase.h"
#include "TradeRequest.h"
#include "SystemPart.h"
#include "MomentResult.h"
#include "../../serialization/KData_serialization.h"

namespace hku {

using json = nlohmann::json;

class HKU_API WalkForwardSystem;

/**
 * Base class of the trading system
 * @ingroup System
 */
class HKU_API System : public enable_shared_from_this<System> {
    PARAMETER_SUPPORT_WITH_CHECK
    friend class HKU_API WalkForwardSystem;

public:
    /** Default constructor */
    System();

    /** Constructor with the system name specified */
    explicit System(const string& name);

    /**
     * @brief Constructor
     *
     * @param tm the given account
     * @param mm the given money management strategy
     * @param ev the given market environment strategy
     * @param cn the given system valid condition strategy
     * @param sg the given signal generator
     * @param st the given stop-loss strategy
     * @param tp the given take-profit strategy
     * @param pg the given profit goal strategy
     * @param sp the given slippage algorithm
     * @param name system name
     */
    System(const TradeManagerPtr& tm, const MoneyManagerPtr& mm, const EnvironmentPtr& ev,
           const ConditionPtr& cn, const SignalPtr& sg, const StoplossPtr& st,
           const StoplossPtr& tp, const ProfitGoalPtr& pg, const SlippagePtr& sp,
           const string& name);

    System(const System&) = default;

    /** Destructor */
    virtual ~System();

    typedef SystemPart Part;

    /** Get the name */
    const string& name() const;

    /** Set the name */
    void name(const string& name);

    /** Get the traded K-line data (TO) */
    KData getTO() const;

    /** Get the managed account */
    TradeManagerPtr getTM() const;

    /** Get the money management strategy */
    MoneyManagerPtr getMM() const;

    /** Get the market environment strategy */
    EnvironmentPtr getEV() const;

    /** Get the system valid condition strategy */
    ConditionPtr getCN() const;

    /** Get the signal generator */
    SignalPtr getSG() const;

    /** Get the stop-loss strategy */
    StoplossPtr getST() const;

    /** Get the take-profit strategy */
    StoplossPtr getTP() const;

    /** Get the profit goal strategy */
    ProfitGoalPtr getPG() const;

    /** Get the slippage strategy */
    SlippagePtr getSP() const;

    /** Set the managed account */
    void setTM(const TradeManagerPtr& tm);

    /** Set the money management strategy */
    void setMM(const MoneyManagerPtr& mm);

    /** Set the market environment strategy */
    void setEV(const EnvironmentPtr& ev);

    /** Set the system valid condition strategy */
    void setCN(const ConditionPtr& cn);

    /** Set the signal generator */
    void setSG(const SignalPtr& sg);

    /** Set the stop-loss strategy */
    void setST(const StoplossPtr& st);

    /** Set the take-profit strategy */
    void setTP(const StoplossPtr& tp);

    /** Set the profit goal strategy */
    void setPG(const ProfitGoalPtr& pg);

    /** Set the slippage algorithm */
    void setSP(const SlippagePtr& sp);

    /** Get the traded security */
    Stock getStock() const;

    /** Set the traded security */
    void setStock(const Stock& stk);

    const KQuery& getQuery() const;

    /** Get the actually executed trade records; unlike TM, the trade records caused by the
     *  equity/dividend adjustment are not included */
    const TradeRecordList& getTradeRecordList() const;

    /** Get the buy request list; in "delay" mode it shows whether a buy operation exists at the
     *  next moment */
    const std::vector<TradeRequest>& getBuyTradeRequestList() const;

    /** Get the sell request list; in "delay" mode it shows whether a sell operation exists at the
     *  next moment */
    const std::vector<TradeRequest>& getSellTradeRequestList() const;

    const std::vector<TradeRequest>& getSellShortTradeRequestList() const;
    const std::vector<TradeRequest>& getBuyShortTradeRequestList() const;

    /** Mark all the parts as not shared */
    void setNotSharedAll();

    /**
     * Reset, excluding the existing trading object and the shared parts
     * @note The actual reset operation depends on the shared parameters of the parts in the system
     */
    void reset();

    /** Force resetting all the parts and clearing the existing trading object, ignoring the
     *  shared attribute of the parts */
    void forceResetAll();

    typedef shared_ptr<System> SystemPtr;

    /**
     * Clone operation, the clone operation of every part is called in turn
     */
    SystemPtr clone();

    /**
     * Set the traded K-line data (TO)
     * @note tm and ev have no setTO interface
     */
    void setTO(const KData& kdata);

    /**
     * After the backtest is finished, return the trade record of the last day together with the
     * delayed buy and sell requests that need to be delayed
     */
    json lastSuggestion() const;

    /**
     * @brief Run without a specified stock; the stock must be set beforehand through setStock
     * @param query query condition
     * @param reset whether to reset according to the shared attribute of the system parts before
     *              execution
     * @param resetAll force resetting all the parts
     */
    void run(const KQuery& query, bool reset = true, bool resetAll = false);

    /**
     * @brief Run the system strategy
     * @param stock the given security
     * @param query the given query condition
     * @param reset whether to reset according to the shared attribute of the system parts before
     *              execution
     * @param resetAll force resetting all the parts
     */
    void run(const Stock& stock, const KQuery& query, bool reset = true, bool resetAll = false);

    /**
     * @brief Run the system
     * @param kdata the given traded K-line data (TO)
     * @param reset whether to reset according to the shared attribute of the system parts before
     *              execution
     * @param resetAll force resetting all the parts
     */
    virtual void run(const KData& kdata, bool reset = true, bool resetAll = false);

    /**
     * @brief Execute one step on the given date, called by the aggregate system (MultiSystem) or
     *        the live trading driver
     * @param datetime the given date
     * @return MomentResult
     */
    virtual MomentResult runMoment(const Datetime& datetime);

    virtual MomentResult runMomentOnOpen(const Datetime& datetime);
    virtual MomentResult runMomentOnClose(const Datetime& datetime);

    //========================================
    // The aggregate form (MultiSystem) interface, the single-security form returns the default
    // value
    //========================================

    /** Whether it is the aggregate form (holding sub-systems). The single-security form returns
     * false. */
    virtual bool isComposite() const {
        return false;
    }

    /** Get the direct sub-system list, overridden by the aggregate form. The single-security form
     * returns an empty list. */
    virtual const std::vector<std::shared_ptr<System>>& getSubSystemList() const;

    /** The hierarchy path (e.g. I/D/A), used by trace and debugging. Maintained by the aggregate
     * form at readyForRun. */
    virtual const string& getPath() const {
        return m_path;
    }

    /** Set the hierarchy path (the aggregate form writes it into the sub-systems recursively at
     * readyForRun) */
    void setPath(const string& path) {
        m_path = path;
    }

    /** [Mode B] The parent writes the allocation quota back to the sub-system (on the rebalancing
     * day only). It is a no-op for the single-security form. */
    virtual void setSubSystemQuota(const std::shared_ptr<System>& sub_sys, const Datetime& date,
                                   price_t quota) {}

    /** Translate the trade of this moment into the parent suggestion (overridden by the aggregate
     * form). The single-security form returns empty. */
    virtual TradeSuggestionList toSuggestions() const {
        return TradeSuggestionList{};
    }

    // Preparation before running; an exception is thrown on failure
    virtual void readyForRun();

    // Called by the related parts to notify sys when the part parameters change, so that
    // it is recalculated
    void partChangedNotify() {
        m_calculated = false;
    }

    virtual void _reset() {}
    virtual void _forceResetAll() {}

    /** Subclass clone interface */
    virtual SystemPtr _clone() {
        return make_shared<System>();
    }

    virtual string str() const;

public:
    //-------------------------
    // For internal use by the aggregate system (MultiSystem) only
    //-------------------------

    // Force selling at the open price, for internal use by the aggregate system only
    // @note from allows PART_SYSTEM; PART_PORTFOLIO is a historical compatibility value of the
    //       deprecated PF (kept for the compatibility with the old serialized data)
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) {
        HKU_ASSERT(from == PART_PORTFOLIO || from == PART_SYSTEM);
        return _sellForce(date, num, from, true);
    }

    // Force selling at the close price, for internal use by the aggregate system only
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) {
        HKU_ASSERT(from == PART_PORTFOLIO || from == PART_SYSTEM);
        return _sellForce(date, num, from, false);
    }

    // Clear the existing trade requests, used by the aggregate system
    virtual void clearDelayBuyRequest();

    // Whether a delayed operation request currently exists, used by the aggregate system
    bool haveDelaySellRequest() const {
        return !m_sellRequestList.empty();
    }

    bool haveDelayBuyRequest() const {
        return !m_buyRequestList.empty();
    }

    // Process the delayed sell request, called by the aggregate system only
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date);

    // Process the delayed buy request, called by the aggregate system only
    virtual TradeRecord pfProcessDelayBuyRequest(const Datetime& date);

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    bool _environmentIsValid(const Datetime& datetime);
    bool _conditionIsValid(const Datetime& datetime);

    // Notify all the parts that need to receive the actual buy trade record
    void _buyNotifyAll(const TradeRecord&);

    // Notify all the parts that need to receive the actual sell trade record
    void _sellNotifyAll(const TradeRecord&);

    double _getBuyNumber(const Datetime&, price_t price, price_t risk, Part from);
    double _getSellNumber(const Datetime&, price_t price, price_t risk, Part from);
    double _getSellShortNumber(const Datetime&, price_t price, price_t risk, Part from);
    double _getBuyShortNumber(const Datetime&, price_t price, price_t risk, Part from);

    price_t _getStoplossPrice(const KRecord& today, const KRecord& src_today, price_t price);
    price_t _getShortStoplossPrice(const KRecord& today, const KRecord& src_today, price_t price);

    price_t _getTakeProfitPrice(const Datetime& datetime, price_t currentPrice);

    price_t _getGoalPrice(const Datetime& datetime, price_t price);
    price_t _getShortGoalPrice(const Datetime&, price_t price);

    price_t _getRealBuyPrice(const Datetime& datetime, price_t planPrice);
    price_t _getRealSellPrice(const Datetime& datetime, price_t planPrice);

    TradeRecord _buy(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _buyNow(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _buyDelay(const KRecord& today, const KRecord& src_today);
    void _submitBuyRequest(const KRecord& today, const KRecord& src_today, Part from);

    TradeRecord _sell(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _sellNow(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _sellDelay(const KRecord& today, const KRecord& src_today);
    void _submitSellRequest(const KRecord& today, const KRecord& src_today, Part from);

    TradeRecord _sellShort(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _sellShortNow(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _sellShortDelay(const KRecord& today, const KRecord& src_today);
    void _submitSellShortRequest(const KRecord& today, const KRecord& src_today, Part from);

    TradeRecord _buyShort(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _buyShortNow(const KRecord& today, const KRecord& src_today, Part from);
    TradeRecord _buyShortDelay(const KRecord& today, const KRecord& src_today);
    void _submitBuyShortRequest(const KRecord& today, const KRecord& src_today, Part from);

    TradeRecord _processRequest(const KRecord& today, const KRecord& src_today);

    TradeRecord _runMoment(const KRecord& today, const KRecord& src_today);
    TradeRecord _runMomentOnOpen(const KRecord& today, const KRecord& src_today);
    TradeRecord _runMomentOnClose(const KRecord& today, const KRecord& src_today);

    // The aggregate system (MultiSystem) instructs an immediate forced sell, so that the funds of
    // the buy_delay system can be adjusted
    TradeRecord _sellForce(const Datetime& date, double num, Part from, bool on_open);

protected:
    TradeManagerPtr m_tm;
    MoneyManagerPtr m_mm;
    string m_path;  // The hierarchy path (used by the aggregate form)
    EnvironmentPtr m_ev;
    ConditionPtr m_cn;
    SignalPtr m_sg;
    StoplossPtr m_st;
    StoplossPtr m_tp;
    ProfitGoalPtr m_pg;
    SlippagePtr m_sp;

    string m_name;
    Stock m_stock;
    KData m_kdata;
    KData m_src_kdata;  // The original K-line data without adjustment

    bool m_is_python_object{false};
    bool m_calculated;  // Controls whether a recalculation is needed
    bool m_pre_ev_valid;
    bool m_pre_cn_valid;

    int m_buy_days;                 // Cleared on every buy and increased by one on every
                                    // calculation, i.e. the number of days after the buy
    int m_sell_short_days;          // Cleared on every short sell
    TradeRecordList m_trade_list;   // Saves the actually executed trade records
    price_t m_lastTakeProfit;       // The last long take-profit price, used to guarantee that the
                                    // take-profit price increases monotonically
    price_t m_lastShortTakeProfit;  // The last short take-profit price

    std::vector<TradeRequest> m_buyRequestList;
    std::vector<TradeRequest> m_sellRequestList;
    std::vector<TradeRequest> m_sellShortRequestList;
    std::vector<TradeRequest> m_buyShortRequestList;

private:
    void initParam();  // Initialize the parameters and their default values

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

        ar& BOOST_SERIALIZATION_NVP(m_tm);
        ar& BOOST_SERIALIZATION_NVP(m_ev);
        ar& BOOST_SERIALIZATION_NVP(m_cn);
        ar& BOOST_SERIALIZATION_NVP(m_mm);
        ar& BOOST_SERIALIZATION_NVP(m_sg);
        ar& BOOST_SERIALIZATION_NVP(m_st);
        ar& BOOST_SERIALIZATION_NVP(m_tp);
        ar& BOOST_SERIALIZATION_NVP(m_pg);
        ar& BOOST_SERIALIZATION_NVP(m_sp);

        ar& BOOST_SERIALIZATION_NVP(m_kdata);
        ar& BOOST_SERIALIZATION_NVP(m_stock);

        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_calculated);
        ar& BOOST_SERIALIZATION_NVP(m_pre_ev_valid);
        ar& BOOST_SERIALIZATION_NVP(m_pre_cn_valid);

        ar& BOOST_SERIALIZATION_NVP(m_buy_days);
        ar& BOOST_SERIALIZATION_NVP(m_sell_short_days);
        ar& BOOST_SERIALIZATION_NVP(m_trade_list);
        ar& BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar& BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);

        ar& BOOST_SERIALIZATION_NVP(m_buyRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_sellRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_sellShortRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_buyShortRequestList);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);

        ar& BOOST_SERIALIZATION_NVP(m_tm);
        ar& BOOST_SERIALIZATION_NVP(m_ev);
        ar& BOOST_SERIALIZATION_NVP(m_cn);
        ar& BOOST_SERIALIZATION_NVP(m_mm);
        ar& BOOST_SERIALIZATION_NVP(m_sg);
        ar& BOOST_SERIALIZATION_NVP(m_st);
        ar& BOOST_SERIALIZATION_NVP(m_tp);
        ar& BOOST_SERIALIZATION_NVP(m_pg);
        ar& BOOST_SERIALIZATION_NVP(m_sp);

        ar& BOOST_SERIALIZATION_NVP(m_kdata);
        ar& BOOST_SERIALIZATION_NVP(m_stock);

        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_calculated);
        ar& BOOST_SERIALIZATION_NVP(m_pre_ev_valid);
        ar& BOOST_SERIALIZATION_NVP(m_pre_cn_valid);

        ar& BOOST_SERIALIZATION_NVP(m_buy_days);
        ar& BOOST_SERIALIZATION_NVP(m_sell_short_days);
        ar& BOOST_SERIALIZATION_NVP(m_trade_list);
        ar& BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar& BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);

        ar& BOOST_SERIALIZATION_NVP(m_buyRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_sellRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_sellShortRequestList);
        ar& BOOST_SERIALIZATION_NVP(m_buyShortRequestList);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/**
 * Client programs should operate through this pointer
 * @ingroup System
 */
typedef shared_ptr<System> SystemPtr;
typedef shared_ptr<System> SYSPtr;
typedef vector<SystemPtr> SystemList;

HKU_API std::ostream& operator<<(std::ostream& os, const System& sys);
HKU_API std::ostream& operator<<(std::ostream& os, const SystemPtr& sys);

inline const string& System::name() const {
    return m_name;
}

inline void System::name(const string& name) {
    m_name = name;
}

inline KData System::getTO() const {
    return m_kdata;
}

inline TradeManagerPtr System::getTM() const {
    return m_tm;
}

inline MoneyManagerPtr System::getMM() const {
    return m_mm;
}

inline EnvironmentPtr System::getEV() const {
    return m_ev;
}

inline ConditionPtr System::getCN() const {
    return m_cn;
}

inline SignalPtr System::getSG() const {
    return m_sg;
}

inline StoplossPtr System::getST() const {
    return m_st;
}

inline StoplossPtr System::getTP() const {
    return m_tp;
}

inline ProfitGoalPtr System::getPG() const {
    return m_pg;
}

inline SlippagePtr System::getSP() const {
    return m_sp;
}

inline void System::setTM(const TradeManagerPtr& tm) {
    if (m_tm != tm) {
        m_tm = tm;
        m_calculated = false;
    }
}

inline void System::setMM(const MoneyManagerPtr& mm) {
    if (m_mm != mm) {
        m_mm = mm;
        m_calculated = false;
    }
}

inline void System::setEV(const EnvironmentPtr& ev) {
    if (m_ev != ev) {
        m_ev = ev;
        m_calculated = false;
    }
}

inline void System::setCN(const ConditionPtr& cn) {
    if (m_cn != cn) {
        m_cn = cn;
        m_calculated = false;
    }
}

inline void System::setSG(const SignalPtr& sg) {
    if (m_sg != sg) {
        m_sg = sg;
        m_calculated = false;
    }
}

inline void System::setST(const StoplossPtr& st) {
    if (m_st != st) {
        m_st = st;
        m_calculated = false;
    }
}

inline void System::setTP(const StoplossPtr& tp) {
    if (m_tp != tp) {
        m_tp = tp;
        m_calculated = false;
    }
}

inline void System::setPG(const ProfitGoalPtr& pg) {
    if (m_pg != pg) {
        m_pg = pg;
        m_calculated = false;
    }
}

inline void System::setSP(const SlippagePtr& sp) {
    if (m_sp != sp) {
        m_sp = sp;
        m_calculated = false;
    }
}

inline Stock System::getStock() const {
    return m_stock;
}

inline void System::setStock(const Stock& stk) {
    if (m_stock != stk) {
        m_stock = stk;
        m_calculated = false;
    }
}

inline const KQuery& System::getQuery() const {
    return m_kdata.getQuery();
}

inline const TradeRecordList& System::getTradeRecordList() const {
    return m_trade_list;
}

inline const std::vector<TradeRequest>& System::getBuyTradeRequestList() const {
    return m_buyRequestList;
}

inline const std::vector<TradeRequest>& System::getSellTradeRequestList() const {
    return m_sellRequestList;
}

inline const std::vector<TradeRequest>& System::getSellShortTradeRequestList() const {
    return m_sellShortRequestList;
}

inline const std::vector<TradeRequest>& System::getBuyShortTradeRequestList() const {
    return m_buyShortRequestList;
}

inline bool System::_environmentIsValid(const Datetime& datetime) {
    return m_ev ? m_ev->isValid(datetime) : true;
}

inline bool System::_conditionIsValid(const Datetime& datetime) {
    return m_cn ? m_cn->isValid(datetime) : true;
}

inline double System ::_getBuyNumber(const Datetime& datetime, price_t price, price_t risk,
                                     Part from) {
    return m_mm ? m_mm->getBuyNumber(datetime, m_stock, price, risk, from) : 0;
}

inline double System ::_getSellNumber(const Datetime& datetime, price_t price, price_t risk,
                                      Part from) {
    return m_mm ? m_mm->getSellNumber(datetime, m_stock, price, risk, from) : 0;
}

inline double System ::_getSellShortNumber(const Datetime& datetime, price_t price, price_t risk,
                                           Part from) {
    return m_mm ? m_mm->getSellShortNumber(datetime, m_stock, price, risk, from) : 0;
}

inline double System ::_getBuyShortNumber(const Datetime& datetime, price_t price, price_t risk,
                                          Part from) {
    return m_mm ? m_mm->getBuyShortNumber(datetime, m_stock, price, risk, from) : 0;
}

inline price_t System ::_getRealBuyPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealBuyPrice(datetime, planPrice) : planPrice;
}

inline price_t System ::_getRealSellPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealSellPrice(datetime, planPrice) : planPrice;
}

inline price_t System ::_getTakeProfitPrice(const Datetime& datetime, price_t currentPrice) {
    return m_tp ? m_tp->getPrice(datetime, currentPrice) : 0.0;
}

inline price_t System ::_getGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getGoal(datetime, price) : Null<price_t>();
}

inline price_t System ::_getShortGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getShortGoal(datetime, price) : 0.0;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::System> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SystemPtr> : ostream_formatter {};
#endif

#endif /* SYSTEMBASE_H_ */