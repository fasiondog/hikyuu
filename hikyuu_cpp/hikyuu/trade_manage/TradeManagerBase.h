/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#pragma once

#include "../utilities/Parameter.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "TradeCostBase.h"
#include "OrderBrokerBase.h"
#include "crt/TC_Zero.h"

namespace hku {

/**
 * 账户交易管理基类，管理帐户的交易记录及资金使用情况
 * @details
 * <pre>
 * 默认参数：
 * precision(int): 2 计算精度
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManagerBase {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    TradeManagerBase() : TradeManagerBase("", TC_Zero()) {}

    TradeManagerBase(const string& name, const TradeCostPtr& costFunc)
    : m_name(name), m_costfunc(costFunc), m_broker_last_datetime(Datetime::now()) {
        setParam<int>("precision", 2);  // 计算精度
    }

    virtual ~TradeManagerBase() {}

    /** 账户名称 */
    const string& name() const {
        return m_name;
    }

    /** 设置账户名称 */
    void name(const string& name) {
        m_name = name;
    }

    /** 交易精度 */
    int precision() const {
        return getParam<int>("precision");
    }

    /** 获取交易成本算法指针 */
    TradeCostPtr costFunc() const {
        return m_costfunc;
    }

    /** 设置交易成本算法指针 */
    void costFunc(const TradeCostPtr& func) {
        m_costfunc = func;
    }

    /**
     * 计算买入成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 买入价格
     * @param num 买入数量
     * @return CostRecord 交易成本记录
     */
    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                          double num) const {
        return m_costfunc ? m_costfunc->getBuyCost(datetime, stock, price, num) : CostRecord();
    }

    /**
     * 计算卖出成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 卖出价格
     * @param num 卖出数量
     * @return CostRecord 交易成本记录
     */
    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const {
        return m_costfunc ? m_costfunc->getSellCost(datetime, stock, price, num) : CostRecord();
    }

    /**
     * 计算计入现金时的费用成本
     * @param datetime 借入日期
     * @param cash 现金额
     */
    CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) {
        return m_costfunc ? m_costfunc->getBorrowCashCost(datetime, cash) : CostRecord();
    }

    /**
     * 计算归还融资成本
     * @param borrow_datetime 借入日期
     * @param return_datetime 归还日期
     * @param cash 归还金额
     */
    CostRecord getReturnCashCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                 price_t cash) {
        return m_costfunc ? m_costfunc->getReturnCashCost(borrow_datetime, return_datetime, cash)
                          : CostRecord();
    }

    /**
     * 计算融劵借入成本
     * @param datetime 融劵日期
     * @param stock 借入的对象
     * @param price 每股价格
     * @param num 借入的数量
     */
    CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) {
        return m_costfunc ? m_costfunc->getBorrowStockCost(datetime, stock, price, num)
                          : CostRecord();
    }

    /**
     * 计算融劵归还成本
     * @param borrow_datetime 借入日期
     * @param return_datetime 归还日期
     * @param stock 归还的对象
     * @param price 归还时每股价格
     * @param num 归还的数量
     */
    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) {
        return m_costfunc ? m_costfunc->getReturnStockCost(borrow_datetime, return_datetime, stock,
                                                           price, num)
                          : CostRecord();
    }

    /** 从哪个时刻开始启动订单代理进行下单操作   */
    Datetime getBrokerLastDatetime() const {
        return m_broker_last_datetime;
    }

    /** 设置开始订单代理操作的时刻 */
    void setBrokerLastDatetime(const Datetime& date) {
        m_broker_last_datetime = date;
    }

    /** 复位，清空交易、持仓记录 */
    void reset() {
        _reset();
    }

    virtual void _reset() {
        HKU_WARN("The subclass does not implement a reset method");
    }

    /** 执行 clone 操作 */
    shared_ptr<TradeManagerBase> clone() {
        shared_ptr<TradeManagerBase> p = _clone();
        HKU_CHECK(p, "Invalid ptr from _clone!");
        p->m_params = m_params;
        p->m_name = m_name;
        p->m_broker_last_datetime = m_broker_last_datetime;
        p->m_costfunc = m_costfunc;
        return p;
    }

    virtual shared_ptr<TradeManagerBase> _clone() {
        HKU_WARN("The subclass does not implement a reset method");
        return shared_ptr<TradeManagerBase>();
    }

    /**
     * 注册订单代理
     * @param broker 订单代理实例
     */
    void regBroker(const OrderBrokerPtr& broker) {
        m_broker_list.push_back(broker);
    }

    /**
     * 清空已注册的订单代理
     */
    void clearBroker() {
        m_broker_list.clear();
    }

    /**
     * 获取指定日期列表中的所有日资产记录
     * @param dates 日期列表
     * @param ktype K线类型，必须与日期列表匹配，默认KQuery::DAY
     * @return 日资产记录列表
     */
    FundsList getFundsList(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) {
        size_t total = dates.size();
        FundsList result(total);
        HKU_IF_RETURN(total == 0, result);
        for (size_t i = 0; i < total; ++i) {
            result[i] = getFunds(dates[i], ktype);
        }
        return result;
    }

    /**
     * 获取资产净值曲线，含借入的资产
     * @param dates 日期列表，根据该日期列表获取其对应的资产净值曲线
     * @param ktype K线类型，必须与日期列表匹配，默认KQuery::DAY
     * @return 资产净值列表
     */
    PriceList getFundsCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        int precision = getParam<int>("precision");
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = roundEx(funds_list[i].total_assets(), precision);
        }
        return ret;
    }

    /**
     * 获取收益曲线，即扣除历次存入资金后的资产净值曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益曲线
     */
    PriceList getProfitCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        int precision = getParam<int>("precision");
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = roundEx(funds_list[i].profit(), precision);
        }
        return ret;
    }

    /**
     * 获取累积收益率曲线
     * @param dates 日期列表
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益率曲线
     */
    PriceList getProfitCumChangeCurve(const DatetimeList& dates,
                                      KQuery::KType ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = funds_list[i].total_assets() / funds_list[i].total_base();
        }
        return ret;
    }

    /**
     * 获取投入本值资产曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 价格曲线
     */
    PriceList getBaseAssetsCurve(const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) {
        FundsList funds_list = getFundsList(dates, ktype);
        PriceList ret(funds_list.size());
        for (size_t i = 0, total = funds_list.size(); i < total; i++) {
            ret[i] = funds_list[i].total_base();
        }
        return ret;
    }

    /**
     * 根据权息信息更新当前持仓与交易情况
     * @note 必须按时间顺序调用
     * @param datetime 当前时刻
     */
    virtual void updateWithWeight(const Datetime& datetime) {
        HKU_WARN("The subclass does not implement a updateWithWeight method");
    }

    /**
     * 获取指定对象的保证金比率
     * @param datetime 日期
     * @param stock 指定对象
     */
    virtual double getMarginRate(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement a getMarginRate method");
        return 0.0;
    }

    /** 初始资金 */
    virtual price_t initCash() const {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 账户建立日期 */
    virtual Datetime initDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** 第一笔买入交易发生日期，如未发生交易返回Null<Datetime>() */
    virtual Datetime firstDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /** 最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期 */
    virtual Datetime lastDatetime() const {
        HKU_WARN("The subclass does not implement this method");
        return Datetime();
    }

    /**
     * 返回当前现金
     * @note 仅返回当前信息，不会根据权息进行调整
     */
    virtual price_t currentCash() const {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /**
     * 获取指定日期的现金
     * @note 如果不带日期参数，无法根据权息信息调整持仓
     */
    virtual price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /**
     * 当前是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    virtual bool have(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * 当前空头仓位是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    virtual bool haveShort(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** 当前持有的证券种类数量 */
    virtual size_t getStockNumber() const {
        HKU_WARN("The subclass does not implement this method");
        return 0;
    }

    /** 当前空头持有的证券种类数量 */
    virtual size_t getShortStockNumber() const {
        HKU_WARN("The subclass does not implement this method");
        return 0;
    }

    /** 获取指定时刻的某证券持有数量 */
    virtual double getHoldNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 获取指定时刻的空头某证券持有数量 */
    virtual double getShortHoldNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 获取指定时刻已借入的股票数量 */
    virtual double getDebtNumber(const Datetime& datetime, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 获取指定时刻已借入的现金额 */
    virtual price_t getDebtCash(const Datetime& datetime) {
        HKU_WARN("The subclass does not implement this method");
        return 0.0;
    }

    /** 获取全部交易记录 */
    virtual TradeRecordList getTradeList() const {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /**
     * 获取指定日期范围内的交易记录[start, end)
     * @param start 起始日期
     * @param end 结束日期
     * @return 交易记录列表
     */
    virtual TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecordList();
    }

    /** 获取当前全部持仓记录 */
    virtual PositionRecordList getPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** 获取全部历史持仓记录，即已平仓记录 */
    virtual PositionRecordList getHistoryPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** 获取当前全部空头仓位记录 */
    virtual PositionRecordList getShortPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /** 获取全部空头历史仓位记录 */
    virtual PositionRecordList getShortHistoryPositionList() const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecordList();
    }

    /**
     * 获取指定证券的持仓记录
     * @param date 指定日期
     * @param stock 指定的证券
     */
    virtual PositionRecord getPosition(const Datetime& date, const Stock& stock) {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecord();
    }

    /**
     * 获取指定证券的空头持仓记录
     * @param stock 指定的证券
     */
    virtual PositionRecord getShortPosition(const Stock& stock) const {
        HKU_WARN("The subclass does not implement this method");
        return PositionRecord();
    }

    /** 获取当前借入的股票列表 */
    virtual BorrowRecordList getBorrowStockList() const {
        HKU_WARN("The subclass does not implement this method");
        return BorrowRecordList();
    }

    /**
     * 存入资金
     * @param datetime 存入时间
     * @param cash 存入的资金量
     * @return true | false
     */
    virtual bool checkin(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * 取出资金
     * @param datetime 取出时间
     * @param cash 取出的资金量
     * @return true | false
     */
    virtual bool checkout(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                              double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                               double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                            price_t planPrice = 0.0, SystemPart from = PART_INVALID) {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
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
                             SystemPart from = PART_INVALID) {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
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
                                  price_t planPrice = 0.0, SystemPart from = PART_INVALID) {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
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
                                 SystemPart from = PART_INVALID) {
        HKU_WARN("The subclass does not implement this method");
        return TradeRecord();
    }

    /**
     * 借入资金，从其他来源借取的资金，如融资
     * @param datetime 借入时间
     * @param cash 借入的现金
     * @return true | false
     */
    virtual bool borrowCash(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * 归还资金
     * @param datetime 归还日期
     * @param cash 归还现金
     * @return true | false
     */
    virtual bool returnCash(const Datetime& datetime, price_t cash) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                             double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
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
                             double number) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * 获取账户当前时刻的资产详情
     * @param ktype 日期的类型
     * @return 资产详情
     */
    virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const {
        HKU_WARN("The subclass does not implement this method");
        return FundsRecord();
    }

    /**
     * 获取指定时刻的资产市值详情
     * @param datetime 必须大于帐户建立的初始日期，或为Null<Datetime>()
     * @param ktype 日期的类型
     * @return 资产详情
     * @note 当datetime等于Null<Datetime>()时，与getFunds(KType)同
     */
    virtual FundsRecord getFunds(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY) {
        HKU_WARN("The subclass does not implement this method");
        return FundsRecord();
    }

    /**
     * 直接加入交易记录
     * @note 如果加入初始化账户记录，将清除全部已有交易及持仓记录
     * @param tr 待加入的交易记录
     * @return bool true 成功 | false 失败
     */
    virtual bool addTradeRecord(const TradeRecord& tr) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /**
     * 直接加入持仓记录
     * @param pr 持仓记录
     * @return true 成功
     * @return false 失败
     */
    virtual bool addPosition(const PositionRecord& pr) {
        HKU_WARN("The subclass does not implement this method");
        return false;
    }

    /** 字符串输出 */
    virtual string str() const {
        HKU_WARN("The subclass does not implement this method");
        return string();
    }

    /**
     * 以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
     * @param path 输出文件所在目录
     */
    virtual void tocsv(const string& path) {
        HKU_WARN("The subclass does not implement this method");
    }

    /**
     * 从订单代理实例同步当前账户资产信息（包含资金、持仓等）
     * @param broker 订单代理实例
     */
    virtual void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker) {
        HKU_WARN("The subclass does not implement this method");
    }

protected:
    string m_name;            // 账户名称
    TradeCostPtr m_costfunc;  // 成本算法

    Datetime m_broker_last_datetime;  // 订单代理最近一次执行操作的时刻,当前启动运行时间
    list<OrderBrokerPtr> m_broker_list;  // 订单代理列表

//============================================
// 序列化支持
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
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_costfunc);
        ar& BOOST_SERIALIZATION_NVP(m_broker_last_datetime);
        ar& BOOST_SERIALIZATION_NVP(m_broker_list);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(TradeManagerBase)
#endif

inline void TradeManagerBase::baseCheckParam(const string& name) const {
    if ("precision" == name) {
        int precision = getParam<int>("precision");
        HKU_ASSERT(precision > 0);
    }
}

inline void TradeManagerBase::paramChanged() {}

/**
 * 客户程序应使用此类型进行实际操作
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