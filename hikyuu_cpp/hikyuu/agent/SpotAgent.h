/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#pragma once

#include <thread>
#include <functional>
#include "../DataType.h"

namespace hku {

class HKU_API SpotAgent {
public:
    SpotAgent() = default;
    ~SpotAgent();

    void start();
    void stop();

    void addPostProcess(std::function<void()> func);

private:
    static const char* ms_pubUrl;
    static const char* ms_startTag;
    static const char* ms_endTag;
    static const char* ms_spotTopic;
    static int ms_spotTopicLength;

    static void parseSpotData(const void* buf, size_t buf_len, bool print);

private:
    void work_thread();

private:
    bool m_stop = false;
    int m_revTimeout = 60000;
    std::thread m_receiveThread;
    list<std::function<void()>> m_postProcessList;
};

}  // namespace hku