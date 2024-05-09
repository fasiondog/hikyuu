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
#include "../utilities/Parameter.h"
#include "../utilities/thread/FuncWrapper.h"
#include "../utilities/thread/ThreadSafeQueue.h"
#include "../global/GlobalSpotAgent.h"
#include "../trade_sys/portfolio/Portfolio.h"

namespace hku {

class HKU_API StrategyBase {
    PARAMETER_SUPPORT

public:
    StrategyBase();
    explicit StrategyBase(const string& name);
    StrategyBase(const string& name, const string& config_file);

    virtual ~StrategyBase();

    const string& name() const {
        return m_name;
    }

    void name(const string& name) {
        m_name = name;
    }

    StockManager& getSM() {
        return StockManager::instance();
    }

    TMPtr getTM() const {
        return m_tm;
    }

    void setTM(const TMPtr& tm) {
        m_tm = tm;
    }

    const StrategyContext& context() const {
        return m_context;
    }

    void context(const StrategyContext& context) {
        m_context = context;
    }

    Datetime startDatetime() const {
        return m_context.startDatetime();
    }

    void startDatetime(const Datetime& d) {
        m_context.startDatetime(d);
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

    void run() {
        _run(false);
    }

    void receivedSpot(const SpotRecord& spot);
    void finishReceivedSpot(Datetime revTime);

    virtual void init() {}
    virtual void onTick() {}
    virtual void onBar(const KQuery::KType& ktype){};

    virtual void onMarketOpen() {}
    virtual void onMarketClose() {}
    virtual void onClock(TimeDelta detla) {}

private:
    string m_name;
    string m_config_file;
    StrategyContext m_context;
    TMPtr m_tm;

    StockList m_stock_list;
    std::unordered_map<KQuery::KType, Datetime> m_ref_last_time;
    std::unordered_map<Stock, SpotRecord> m_spot_map;

private:
    void _initDefaultParam();

    void _addTimer();
    void _addClockEvent(const string& enable, TimeDelta delta, TimeDelta openTime,
                        TimeDelta closeTime);

    void _run(bool forTest);

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

typedef shared_ptr<StrategyBase> StrategyPtr;

}  // namespace hku