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

#define NODE_STATUS_TIMEOUT 150  ///< 节点状态超时时长（秒），超时认为连接中断
#define NODE_STATUS_INTERVAL 60  ///< 发送状态的间隔时间（秒）（心跳）

/*
 * 消息格式
 * req ->
 *  {"cmd": int, ...}
 *
 * <- res
 *  {"ret": code, "msg": str} // msg 存在错误时返回错误信息 （可选）
 *
 *
 */

/**
 * 对消息进行解码，消息类型和消息体必须匹配
 * @tparam T 消息体类型
 * @param msg 消息
 * @exception NodeErrorCode 消息类型不匹配
 */
inline json decodeMsg(nng_msg *msg) {
    HKU_ASSERT(msg != nullptr);
    size_t len = nng_msg_len(msg);
    uint8_t *data = (uint8_t *)nng_msg_body(msg);
    json result = json::from_msgpack(data, data + len);
    return result;
}

/**
 * @brief 消息编码
 * @tparam T 消息体类型
 * @param msg 消息
 * @param in 编码输入
 * @exception NodeNngError nng 操作失败
 * @exception yas::io_exception yas 序列化异常
 */
inline void encodeMsg(nng_msg *msg, const json &in) {
    HKU_ASSERT(msg != nullptr);
    nng_msg_clear(msg);

    std::vector<std::uint8_t> v = json::to_msgpack(in);
    int rv = nng_msg_append(msg, v.data(), v.size());
    NODE_NNG_CHECK(rv, "Failed nng_msg_append!");
}

/**
 * 构造错误消息响应
 * @param msg [out] 消息
 * @param errcode 错误码
 * @param errmsg 错误消息
 */
inline void errorMsg(nng_msg *msg, NodeErrorCode errcode, const std::string &errmsg) {
    json res;
    res["ret"] = errcode;
    res["msg"] = errmsg;
    encodeMsg(msg, res);
}

}  // namespace hku
