/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#pragma once

#include <future>
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
    PARAMETER_SUPPORT

public:
    Strategy();
    explicit Strategy(const string& name, const string& config_file = "");
    Strategy(const vector<string>& codeList, const vector<KQuery::KType>& ktypeList,
             const string& name = "Strategy", const string& config_file = "");
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
     * @param market 指定的市场
     * @param ignoreMarket 是否忽略市场时间限制，如为 true，则为定时循环不受开闭市时间限制
     */
    void runDaily(std::function<void()>&& func, const TimeDelta& delta,
                  const std::string& market = "SH", bool ignoreMarket = false);

    /**
     * 每日在指定时刻执行任务
     * @param func 待执行的任务
     * @param delta 指定时刻
     * @param ignoreHoliday 忽略节假日，即节假日不执行
     */
    void runDailyAt(std::function<void()>&& func, const TimeDelta& delta,
                    bool ignoreHoliday = true);

    /**
     * 正确数据发生变化调用，即接收到相应行情数据变更
     * @note 通常用于调试。且只要收到行情采集消息就会触发，不受开、闭市时间限制
     * @param changeFunc 回调函数
     */
    void onChange(std::function<void(const Stock&, const SpotRecord& spot)>&& changeFunc);

    /**
     * 一批行情数据接受完毕后通知
     * @note 通常仅用于调试打印，该批行情数据中不一定含有上下文中包含的 stock
     *       且只要收到行情采集消息就会触发，不受开、闭市时间限制。
     * @param recievedFucn 回调函数
     */
    void onReceivedSpot(std::function<void(const Datetime&)>&& recievedFucn);

    /**
     * 启动策略执行，必须在已注册相关处理函数后执行
     */
    void start(bool autoRecieveSpot = true);

private:
    string m_name;
    string m_config_file;
    StrategyContext m_context;
    std::function<void(const Datetime&)> m_on_recieved_spot;
    std::function<void(const Stock&, const SpotRecord& spot)> m_on_change;

    std::function<void()> m_run_daily_func;
    TimeDelta m_run_daily_delta;
    string m_run_daily_market;
    bool m_ignoreMarket{false};

    std::function<void()> m_run_daily_at_func;
    TimeDelta m_run_daily_at_delta;

private:
    void _init();
    void _receivedSpot(const SpotRecord& spot);
    void _runDaily();
    void _runDailyAt();

private:
    static std::atomic_bool ms_keep_running;
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
    event_handle<typename std::result_of<FunctionType()>::type> event(FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
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
 * @param broker 订单代理
 * @param costfunc 成本函数
 */
void HKU_API runInStrategy(const SYSPtr& sys, const Stock& stk, const KQuery& query,
                           const OrderBrokerPtr& broker, const TradeCostPtr& costfunc);

/**
 * @brief 在策略运行时中执行组合策略 PF
 * @note 目前仅支持 buy_delay| sell_delay 均为 false 的系统，即 close 时执行交易
 * @param pf 资产组合
 * @param query 查询条件
 * @param adjust_cycle 调仓周期
 * @param broker 订单代理
 * @param costfunc 成本函数
 */
void HKU_API runInStrategy(const PFPtr& pf, const KQuery& query, int adjust_cycle,
                           const OrderBrokerPtr& broker, const TradeCostPtr& costfunc);

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