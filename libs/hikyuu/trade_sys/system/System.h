/*
 * SystemBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#ifndef SYSTEMBASE_H_
#define SYSTEMBASE_H_

//#include "../../KData.h"
//#include "../../utilities/Parameter.h"
//#include "../../trade_manage/TradeManager.h"
#include "../environment/EnvironmentBase.h"
#include "../condition/ConditionBase.h"
#include "../moneymanager/MoneyManagerBase.h"
#include "../signal/SignalBase.h"
#include "../stoploss/StoplossBase.h"
#include "../profitgoal/ProfitGoalBase.h"
#include "../slippage/SlippageBase.h"
#include "TradeRequest.h"
#include "SystemPart.h"
#include "../../serialization/KData_serialization.h"

namespace hku {

/**
 * 交易系统基类
 * @ingroup System
 */
class HKU_API System {
    PARAMETER_SUPPORT

public:
    System();
    System(const string& name);
    System(const TradeManagerPtr& tm,
            const MoneyManagerPtr& mm,
            const EnvironmentPtr& ev,
            const ConditionPtr& cn,
            const SignalPtr& sg,
            const StoplossPtr& st,
            const StoplossPtr& tp,
            const ProfitGoalPtr& pg,
            const SlippagePtr& sp,
            const string& name);
    virtual ~System();

    typedef SystemPart Part;

    /** 获取名称 */
    string name() const;

    /** 设置名称 */
    void name(const string& name);

    KData getTO() const { return m_kdata; }
    TradeManagerPtr getTM() const { return m_tm; }
    MoneyManagerPtr getMM() const { return m_mm; }
    EnvironmentPtr getEV() const { return m_ev; }
    ConditionPtr getCN() const { return m_cn; }
    SignalPtr getSG() const { return m_sg; }
    StoplossPtr getST() const { return m_st; }
    StoplossPtr getTP() const { return m_tp; }
    ProfitGoalPtr getPG() const { return m_pg; }
    SlippagePtr getSP() const { return m_sp; }

    void setTM(const TradeManagerPtr& tm) { m_tm = tm; }
    void setMM(const MoneyManagerPtr& mm) { m_mm = mm; }
    void setEV(const EnvironmentPtr& ev) { m_ev = ev; }
    void setCN(const ConditionPtr& cn) { m_cn = cn; }
    void setSG(const SignalPtr& sg) { m_sg = sg; }
    void setST(const StoplossPtr& st) { m_st = st; }
    void setTP(const StoplossPtr& tp) { m_tp = tp; }
    void setPG(const ProfitGoalPtr& pg) { m_pg = pg; }
    void setSP(const SlippagePtr& sp) { m_sp = sp; }

    Stock getStock() const { return m_stock; }
    void setStock(const Stock& stk) { m_stock = stk; }

    const TradeRecordList& getTradeRecordList() const { return m_trade_list;}

    const TradeRequest& getBuyTradeRequest() const { return m_buyRequest; }
    const TradeRequest& getSellTradeRequest() const { return m_sellRequest; }

    const TradeRequest& getSellShortTradeRequest() const {
        return m_sellShortRequest;
    }

    const TradeRequest& getBuyShortTradeRequest() const {
        return m_buyShortRequest;
    }


    /**
     * 复位
     * @param with_tm 是否复位TM组件
     * @param with_ev 是否复位EV组件
     * @note TM、EV都是和具体系统无关的策略组件，可以在不同的系统中进行共享，复位将引起系统
     * 运行时被重新清空并计算。尤其是在共享TM时需要注意！
     */
    void reset(bool with_tm, bool with_ev);

    typedef shared_ptr<System> SystemPtr;

    /** 克隆操作，会依次调用所有部件的clone操作
     * @details TM、EV都是和具体系统无关的策略组件，可以在不同的系统中进行共享。clone将
     * 生成新的独立实例，此时非共享状态。尤其需要注意TM是否共享的情况！
     * @param with_tm 是clone还是共享
     * @param with_ev 是clone还是共享
     * @note 不设默认值，强迫用户了解其间的区别，避免误用。
     */
    SystemPtr clone(bool with_tm, bool with_ev);

    /**
     * 设置交易对象
     * @note 其中tm, ev没有setTO接口
     */
    void setTO(const KData& kdata);

    //不指定stock的方式下run，需要事先通过setStock设定stock
    void run(const KQuery& query, bool reset=true);
    void run(const Stock& stock, const KQuery& query, bool reset=true);

    void runMoment(const Datetime& datetime);
    void runMoment(const KRecord& record);

    //清除已有的交易请求，供Portfolio使用
    void clearDelayRequest();

    //当前是否存在延迟的操作请求，供Portfolio
    bool haveDelayRequest() const;

    //运行前准备工作
    bool readyForRun();

    bool _environmentIsValid(const Datetime& datetime);

    bool _conditionIsValid(const Datetime& datetime);

    //通知所有需要接收实际买入交易记录的部件
    void _buyNotifyAll(const TradeRecord&);

    //通知所有需要接收实际卖出交易记录的部件
    void _sellNotifyAll(const TradeRecord&);

    size_t _getBuyNumber(const Datetime&, price_t price, price_t risk, Part from);
    size_t _getSellNumber(const Datetime&, price_t price, price_t risk, Part from);
    size_t _getSellShortNumber(const Datetime&, price_t price, price_t risk, Part from);
    size_t _getBuyShortNumber(const Datetime&, price_t price, price_t risk, Part from);

    price_t _getStoplossPrice(const Datetime& datetime, price_t price);
    price_t _getShortStoplossPrice(const Datetime& datetime, price_t price);

    price_t _getTakeProfitPrice(const Datetime& datetime);

    price_t _getGoalPrice(const Datetime& datetime, price_t price);
    price_t _getShortGoalPrice(const Datetime&, price_t price);

    price_t _getRealBuyPrice(const Datetime& datetime, price_t planPrice);
    price_t _getRealSellPrice(const Datetime& datetime, price_t planPrice);


    void _buy(const KRecord& today, Part from);
    void _buyNow(const KRecord& today, Part from);
    void _buyDelay(const KRecord& today);
    void _submitBuyRequest(const KRecord& today, Part from);

    void _sell(const KRecord& today, Part from);
    void _sellNow(const KRecord& today, Part from);
    void _sellDelay(const KRecord& today);
    void _submitSellRequest(const KRecord& today, Part from);

    void _sellShort(const KRecord& today, Part from);
    void _sellShortNow(const KRecord& today, Part from);
    void _sellShortDelay(const KRecord& today);
    void _submitSellShortRequest(const KRecord& today, Part from);

    void _buyShort(const KRecord& today, Part from);
    void _buyShortNow(const KRecord& today, Part from);
    void _buyShortDelay(const KRecord& today);
    void _submitBuyShortRequest(const KRecord& today, Part from);

    void _processRequest(const KRecord& today);

    void _runMoment(const KRecord& record);

protected:
    TradeManagerPtr m_tm;
    MoneyManagerPtr m_mm;
    EnvironmentPtr  m_ev;
    ConditionPtr    m_cn;
    SignalPtr       m_sg;
    StoplossPtr     m_st;
    StoplossPtr     m_tp;
    ProfitGoalPtr   m_pg;
    SlippagePtr     m_sp;

    string m_name;
    Stock m_stock;
    KData m_kdata;

    bool m_pre_ev_valid;
    bool m_pre_cn_valid;

    int m_buy_days; //每一次买入清零，计算一次加1，即买入后的天数
    int m_sell_short_days; //每一次卖空清零
    TradeRecordList m_trade_list; //保存实际执行的交易记录
    price_t m_lastTakeProfit;     //上一次多头止损价，用于保证止赢价单调递增
    price_t m_lastShortTakeProfit; //上一次空头止赢价

    TradeRequest m_buyRequest;
    TradeRequest m_sellRequest;
    TradeRequest m_sellShortRequest;
    TradeRequest m_buyShortRequest;

private:
    void initParam(); //初始化参数及其默认值

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        string name(GBToUTF8(m_name));
        ar & boost::serialization::make_nvp("m_name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);

        ar & BOOST_SERIALIZATION_NVP(m_tm);
        ar & BOOST_SERIALIZATION_NVP(m_ev);
        ar & BOOST_SERIALIZATION_NVP(m_cn);
        ar & BOOST_SERIALIZATION_NVP(m_mm);
        ar & BOOST_SERIALIZATION_NVP(m_sg);
        ar & BOOST_SERIALIZATION_NVP(m_st);
        ar & BOOST_SERIALIZATION_NVP(m_tp);
        ar & BOOST_SERIALIZATION_NVP(m_pg);
        ar & BOOST_SERIALIZATION_NVP(m_sp);

        //m_kdata中包含了stock和query的信息，不用保存m_stock
        ar & BOOST_SERIALIZATION_NVP(m_kdata);

        ar & BOOST_SERIALIZATION_NVP(m_pre_ev_valid);
        ar & BOOST_SERIALIZATION_NVP(m_pre_cn_valid);

        ar & BOOST_SERIALIZATION_NVP(m_buy_days);
        ar & BOOST_SERIALIZATION_NVP(m_sell_short_days);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
        ar & BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);

        ar & BOOST_SERIALIZATION_NVP(m_buyRequest);
        ar & BOOST_SERIALIZATION_NVP(m_sellRequest);
        ar & BOOST_SERIALIZATION_NVP(m_sellShortRequest);
        ar & BOOST_SERIALIZATION_NVP(m_buyShortRequest);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("m_name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);

        ar & BOOST_SERIALIZATION_NVP(m_tm);
        ar & BOOST_SERIALIZATION_NVP(m_ev);
        ar & BOOST_SERIALIZATION_NVP(m_cn);
        ar & BOOST_SERIALIZATION_NVP(m_mm);
        ar & BOOST_SERIALIZATION_NVP(m_sg);
        ar & BOOST_SERIALIZATION_NVP(m_st);
        ar & BOOST_SERIALIZATION_NVP(m_tp);
        ar & BOOST_SERIALIZATION_NVP(m_pg);
        ar & BOOST_SERIALIZATION_NVP(m_sp);

        //m_kdata中包含了stock和query的信息，不用保存m_stock
        ar & BOOST_SERIALIZATION_NVP(m_kdata);
        m_stock = m_kdata.getStock();

        ar & BOOST_SERIALIZATION_NVP(m_pre_ev_valid);
        ar & BOOST_SERIALIZATION_NVP(m_pre_cn_valid);

        ar & BOOST_SERIALIZATION_NVP(m_buy_days);
        ar & BOOST_SERIALIZATION_NVP(m_sell_short_days);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
        ar & BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);

        ar & BOOST_SERIALIZATION_NVP(m_buyRequest);
        ar & BOOST_SERIALIZATION_NVP(m_sellRequest);
        ar & BOOST_SERIALIZATION_NVP(m_sellShortRequest);
        ar & BOOST_SERIALIZATION_NVP(m_buyShortRequest);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/**
 * 客户程序应使用该指针进行操作
 * @ingroup System
 */
typedef shared_ptr<System> SystemPtr;
typedef shared_ptr<System> SYSPtr;
typedef vector<SystemPtr> SystemList;

HKU_API std::ostream& operator <<(std::ostream &os, const System& sys);
HKU_API std::ostream& operator <<(std::ostream &os, const SystemPtr& sys);


inline string System::name() const {
    return m_name;
}

inline void System::name(const string& name) {
    m_name = name;
}

inline bool System::
_environmentIsValid(const Datetime& datetime) {
    return m_ev ? m_ev->isValid(datetime) : true;
}

inline bool System::
_conditionIsValid(const Datetime& datetime) {
    return m_cn ? m_cn->isValid(datetime) : true;
}

inline size_t System
::_getBuyNumber(const Datetime& datetime, price_t price, price_t risk, Part from) {
    return m_mm ? m_mm->getBuyNumber(datetime, m_stock, price, risk, from) : 0;
}

inline size_t System
::_getSellNumber(const Datetime& datetime, price_t price, price_t risk, Part from) {
    return m_mm ? m_mm->getSellNumber(datetime, m_stock, price, risk, from) : 0;
}

inline size_t System
::_getSellShortNumber(const Datetime& datetime, price_t price, price_t risk, Part from) {
    return m_mm ? m_mm->getSellShortNumber(datetime, m_stock, price, risk, from) : 0;
}

inline size_t System
::_getBuyShortNumber(const Datetime& datetime, price_t price, price_t risk, Part from) {
    return m_mm ? m_mm->getBuyShortNumber(datetime, m_stock, price, risk, from) : 0;
}

inline price_t System
::_getRealBuyPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealBuyPrice(datetime, planPrice) : planPrice;
}

inline price_t System
::_getRealSellPrice(const Datetime& datetime, price_t planPrice) {
    return m_sp ? m_sp->getRealSellPrice(datetime, planPrice) : planPrice;
}

inline price_t System
::_getStoplossPrice(const Datetime& datetime, price_t price) {
    return m_st ? m_st->getPrice(datetime, price) : 0.0;
}

inline price_t System
::_getShortStoplossPrice(const Datetime& datetime, price_t price) {
    return m_st ? m_st->getShortPrice(datetime, price) : 0.0;
}

inline price_t System
::_getTakeProfitPrice(const Datetime& datetime) {
    return m_tp ? m_tp->getPrice(datetime, 0.0) : 0.0;
}

inline price_t System
::_getGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getGoal(datetime, price) : Null<price_t>();
}

inline price_t System
::_getShortGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getShortGoal(datetime, price) : 0.0;
}

} /* namespace hku */
#endif /* SYSTEMBASE_H_ */
