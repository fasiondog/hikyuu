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
#include "receive.h"
#include <thread>

using namespace hikyuu::flat;

namespace hku {

void receive_data(const void* buf) {
    auto spot_list = GetSpotList(buf);
    auto* spots = spot_list->spot();
    size_t total = spots->size();
    for (size_t i = 0; i < total; i++) {
        HKU_INFO("{}{} {}", spots->Get(i)->market()->str(), spots->Get(i)->code()->str(),
                 spots->Get(i)->name()->str());
    }
}

void HKU_API start_receive() {
    string url{"ipc:///tmp/hikyuu_real_pub.ipc"};
    nng_socket sock;
    int rv = 0;
    try {
        HKU_CHECK((rv = nng_sub0_open(&sock)) == 0, "Can't open nng sub0! {}", nng_strerror(rv));
        HKU_CHECK((rv = nng_setopt(sock, NNG_OPT_SUB_SUBSCRIBE, "", 0)) == 0,
                  "Failed set nng socket option! {}", nng_strerror(rv));
        HKU_CHECK((rv = nng_dial(sock, url.c_str(), nullptr, 0) == 0),
                  "Faied nng_dial! Please start the collection service first.");

        HKU_INFO("已做好接收准备");
        while (true) {
            char* buf = NULL;
            size_t length = 0;
            HKU_CHECK(rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC) == 0,
                      "Failed nng_recv! {}", nng_strerror(rv));
            // HKU_INFO("length: {}", length);
            receive_data(buf);
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