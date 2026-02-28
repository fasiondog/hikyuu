/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#include <chrono>
#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include "hikyuu/StockManager.h"
#include "hikyuu/lang.h"
#include "spot_generated.h"
#include "SpotAgent.h"

using namespace hikyuu::flat;

namespace hku {

string SpotAgent::ms_pubUrl{"ipc:///tmp/hikyuu_real.ipc"};  // 数据发送服务地址
const char* SpotAgent::ms_startTag = ":spot:[start spot]";
const char* SpotAgent::ms_endTag = ":spot:[end spot]";
const char* SpotAgent::ms_spotTopic = ":spot:";
const size_t SpotAgent::ms_spotTopicLength = strlen(SpotAgent::ms_spotTopic);
const size_t SpotAgent::ms_startTagLength = strlen(SpotAgent::ms_startTag);
const size_t SpotAgent::ms_endTagLength = strlen(SpotAgent::ms_endTag);

Datetime SpotAgent::ms_start_rev_time;

SpotAgent::~SpotAgent() {
    stop();
}

void SpotAgent::setQuotationServer(const string& server) {
    ms_pubUrl = server;
}

void SpotAgent::start() {
    HKU_INFO(htr("Start spot agent."));
    stop();

    std::lock_guard<std::mutex> lock(m_run_mutex);
    if (m_stop) {
        m_stop = false;
        m_receive_data_tg = std::make_unique<ThreadPool>(1);
        m_tg = std::make_unique<ThreadPool>(m_work_num);
        m_receiveThread = std::thread([this]() { work_thread(); });
    }
}

void SpotAgent::stop() {
    std::lock_guard<std::mutex> lock(m_run_mutex);
    m_stop = true;
    if (m_receive_data_tg) {
        m_receive_data_tg->stop();
    }
    if (m_tg) {
        m_tg->stop();
    }
    if (m_receiveThread.joinable()) {
        m_receiveThread.join();
    }
    if (m_receive_data_tg) {
        m_receive_data_tg.reset();
    }
    if (m_tg) {
        m_tg.reset();
    }
    m_connected = false;
}

class ProcessTask {
public:
    ProcessTask(const std::function<void(const SpotRecord&)>& func, const SpotRecord& spot)
    : m_func(func), m_spot(spot) {}

    void operator()() {
        try {
            m_func(m_spot);
        } catch (const std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR_UNKNOWN;
        }
    }

private:
    std::function<void(const SpotRecord&)> m_func;
    SpotRecord m_spot;
};

unique_ptr<SpotRecord> SpotAgent::parseFlatSpot(const hikyuu::flat::Spot* spot) {
    SpotRecord* result = nullptr;
    try {
        result = new SpotRecord;
        if (spot->market())
            result->market = spot->market()->str();
        if (spot->code())
            result->code = spot->code()->str();
        if (spot->name())
            result->name = spot->name()->str();
        result->datetime = Datetime(spot->datetime()->str());
        result->yesterday_close = spot->yesterday_close();
        result->open = spot->open();
        result->high = spot->high();
        result->low = spot->low();
        result->close = spot->close();
        result->amount = spot->amount();
        result->volume = spot->volume();
        auto* bids = spot->bid();
        if (bids) {
            size_t length = bids->size();
            result->bid.resize(length);
            for (size_t i = 0; i < length; i++) {
                result->bid[i] = bids->Get(i);
            }
        }

        auto* bid_amounts = spot->bid_amount();
        if (bid_amounts) {
            size_t length = bid_amounts->size();
            result->bid_amount.resize(length);
            for (size_t i = 0; i < length; i++) {
                result->bid_amount[i] = bid_amounts->Get(i);
            }
        }

        auto* asks = spot->ask();
        if (asks) {
            size_t length = asks->size();
            result->ask.resize(length);
            for (size_t i = 0; i < length; i++) {
                result->ask[i] = asks->Get(i);
            }
        }

        auto* asks_amounts = spot->ask_amount();
        if (asks_amounts) {
            size_t length = asks_amounts->size();
            result->ask_amount.resize(length);
            for (size_t i = 0; i < length; i++) {
                result->ask_amount[i] = asks_amounts->Get(i);
            }
        }

    } catch (std::exception& e) {
        result = nullptr;
        HKU_ERROR(e.what());
    } catch (...) {
        result = nullptr;
        HKU_ERROR_UNKNOWN;
    }

    return unique_ptr<SpotRecord>(result);
}

void SpotAgent::parseSpotData(const void* buf, size_t buf_len) {
    // SPEND_TIME(receive_data);
    const uint8_t* spot_list_buf = (const uint8_t*)(buf) + ms_spotTopicLength;

    // 校验数据
    flatbuffers::Verifier verify(spot_list_buf, buf_len);
    HKU_CHECK(VerifySpotListBuffer(verify), "Invalid data!");

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

    // 更新K线数据
    auto* spot_list = GetSpotList(spot_list_buf);
    auto* spots = spot_list->spot();
    size_t total = spots->size();
    vector<std::future<void>> tasks;
    for (size_t i = 0; i < total; i++) {
        auto* spot = spots->Get(i);
        auto spot_record = parseFlatSpot(spot);
        if (spot_record) {
            for (const auto& process : m_processList) {
                tasks.emplace_back(m_tg->submit(ProcessTask(process, *spot_record)));
            }
        }
    }

    for (auto& task : tasks) {
        task.get();
    }
    HKU_DEBUG("received count: {}", total);
    for (const auto& postProcess : m_postProcessList) {
        postProcess(ms_start_rev_time);
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

void SpotAgent::work_thread() {
    nng_socket sock;

    int rv = nng_sub0_open(&sock);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Can't open nng sub0! {}", nng_strerror(rv));

    rv = nng_socket_set(sock, NNG_OPT_SUB_SUBSCRIBE, ms_spotTopic, ms_spotTopicLength);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Failed set nng socket option! {}", nng_strerror(rv));

    rv = nng_socket_set_ms(sock, NNG_OPT_RECVTIMEO, m_revTimeout);
    HKU_ERROR_IF_RETURN(rv != 0, void(), "Failed set receive timeout option!");

    rv = -1;
    Datetime pretime = Datetime::now();
    while (!m_stop && rv != 0) {
        rv = nng_dial(sock, ms_pubUrl.c_str(), nullptr, 0);
        auto now = Datetime::now();
        HKU_WARN_IF(m_print && rv != 0 && (now - pretime) > Seconds(5),
                    "Faied connect quotation server {}, will retry after 5 seconds!", ms_pubUrl);
        pretime = now;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    HKU_INFO_IF(!m_stop && m_print, "Ready to receive quotation from {} ...", ms_pubUrl);

    m_connected = true;
    while (!m_stop) {
        char* buf = nullptr;
        size_t length = 0;
        try {
            rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC);
            HKU_CHECK(rv == 0 || rv == NNG_ETIMEDOUT, "Failed nng_recv! {} ", nng_strerror(rv));
            if (!buf || length == 0) {
                continue;
            }
            switch (m_status) {
                case WAITING:
                    if (memcmp(buf, ms_startTag, ms_startTagLength) == 0) {
                        ms_start_rev_time = Datetime::now();
                        m_status = RECEIVING;
                    }
                    break;
                case RECEIVING:
                    if (memcmp(buf, ms_endTag, ms_endTagLength) == 0) {
                        m_status = WAITING;
                    } else if (memcmp(buf, ms_startTag, ms_startTagLength) != 0) {
                        std::shared_ptr<char[]> data_buf(new char[length]);
                        memcpy(data_buf.get(), buf, length);
                        m_receive_data_tg->submit([this, length, new_buf = std::move(data_buf)]() {
                            try {
                                this->parseSpotData(new_buf.get(), length);
                            } catch (const std::exception& e) {
                                HKU_ERROR(e.what());
                            } catch (...) {
                                HKU_ERROR_UNKNOWN;
                            }
                        });
                    }  // else {继续等待数据}
                    break;
            }
        } catch (std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR_UNKNOWN;
        }
        if (buf) {
            nng_free(buf, length);
        }
    }

    m_connected = false;
    nng_close(sock);
}

void SpotAgent::addProcess(std::function<void(const SpotRecord&)> process) {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_processList.push_back(process);
}

void SpotAgent::addPostProcess(std::function<void(Datetime)> func) {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_postProcessList.push_back(func);
}

void SpotAgent::clearProcessList() {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_processList.clear();
}

void SpotAgent::clearPostProcessList() {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_postProcessList.clear();
}

}  // namespace hku