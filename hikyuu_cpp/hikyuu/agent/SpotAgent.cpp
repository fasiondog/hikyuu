/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#include "../GlobalInitializer.h"
#include <chrono>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include "../utilities/util.h"
#include "../StockManager.h"
#include "SpotAgent.h"

using namespace hikyuu::flat;

namespace hku {

SpotAgent* SpotAgent::ms_spotAgent = nullptr;
const char* SpotAgent::ms_pubUrl = "ipc:///tmp/hikyuu_real_pub.ipc";
const char* SpotAgent::ms_startTag = ":spot:[start spot]";
const char* SpotAgent::ms_endTag = ":spot:[end spot]";
const char* SpotAgent::ms_spotTopic = ":spot:";
const int SpotAgent::ms_spotTopicLength = strlen(SpotAgent::ms_spotTopic);
const int SpotAgent::ms_startTagLength = strlen(SpotAgent::ms_startTag);
const int SpotAgent::ms_endTagLength = strlen(SpotAgent::ms_endTag);

SpotAgent& SpotAgent::instance() {
    if (!ms_spotAgent) {
        ms_spotAgent = new SpotAgent;
    }
    return *ms_spotAgent;
}

void SpotAgent::release() {
    if (ms_spotAgent) {
        HKU_INFO("relase spot agent");
        delete ms_spotAgent;
        ms_spotAgent = nullptr;
    }
}

SpotAgent::~SpotAgent() {
    if (m_receiveThread.joinable()) {
        m_stop = true;
        m_receiveThread.join();
    }
}

static void updateStockDayData(const hikyuu::flat::Spot* spot) {
    auto& sm = StockManager::instance();
    std::stringstream market_code_buf;
    market_code_buf << spot->market()->str() << spot->code()->str();
    Stock stk = sm[market_code_buf.str()];
    if (stk.isNull())
        return;

    KRecord krecord(Datetime(spot->datetime()->str()), spot->open(), spot->high(), spot->low(),
                    spot->close(), spot->amount(), spot->volumn());
    HKU_INFO("{} {}", stk.market_code(), krecord);
    stk.realtimeUpdate(krecord, KQuery::DAY);
}

void HKU_API start_spot_agent() {
    auto& agent = SpotAgent::instance();
    agent.addProcess(updateStockDayData);
    agent.start();
}

class ProcessTask {
public:
    ProcessTask(std::function<void(const hikyuu::flat::Spot*)> func, const void* buf, size_t len)
    : m_func(func) {
        m_buf.resize(len);
        memcpy(m_buf.data(), buf, len);
    }

    ProcessTask(ProcessTask&& other) {
        m_func = other.m_func;
        m_buf = std::move(other.m_buf);
    }

    void operator()() {
        m_func((const hikyuu::flat::Spot*)m_buf.data());
    }

private:
    std::function<void(const hikyuu::flat::Spot*)> m_func;
    std::vector<char> m_buf;
};

void SpotAgent::parseSpotData(const void* buf, size_t buf_len) {
    SPEND_TIME(receive_data);
    const uint8_t* spot_list_buf = (const uint8_t*)(buf) + ms_spotTopicLength;

    // 校验数据
    flatbuffers::Verifier verify(spot_list_buf, buf_len);
    HKU_CHECK(VerifySpotListBuffer(verify), "Invalid data!");

    // 更新日线数据
    auto* spot_list = GetSpotList(spot_list_buf);
    auto* spots = spot_list->spot();
    size_t total = spots->size();
    m_batch_count += total;
    for (size_t i = 0; i < total; i++) {
        auto* spot = spots->Get(i);
        for (auto& process : m_processList) {
            process(spot);
            // m_tg.submit(ProcessTask(process, spot, spot->GetSize()));
        }
    }
}

void SpotAgent::work_thread() {
    nng_socket sock;

    int rv = nng_sub0_open(&sock);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Can't open nng sub0! {}", nng_strerror(rv));

    rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, ms_spotTopic, ms_spotTopicLength);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Failed set nng socket option! {}", nng_strerror(rv));

    rv = nng_setopt_ms(sock, NNG_OPT_RECVTIMEO, m_revTimeout);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Failed set receive timeout option!");

    rv = -1;
    while (!m_stop && rv != 0) {
        rv = nng_dial(sock, ms_pubUrl, nullptr, 0);
        HKU_WARN_IF(
          rv != 0,
          "Faied nng_dial, will retry after 5 seconds! You Maybe need start the collection service "
          "first.");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    HKU_INFO("Ready to receive ...");

    while (!m_stop) {
        char* buf = nullptr;
        size_t length = 0;
        try {
            rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC);
            if (!buf || length == 0) {
                continue;
            }
            HKU_CHECK(rv == 0 || rv == NNG_ETIMEDOUT, "Failed nng_recv! {} ", nng_strerror(rv));
            switch (m_status) {
                case WAITING:
                    if (memcmp(buf, ms_startTag, ms_startTagLength) == 0) {
                        m_status = RECEIVING;
                    }
                    break;
                case RECEIVING:
                    if (length == ms_endTagLength) {
                        m_status = WAITING;
                        HKU_INFO("received: {}", m_batch_count);
                        m_batch_count = 0;
                        for (auto& f : m_postProcessList) {
                            f();
                        }
                    } else {
                        HKU_CHECK(memcmp(buf, ms_startTag, ms_startTagLength) != 0,
                                  "Data not received in time, maybe the send speed is too fast!");
                        parseSpotData(buf, length);
                    }
                    break;
            }
        } catch (std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknow error!");
        }
        if (buf) {
            nng_free(buf, length);
        }
    }

    nng_close(sock);
}

void SpotAgent::start() {
    m_receiveThread = std::thread(&SpotAgent::work_thread, this);
}

void SpotAgent::stop() {
    m_stop = true;
}

void SpotAgent::addProcess(std::function<void(const hikyuu::flat::Spot*)> process) {
    m_processList.push_back(process);
}

void SpotAgent::addPostProcess(std::function<void()> func) {
    m_postProcessList.push_back(func);
}

}  // namespace hku