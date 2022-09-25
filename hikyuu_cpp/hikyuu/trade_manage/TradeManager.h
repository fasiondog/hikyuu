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
#include "../utilities/util.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "FundsRecord.h"
#include "ContractRecord.h"
#include "OrderBrokerBase.h"
#include "crt/TC_Zero.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#endif

namespace hku {

/**
 * 回测模拟账户交易管理模块，管理帐户的交易记录及资金使用情况
 * @details
 * <pre>
 * 默认参数：
 * precision(int): 2 计算精度
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManager : public TradeManagerBase {
    // PARAMETER_SUPPORT

public:
    TradeManager(const Datetime& datetime = Datetime(199001010000LL), price_t initcash = 100000.0,
                 const TradeCostPtr& costfunc = TC_Zero(),
                 const MarginRatioPtr& mrfunc = MarginRatioPtr(), const string& name = "SYS");
    virtual ~TradeManager();

    /** 复位，清空交易、持仓记录 */
    virtual void _reset() override;

    virtual shared_ptr<TradeManagerBase> _clone() override;

    /** 初始资金 */
    virtual price_t initCash() const override {
        return m_init_cash;
    }

    /** 账户建立日期 */
    virtual Datetime initDatetime() const override {
        return m_init_datetime;
    }

    /** 第一笔买入交易发生日期，如未发生交易返回Null<Datetime>() */
    virtual Datetime firstDatetime() const override;

    /** 最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期 */
    virtual Datetime lastDatetime() const override {
        return m_trade_list.empty() ? m_init_datetime : m_trade_list.back().datetime;
    }

    /**
     * 根据权息信息更新当前持仓与交易情况
     * @note 必须按时间顺序调用
     * @param datetime 当前时刻
     */
    virtual void updateWithWeight(const Datetime& datetime) override;

    /**
     * 返回当前现金
     * @note 仅返回当前信息，不会根据权息进行调整
     */
    virtual price_t currentCash() const override {
        return m_cash;
    }

    /**
     * 获取指定日期的现金
     * @note 如果不带日期参数，无法根据权息信息调整持仓
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override;

    /**
     * 当前是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    virtual bool have(const Stock& stock) const override {
        return m_position.count(stock.id()) ? true : false;
    }

    /** 当前持有的证券种类数量 */
    virtual size_t getStockNumber() const override {
        return m_position.size();
    }

    /** 获取指定时刻的某证券持有数量 */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) override;

    /** 获取全部交易记录 */
    virtual TradeRecordList getTradeList() const override {
        return m_trade_list;
    }

    /**
     * 获取指定日期范围内的交易记录[start, end)
     * @param start 起始日期
     * @param end 结束日期
     * @return 交易记录列表
     */
    virtual TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const override;

    /** 获取当前全部持仓记录 */
    virtual PositionRecordList getPositionList() const override;

    /** 获取全部历史持仓记录，即已平仓记录 */
    virtual PositionRecordList getHistoryPositionList() const override {
        return m_position_history;
    }

    /**
     * 获取指定证券的持仓记录
     * @param date 指定日期
     * @param stock 指定的证券
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) override;

    /**
     * 存入资金
     * @param datetime 存入时间
     * @param cash 存入的资金量
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) override;

    /**
     * 取出资金
     * @param datetime 取出时间
     * @param cash 取出的资金量
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) override;

    /**
     * 买入操作
     * @param datetime 买入时间
     * @param stock 买入的证券
     * @param realPrice 实际买入价格
     * @param number 买入数量
     * @param stoploss 止损价
     * @param goalPrice 目标价格
     * @param planPrice 计划买入价格
     * @param from 记录是哪个系统部件发出的买入指示
     * @return 返回对应的交易记录，如果操作失败，business等于BUSINESS_INVALID
     */
    virtual TradeRecord buy(const Datetime& datetime, const Stock& stock, price_t realPrice,
                            double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                            price_t planPrice = 0.0, SystemPart from = PART_INVALID) override;

    /**
     * 卖出操作
     * @param datetime 卖出时间
     * @param stock 卖出的证券
     * @param realPrice 实际卖出价格
     * @param number 卖出数量，如果是 MAX_DOUBLE, 表示全部卖出
     * @param stoploss 新的止损价
     * @param goalPrice 新的目标价格
     * @param planPrice 原计划卖出价格
     * @param from 记录是哪个系统部件发出的卖出指示
     * @return 返回对应的交易记录，如果操作失败，business等于BUSINESS_INVALID
     */
    virtual TradeRecord sell(const Datetime& datetime, const Stock& stock, price_t realPrice,
                             double number = MAX_DOUBLE, price_t stoploss = 0.0,
                             price_t goalPrice = 0.0, price_t planPrice = 0.0,
                             SystemPart from = PART_INVALID) override;

    /**
     * 获取账户当前时刻的资产详情
     * @param ktype 日期的类型
     * @return 资产详情
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const override;

    /**
     * 获取指定时刻的资产市值详情
     * @param datetime 必须大于帐户建立的初始日期，或为Null<Datetime>()
     * @param ktype 日期的类型
     * @return 资产详情
     * @note 当datetime等于Null<Datetime>()时，与getFunds(KType)同
     */
    virtual FundsRecord getFunds(const Datetime& datetime,
                                 KQuery::KType ktype = KQuery::DAY) override;

    /**
     * 获取净资产净值曲线，不含借入的资产
     * @param dates 日期列表，根据该日期列表获取其对应的资产净值曲线
     * @param ktype K线类型，必须与日期列表匹配，默认KQuery::DAY
     * @return 资产净值列表
     */
    virtual PriceList getFundsCurve(const DatetimeList& dates,
                                    KQuery::KType ktype = KQuery::DAY) override;

    /**
     * 获取收益曲线，即扣除历次存入资金后的资产净值曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益曲线
     */
    virtual PriceList getProfitCurve(const DatetimeList& dates,
                                     KQuery::KType ktype = KQuery::DAY) override;

    /**
     * 直接加入交易记录
     * @note 如果加入初始化账户记录，将清除全部已有交易及持仓记录
     * @param tr 待加入的交易记录
     * @return bool true 成功 | false 失败
     */
    virtual bool addTradeRecord(const TradeRecord& tr) override;

    /** 字符串输出 */
    virtual string str() const override;

    /**
     * 以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
     * @param path 输出文件所在目录
     */
    virtual void tocsv(const string& path) override;

private:
    //以脚本的形式保存交易动作，便于修正和校准
    void _saveAction(const TradeRecord&);

    // 更新股票权息信息
    void _updateWithWeight(const Datetime& datetime);

    // 合约类交易结算，更新当前时刻前一结算日权益
    void _updateSettleByDay(const Datetime& datetime);

    FundsRecord _getFundsByContract(const Datetime& datetime, KQuery::KType ktype);

    bool _add_init_tr(const TradeRecord&);
    bool _add_buy_tr(const TradeRecord&);
    bool _add_sell_tr(const TradeRecord&);
    bool _add_checkin_tr(const TradeRecord&);
    bool _add_checkout_tr(const TradeRecord&);
    bool _add_checkin_stock_tr(const TradeRecord&);
    bool _add_checkout_stock_tr(const TradeRecord&);

private:
    Datetime m_init_datetime;         // 账户建立日期
    price_t m_init_cash;              // 初始资金
    Datetime m_last_update_datetime;  // 最后一次根据权息调整持仓与交易记录的时刻

    price_t m_cash;            //当前现金
    price_t m_checkin_cash;    //累计存入自有资金，初始资金视为存入
    price_t m_checkout_cash;   //累计取出自有资金
    price_t m_checkin_stock;   //累计存入股票价值
    price_t m_checkout_stock;  //累计取出股票价值

    TradeRecordList m_trade_list;  //交易记录

    typedef map<uint64_t, PositionRecord> position_map_type;
    position_map_type m_position;  //当前持仓交易对象的持仓记录 ["sh000001"-> ]
    PositionRecordList m_position_history;  //持仓历史记录

    list<string> m_actions;  //记录交易动作，便于修改或校准实盘时的交易

//==================================================
// 支持序列化
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
        namespace bs = boost::serialization;
        PositionRecordList position = getPositionList();
        ar& bs::make_nvp<PositionRecordList>("m_position", position);
        ar& BOOST_SERIALIZATION_NVP(m_position_history);
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
        namespace bs = boost::serialization;
        PositionRecordList position;
        ar& bs::make_nvp<PositionRecordList>("m_position", position);
        PositionRecordList::const_iterator iter = position.begin();
        for (; iter != position.end(); ++iter) {
            m_position[iter->stock.id()] = *iter;
        }
        ar& BOOST_SERIALIZATION_NVP(m_position_history);
        ar& BOOST_SERIALIZATION_NVP(m_trade_list);
        ar& BOOST_SERIALIZATION_NVP(m_actions);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */
#endif /* TRADEMANAGER_H_ */
