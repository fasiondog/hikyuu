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
            const StoplossPtr& sl,
            const StoplossPtr& tp,
            const ProfitGoalPtr& pg,
            const SlippagePtr& sp,
            const string& name);
    virtual ~System();

    typedef SystemPart Part;

    /** 获取名称 */
    const string& name() const;

    TradeManagerPtr getTM() const { return m_tm; }
    MoneyManagerPtr getMM() const { return m_mm; }
    EnvironmentPtr getEV() const { return m_ev; }
    ConditionPtr getCN() const { return m_cn; }
    SignalPtr getSG() const { return m_sg; }
    StoplossPtr getSL() const { return m_sl; }
    StoplossPtr getTP() const { return m_tp; }
    ProfitGoalPtr getPG() const { return m_pg; }
    SlippagePtr getSP() const { return m_sp; }

    void setTM(const TradeManagerPtr& tm) { m_tm = tm; }
    void setMM(const MoneyManagerPtr& mm) { m_mm = mm; }
    void setEV(const EnvironmentPtr& ev) { m_ev = ev; }
    void setCN(const ConditionPtr& cn) { m_cn = cn; }
    void setSG(const SignalPtr& sg) { m_sg = sg; }
    void setSL(const StoplossPtr& sl) { m_sl = sl; }
    void setTP(const StoplossPtr& tp) { m_tp = tp; }
    void setPG(const ProfitGoalPtr& pg) { m_pg = pg; }
    void setSP(const SlippagePtr& sp) { m_sp = sp; }

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
     * @note 不包含TradeManager的复位
     */
    void reset();

    /** 克隆操作，会依次调用所有部件的clone操作 */
    shared_ptr<System> clone();

    /**
     * 设置交易对象
     * @note 其中tm, ev没有setTO接口
     */
    void setTO(const KData& kdata);

    /**
     * 设置时刻类型
     */
    void setKType(KQuery::KType ktype);

    void run(const Stock& stock, const KQuery& query);
    void runMoment(const Datetime& datetime);
    void runMoment(const KRecord& record);

    //清除已有的交易请求，供Portolio使用
    void clearRequest();

    bool _environmentIsValid(const string& market, const Datetime& datetime);

    bool _conditionIsValid(const Datetime& datetime);

    bool _shouldBuy(const Datetime& datetime);

    //通知所有需要接收实际买入交易记录的部件
    void _buyNotifyAll(const TradeRecord&);

    //通知所有需要接收实际卖出交易记录的部件
    void _sellNotifyAll(const TradeRecord&);

    size_t _getBuyNumber(const Datetime&, price_t price, price_t risk);
    size_t _getSellNumber(const Datetime&, price_t price, price_t risk);
    size_t _getSellShortNumber(const Datetime&, price_t price, price_t risk);
    size_t _getBuyShortNumber(const Datetime&, price_t price, price_t risk);

    price_t _getStoplossPrice(const Datetime& datetime, price_t price);
    price_t _getShortStoplossPrice(const Datetime& datetime, price_t price);

    price_t _getTakeProfitPrice(const Datetime& datetime);

    price_t _getGoalPrice(const Datetime& datetime, price_t price);
    price_t _getShortGoalPrice(const Datetime&, price_t price);

    price_t _getRealBuyPrice(const Datetime& datetime, price_t planPrice);
    price_t _getRealSellPrice(const Datetime& datetime, price_t planPrice);


    void _buy(const KRecord& today);
    void _buyNow(const KRecord& today);
    void _buyDelay(const KRecord& today);
    void _submitBuyRequest(const KRecord& today);

    void _sell(const KRecord& today, Part from);
    void _sellNow(const KRecord& today, Part from);
    void _sellDelay(const KRecord& today);
    void _submitSellRequest(const KRecord& today, Part from);

    void _sellShort(const KRecord& today);
    void _sellShortNow(const KRecord& today);
    void _sellShortDelay(const KRecord& today);
    void _submitSellShortRequest(const KRecord& today);

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
    StoplossPtr     m_sl;
    StoplossPtr     m_tp;
    ProfitGoalPtr   m_pg;
    SlippagePtr     m_sp;

    string m_name;
    Stock m_stock;
    KData m_kdata;

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
        ar & BOOST_SERIALIZATION_NVP(m_sl);
        ar & BOOST_SERIALIZATION_NVP(m_tp);
        ar & BOOST_SERIALIZATION_NVP(m_pg);
        ar & BOOST_SERIALIZATION_NVP(m_sp);

        //m_kdata中包含了stock和query的信息，不用保存m_stock
        ar & BOOST_SERIALIZATION_NVP(m_kdata);

        ar & BOOST_SERIALIZATION_NVP(m_buy_days);
        ar & BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
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
        ar & BOOST_SERIALIZATION_NVP(m_sl);
        ar & BOOST_SERIALIZATION_NVP(m_tp);
        ar & BOOST_SERIALIZATION_NVP(m_pg);
        ar & BOOST_SERIALIZATION_NVP(m_sp);

        //m_kdata中包含了stock和query的信息，不用保存m_stock
        ar & BOOST_SERIALIZATION_NVP(m_kdata);
        m_stock = m_kdata.getStock();

        ar & BOOST_SERIALIZATION_NVP(m_buy_days);
        ar & BOOST_SERIALIZATION_NVP(m_lastTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_lastShortTakeProfit);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/**
 * 客户程序应使用该指针进行操作
 * @ingroup System
 */
typedef shared_ptr<System> SystemPtr;
typedef vector<SystemPtr> SystemList;

HKU_API std::ostream& operator <<(std::ostream &os, const System& sys);
HKU_API std::ostream& operator <<(std::ostream &os, const SystemPtr& sys);


inline const string& System::name() const {
    return m_name;
}

inline bool System::
_environmentIsValid(const string& market, const Datetime& datetime) {
    return m_ev ? m_ev->isValid(market, datetime) : true;
}

inline bool System::
_conditionIsValid(const Datetime& datetime) {
    return m_cn ? m_cn->isValid(datetime) : true;
}

inline bool System::
_shouldBuy(const Datetime& datetime) {
    return m_sg ? m_sg->shouldBuy(datetime) : false;
}

inline size_t System
::_getBuyNumber(const Datetime& datetime, price_t price, price_t risk) {
    return m_mm ? m_mm->getBuyNumber(datetime, m_stock, price, risk) : 0;
}

inline size_t System
::_getSellNumber(const Datetime& datetime, price_t price, price_t risk) {
    return m_mm ? m_mm->getSellNumber(datetime, m_stock, price, risk) : 0;
}

inline size_t System
::_getSellShortNumber(const Datetime& datetime, price_t price, price_t risk) {
    return m_mm ? m_mm->getSellShortNumber(datetime, m_stock, price, risk) : 0;
}

inline size_t System
::_getBuyShortNumber(const Datetime& datetime, price_t price, price_t risk) {
    return m_mm ? m_mm->getBuyShortNumber(datetime, m_stock, price, risk) : 0;
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
    return m_sl ? m_sl->getPrice(datetime, price) : 0.0;
}

inline price_t System
::_getShortStoplossPrice(const Datetime& datetime, price_t price) {
    return m_sl ? m_sl->getShortPrice(datetime, price) : 0.0;
}

inline price_t System
::_getTakeProfitPrice(const Datetime& datetime) {
    return m_tp ? m_tp->getPrice(datetime, 0.0) : 0.0;
}

inline price_t System
::_getGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getGoal(datetime, price) : 0.0;
}

inline price_t System
::_getShortGoalPrice(const Datetime& datetime, price_t price) {
    return m_pg ? m_pg->getShortGoal(datetime, price) : 0.0;
}

} /* namespace hku */
#endif /* SYSTEMBASE_H_ */
