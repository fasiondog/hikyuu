/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-16
 *     Author: fasiondog
 */

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include "../flatbuffers/spot_generated.h"
#include "../utilities/util.h"
#include "../StockManager.h"
#include "receive.h"

using namespace hikyuu::flat;

namespace hku {

static const char* s_spot_topic = "spot";
static int s_spot_topic_len = 4;

static void receive_data(const void* buf, size_t buf_len, bool print) {
    // SPEND_TIME(receive_data);
    const uint8_t* spot_list_buf = (const uint8_t*)(buf) + s_spot_topic_len;

    // 校验数据
    flatbuffers::Verifier verify(spot_list_buf, buf_len);
    HKU_CHECK(VerifySpotListBuffer(verify), "Invalid data!");

    // 更新日线数据
    auto* spot_list = GetSpotList(spot_list_buf);
    auto* spots = spot_list->spot();
    size_t total = spots->size();
    auto& sm = StockManager::instance();
    HKU_INFO_IF(print, "received total {}", total);
    for (size_t i = 0; i < total; i++) {
        auto* spot = spots->Get(i);
        std::stringstream market_code_buf;
        market_code_buf << spot->market()->str() << spot->code()->str();
        Stock stk = sm[market_code_buf.str()];
        if (stk.isNull())
            continue;

        KRecord krecord(Datetime(spot->datetime()->str()), spot->open(), spot->high(), spot->low(),
                        spot->close(), spot->amount(), spot->volumn());
        stk.realtimeUpdate(krecord, KQuery::DAY);
    }
}

void HKU_API start_receive() {
    string url{"ipc:///tmp/hikyuu_real_pub.ipc"};
    nng_socket sock;
    int rv = 0;
    try {
        HKU_CHECK((rv = nng_sub0_open(&sock)) == 0, "Can't open nng sub0! {}", nng_strerror(rv));
        HKU_CHECK(
          (rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, s_spot_topic, s_spot_topic_len)) == 0,
          "Failed set nng socket option! {}", nng_strerror(rv));
        HKU_CHECK((rv = nng_dial(sock, url.c_str(), nullptr, 0) == 0),
                  "Faied nng_dial! Please start the collection service first.");

        HKU_INFO("Ready to receive ...");
        while (true) {
            char* buf = NULL;
            size_t length = 0;
            HKU_CHECK(rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC) == 0,
                      "Failed nng_recv! {}", nng_strerror(rv));
            // HKU_INFO("received length: {}", length);
            receive_data(buf, length, true);
            nng_free(buf, length);
        }
    } catch (std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR("Unknow error!");
    }
    nng_close(sock);
}

}  // namespace hku