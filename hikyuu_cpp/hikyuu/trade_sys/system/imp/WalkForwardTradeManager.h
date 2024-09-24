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

    // 只允许执行>=run_start的实际交易
    WalkForwardTradeManager(const TMPtr& tm, const Datetime& run_start) : m_run_start(run_start) {
        HKU_ASSERT(tm);
        m_tm = tm;

        m_name = tm->name();
        m_costfunc = m_tm->costFunc();
    }

    virtual ~WalkForwardTradeManager() {}

    virtual void _reset() override {
        m_tm->_reset();
    }

    virtual shared_ptr<TradeManagerBase> _clone() override {
        WalkForwardTradeManager* p = new WalkForwardTradeManager();
        p->m_tm = m_tm->clone();
        p->m_run_start = m_run_start;
        return shared_ptr<TradeManagerBase>(p);
    }

    /**
     * 根据权息信息更新当前持仓与交易情况
     * @note 必须按时间顺序调用
     * @param datetime 当前时刻
     */
    virtual void updateWithWeight(const Datetime& datetime) override {
        m_tm->updateWithWeight(datetime);
    }

    /**
     * 获取指定对象的保证金比率
     * @param datetime 日期
     * @param stock 指定对象
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) override {
        return (datetime >= m_run_start) ? m_tm->getMarginRate(datetime, stock) : 0.0;
    }

    /** 初始资金 */
    virtual price_t initCash() const override {
        return m_tm->initCash();
    }

    /** 账户建立日期 */
    virtual Datetime initDatetime() const override {
        return m_tm->initDatetime();
    }

    /** 第一笔买入交易发生日期，如未发生交易返回Null<Datetime>() */
    virtual Datetime firstDatetime() const override {
        return m_tm->firstDatetime();
    }

    /** 最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期 */
    virtual Datetime lastDatetime() const override {
        return m_tm->lastDatetime();
    }

    /**
     * 返回当前现金
     * @note 仅返回当前信息，不会根据权息进行调整
     */
    virtual price_t currentCash() const override {
        return m_tm->currentCash();
    }

    /**
     * 获取指定日期的现金
     * @note 如果不带日期参数，无法根据权息信息调整持仓
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) override {
        return m_tm->cash(datetime, ktype);
    }

    /**
     * 当前是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    virtual bool have(const Stock& stock) const override {
        return m_tm->have(stock);
    }

    /**
     * 当前空头仓位是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    virtual bool haveShort(const Stock& stock) const override {
        return m_tm->haveShort(stock);
    }

    /** 当前持有的证券种类数量 */
    virtual size_t getStockNumber() const override {
        return m_tm->getStockNumber();
    }

    /** 当前空头持有的证券种类数量 */
    virtual size_t getShortStockNumber() const override {
        return m_tm->getShortStockNumber();
    }

    /** 获取指定时刻的某证券持有数量 */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getHoldNumber(datetime, stock);
    }

    /** 获取指定时刻的空头某证券持有数量 */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getShortHoldNumber(datetime, stock);
    }

    /** 获取指定时刻已借入的股票数量 */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) override {
        return m_tm->getDebtNumber(datetime, stock);
    }

    /** 获取指定时刻已借入的现金额 */
    virtual price_t getDebtCash(const Datetime& datetime) override {
        return m_tm->getDebtCash(datetime);
    }

    /** 获取全部交易记录 */
    virtual TradeRecordList getTradeList() const override {
        return m_tm->getTradeList();
    }

    /**
     * 获取指定日期范围内的交易记录[start, end)
     * @param start 起始日期
     * @param end 结束日期
     * @return 交易记录列表
     */
    virtual TradeRecordList getTradeList(const Datetime& start,
                                         const Datetime& end) const override {
        return m_tm->getTradeList(start, end);
    }

    /** 获取当前全部持仓记录 */
    virtual PositionRecordList getPositionList() const override {
        return m_tm->getPositionList();
    }

    /** 获取全部历史持仓记录，即已平仓记录 */
    virtual PositionRecordList getHistoryPositionList() const override {
        return m_tm->getHistoryPositionList();
    }

    /** 获取当前全部空头仓位记录 */
    virtual PositionRecordList getShortPositionList() const override {
        return m_tm->getShortPositionList();
    }

    /** 获取全部空头历史仓位记录 */
    virtual PositionRecordList getShortHistoryPositionList() const override {
        return m_tm->getShortHistoryPositionList();
    }

    /**
     * 获取指定证券的持仓记录
     * @param date 指定日期
     * @param stock 指定的证券
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) override {
        return m_tm->getPosition(date, stock);
    }

    /**
     * 获取指定证券的空头持仓记录
     * @param stock 指定的证券
     */
    virtual PositionRecord getShortPosition(const Stock& stock) const override {
        return m_tm->getShortPosition(stock);
    }

    /** 获取当前借入的股票列表 */
    virtual BorrowRecordList getBorrowStockList() const override {
        return m_tm->getBorrowStockList();
    }

    /**
     * 存入资金
     * @param datetime 存入时间
     * @param cash 存入的资金量
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->checkin(datetime, cash) : false;
    }

    /**
     * 取出资金
     * @param datetime 取出时间
     * @param cash 取出的资金量
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->checkout(datetime, cash) : false;
    }

    /**
     * 存入资产
     * @param datetime 存入日期
     * @param stock 待存入的股票
     * @param price 存入股票的每股价格
     * @param number 存入股票的数量
     * @return true | false
     */
    virtual bool checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                              double number) override {
        return (datetime >= m_run_start) ? m_tm->checkinStock(datetime, stock, price, number)
                                         : false;
    }

    /**
     * 取出当前资产
     * @param datetime 取出日期
     * @param stock 待取出的股票
     * @param price 取出的每股价格
     * @param number 取出的数量
     * @return true | false
     * @note 应该不会被用到
     */
    virtual bool checkoutStock(const Datetime& datetime, const Stock& stock, price_t price,
                               double number) override {
        return (datetime >= m_run_start) ? m_tm->checkoutStock(datetime, stock, price, number)
                                         : false;
    }

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
                            price_t planPrice = 0.0, SystemPart from = PART_INVALID) override {
        return (datetime >= m_run_start) ? m_tm->buy(datetime, stock, realPrice, number, stoploss,
                                                     goalPrice, planPrice, from)
                                         : TradeRecord();
    }

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
                             SystemPart from = PART_INVALID) override {
        return (datetime >= m_run_start) ? m_tm->sell(datetime, stock, realPrice, number, stoploss,
                                                      goalPrice, planPrice, from)
                                         : TradeRecord();
    }

    /**
     * 卖空
     * @param datetime 卖空时间
     * @param stock 卖空的证券
     * @param realPrice 实际卖空价格
     * @param number 卖出数量
     * @param stoploss 止损价
     * @param goalPrice 目标价格
     * @param planPrice 计划卖空价格
     * @param from 记录是哪个系统部件发出的买入指示
     * @return 返回对应的交易记录，如果操作失败，business等于BUSINESS_INVALID
     */
    virtual TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                  double number, price_t stoploss = 0.0, price_t goalPrice = 0.0,
                                  price_t planPrice = 0.0,
                                  SystemPart from = PART_INVALID) override {
        return (datetime >= m_run_start) ? m_tm->sellShort(datetime, stock, realPrice, number,
                                                           stoploss, goalPrice, planPrice, from)
                                         : TradeRecord();
    }

    /**
     * 卖空后回补
     * @param datetime 买入时间
     * @param stock 买入的证券
     * @param realPrice 实际买入价格
     * @param number 卖出数量，如果是 MAX_DOUBLE, 表示全部卖出
     * @param stoploss 止损价
     * @param goalPrice 目标价格
     * @param planPrice 计划买入价格
     * @param from 记录是哪个系统部件发出的卖出指示
     * @return 返回对应的交易记录，如果操作失败，business等于BUSINESS_INVALID
     */
    virtual TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                                 double number = MAX_DOUBLE, price_t stoploss = 0.0,
                                 price_t goalPrice = 0.0, price_t planPrice = 0.0,
                                 SystemPart from = PART_INVALID) override {
        return (datetime >= m_run_start) ? m_tm->buyShort(datetime, stock, realPrice, number,
                                                          stoploss, goalPrice, planPrice, from)
                                         : TradeRecord();
    }

    /**
     * 借入资金，从其他来源借取的资金，如融资
     * @param datetime 借入时间
     * @param cash 借入的现金
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->borrowCash(datetime, cash) : false;
    }

    /**
     * 归还资金
     * @param datetime 归还日期
     * @param cash 归还现金
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) override {
        return (datetime >= m_run_start) ? m_tm->returnCash(datetime, cash) : false;
    }

    /**
     * 借入证券
     * @param datetime 借入时间
     * @param stock 借入的stock
     * @param price 借入时单股价格
     * @param number 借入时数量
     * @return true | false
     */
    virtual bool borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override {
        return (datetime >= m_run_start) ? m_tm->borrowStock(datetime, stock, price, number)
                                         : false;
    }

    /**
     * 归还证券
     * @param datetime 归还时间
     * @param stock 归还的stock
     * @param price 归还时单股价格
     * @param number 归还数量
     * @return true | false
     */
    virtual bool returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                             double number) override {
        return (datetime >= m_run_start) ? m_tm->returnStock(datetime, stock, price, number)
                                         : false;
    }

    /**
     * 获取账户当前时刻的资产详情
     * @param ktype 日期的类型
     * @return 资产详情
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const override {
        return m_tm->getFunds(ktype);
    }

    /**
     * 获取指定时刻的资产市值详情
     * @param datetime 必须大于帐户建立的初始日期，或为Null<Datetime>()
     * @param ktype 日期的类型
     * @return 资产详情
     * @note 当datetime等于Null<Datetime>()时，与getFunds(KType)同
     */
    virtual FundsRecord getFunds(const Datetime& datetime,
                                 KQuery::KType ktype = KQuery::DAY) override {
        return m_tm->getFunds(datetime, ktype);
    }

    /**
     * 直接加入交易记录
     * @note 如果加入初始化账户记录，将清除全部已有交易及持仓记录
     * @param tr 待加入的交易记录
     * @return bool true 成功 | false 失败
     */
    virtual bool addTradeRecord(const TradeRecord& tr) override {
        return (tr.datetime >= m_run_start) ? m_tm->addTradeRecord(tr) : false;
    }

    /**
     * 直接加入持仓记录
     * @param pr 持仓记录
     * @return true 成功
     * @return false 失败
     */
    virtual bool addPosition(const PositionRecord& pr) override {
        return m_tm->addPosition(pr);
    }

    /** 字符串输出 */
    virtual string str() const override {
        return m_tm->str();
    }

    /**
     * 以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
     * @param path 输出文件所在目录
     */
    virtual void tocsv(const string& path) override {
        m_tm->tocsv(path);
    }

    /**
     * 从订单代理实例同步当前账户资产信息（包含资金、持仓等）
     * @param broker 订单代理实例
     */
    virtual void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker) override {
        m_tm->fetchAssetInfoFromBroker(broker);
    }

private:
    TMPtr m_tm;
    Datetime m_run_start;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(TradeManagerBase);
        ar& BOOST_SERIALIZATION_NVP(m_tm);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

inline TMPtr crtWalkForwardTM(const TMPtr& tm, const Datetime& run_start) {
    return std::make_shared<WalkForwardTradeManager>(tm, run_start);
}

}  // namespace hku
