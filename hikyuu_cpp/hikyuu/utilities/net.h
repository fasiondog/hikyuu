/*
 * net.h - Network abstraction layer for Asio
 *
 * Copyright (c) 2025 hikyuu.org
 *
 * Created on: 2025-05-07
 *     Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_NET_H
#define HKU_UTILS_NET_H

#include "hikyuu/utilities/config.h"

/**
 * @brief 网络抽象层
 *
 * 提供统一的 Asio 接口，支持在 Boost.Asio 和独立 Asio 之间切换。
 * 通过条件编译宏 HKU_USE_BOOST_ASIO 控制使用哪个实现：
 * - HKU_USE_BOOST_ASIO = 1 (默认): 使用 Boost.Asio
 * - HKU_USE_BOOST_ASIO = 0: 使用独立 Asio
 *
 * @example
 * @code
 * #include "hikyuu/utilities/net.h"
 *
 * namespace net = hku::net;
 * using tcp = net::tcp;
 *
 * net::io_context ctx;
 * net::steady_timer timer(ctx);
 * @endcode
 */

// 默认使用 Boost.Asio
#ifndef HKU_USE_BOOST_ASIO
#define HKU_USE_BOOST_ASIO 1
#endif

#if HKU_USE_BOOST_ASIO
// 使用 Boost.Asio
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/any_io_executor.hpp>

namespace hku {
namespace net {
// 核心类型别名
using io_context = boost::asio::io_context;
using executor_type = boost::asio::any_io_executor;
using strand = boost::asio::strand<boost::asio::io_context::executor_type>;
namespace asio = boost::asio;

// Work guard
template <typename Executor>
using executor_work_guard = boost::asio::executor_work_guard<Executor>;

// TCP 相关
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
using udp = boost::asio::ip::udp;

// Timer 相关
using steady_timer = boost::asio::steady_timer;
using system_timer = boost::asio::system_timer;

// Buffer 相关
using mutable_buffer = boost::asio::mutable_buffer;
using const_buffer = boost::asio::const_buffer;

// Error 相关
using error_code = boost::system::error_code;
using system_error = boost::system::system_error;
namespace error = boost::asio::error;

// Coroutine 相关
template <typename T>
using awaitable = boost::asio::awaitable<T>;

// this_coro 是命名空间，不是类型
namespace this_coro = boost::asio::this_coro;

// 常用函数和对象包装
inline auto use_awaitable = boost::asio::use_awaitable;
inline auto detached = boost::asio::detached;
inline auto as_tuple = boost::asio::as_tuple;

template <typename Executor, typename F>
inline void co_spawn(const Executor& ex, F&& f) {
    boost::asio::co_spawn(ex, std::forward<F>(f), boost::asio::detached);
}

template <typename Executor, typename F>
inline void co_spawn(const Executor& ex, F&& f, boost::system::error_code& ec) {
    boost::asio::co_spawn(ex, std::forward<F>(f),
                          boost::asio::redirect_error(boost::asio::detached, ec));
}

// Asio 常用函数转发
template <typename... Args>
inline auto redirect_error(Args&&... args)
  -> decltype(boost::asio::redirect_error(std::forward<Args>(args)...)) {
    return boost::asio::redirect_error(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto post(Args&&... args) -> decltype(boost::asio::post(std::forward<Args>(args)...)) {
    return boost::asio::post(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto async_initiate(Args&&... args)
  -> decltype(boost::asio::async_initiate(std::forward<Args>(args)...)) {
    return boost::asio::async_initiate(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto get_associated_executor(Args&&... args)
  -> decltype(boost::asio::get_associated_executor(std::forward<Args>(args)...)) {
    return boost::asio::get_associated_executor(std::forward<Args>(args)...);
}

// Socket 类型
using tcp_socket = boost::asio::ip::tcp::socket;
using tcp_acceptor = boost::asio::ip::tcp::acceptor;
using tcp_resolver = boost::asio::ip::tcp::resolver;

using udp_socket = boost::asio::ip::udp::socket;
using udp_resolver = boost::asio::ip::udp::resolver;

}  // namespace net
}  // namespace hku

#else
// 使用独立 Asio
#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/awaitable.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/as_tuple.hpp>
#include <asio/steady_timer.hpp>
#include <asio/system_timer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ip/udp.hpp>
#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <asio/strand.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/any_io_executor.hpp>

namespace hku {
namespace net {
// 核心类型别名
using io_context = asio::io_context;
using executor_type = asio::any_io_executor;
using strand = asio::strand<asio::io_context::executor_type>;

// Work guard
template <typename Executor>
using executor_work_guard = asio::executor_work_guard<Executor>;

// TCP 相关
namespace ip = asio::ip;
using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

// Timer 相关
using steady_timer = asio::steady_timer;
using system_timer = asio::system_timer;

// Buffer 相关
using mutable_buffer = asio::mutable_buffer;
using const_buffer = asio::const_buffer;

// Error 相关
using error_code = asio::error_code;
using system_error = asio::system_error;
namespace error = asio::error;

// Coroutine 相关
template <typename T>
using awaitable = asio::awaitable<T>;

// this_coro 是命名空间，不是类型
namespace this_coro = asio::this_coro;

// 常用函数和对象包装
inline auto use_awaitable = asio::use_awaitable;
inline auto detached = asio::detached;
inline auto as_tuple = asio::as_tuple;

template <typename Executor, typename F>
inline void co_spawn(const Executor& ex, F&& f) {
    asio::co_spawn(ex, std::forward<F>(f), asio::detached);
}

template <typename Executor, typename F>
inline void co_spawn(const Executor& ex, F&& f, asio::error_code& ec) {
    asio::co_spawn(ex, std::forward<F>(f), asio::redirect_error(asio::detached, ec));
}

// Asio 常用函数转发
template <typename... Args>
inline auto redirect_error(Args&&... args)
  -> decltype(asio::redirect_error(std::forward<Args>(args)...)) {
    return asio::redirect_error(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto post(Args&&... args) -> decltype(asio::post(std::forward<Args>(args)...)) {
    return asio::post(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto async_initiate(Args&&... args)
  -> decltype(asio::async_initiate(std::forward<Args>(args)...)) {
    return asio::async_initiate(std::forward<Args>(args)...);
}

template <typename... Args>
inline auto get_associated_executor(Args&&... args)
  -> decltype(asio::get_associated_executor(std::forward<Args>(args)...)) {
    return asio::get_associated_executor(std::forward<Args>(args)...);
}

// Socket 类型
using tcp_socket = asio::ip::tcp::socket;
using tcp_acceptor = asio::ip::tcp::acceptor;
using tcp_resolver = asio::ip::tcp::resolver;

using udp_socket = asio::ip::udp::socket;
using udp_resolver = asio::ip::udp::resolver;

}  // namespace net
}  // namespace hku

#endif

#endif  // HKU_UTILS_NET_H
