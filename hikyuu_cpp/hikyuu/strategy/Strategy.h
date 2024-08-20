/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#pragma once

#include <future>
#include "../DataType.h"
#include "../StrategyContext.h"
#include "../global/SpotRecord.h"
#include "../utilities/thread/FuncWrapper.h"
#include "../utilities/thread/ThreadSafeQueue.h"
#include "../trade_sys/portfolio/Portfolio.h"

namespace hku {

/**
 * @brief 策略运行时
 * @ingroup Stratgy
 */
class HKU_API Strategy {
    CLASS_LOGGER_IMP(Strategy)

public:
    Strategy();
    explicit Strategy(const string& name, const string& config_file = "");
    Strategy(const vector<string>& codeList, const vector<KQuery::KType>& ktypeList,
             const string& name = "Strategy", const string& config_file = "");

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

    void context(const StrategyContext& context) {
        m_context = context;
    }

    void setStockCodeList(vector<string>&& stockList) {
        m_context.setStockCodeList(std::move(stockList));
    }

    void setStockCodeList(const vector<string>& stockList) {
        m_context.setStockCodeList(stockList);
    }

    const vector<string>& getStockCodeList() const {
        return m_context.getStockCodeList();
    }

    void setKTypeList(const vector<KQuery::KType>& ktypeList) {
        m_context.setKTypeList(ktypeList);
    }

    const vector<KQuery::KType>& getKTypeList() const {
        return m_context.getKTypeList();
    }

    /**
     * 每日开盘时间内，以 delta 为周期循环定时执行指定任务
     * @param func 待执行的任务
     * @param delta 间隔时间
     * @param market 指定的市场
     */
    void runDaily(std::function<void()>&& func, const TimeDelta& delta,
                  const std::string& market = "SH");

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
     * @note 通常用于调试
     * @param stk 数据发生变化的 stock
     * @param spot 接收到的具体数据
     */
    void onChange(std::function<void(const Stock&, const SpotRecord& spot)>&& changeFunc);

    /**
     * 一批行情数据接受完毕后通知
     * @note 通常仅用于调试打印，该批行情数据中不一定含有上下文中包含的 stock
     */
    void onReceivedSpot(std::function<void(const Datetime&)>&& recievedFucn);

    /**
     * 启动策略执行，必须在已注册相关处理函数后执行
     */
    void start();

private:
    string m_name;
    string m_config_file;
    StrategyContext m_context;
    std::function<void(const Datetime&)> m_on_recieved_spot;
    std::function<void(const Stock&, const SpotRecord& spot)> m_on_change;
    bool m_running{false};

private:
    void run();
    void receivedSpot(const SpotRecord& spot);

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

}  // namespace hku