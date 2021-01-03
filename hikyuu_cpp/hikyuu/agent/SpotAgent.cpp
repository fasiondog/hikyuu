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
    stop();
}

void SpotAgent::start() {
    if (m_stop) {
        m_stop = false;
        m_receiveThread = std::thread(&SpotAgent::work_thread, this);
    }
}

void SpotAgent::stop() {
    m_stop = true;
    if (m_receiveThread.joinable()) {
        m_receiveThread.join();
    }
}

class ProcessTask {
public:
    ProcessTask(std::function<void(const SpotRecord&)> func, const SpotRecord& spot)
    : m_func(func), m_spot(spot) {}

    void operator()() {
        m_func(m_spot);
    }

private:
    std::function<void(const SpotRecord&)> m_func;
    SpotRecord m_spot;
};

unique_ptr<SpotRecord> SpotAgent::parseFlatSpot(const hikyuu::flat::Spot* spot) {
    SpotRecord* result = nullptr;
    try {
        result = new SpotRecord;
        result->market = spot->market()->str();
        result->code = spot->code()->str();
        result->name = spot->name()->str();
        result->datetime = Datetime(spot->datetime()->str());
        result->yesterday_close = spot->yesterday_close();
        result->open = spot->open();
        result->high = spot->high();
        result->low = spot->low();
        result->close = spot->close();
        result->amount = spot->amount();
        result->volumn = spot->volumn();
        result->bid1 = spot->bid1();
        result->bid1_amount = spot->bid1_amount();
        result->bid2 = spot->bid2();
        result->bid2_amount = spot->bid2_amount();
        result->bid3 = spot->bid3();
        result->bid3_amount = spot->bid3_amount();
        result->bid4 = spot->bid4();
        result->bid4_amount = spot->bid4_amount();
        result->bid5 = spot->bid5();
        result->bid5_amount = spot->bid5_amount();
        result->ask1 = spot->ask1();
        result->ask1_amount = spot->ask1_amount();
        result->ask2 = spot->ask2();
        result->ask2_amount = spot->ask2_amount();
        result->ask3 = spot->ask3();
        result->ask3_amount = spot->ask3_amount();
        result->ask4 = spot->ask4();
        result->ask4_amount = spot->ask4_amount();
        result->ask5 = spot->ask5();
        result->ask5_amount = spot->ask5_amount();

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

    // 更新日线数据
    auto* spot_list = GetSpotList(spot_list_buf);
    auto* spots = spot_list->spot();
    size_t total = spots->size();
    m_batch_count += total;
    for (size_t i = 0; i < total; i++) {
        auto* spot = spots->Get(i);
        for (auto& process : m_processList) {
            auto spot_record = parseFlatSpot(spot);
            if (spot_record) {
                m_process_task_list.push_back(m_tg.submit(ProcessTask(process, *spot_record)));
            }
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
                    if (memcmp(buf, ms_endTag, ms_endTagLength) == 0) {
                        m_status = WAITING;
                        for (auto& task : m_process_task_list) {
                            task.get();
                        }
                        HKU_INFO_IF(m_print, "received count: {}", m_batch_count);
                        m_batch_count = 0;
                        // 执行后处理
                        for (auto& postProcess : m_postProcessList) {
                            postProcess();
                        }
                        m_process_task_list.clear();
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
            HKU_ERROR_UNKNOWN;
        }
        if (buf) {
            nng_free(buf, length);
        }
    }

    nng_close(sock);
}

void SpotAgent::addProcess(std::function<void(const SpotRecord&)> process) {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    m_processList.push_back(process);
}

void SpotAgent::addPostProcess(std::function<void()> func) {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    m_postProcessList.push_back(func);
}

void SpotAgent::clearProcessList() {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    m_processList.clear();
}

void SpotAgent::clearPostProcessList() {
    HKU_CHECK(m_stop, "SpotAgent is running, please stop agent first!");
    m_postProcessList.clear();
}

static string getSpotMarketCode(const SpotRecord& spot) {
    std::stringstream market_code_buf;
    market_code_buf << spot.market << spot.code;
    return market_code_buf.str();
}

static void updateStockDayData(const SpotRecord& spot) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());
    KRecord krecord(Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day()),
                    spot.open, spot.high, spot.low, spot.close, spot.amount, spot.volumn);
    stk.realtimeUpdate(krecord, KQuery::DAY);
}

static void updateStockWeekData(const SpotRecord& spot) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());

    std::function<Datetime(Datetime*)> endOfPhase = &Datetime::endOfWeek;
    std::function<Datetime(Datetime*)> startOfPhase = &Datetime::startOfWeek;

    Datetime spot_day = Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day());
    Datetime spot_end_of_week = endOfPhase(&spot_day) - TimeDelta(2);  // 周五日期
    KQuery::KType ktype(KQuery::WEEK);
    size_t total = stk.getCount(ktype);

    // 没有历史数据，则直接更新并返回
    if (total == 0) {
        stk.realtimeUpdate(KRecord(spot_end_of_week, spot.open, spot.high, spot.low, spot.close,
                                   spot.amount, spot.volumn),
                           ktype);
        return;
    }

    KRecord last_record = stk.getKRecord(total - 1, ktype);
    if (spot_end_of_week > last_record.datetime) {
        // 如果当前的日期大于最后记录的日期，则为新增数据，直接更新并返回
        stk.realtimeUpdate(KRecord(spot_end_of_week, spot.open, spot.high, spot.low, spot.close,
                                   spot.amount, spot.volumn),
                           ktype);

    } else if (spot_end_of_week == last_record.datetime) {
        // 如果当前日期等于最后记录的日期，则需要重新计算最高价、最低价、交易金额、交易量
        Datetime spot_start_of_week = startOfPhase(&spot_day);
        KRecordList klist =
          stk.getKRecordList(KQuery(spot_start_of_week, spot_end_of_week + TimeDelta(1), ktype));
        price_t amount = 0.0, volumn = 0.0;
        for (auto& k : klist) {
            amount += k.transAmount;
            volumn += k.transCount;
        }
        stk.realtimeUpdate(
          KRecord(spot_end_of_week, last_record.openPrice,
                  spot.high > last_record.highPrice ? spot.high : last_record.highPrice,
                  spot.low < last_record.lowPrice ? spot.low : last_record.lowPrice, spot.close,
                  amount, volumn),
          ktype);
    } else {
        // 不应该出现的情况：当前日期小于最后记录的日期
        HKU_WARN(
          "Ignore, What should not happen, the current date is less than the last recorded date!");
    }
}

static void updateStockMinData(const SpotRecord& spot) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());

    KQuery::KType ktype(KQuery::MIN);
    Datetime minute = Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day(),
                               spot.datetime.hour(), spot.datetime.minute());
    KRecordList klist = stk.getKRecordList(KQuery(minute, minute + TimeDelta(0, 0, 1), ktype));
    price_t sum_amount = 0.0, sum_volumn = 0.0;
    for (auto& k : klist) {
        sum_amount += k.transAmount;
        sum_volumn += k.transCount;
    }

    price_t amount = spot.amount > sum_amount ? spot.amount - sum_amount : spot.amount;
    price_t volumn = spot.volumn > sum_volumn ? spot.volumn - sum_volumn : spot.volumn;
    KRecord krecord(Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day(),
                             spot.datetime.hour(), spot.datetime.minute()),
                    spot.open, spot.high, spot.low, spot.close, amount, volumn);
    stk.realtimeUpdate(krecord, ktype);
}

void HKU_API start_spot_agent(bool print) {
    auto& agent = SpotAgent::instance();
    HKU_CHECK(!agent.isRunning(), "The agent is running, please stop first!");

    agent.setPrintFlag(print);

    const auto& preloadParam = StockManager::instance().getPreloadParameter();
    if (preloadParam.tryGet<bool>("min", false)) {
        agent.addProcess(updateStockMinData);
    }

    if (preloadParam.tryGet<bool>("day", false)) {
        agent.addProcess(updateStockDayData);
    }

    if (preloadParam.tryGet<bool>("week", false)) {
        agent.addProcess(updateStockWeekData);
    }

    agent.start();
}

void HKU_API stop_spot_agent() {
    auto& agent = SpotAgent::instance();
    agent.stop();
}

}  // namespace hku