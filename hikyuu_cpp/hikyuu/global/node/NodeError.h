/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-04-16
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/exception.h"

namespace hku {

/**
 * 节点返回码
 */
enum NodeErrorCode {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,  ///< 未知错误
    NNG_ERROR,          ///< nng内部错误
    MISSING_CMD,        ///< 缺失命令
    INVALID_CMD,        ///< 无效命令，没有相应的处理服务
};

class NodeError : public hku::exception {
public:
    NodeError() : NodeError(NodeErrorCode::UNKNOWN_ERROR, "Unknow error!") {}
    NodeError(NodeErrorCode errcode, const char* errmsg)
    : hku::exception(fmt::format("{} errcode: {}", errmsg, int(errcode))), m_errcode(errcode) {}
    NodeError(NodeErrorCode errcode, const std::string& errmsg)
    : hku::exception(errmsg), m_errcode(errcode) {}

    NodeErrorCode errcode() const {
        return m_errcode;
    }

private:
    NodeErrorCode m_errcode = NodeErrorCode::UNKNOWN_ERROR;
};

class NodeNngError : public NodeError {
public:
    NodeNngError() = delete;
    NodeNngError(int rv, const char* msg)
    : NodeError(NodeErrorCode::NNG_ERROR,
                fmt::format("{} nng error: {} (errcode: {})", msg, nng_strerror(rv), rv)) {}
    NodeNngError(int rv, const std::string& msg)
    : NodeError(NodeErrorCode::NNG_ERROR,
                fmt::format("{} nng error: {} (errcode: {})", msg, nng_strerror(rv), rv)) {}
};

#define NODE_CHECK(expr, errcode, ...)                          \
    {                                                           \
        if (!(expr)) {                                          \
            throw NodeError(errcode, fmt::format(__VA_ARGS__)); \
        }                                                       \
    }

#define NODE_NNG_CHECK(rv, ...)                               \
    {                                                         \
        if (rv != 0) {                                        \
            throw NodeNngError(rv, fmt::format(__VA_ARGS__)); \
        }                                                     \
    }

}  // namespace hku