/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-03-27
 *      Author: fasiondog
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nng/nng.h>
#include <nlohmann/json.hpp>
#include <hikyuu/utilities/Log.h>
#include "NodeError.h"

using json = nlohmann::json;

namespace hku {

#define NODE_STATUS_TIMEOUT \
    150                          ///< The node status timeout in seconds; the connection is regarded
                                 ///< as interrupted on a timeout
#define NODE_STATUS_INTERVAL 60  ///< The interval of sending the status in seconds (the heartbeat)

/*
 * Message format
 * req ->
 *  {"cmd": int, ...}
 *
 * <- res
 *  {"ret": code, "msg": str}  // msg is the error information returned on an error (optional)
 *
 *
 */

/**
 * Decode the message; the message type and the message body must match
 * @tparam T the message body type
 * @param msg message
 * @exception NodeErrorCode the message type does not match
 */
inline json decodeMsg(nng_msg *msg) {
    HKU_ASSERT(msg != nullptr);
    size_t len = nng_msg_len(msg);
    uint8_t *data = (uint8_t *)nng_msg_body(msg);
    json result = json::from_msgpack(data, data + len);
    return result;
}

/**
 * @brief Message encoding
 * @tparam T the message body type
 * @param msg message
 * @param in the encoding input
 * @exception NodeNngError the nng operation failed
 * @exception yas::io_exception the yas serialization exception
 */
inline void encodeMsg(nng_msg *msg, const json &in) {
    HKU_ASSERT(msg != nullptr);
    nng_msg_clear(msg);

    std::vector<std::uint8_t> v = json::to_msgpack(in);
    int rv = nng_msg_append(msg, v.data(), v.size());
    NODE_NNG_CHECK(rv, "Failed nng_msg_append!");
}

/**
 * Build an error message response
 * @param msg [out] message
 * @param errcode error code
 * @param errmsg error message
 */
inline void errorMsg(nng_msg *msg, NodeErrorCode errcode, const std::string &errmsg) {
    json res;
    res["ret"] = errcode;
    res["msg"] = errmsg;
    encodeMsg(msg, res);
}

}  // namespace hku
