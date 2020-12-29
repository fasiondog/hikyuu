/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#pragma once

#include <thread>
#include <functional>
#include "spot_generated.h"
#include "../DataType.h"
#include "../utilities/thread/ThreadPool.h"

namespace hku {

/**
 * 接收外部实时数据结构
 * @ingroup Agent
 */
struct HKU_API SpotRecord {
    string market;            ///< 市场标识
    string code;              ///< 证券代码
    string name;              ///< 证券名称
    Datetime datetime;        ///< 数据时间
    price_t yesterday_close;  ///< 昨日收盘价
    price_t open;             ///< 开盘价
    price_t high;             ///< 最高价
    price_t low;              ///< 最低价
    price_t close;            ///< 收盘价
    price_t amount;           ///< 成交金额 （千元）
    price_t volumn;           ///< 成交量（手）
    price_t bid1;             ///< 买一价
    price_t bid1_amount;      ///< 买一数量（手）
    price_t bid2;             ///< 买二价
    price_t bid2_amount;      ///< 买二数量
    price_t bid3;             ///< 买三价
    price_t bid3_amount;      ///< 买三数量
    price_t bid4;             ///< 买四价
    price_t bid4_amount;      ///< 买四数量
    price_t bid5;             ///< 买五价
    price_t bid5_amount;      ///< 买五数量
    price_t ask1;             ///< 卖一价
    price_t ask1_amount;      ///< 卖一数量
    price_t ask2;             ///< 卖二价
    price_t ask2_amount;      ///< 卖二数量
    price_t ask3;             ///< 卖三价
    price_t ask3_amount;      ///< 卖三数量
    price_t ask4;             ///< 卖四价
    price_t ask4_amount;      ///< 卖四数量
    price_t ask5;             ///< 卖五价
    price_t ask5_amount;      ///< 卖五数量
};

/**
 * 接收外部实时数据代理
 * @ingroup Agent
 */
class HKU_API SpotAgent {
public:
    /** 获取代理实例 */
    static SpotAgent& instance();

    /** 释放代理实例，用于全局退出时主动释放资源 */
    static void release();

    /** 析构函数 */
    virtual ~SpotAgent();

    /** 启动代理 */
    void start();

    /** 停止代理 */
    void stop();

    /** 设置是否打印数据接收进展情况，主要用于在交互环境下关闭打印 */
    void setPrintFlag(bool print) {
        m_print = print;
    }

    /**
     * 增加收到 Spot 数据时的处理函数
     * @param process 处理函数，仅处理单条 spot 数据
     */
    void addProcess(std::function<void(const SpotRecord&)> process);

    /**
     * 在接受某时刻全部批次的数据后，进行相应的后处理函数
     * @param func 后处理函数
     */
    void addPostProcess(std::function<void()> func);

private:
    static SpotAgent* ms_spotAgent;  // 全局单例
    static const char* ms_pubUrl;    // 数据发送服务地址
    static const char* ms_startTag;  // 批次数据接收起始标记，用于判断启动了新的批次数据接收
    static const char* ms_endTag;  // 批次数据接收接收标记，用于判断该批次数据更新结束
    static const char* ms_spotTopic;      // 向数据发送服务订阅的主题
    static const int ms_startTagLength;   // 批次数据接收起始标记长度
    static const int ms_endTagLength;     // 批次数据接收结束标记长度
    static const int ms_spotTopicLength;  // 订阅主题标记长度

private:
    SpotAgent() = default;

    unique_ptr<SpotRecord> parseFlatSpot(const hikyuu::flat::Spot* spot);
    void parseSpotData(const void* buf, size_t buf_len);

    void work_thread();

private:
    enum STATUS { WAITING, RECEIVING };  // 等待新的批次数据，正在接收批次数据中
    enum STATUS m_status = WAITING;      // 当前内部状态
    bool m_stop = false;                 // 结束代理工作标识
    bool m_print = true;          // 是否打印接收进度，防止的交互模式的影响
    int m_revTimeout = 100;       // 连接数据服务超时时长（毫秒）
    size_t m_batch_count = 0;     // 记录本次批次接收的数据数量
    std::thread m_receiveThread;  // 数据接收线程
    ThreadPool m_tg;              // 数据处理任务线程池
    list<std::function<void(const SpotRecord&)>> m_processList;  // 已注册的 spot 处理函数列表
    list<std::function<void()>> m_postProcessList;  // 已注册的批次后处理函数列表
    vector<std::future<void>> m_process_task_list;
};

/**
 * 启动 Spot 数据接收代理
 * @details 默认增加缓存日线数据更新处理
 * @param print 打印接收数据进展
 * @ingroup Agent
 */
void HKU_API start_spot_agent(bool print = true);

/**
 * 终止 Spot 数据接收代理
 * @ingroup Agent
 */
void HKU_API stop_spot_agent();

}  // namespace hku