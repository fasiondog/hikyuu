/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#pragma once

#include <future>
#include <forward_list>
#include "hikyuu/DataType.h"
#include "hikyuu/StrategyContext.h"
#include "hikyuu/global/SpotRecord.h"
#include "hikyuu/utilities/thread/FuncWrapper.h"
#include "hikyuu/utilities/thread/ThreadSafeQueue.h"
#include "hikyuu/trade_sys/portfolio/Portfolio.h"
#include "BrokerTradeManager.h"

namespace hku {

/**
 * @ingroup Strategy
 * @{
 */

/**
 * @brief 策略运行时
 */
class HKU_API Strategy {
    CLASS_LOGGER_IMP(Strategy)
    PARAMETER_SUPPORT_WITH_CHECK

public:
    Strategy();
    explicit Strategy(const string& name, const string& config_file = "");
    Strategy(const vector<string>& codeList, const vector<KQuery::KType>& ktypeList,
             const unordered_map<string, int>& preloadNum = {}, const string& name = "Strategy",
             const string& config_file = "");
    explicit Strategy(const StrategyContext& context, const string& name = "Strategy",
                      const string& config_file = "");

    Strategy(const Strategy&) = delete;
    Strategy& operator=(const Strategy&) = delete;

    virtual ~Strategy();

    const string& name() const {
        return m_name;
    }

    void name(const string& name) {
        m_name = name;
    }

    const StrategyContext& context() const {
        return m_context;
    }

    /**
     * 每日开盘时间内，以 delta 为周期循环定时执行指定任务
     * @param func 待执行的任务
     * @param delta 间隔时间
     * @param market 指定的市场, 用于获取开/收盘时间
     * @param ignoreMarket 是否忽略市场时间限制，如为 true，则为定时循环不受开闭市时间限制
     */
    void runDaily(const std::function<void(Strategy*)>& func, const TimeDelta& delta,
                  const std::string& market = "SH", bool ignoreMarket = false);

    /**
     * 每日在指定时刻执行任务
     * @param func 待执行的任务
     * @param delta 指定时刻
     * @param ignoreHoliday 忽略节假日，即节假日不执行
     */
    void runDailyAt(const std::function<void(Strategy*)>& func, const TimeDelta& delta,
                    bool ignoreHoliday = true);

    /**
     * 正确数据发生变化调用，即接收到相应行情数据变更
     * @note 通常用于调试。且只要收到行情采集消息就会触发，不受开、闭市时间限制
     * @param changeFunc 回调函数
     */
    void onChange(
      const std::function<void(Strategy*, const Stock&, const SpotRecord& spot)>& changeFunc);

    /**
     * 一批行情数据接受完毕后通知
     * @note 通常仅用于调试打印，该批行情数据中不一定含有上下文中包含的 stock
     *       且只要收到行情采集消息就会触发，不受开、闭市时间限制。
     * @param recievedFucn 回调函数
     */
    void onReceivedSpot(const std::function<void(Strategy*, const Datetime&)>& recievedFucn);

    /**
     * 启动策略执行，必须在已注册相关处理函数后执行
     */
    void start(bool autoRecieveSpot = true);

    //==========================================================================
    // 以下为策略运行时对外接口，建议使用这些接口代替同名其他功能函数，已保证回测和实盘一致
    //==========================================================================

    TradeManagerPtr getTM() const noexcept {
        return m_tm;
    }

    void setTM(const TradeManagerPtr& tm) noexcept {
        m_tm = tm;
    }

    /** 仅在回测状态下使用 */
    SlippagePtr getSP() const noexcept {
        return m_sp;
    }

    void setSP(const SlippagePtr& slippage) noexcept {
        m_sp = slippage;
    }

    KData getKData(const Stock& stk, const Datetime& start_date, const Datetime& end_date,
                   const KQuery::KType& ktype,
                   KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const {
        return stk.getKData(KQueryByDate(start_date, end_date, ktype, recover_type));
    }

    KData getLastKData(const Stock& stk, const Datetime& start_date, const KQuery::KType& ktype,
                       KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const {
        return getKData(stk, start_date, Null<Datetime>(), ktype, recover_type);
    }

    KData getLastKData(const Stock& stk, size_t lastnum, const KQuery::KType& ktype,
                       KQuery::RecoverType recover_type = KQuery::NO_RECOVER) const;

    virtual Datetime today() const {
        return Datetime::today();
    }

    virtual Datetime now() const {
        return Datetime::now();
    }

    virtual Datetime nextDatetime() const {
        return Null<Datetime>();
    }

    virtual TradeRecord buy(const Stock& stk, price_t price, double num, double stoploss = 0.0,
                            double goal_price = 0.0,
                            SystemPart part_from = SystemPart::PART_SIGNAL) {
        HKU_ASSERT(m_tm);
        return m_tm->buy(Datetime::now(), stk, price, num, stoploss, goal_price, price, part_from);
    }

    virtual TradeRecord sell(const Stock& stk, price_t price, double num, price_t stoploss = 0.0,
                             price_t goal_price = 0.0,
                             SystemPart part_from = SystemPart::PART_SIGNAL) {
        HKU_ASSERT(m_tm);
        return m_tm->sell(Datetime::now(), stk, price, num, stoploss, goal_price, price, part_from);
    }

    virtual bool isBacktesting() const {
        return false;
    }

protected:
    string m_name;
    string m_config_file;
    StrategyContext m_context;
    TradeManagerPtr m_tm;
    SlippagePtr m_sp;

    std::function<void(Strategy*, const Datetime&)> m_on_recieved_spot;
    std::function<void(Strategy*, const Stock&, const SpotRecord& spot)> m_on_change;

    struct RunDailyAt {
        std::function<void()> func;
        TimeDelta delta;
        string market;
        bool ignoreMarket{false};
    };
    std::forward_list<RunDailyAt> m_run_daily_at_list;

    std::unordered_map<TimeDelta, std::function<void()>> m_run_daily_at_funcs;

protected:
    static std::atomic_bool ms_keep_running;

protected:
    void _init();

private:
    void _initParam();
    void _receivedSpot(const SpotRecord& spot);
    void _runDaily();
    void _runDailyAt();

private:
    static void sig_handler(int sig);

    typedef FuncWrapper event_type;
    ThreadSafeQueue<event_type> m_event_queue;  // 消息队列

    /** 先消息队列提交任务后返回的对应 future 的类型 */
    template <typename ResultType>
    using event_handle = std::future<ResultType>;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    /** 向线程池提交任务 */
    template <typename FunctionType>
    auto event(FunctionType f) {
        typedef typename std::invoke_result<FunctionType>::type result_type;
        std::packaged_task<result_type()> task(f);
        event_handle<result_type> res(task.get_future());
        m_event_queue.push(std::move(task));
        return res;
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    void _startEventLoop();
};

typedef shared_ptr<Strategy> StrategyPtr;

/**
 * @brief 在策略运行时中执行系统交易 SYS
 * @note 目前仅支持 buy_delay| sell_delay 均为 false 的系统，即 close 时执行交易
 * @param sys 交易系统
 * @param stk 交易对象
 * @param query 查询条件
 * @param broker 订单代理（专用与和账户资产同步的订单代理）
 * @param costfunc 成本函数
 * @param other_brokers 其他的订单代理
 */
void HKU_API runInStrategy(const SYSPtr& sys, const Stock& stk, const KQuery& query,
                           const OrderBrokerPtr& broker, const TradeCostPtr& costfunc,
                           const std::vector<OrderBrokerPtr>& other_brokers = {});

/**
 * @brief 在策略运行时中执行组合策略 PF
 * @note 目前仅支持 buy_delay| sell_delay 均为 false 的系统，即 close 时执行交易
 * @param pf 资产组合
 * @param query 查询条件
 * @param broker 订单代理（专用与和账户资产同步的订单代理）
 * @param costfunc 成本函数
 * @param other_brokers 其他的订单代理
 */
void HKU_API runInStrategy(const PFPtr& pf, const KQuery& query, const OrderBrokerPtr& broker,
                           const TradeCostPtr& costfunc,
                           const std::vector<OrderBrokerPtr>& other_brokers = {});

/**
 * 从 hikyuu 数据缓存服务器拉取更新最新的缓存数据
 * @param addr 缓存服务地址，如: tcp://192.168.1.1:9201
 * @param stklist 待更新的股票列表
 * @param ktype 指定更新的K线类型
 */
void HKU_API getDataFromBufferServer(const std::string& addr, const StockList& stklist,
                                     const KQuery::KType& ktype);

/** @} */
}  // namespace hku