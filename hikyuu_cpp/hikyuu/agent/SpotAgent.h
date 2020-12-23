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

class HKU_API SpotAgent {
public:
    static SpotAgent& instance();
    static void release();
    virtual ~SpotAgent();

    void start();
    void stop();

    void addProcess(std::function<void(const hikyuu::flat::Spot*)> process);
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
    list<std::function<void(const hikyuu::flat::Spot*)>> m_processList;
    list<std::function<void()>> m_postProcessList;
};

void HKU_API start_spot_agent();

}  // namespace hku