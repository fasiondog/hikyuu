/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#pragma once

#include <thread>
#include <functional>
#include "../flatbuffers/spot_generated.h"
#include "../DataType.h"
#include "../utilities/thread/ThreadPool.h"

namespace hku {

struct HKU_API SpotRecord {
    string market;
    string code;
    string name;
    Datetime datetime;
    price_t yesterday_close;
    price_t open;
    price_t high;
    price_t low;
    price_t close;
    price_t amount;
    price_t volumn;
    price_t bid1;
    price_t bid1_amount;
    price_t bid2;
    price_t bid2_amount;
    price_t bid3;
    price_t bid3_amount;
    price_t bid4;
    price_t bid4_amount;
    price_t bid5;
    price_t bid5_amount;
    price_t ask1;
    price_t ask1_amount;
    price_t ask2;
    price_t ask2_amount;
    price_t ask3;
    price_t ask3_amount;
    price_t ask4;
    price_t ask4_amount;
    price_t ask5;
    price_t ask5_amount;
};

class HKU_API SpotAgent {
public:
    static SpotAgent& instance();
    static void release();
    virtual ~SpotAgent();

    void start();
    void stop();

    void addProcess(std::function<void(const SpotRecord&)> process);
    void addPostProcess(std::function<void()> func);

private:
    static SpotAgent* ms_spotAgent;
    static const char* ms_pubUrl;
    static const char* ms_startTag;
    static const char* ms_endTag;
    static const char* ms_spotTopic;
    static const int ms_startTagLength;
    static const int ms_endTagLength;
    static const int ms_spotTopicLength;

private:
    SpotAgent() = default;

    unique_ptr<SpotRecord> parseFlatSpot(const hikyuu::flat::Spot* spot);
    void parseSpotData(const void* buf, size_t buf_len);

    void work_thread();

private:
    enum STATUS { WAITING, RECEIVING };
    enum STATUS m_status = WAITING;
    bool m_stop = false;
    int m_revTimeout = 100;
    size_t m_batch_count = 0;
    std::thread m_receiveThread;
    ThreadPool m_tg;
    list<std::function<void(const SpotRecord&)>> m_processList;
    list<std::function<void()>> m_postProcessList;
};

void HKU_API start_spot_agent();

}  // namespace hku