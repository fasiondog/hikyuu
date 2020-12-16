/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-16
 *     Author: fasiondog
 */

#include <nng/nng.h>
#include <nng/protocol/pubsub0/sub.h>
#include "../flatbuffers/spot_generated.h"
#include "receive.h"

namespace hku {

void receive_data() {}

void HKU_API start_receive() {
    string url{"ipc:///tmp/hikyuu_real_pub.ipc"};
    nng_socket sock;
    int rv = 0;
    HKU_CHECK((rv = nng_sub0_open(&sock)) == 0, "Can't open nng sub0! {}", nng_strerror(rv));
    HKU_CHECK((rv = nng_socket_set_string(sock, NNG_OPT_SUB_SUBSCRIBE, "")) == 0,
              "Failed set nng socket option! {}", nng_strerror(rv));
    HKU_CHECK((rv = nng_dial(sock, url.c_str(), nullptr, 0) == 0), "Faied nng_dial! {}",
              nng_strerror(rv));

    // while (true) {
    for (int i = 0; i < 3; i++) {
        char* buf = NULL;
        size_t length = 0;
        HKU_INFO("{}", i);
        HKU_CHECK(rv = nng_recv(sock, &buf, &length, NNG_FLAG_ALLOC) == 0, "Failed nng_recv! {}",
                  nng_strerror(rv));
        nng_free(buf, length);
    }
    nng_close(sock);
}

}  // namespace hku