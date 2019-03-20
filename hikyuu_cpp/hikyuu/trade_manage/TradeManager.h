/*
 * TradeManager.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#ifndef TRADEMANAGER_H_
#define TRADEMANAGER_H_

#include <boost/tuple/tuple.hpp>
#include "../utilities/Parameter.h"
#include "../utilities/util.h"
#include "TradeRecord.h"
#include "PositionRecord.h"
#include "BorrowRecord.h"
#include "FundsRecord.h"
#include "LoanRecord.h"
#include "OrderBrokerBase.h"
#include "crt/TC_Zero.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#endif

namespace hku {

/**
 * 账户交易管理模块，管理帐户的交易记录及资金使用情况
 * @details
 * <pre>
 * 默认参数：
 * reinvest(bool): false 红利/股息/送股再投资标志，即是否忽略权息信息
 * precision(int): 2 计算精度
 * support_borrow_cash(bool): false 买入操作时是否自动融资
 * support_borrow_stock(bool): false 卖空时是否自动融劵
 * </pre>
 * @ingroup TradeManagerClass
 */
class HKU_API TradeManager {
    PARAMETER_SUPPORT

public:
    TradeManager(const Datetime& datetime = Datetime(199001010000LL),
            price_t initcash = 100000.0,
            const TradeCostPtr& costfunc = TC_Zero(),
            const string& name = "SYS");
    virtual ~TradeManager();

    /** 复位，清空交易、持仓记录 */
    void reset();

    /**
     * 注册订单代理
     * @param broker 订单代理实例
     */
    void regBroker(const OrderBrokerPtr& broker);

    /**
     * 清空已注册的订单代理
     */
    void clearBroker();

    /** 从哪个时刻开始启动订单代理进行下单操作   */
    Datetime getBrokerLastDatetime() const { return m_broker_last_datetime; }

    /** 设置开始订单代理操作的时刻 */
    void setBrokerLastDatetime(const Datetime& date) {
        m_broker_last_datetime = date;
    }

    shared_ptr<TradeManager> clone();

    /**
     * 获取指定对象的保证金比率
     * @param datetime 日期
     * @param stock 指定对象
     */
    double getMarginRate(const Datetime& datetime, const Stock& stock);

    /** 账户名称 */
    string name() const { return m_name; }

    /** 设置账户名称 */
    void name(const string& name) { m_name = name; }

    /** 初始资金 */
    price_t initCash() const { return m_init_cash; }

    /** 账户建立日期 */
    Datetime initDatetime() const { return m_init_datetime; }

    /** 第一笔买入交易发生日期，如未发生交易返回Null<Datetime>() */
    Datetime firstDatetime() const;

    /** 最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期 */
    Datetime lastDatetime() const {
        return m_trade_list.empty()
                ? m_init_datetime : m_trade_list.back().datetime;
    }

    /** 红利/股息/送股再投资标志，即是否忽略权息信息 **/
    bool reinvest() const { return getParam<bool>("reinvest"); }

    /** 交易精度 */
    int precision() const { return getParam<int>("precision"); }

    /** 获取交易算法指针 */
    TradeCostPtr costFunc() const { return m_costfunc; }

    /** 设置交易算法指针 */
    void costFunc(const TradeCostPtr& func) { m_costfunc = func; }

    /**
     * 返回当前现金
     * @note 仅返回当前信息，不会根据权息进行调整
     */
    price_t currentCash() const { return m_cash; }

    /**
     * 获取指定日期的现金
     * @note 如果不带日期参数，无法根据权息信息调整持仓
     */
    price_t cash(const Datetime& datetime, KQuery::KType ktype = KQuery::DAY);

    /**
     * 当前是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    bool have(const Stock& stock) const{
        return m_position.count(stock.id()) ? true : false;
    }

    /**
     * 当前空头仓位是否持有指定的证券
     * @note 这里未使用日期参数，必须保证是按日期顺序执行
     * @param stock 指定证券
     * @return true 是 | false 否
     */
    bool haveShort(const Stock& stock) const{
        return m_short_position.count(stock.id()) ? true : false;
    }

    /** 当前持有的证券种类数量 */
    size_t getStockNumber() const { return m_position.size(); }

    /** 当前空头持有的证券种类数量 */
    size_t getShortStockNumber() const { return m_short_position.size(); }

    /** 获取指定时刻的某证券持有数量 */
    size_t getHoldNumber(const Datetime& datetime, const Stock& stock);

    /** 获取指定时刻的空头某证券持有数量 */
    size_t getShortHoldNumber(const Datetime& datetime, const Stock& stock);

    /** 获取指定时刻已借入的股票数量 */
    size_t getDebtNumber(const Datetime& datetime, const Stock& stock);

    /** 获取指定时刻已借入的现金额 */
    price_t getDebtCash(const Datetime& datetime);

    /** 获取全部交易记录 */
    const TradeRecordList& getTradeList() const { return m_trade_list; }

    /**
     * 获取指定日期范围内的交易记录[start, end)
     * @param start 起始日期
     * @param end 结束日期
     * @return 交易记录列表
     */
    TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const;

    /** 获取当前全部持仓记录 */
    PositionRecordList getPositionList() const;

    /** 获取全部历史持仓记录，即已平仓记录 */
    const PositionRecordList& getHistoryPositionList() const {
        return m_position_history;
    }

    /** 获取当前全部空头仓位记录 */
    PositionRecordList getShortPositionList() const;

    /** 获取全部空头历史仓位记录 */
    const PositionRecordList& getShortHistoryPositionList() const {
        return m_short_position_history;
    }

    /** 获取指定证券的当前持仓记录，如当前未持有该票，返回Null<PositionRecord>() */
    PositionRecord getPosition(const Stock&) const;

    /** 获取指定证券的当前空头仓位持仓记录，如当前未持有该票，返回Null<PositionRecord>() */
    PositionRecord getShortPosition(const Stock&) const;

    /** 获取当前借入的股票列表 */
    BorrowRecordList getBorrowStockList() const;

    /**
     * 计算买入成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 买入价格
     * @param num 买入数量
     * @return CostRecord 交易成本记录
     */
    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock,
            price_t price, size_t num) const{
        return m_costfunc
                ? m_costfunc->getBuyCost(datetime, stock, price, num)
                : CostRecord();
    }

    /**
     * 计算卖出成本
     * @param datetime 交易日期
     * @param stock 交易的证券对象
     * @param price 卖出价格
     * @param num 卖出数量
     * @return CostRecord 交易成本记录
     */
    CostRecord getSellCost(const Datetime& datetime, const Stock& stock,
            price_t price, size_t num) const{
        return m_costfunc
                ? m_costfunc->getSellCost(datetime, stock, price, num)
                : CostRecord();
    }

    /**
     * 计算计入现金时的费用成本
     * @param datetime 借入日期
     * @param cash 现金额
     */
    CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) {
        return m_costfunc
                ? m_costfunc->getBorrowCashCost(datetime, cash)
                : CostRecord();
    }

    /**
     * 计算归还融资成本
     * @param borrow_datetime 借入日期
     * @param return_datetime 归还日期
     * @param cash 归还金额
     */
    CostRecord getReturnCashCost(const Datetime& borrow_datetime,
            const Datetime& return_datetime,
            price_t cash) {
        return m_costfunc
                ? m_costfunc->getReturnCashCost(borrow_datetime,
                                                return_datetime, cash)
                : CostRecord();
    }

    /**
     * 计算融劵借入成本
     * @param datetime 融劵日期
     * @param stock 借入的对象
     * @param price 每股价格
     * @param num 借入的数量
     */
    CostRecord getBorrowStockCost(const Datetime& datetime,
            const Stock& stock, price_t price, size_t num) {
        return m_costfunc
                ? m_costfunc->getBorrowStockCost(datetime, stock, price, num)
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
    CostRecord getReturnStockCost(const Datetime& borrow_datetime,
            const Datetime& return_datetime,
            const Stock& stock, price_t price, size_t num) {
        return m_costfunc
                ? m_costfunc->getReturnStockCost(borrow_datetime,
                        return_datetime, stock, price, num)
                : CostRecord();
    }

    /**
     * 存入资金
     * @param datetime 存入时间
     * @param cash 存入的资金量
     * @return true | false
     */
    bool checkin(const Datetime& datetime, price_t cash);

    /**
     * 取出资金
     * @param datetime 取出时间
     * @param cash 取出的资金量
     * @return true | false
     */
    bool checkout(const Datetime& datetime, price_t cash);

    /**
     * 存入资产
     * @param datetime 存入日期
     * @param stock 待存入的股票
     * @param price 存入股票的每股价格
     * @param number 存入股票的数量
     * @return true | false
     */
    bool checkinStock(const Datetime& datetime, const Stock& stock,
            price_t price, size_t number);

    /**
     * 取出当前资产
     * @param datetime 取出日期
     * @param stock 待取出的股票
     * @param price 取出的每股价格
     * @param number 取出的数量
     * @return true | false
     * @note 应该不会被用到
     */
    bool checkoutStock(const Datetime& datetime, const Stock& stock,
            price_t price, size_t number);

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
     * @return 返回对应的交易记录，如果操作失败，business等于INVALID_BUSINESS
     */
    TradeRecord buy(const Datetime& datetime, const Stock& stock,
            price_t realPrice, size_t number,
            price_t stoploss = 0.0,
            price_t goalPrice = 0.0,
            price_t planPrice = 0.0,
            SystemPart from = PART_INVALID);

    /**
     * 卖出操作
     * @param datetime 卖出时间
     * @param stock 卖出的证券
     * @param realPrice 实际卖出价格
     * @param number 卖出数量，如果等于Null<size_t>(), 表示全部卖出
     * @param stoploss 新的止损价
     * @param goalPrice 新的目标价格
     * @param planPrice 原计划卖出价格
     * @param from 记录是哪个系统部件发出的卖出指示
     * @return 返回对应的交易记录，如果操作失败，business等于INVALID_BUSINESS
     */
    TradeRecord sell(const Datetime& datetime, const Stock& stock,
            price_t realPrice,
            size_t number = Null<size_t>(),
            price_t stoploss = 0.0,
            price_t goalPrice = 0.0,
            price_t planPrice = 0.0,
            SystemPart from = PART_INVALID);

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
     * @return 返回对应的交易记录，如果操作失败，business等于INVALID_BUSINESS
     */
    TradeRecord sellShort(const Datetime& datetime, const Stock& stock,
            price_t realPrice, size_t number,
            price_t stoploss = 0.0,
            price_t goalPrice = 0.0,
            price_t planPrice = 0.0,
            SystemPart from = PART_INVALID);

    /**
     * 卖空后回补
     * @param datetime 买入时间
     * @param stock 买入的证券
     * @param realPrice 实际买入价格
     * @param number 卖出数量，如果等于Null<size_t>(), 表示全部卖出
     * @param stoploss 止损价
     * @param goalPrice 目标价格
     * @param planPrice 计划买入价格
     * @param from 记录是哪个系统部件发出的卖出指示
     * @return 返回对应的交易记录，如果操作失败，business等于INVALID_BUSINESS
     */
    TradeRecord buyShort(const Datetime& datetime, const Stock& stock,
            price_t realPrice,
            size_t number = Null<size_t>(),
            price_t stoploss = 0.0,
            price_t goalPrice = 0.0,
            price_t planPrice = 0.0,
            SystemPart from = PART_INVALID);

    /**
     * 借入资金，从其他来源借取的资金，如融资
     * @param datetime 借入时间
     * @param cash 借入的现金
     * @return true | false
     */
    bool borrowCash(const Datetime& datetime, price_t cash);

    /**
     * 归还资金
     * @param datetime 归还日期
     * @param cash 归还现金
     * @return true | false
     */
    bool returnCash(const Datetime& datetime, price_t cash);

    /**
     * 借入证券
     * @param datetime 借入时间
     * @param stock 借入的stock
     * @param price 借入时单股价格
     * @param number 借入时数量
     * @return true | false
     */
    bool borrowStock(const Datetime& datetime, const Stock& stock,
            price_t price, size_t number);

    /**
     * 归还证券
     * @param datetime 归还时间
     * @param stock 归还的stock
     * @param price 归还时单股价格
     * @param number 归还数量
     * @return true | false
     */
    bool returnStock(const Datetime& datetime, const Stock& stock,
            price_t price, size_t number);

    /**
     * 获取账户当前时刻的资产详情
     * @param ktype 日期的类型
     * @return 资产详情
     */
    FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const;

    /**
     * 获取指定时刻的资产市值详情
     * @param datetime 必须大于帐户建立的初始日期，或为Null<Datetime>()
     * @param ktype 日期的类型
     * @return 资产详情
     * @note 当datetime等于Null<Datetime>()时，与getFunds(KType)同
     */
    FundsRecord getFunds(const Datetime& datetime,
            KQuery::KType ktype = KQuery::DAY);

    /**
     * 获取资产净值曲线，含借入的资产
     * @param dates 日期列表，根据该日期列表获取其对应的资产净值曲线
     * @param ktype K线类型，必须与日期列表匹配，默认KQuery::DAY
     * @return 资产净值列表
     */
    PriceList getFundsCurve(const DatetimeList& dates,
            KQuery::KType ktype = KQuery::DAY);

    /**
     * 获取收益曲线，即扣除历次存入资金后的资产净值曲线
     * @param dates 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
     * @param ktype K线类型，必须与日期列表匹配，默认为KQuery::DAY
     * @return 收益曲线
     */
    PriceList getProfitCurve(const DatetimeList& dates,
            KQuery::KType ktype = KQuery::DAY);

    /**
     * 直接加入交易记录
     * @note 如果加入初始化账户记录，将清除全部已有交易及持仓记录
     * @param tr 待加入的交易记录
     * @return bool true 成功 | false 失败
     */
    bool addTradeRecord(const TradeRecord& tr);

    /** 字符串输出 */
    string toString() const;

    /**
     * 以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线
     * @param path 输出文件所在目录
     */
    void tocsv(const string& path);

private:
    //根据权息信息，更新交易记录及持仓
    void _update(const Datetime&);

    //以脚本的形式保存交易动作，便于修正和校准
    void _saveAction(const TradeRecord&);

    bool _add_init_tr(const TradeRecord&);
    bool _add_buy_tr(const TradeRecord&);
    bool _add_sell_tr(const TradeRecord&);
    bool _add_gift_tr(const TradeRecord&);
    bool _add_bonus_tr(const TradeRecord&);
    bool _add_checkin_tr(const TradeRecord&);
    bool _add_checkout_tr(const TradeRecord&);
    bool _add_checkin_stock_tr(const TradeRecord&);
    bool _add_checkout_stock_tr(const TradeRecord&);
    bool _add_borrow_cash_tr(const TradeRecord&);
    bool _add_return_cash_tr(const TradeRecord&);
    bool _add_borrow_stock_tr(const TradeRecord&);
    bool _add_return_stock_tr(const TradeRecord&);
    bool _add_sell_short_tr(const TradeRecord&);
    bool _add_buy_short_tr(const TradeRecord&);

private:
    string       m_name;            //账户名称
    Datetime     m_init_datetime;   //账户建立日期
    price_t      m_init_cash;       //初始资金
    TradeCostPtr m_costfunc;        //成本算法

    price_t      m_cash;            //当前现金
    price_t      m_checkin_cash;    //累计存入资金，初始资金视为存入
    price_t      m_checkout_cash;   //累计取出资金
    price_t      m_checkin_stock;   //累计存入股票价值
    price_t      m_checkout_stock;  //累计取出股票价值
    price_t      m_borrow_cash;     //当前借入资金，负债

    list<LoanRecord> m_loan_list;   //当前融资情况

    typedef map<hku_uint64, BorrowRecord> borrow_stock_map_type;
    borrow_stock_map_type m_borrow_stock;  //当前借入的股票及其数量

    TradeRecordList m_trade_list;  //交易记录

    typedef map<hku_uint64, PositionRecord > position_map_type;
    position_map_type m_position; //当前持仓交易对象的持仓记录 ["sh000001"-> ]
    PositionRecordList m_position_history; //持仓历史记录
    position_map_type m_short_position; //空头仓位记录
    PositionRecordList m_short_position_history; //空头仓位历史记录

    list<OrderBrokerPtr>  m_broker_list; //订单代理列表
    Datetime m_broker_last_datetime; //订单代理最近一次执行操作的时刻

    list<string> m_actions; //记录交易动作，便于修改或校准实盘时的交易

//==================================================
// 支持序列化
//==================================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        string name(GBToUTF8(m_name));
        ar & boost::serialization::make_nvp("m_name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_init_datetime);
        ar & BOOST_SERIALIZATION_NVP(m_init_cash);
        ar & BOOST_SERIALIZATION_NVP(m_costfunc);
        ar & BOOST_SERIALIZATION_NVP(m_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkin_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkout_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkin_stock);
        ar & BOOST_SERIALIZATION_NVP(m_checkout_stock);
        ar & BOOST_SERIALIZATION_NVP(m_borrow_cash);
        ar & BOOST_SERIALIZATION_NVP(m_loan_list);
        namespace bs = boost::serialization;
        BorrowRecordList borrow = getBorrowStockList();
        ar & bs::make_nvp<BorrowRecordList>("m_borrow_stock", borrow);
        PositionRecordList position = getPositionList();
        ar & bs::make_nvp<PositionRecordList>("m_position", position);
        ar & BOOST_SERIALIZATION_NVP(m_position_history);
        position = getShortPositionList();
        ar & bs::make_nvp<PositionRecordList>("m_short_position", position);
        ar & BOOST_SERIALIZATION_NVP(m_short_position_history);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
        ar & BOOST_SERIALIZATION_NVP(m_actions);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("m_name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_init_datetime);
        ar & BOOST_SERIALIZATION_NVP(m_init_cash);
        ar & BOOST_SERIALIZATION_NVP(m_costfunc);
        ar & BOOST_SERIALIZATION_NVP(m_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkin_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkout_cash);
        ar & BOOST_SERIALIZATION_NVP(m_checkin_stock);
        ar & BOOST_SERIALIZATION_NVP(m_checkout_stock);
        ar & BOOST_SERIALIZATION_NVP(m_borrow_cash);
        ar & BOOST_SERIALIZATION_NVP(m_loan_list);
        namespace bs = boost::serialization;
        BorrowRecordList borrow;
        ar & bs::make_nvp<BorrowRecordList>("m_borrow_stock", borrow);
        BorrowRecordList::const_iterator bor_iter = borrow.begin();
        for (; bor_iter != borrow.end(); ++bor_iter) {
            m_borrow_stock[bor_iter->stock.id()] = *bor_iter;
        }
        PositionRecordList position;
        ar & bs::make_nvp<PositionRecordList>("m_position", position);
        PositionRecordList::const_iterator iter = position.begin();
        for (; iter != position.end(); ++iter) {
            m_position[iter->stock.id()] = *iter;
        }
        ar & BOOST_SERIALIZATION_NVP(m_position_history);

        position.clear();
        ar & bs::make_nvp<PositionRecordList>("m_short_position", position);
        iter = position.begin();
        for (; iter != position.end(); ++iter) {
            m_short_position[iter->stock.id()] = *iter;
        }
        ar & BOOST_SERIALIZATION_NVP(m_short_position_history);
        ar & BOOST_SERIALIZATION_NVP(m_trade_list);
        ar & BOOST_SERIALIZATION_NVP(m_actions);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};


/**
 * 客户程序应使用此类型进行实际操作
 * @ingroup TradeManagerClass
 */
typedef shared_ptr<TradeManager> TradeManagerPtr;
typedef shared_ptr<TradeManager> TMPtr;

HKU_API std::ostream & operator<<(std::ostream &, const TradeManager&);
HKU_API std::ostream & operator<<(std::ostream &, const TradeManagerPtr&);

} /* namespace hku */
#endif /* TRADEMANAGER_H_ */
