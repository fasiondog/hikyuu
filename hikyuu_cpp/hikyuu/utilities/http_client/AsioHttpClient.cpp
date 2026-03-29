/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#include "AsioHttpClient.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/ResourceAsioPool.h"
#include "url.h"

#include <sstream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#if HKU_ENABLE_HTTP_CLIENT_SSL
#include <boost/asio/ssl.hpp>
#endif

#if HKU_ENABLE_HTTP_CLIENT_ZIP
#include "gzip/compress.hpp"
#include "gzip/decompress.hpp"
#endif

#if HKU_OS_OSX || HKU_OS_IOS
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

namespace hku {

#if HKU_ENABLE_HTTP_CLIENT_SSL
namespace ssl = net::ssl;
#endif

AsioHttpResponse::AsioHttpResponse(AsioHttpResponse&& rhs)
: m_status(rhs.m_status),
  m_reason(std::move(rhs.m_reason)),
  m_body(std::move(rhs.m_body)),
  m_headers(std::move(rhs.m_headers)) {}

AsioHttpResponse& AsioHttpResponse::operator=(AsioHttpResponse&& rhs) noexcept {
    if (this != &rhs) {
        m_status = rhs.m_status;
        m_reason = std::move(rhs.m_reason);
        m_body = std::move(rhs.m_body);
        m_headers = std::move(rhs.m_headers);
    }
    return *this;
}

AsioHttpStreamResponse::AsioHttpStreamResponse(AsioHttpStreamResponse&& rhs)
: m_status(rhs.m_status),
  m_reason(std::move(rhs.m_reason)),
  m_headers(std::move(rhs.m_headers)),
  m_total_bytes_read(rhs.m_total_bytes_read) {}

AsioHttpStreamResponse& AsioHttpStreamResponse::operator=(AsioHttpStreamResponse&& rhs) noexcept {
    if (this != &rhs) {
        m_status = rhs.m_status;
        m_reason = std::move(rhs.m_reason);
        m_headers = std::move(rhs.m_headers);
        m_total_bytes_read = rhs.m_total_bytes_read;
    }
    return *this;
}

// HttpConnection 类定义 - 用于连接池的可复用连接
struct HttpConnection : public AsyncResourceWithVersion {
    using SocketType = tcp::socket;

    std::vector<tcp::endpoint> endpoints;                  // DNS 解析结果缓存
    std::chrono::steady_clock::time_point last_used_time;  // 最后使用时间

    // socket 在连接被获取时创建
    std::optional<SocketType> socket;

#if HKU_ENABLE_HTTP_CLIENT_SSL
    std::optional<ssl::stream<tcp::socket>> ssl_socket;

    HttpConnection(const Parameter& params) : last_used_time(std::chrono::steady_clock::now()) {}

    ~HttpConnection() {
        close();
    }

    void close() {
        if (ssl_socket) {
            boost::system::error_code ec;
            ssl_socket->lowest_layer().close(ec);
            ssl_socket.reset();
        }
        if (socket) {
            boost::system::error_code ec;
            socket->close(ec);
            socket.reset();
        }
    }

    bool is_open() const {
        if (ssl_socket) {
            return ssl_socket->lowest_layer().is_open();
        } else if (socket) {
            return socket->is_open();
        }
        return false;
    }

    SocketType& lowest_layer() {
        if (ssl_socket) {
            return static_cast<SocketType&>(ssl_socket->lowest_layer());
        } else if (socket) {
            return *socket;
        }
        HKU_THROW("Socket not initialized");
    }
#else
    HttpConnection(const Parameter& params) {
        last_used_time = std::chrono::steady_clock::now();
    }

    ~HttpConnection() {
        close();
    }

    void close() {
        if (socket) {
            boost::system::error_code ec;
            socket->close(ec);
            socket.reset();
        }
    }

    bool is_open() const {
        return socket && socket->is_open();
    }

    SocketType& lowest_layer() {
        if (socket) {
            return *socket;
        }
        HKU_THROW("Socket not initialized");
    }
#endif
};

#if HKU_ENABLE_HTTP_CLIENT_SSL
struct AsioHttpClient::SslContext {
    net::ssl::context ssl_ctx;

    SslContext() : ssl_ctx(net::ssl::context::tls_client) {
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_options(net::ssl::context::default_workarounds | net::ssl::context::no_sslv2 |
                            net::ssl::context::no_sslv3 | net::ssl::context::no_tlsv1 |
                            net::ssl::context::no_tlsv1_1);
        // 使用 OpenSSL 原生 API 设置最低 TLS 版本
        SSL_CTX_set_min_proto_version(ssl_ctx.native_handle(), TLS1_2_VERSION);
    }

    /**
     * @brief 设置自定义 CA 证书文件
     * @param ca_file CA 证书文件路径（PEM 格式）
     */
    void setCaFile(const std::string& ca_file) {
        if (!ca_file.empty()) {
            ssl_ctx.load_verify_file(ca_file);
        }
    }
};
#endif

AsioHttpClient::AsioHttpClient(int32_t thread_count, size_t max_concurrency)
: m_own_ctx(std::make_unique<net::io_context>()), m_ctx(m_own_ctx.get()) {
    // 创建工作守护，防止 io_context 在无任务时退出
    m_work_guard = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(
      m_own_ctx->get_executor());

#if HKU_ENABLE_HTTP_CLIENT_SSL
    m_ssl_ctx = std::make_unique<SslContext>();
#endif

    // 初始化连接池
    m_connection_pool =
      std::make_unique<ResourceAsioVersionPool<HttpConnection>>(Parameter(), max_concurrency);

    // 使用内部 io_context，启动工作线程池运行事件循环
    m_worker_threads.reserve(thread_count);
    for (int32_t i = 0; i < thread_count; ++i) {
        m_worker_threads.emplace_back([this] { m_ctx->run(); });
    }
}

AsioHttpClient::AsioHttpClient(const std::string& url, int32_t timeout, int32_t thread_count,
                               size_t max_concurrency)
: m_url(url),
  m_timeout(std::chrono::milliseconds(timeout <= 0 ? MAX_TIMEOUT_MS : timeout)),
  m_own_ctx(std::make_unique<net::io_context>()),
  m_ctx(m_own_ctx.get()) {
    _parseUrl();

    if (m_is_valid_url && m_ctx) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
        m_ssl_ctx = std::make_unique<SslContext>();
#endif

        // 创建工作守护，防止 io_context 在无任务时退出
        m_work_guard = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(
          m_own_ctx->get_executor());

        // 初始化连接池参数
        m_connection_pool =
          std::make_unique<ResourceAsioVersionPool<HttpConnection>>(Parameter(), max_concurrency);

        // 启动后台线程池运行 io_context
        m_worker_threads.reserve(thread_count);
        for (int32_t i = 0; i < thread_count; ++i) {
            m_worker_threads.emplace_back([this]() { m_ctx->run(); });
        }
    }
}

AsioHttpClient::AsioHttpClient(net::io_context& ctx, const std::string& url, int32_t timeout,
                               size_t max_concurrency)
: m_url(url),
  m_timeout(std::chrono::milliseconds(timeout <= 0 ? MAX_TIMEOUT_MS : timeout)),
  m_ctx(&ctx),  // 使用外部 io_context，不拥有所有权
  m_worker_threads() {
    _parseUrl();

    if (m_is_valid_url && m_ctx) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
        m_ssl_ctx = std::make_unique<SslContext>();
#endif

        // 初始化连接池参数
        m_connection_pool =
          std::make_unique<ResourceAsioVersionPool<HttpConnection>>(Parameter(), max_concurrency);
    }
}

AsioHttpClient::~AsioHttpClient() {
    if (m_own_ctx) {
        m_work_guard.reset();

        if (!m_own_ctx->stopped()) {
            m_own_ctx->stop();
        }

        m_connection_pool.reset();

        // 等待所有工作线程结束
        for (auto& thread : m_worker_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_own_ctx.reset();
    }

    m_connection_pool.reset();
    m_ctx = nullptr;
}

void AsioHttpClient::setCaFile(const std::string& filename) {
    m_ca_file = filename;
#if HKU_ENABLE_HTTP_CLIENT_SSL
    if (m_ssl_ctx) {
        m_ssl_ctx->setCaFile(filename);
    }
#endif
}

void AsioHttpClient::setTimeout(int32_t ms) {
    if (ms <= 0) {
        ms = MAX_TIMEOUT_MS;
    }
    auto new_timeout = std::chrono::milliseconds(ms);
    if (m_timeout != new_timeout) {
        m_timeout = new_timeout;
        // 超时时间变更时更新连接池参数，自动递增版本号
        if (m_connection_pool) {
            Parameter pool_param;
            m_connection_pool->setParameter(std::move(pool_param));
        }
    }
}

void AsioHttpClient::setUrl(const std::string& url) {
    m_url = url;

    // 保存旧的 host 和 port 用于比较
    std::string old_host = m_host;
    std::string old_port = m_port;

    // 解析 URL
    _parseUrl();

    // 如果解析失败，不更新连接池
    if (!m_is_valid_url) {
        return;
    }

    // 检查 host 或 port 是否变化，如果变化则更新连接池参数（自动递增版本）
    bool host_changed = (old_host != m_host || old_port != m_port);

    if (host_changed && m_connection_pool) {
        Parameter pool_param;
        // 设置新参数，资源池会自动递增版本并释放空闲的旧版本连接
        m_connection_pool->setParameter(std::move(pool_param));
    }
}

void AsioHttpClient::_parseUrl() noexcept {
    size_t pos = m_url.find("://");
    if (pos == std::string::npos) {
        m_is_valid_url = false;
        return;
    }

    m_is_valid_url = true;
    std::string proto = m_url.substr(0, pos);

    uint16_t port;
    if (proto == "http") {
        port = 80;
    } else if (proto == "https") {
        port = 443;
#if HKU_ENABLE_HTTP_CLIENT_SSL
        m_is_https = true;
#else
        HKU_ERROR("Not support https protocol, please enable http_client_ssl feature!");
#endif
    } else {
        m_is_valid_url = false;
        HKU_ERROR("Invalid protocol: {}", proto);
        return;
    }

    std::string base_path;
    std::string host = m_url.substr(pos + 3);
    pos = host.find('/');
    if (pos != std::string::npos) {
        base_path = host.substr(pos);
        host = host.substr(0, pos);
    }
    pos = host.find(':');
    if (pos != std::string::npos) {
        try {
            port = std::stoi(host.substr(pos + 1));
        } catch (...) {
            m_is_valid_url = false;
            HKU_ERROR("Invalid port: {}", host.substr(pos + 1));
            return;
        }
        host = host.substr(0, pos);
    }

    m_base_path = std::move(base_path);
    m_host = std::move(host);
    m_port = std::to_string(port);
}

// URI 构建辅助方法
std::string AsioHttpClient::_buildURI(const std::string& path, const HttpParams& params) {
    std::ostringstream uri_stream;

    // 处理 base_path：如果为空或仅为 "/"，则不添加
    if (!m_base_path.empty() && m_base_path != "/") {
        uri_stream << m_base_path;
    }

    if (!path.empty()) {
        // 判断是否需要添加分隔符 /
        bool need_separator = false;
        if (!m_base_path.empty() && m_base_path != "/") {
            // base_path 不为空且不是单斜杠时，检查是否需要添加分隔符
            need_separator = (m_base_path.back() != '/' && path.front() != '/');
        } else {
            // base_path 为空或单斜杠时，仅当 path 不以 '/' 开头才需要添加
            need_separator = (path.front() != '/');
        }

        if (need_separator) {
            uri_stream << '/';
        }

        // 对 path 进行分段 URL 编码（保持 / 作为路径分隔符不编码）
        // 例如："ipquery/index" → 编码为 "ipquery/index"
        //      "/api/v1/users" → 编码为 "/api/v1/users"
        std::string_view path_view = path;
        size_t pos = 0;

        while (pos < path_view.size()) {
            // 找到下一个 / 的位置
            size_t slash_pos = path_view.find('/', pos);

            if (slash_pos == std::string_view::npos) {
                // 最后一个段（或整个 path 没有 /）
                std::string segment = std::string(path_view.substr(pos));
                if (!segment.empty()) {
                    uri_stream << url_escape(segment.c_str());
                }
                break;
            } else {
                // 提取并编码当前段（不包括 /）
                if (slash_pos > pos) {
                    std::string segment = std::string(path_view.substr(pos, slash_pos - pos));
                    uri_stream << url_escape(segment.c_str());
                }
                // 保留原始的 / 分隔符
                uri_stream << '/';
                pos = slash_pos + 1;
            }
        }

        // 处理末尾的 /
        if (!path.empty() && path.back() == '/') {
            uri_stream << '/';
        }
    }

    // 添加查询参数
    bool first = true;
    for (const auto& [key, value] : params) {
        uri_stream << (first ? "?" : "&");
        // 对 key 和 value 分别进行 URL 编码
        uri_stream << url_escape(key.c_str()) << "=" << url_escape(value.c_str());
        first = false;
    }

    return uri_stream.str();
}

// 异步 DNS 解析方法
net::awaitable<std::vector<tcp::endpoint>> AsioHttpClient::_resolveDNS() {
#if HKU_OS_OSX || HKU_OS_IOS
    // macOS 使用原生 getaddrinfo 方式（beast 解析存在已知问题会卡死）
    struct addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &res);
    HKU_CHECK(ret == 0, "DNS resolve failed!");

    std::vector<tcp::endpoint> dns_endpoints;
    for (struct addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        if (ai->ai_family == AF_INET) {
            auto* sin = reinterpret_cast<sockaddr_in*>(ai->ai_addr);
            net::ip::address_v4::bytes_type v4_bytes;
            std::memcpy(&v4_bytes, &(sin->sin_addr.s_addr), sizeof(v4_bytes));
            dns_endpoints.push_back(
              tcp::endpoint(net::ip::make_address_v4(v4_bytes), ntohs(sin->sin_port)));
        } else if (ai->ai_family == AF_INET6) {
            auto* sin6 = reinterpret_cast<sockaddr_in6*>(ai->ai_addr);
            net::ip::address_v6::bytes_type v6_bytes;
            std::memcpy(&v6_bytes, &(sin6->sin6_addr.s6_addr), sizeof(v6_bytes));
            dns_endpoints.push_back(
              tcp::endpoint(net::ip::make_address_v6(v6_bytes), ntohs(sin6->sin6_port)));
        }
    }

    freeaddrinfo(res);

    if (dns_endpoints.empty()) {
        HKU_THROW("No valid endpoints from DNS resolve");
    }

    co_return dns_endpoints;

#else
    // 其他平台使用 Boost.ASIO 异步 DNS解析
    auto resolver = tcp::resolver{*m_ctx};

    struct ResolveOp {
        tcp::resolver& resolver;
        std::string host, port;
        tcp::resolver::results_type endpoints;
        boost::system::error_code ec;
        bool done = false;

        ResolveOp(tcp::resolver& r, const std::string& h, const std::string& p)
        : resolver(r), host(h), port(p) {}

        net::awaitable<boost::system::error_code> run() {
            auto [e, eps] =
              co_await resolver.async_resolve(host, port, net::as_tuple(net::use_awaitable));
            ec = e;
            endpoints = std::move(eps);
            done = true;
            co_return e;
        }
    };

    auto op = std::make_shared<ResolveOp>(resolver, m_host, m_port);

    // 启动定时器和 DNS解析
    auto timer = net::steady_timer{*m_ctx};
    timer.expires_after(m_timeout);

    timer.async_wait([&resolver, &op](const boost::system::error_code& ec) {
        if (!ec && !op->done) {
            // 超时后取消 resolver 的所有异步操作
            resolver.cancel();
        }
    });

    auto resolve_result = co_await op->run();

    // 取消定时器
    timer.cancel();

    // 检查是否因超时而取消（operation_aborted 表示被 cancel() 取消）
    if (resolve_result == boost::asio::error::operation_aborted) {
        HKU_THROW_EXCEPTION(HttpTimeoutException, "DNS resolve timeout");
    }

    if (resolve_result) {
        HKU_THROW("DNS resolve failed: {}", resolve_result.message());
    }

    // 转换为 endpoint 列表
    std::vector<tcp::endpoint> dns_endpoints;
    for (const auto& ep : op->endpoints) {
        dns_endpoints.push_back(ep.endpoint());
    }

    if (dns_endpoints.empty()) {
        HKU_THROW("No valid endpoints from DNS resolve");
    }

    co_return dns_endpoints;
#endif
}

// 从连接池获取已连接的连接（带 DNS 缓存）
net::awaitable<std::pair<std::shared_ptr<HttpConnection>, bool>> AsioHttpClient::_getConnection() {
    HKU_ASSERT(m_connection_pool != nullptr);

    // 从池中获取连接（资源池自动进行版本检查，旧版本连接会被自动淘汰）
    auto conn_ptr = co_await m_connection_pool->get();
    HKU_CHECK(conn_ptr != nullptr, "Failed to get connection from pool");

    bool is_new_connection = false;

    // 检查连接是否需要重新创建
    if (!conn_ptr->is_open()) {
        // 连接已关闭，需要重新创建
        is_new_connection = true;

        // 如果 DNS 缓存为空或超时（5 分钟），重新解析 DNS
        auto now = std::chrono::steady_clock::now();
        bool need_dns_resolve = conn_ptr->endpoints.empty();

        if (!need_dns_resolve) {
            auto elapsed =
              std::chrono::duration_cast<std::chrono::minutes>(now - conn_ptr->last_used_time)
                .count();
            if (elapsed > 5) {
                need_dns_resolve = true;
            }
        }

        if (need_dns_resolve) {
            // DNS 解析（带超时）
            conn_ptr->endpoints = co_await _resolveDNS();
        }

        // 关闭旧连接（如果有）
        conn_ptr->close();

        if (m_is_https) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
            // 连接到服务器
            bool connected = false;
            for (const auto& endpoint : conn_ptr->endpoints) {
                conn_ptr->ssl_socket.emplace(*m_ctx, m_ssl_ctx->ssl_ctx);
                SSL_set_tlsext_host_name(conn_ptr->ssl_socket->native_handle(), m_host.c_str());

                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool connect_completed = false;
                boost::system::error_code captured_ec;

                struct ConnectOp {
                    tcp::socket* socket;
                    tcp::endpoint endpoint;
                    bool& completed_flag;
                    boost::system::error_code& captured_ec;

                    net::awaitable<boost::system::error_code> run() {
                        auto [ec] = co_await socket->async_connect(
                          endpoint, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec
                          ? ec
                          : (socket->is_open()
                               ? boost::system::errc::make_error_code(boost::system::errc::success)
                               : boost::system::errc::make_error_code(
                                   boost::system::errc::not_connected));
                    }
                };

                // 启动定时器和连接操作
                timer.async_wait(
                  [&timer, &connect_completed, &conn_ptr](const boost::system::error_code& ec) {
                      if (!ec && !connect_completed && conn_ptr->ssl_socket.has_value()) {
                          conn_ptr->ssl_socket->lowest_layer().cancel();
                      }
                  });

                ConnectOp connect_op{&conn_ptr->ssl_socket->next_layer(), endpoint,
                                     connect_completed, captured_ec};
                co_await connect_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消
                if (captured_ec == boost::asio::error::operation_aborted) {
                    break;  // 超时后不再尝试其他 endpoint
                }

                // 检查连接是否成功
                if (!captured_ec && conn_ptr->ssl_socket->lowest_layer().is_open()) {
                    connected = true;
                    break;
                }

                // 连接失败，重置并继续尝试下一个 endpoint
                conn_ptr->ssl_socket.reset();
            }

            if (!connected) {
                HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host,
                                    m_port);
            }

            // 设置 socket 选项
            conn_ptr->ssl_socket->lowest_layer().set_option(tcp::no_delay(true));

            // SSL 握手（带超时）
            {
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool handshake_completed = false;
                boost::system::error_code captured_ec;

                struct SslHandshakeOp {
                    ssl::stream<tcp::socket>* stream;
                    bool& completed_flag;
                    boost::system::error_code& captured_ec;

                    net::awaitable<boost::system::error_code> run() {
                        auto [ec] = co_await stream->async_handshake(
                          ssl::stream_base::client, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec;
                    }
                };

                // 启动定时器和握手操作
                timer.async_wait(
                  [&timer, &handshake_completed, &conn_ptr](const boost::system::error_code& ec) {
                      if (!ec && !handshake_completed && conn_ptr->ssl_socket.has_value()) {
                          conn_ptr->ssl_socket->lowest_layer().cancel();
                      }
                  });

                SslHandshakeOp handshake_op{&conn_ptr->ssl_socket.value(), handshake_completed,
                                            captured_ec};
                auto handshake_result = co_await handshake_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消
                if (captured_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "SSL handshake timeout");
                }

                if (captured_ec) {
                    HKU_THROW("SSL handshake failed: {}", captured_ec.message());
                }
            }
#endif
        } else {
            bool connected = false;
            for (const auto& endpoint : conn_ptr->endpoints) {
                // 普通 HTTP 连接（SSL 已启用但当前使用 HTTP）
                conn_ptr->socket.emplace(*m_ctx);

                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool connect_completed = false;
                boost::system::error_code captured_ec;

                struct ConnectOp {
                    tcp::socket* socket;
                    tcp::endpoint endpoint;
                    bool& completed_flag;
                    boost::system::error_code& captured_ec;

                    net::awaitable<boost::system::error_code> run() {
                        auto [ec] = co_await socket->async_connect(
                          endpoint, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec
                          ? ec
                          : (socket->is_open()
                               ? boost::system::errc::make_error_code(boost::system::errc::success)
                               : boost::system::errc::make_error_code(
                                   boost::system::errc::not_connected));
                    }
                };

                // 启动定时器和连接操作
                timer.async_wait(
                  [&connect_completed, &conn_ptr](const boost::system::error_code& ec) {
                      if (!ec && !connect_completed && conn_ptr->socket.has_value()) {
                          conn_ptr->socket->cancel();
                      }
                  });

                ConnectOp connect_op{&conn_ptr->socket.value(), endpoint, connect_completed,
                                     captured_ec};
                co_await connect_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消
                if (captured_ec == boost::asio::error::operation_aborted) {
                    break;  // 超时后不再尝试其他 endpoint
                }

                // 检查连接是否成功
                if (!captured_ec && conn_ptr->socket->is_open()) {
                    connected = true;
                    break;
                }

                // 连接失败但未超时，继续尝试下一个 endpoint

                // 关闭并重置 socket 以便下一次尝试
                boost::system::error_code ec;
                conn_ptr->socket->close(ec);
                conn_ptr->socket.reset();
            }

            if (!connected) {
                HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host,
                                    m_port);
            }

            // 设置 socket 选项
            conn_ptr->socket->set_option(tcp::no_delay(true));
        }

        // 更新最后使用时间
        conn_ptr->last_used_time = std::chrono::steady_clock::now();
    }
    // 复用已有连接，无需额外操作，连接池会自动管理其生命周期
    // 只需在每次使用后更新 last_used_time 即可
    conn_ptr->last_used_time = std::chrono::steady_clock::now();

    // 复用的连接不需要在此处再次设置 socket 选项，
    // 因为它们在 _connect 或之前的连接建立时已经设置过。
    // 如果连接被复用，其 socket 状态是保持的。

    co_return std::make_pair(conn_ptr, is_new_connection);
}

// 创建 socket（使用 variant 存储普通 socket 或 SSL socket）
struct AsioHttpClient::SocketVariant {
    std::optional<tcp::socket> plain;
#if HKU_ENABLE_HTTP_CLIENT_SSL
    std::optional<ssl::stream<tcp::socket>> ssl;

    void close(boost::system::error_code& ec) {
        if (plain) {
            plain->close(ec);
            plain.reset();
        }
        if (ssl) {
            ssl->lowest_layer().close(ec);
            ssl.reset();
        }
    }

    bool is_ssl() const {
        return ssl.has_value();
    }

    tcp::socket& socket() {
        if (ssl) {
            return ssl->next_layer();
        } else if (plain) {
            return *plain;
        }
        HKU_THROW("Socket not initialized");
    }
#else
    void close(boost::system::error_code& ec) {
        if (plain) {
            plain->close(ec);
            plain.reset();
        }
    }

    bool is_ssl() const {
        return false;
    }

    tcp::socket& socket() {
        if (plain) {
            return *plain;
        }
        HKU_THROW("Socket not initialized");
    }
#endif
};

net::awaitable<void> AsioHttpClient::_connect(SocketVariant& socket_variant,
                                              const std::vector<tcp::endpoint>& dns_endpoints) {
    // 连接（带超时）
    boost::system::error_code connect_ec;
    bool connected = false;

    for (const auto& endpoint : dns_endpoints) {
        socket_variant.close(connect_ec);

        {
            // 先创建普通 socket
            socket_variant.plain.emplace(*m_ctx);

            // 使用事件驱动异步连接配合超时
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool connect_completed = false;
            boost::system::error_code captured_ec;

            struct ConnectOp {
                tcp::socket* socket;
                tcp::endpoint endpoint;
                bool& completed_flag;
                boost::system::error_code& captured_ec;

                net::awaitable<boost::system::error_code> run() {
                    auto [ec] =
                      co_await socket->async_connect(endpoint, net::as_tuple(net::use_awaitable));
                    completed_flag = true;
                    captured_ec = ec;
                    co_return ec
                      ? ec
                      : (socket->is_open()
                           ? boost::system::errc::make_error_code(boost::system::errc::success)
                           : boost::system::errc::make_error_code(
                               boost::system::errc::not_connected));
                }
            };

            // 启动定时器和连接操作
            timer.async_wait(
              [&connect_completed, &socket_variant](const boost::system::error_code& ec) {
                  if (!ec && !connect_completed && socket_variant.plain.has_value()) {
                      socket_variant.plain->cancel();
                  }
              });

            ConnectOp connect_op{&socket_variant.plain.value(), endpoint, connect_completed,
                                 captured_ec};

            // 等待连接完成
            co_await connect_op.run();

            // 取消定时器
            timer.cancel();

            // 检查是否因超时而取消
            if (captured_ec == boost::asio::error::operation_aborted) {
                continue;  // 尝试下一个端点
            }

            // 检查连接是否成功
            if (!captured_ec && socket_variant.plain->is_open()) {
                connected = true;
                break;
            }

            // 连接失败，关闭 socket
            socket_variant.close(connect_ec);
        }
    }

    if (!connected) {
        HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host, m_port);
    }

    // 设置 socket 选项
    socket_variant.socket().set_option(tcp::no_delay(true));

#if HKU_ENABLE_HTTP_CLIENT_SSL
    // 如果是 HTTPS，进行 SSL 握手（带超时）
    if (m_is_https) {
        // 移动到 SSL socket
        socket_variant.ssl.emplace(std::move(*socket_variant.plain), m_ssl_ctx->ssl_ctx);
        socket_variant.plain.reset();

        // 设置 SNI（Server Name Indication）
        SSL_set_tlsext_host_name(socket_variant.ssl->native_handle(), m_host.c_str());

        // 使用事件驱动的 SSL 握手配合超时
        auto timer = net::steady_timer{*m_ctx};
        timer.expires_after(m_timeout);

        bool handshake_completed = false;
        boost::system::error_code captured_ec;

        struct SslHandshakeOp {
            ssl::stream<tcp::socket>* stream;
            bool& completed_flag;
            boost::system::error_code& captured_ec;

            net::awaitable<boost::system::error_code> run() {
                auto [ec] = co_await stream->async_handshake(ssl::stream_base::client,
                                                             net::as_tuple(net::use_awaitable));
                completed_flag = true;
                captured_ec = ec;
                co_return ec;
            }
        };

        // 启动定时器和握手操作
        timer.async_wait(
          [&timer, &handshake_completed, &socket_variant](const boost::system::error_code& ec) {
              if (!ec && !handshake_completed && socket_variant.ssl.has_value()) {
                  socket_variant.ssl->lowest_layer().cancel();
              }
          });

        SslHandshakeOp handshake_op{&socket_variant.ssl.value(), handshake_completed, captured_ec};
        auto handshake_result = co_await handshake_op.run();

        // 取消定时器
        timer.cancel();

        // 检查是否因超时而取消
        if (captured_ec == boost::asio::error::operation_aborted) {
            HKU_THROW_EXCEPTION(HttpTimeoutException, "SSL handshake timeout");
        }

        if (captured_ec) {
            HKU_THROW("SSL handshake failed: {}", captured_ec.message());
        }
    }
#endif

    co_return;
}

net::awaitable<AsioHttpResponse> AsioHttpClient::async_request(
  const std::string& method, const std::string& path, const HttpParams& params,
  const HttpHeaders& headers, const char* body, size_t body_len, const std::string& content_type) {
    HKU_CHECK(m_is_valid_url, "Invalid url: {}", m_url);

    // 确保 io_context 已设置（默认构造函数可能没有初始化）
    if (m_ctx == nullptr) {
        auto exec = co_await net::this_coro::executor;
        m_ctx = &static_cast<net::io_context&>(exec.context());
        HKU_CHECK(m_ctx != nullptr, "Cannot get io_context from execution context");
    }

#if !HKU_ENABLE_HTTP_CLIENT_SSL
    HKU_CHECK(!m_is_https,
              "HTTPS is not supported. Please enable SSL support with --http_client_ssl=y");
#endif

    // 构建完整的 URI
    std::string uri = _buildURI(path, params);

    AsioHttpResponse response;

    try {
        // 从连接池获取连接（自动处理 DNS 缓存和连接复用）
        auto [conn, is_new] = co_await _getConnection();
        HKU_CHECK(conn != nullptr, "Failed to get connection from pool");

        // 创建 HTTP 请求
        http::request<http::string_body> req;
        req.method(http::string_to_verb(method));
        req.target(uri);
        req.version(11);  // HTTP/1.1

        // 添加默认头
        for (const auto& [key, value] : m_default_headers) {
            req.set(key, value);
        }

        // 添加额外头
        for (const auto& [key, value] : headers) {
            req.set(key, value);
        }

        // 添加 User-Agent
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::host, m_host);
        // 注意：不使用 "close"，允许连接复用

        // 添加请求体
        if (body != nullptr && body_len > 0) {
#if HKU_ENABLE_HTTP_CLIENT_ZIP
            auto content_type = req["Content-Type"];
            if (content_type == "gzip") {
                gzip::Compressor comp(Z_DEFAULT_COMPRESSION);
                std::string output;
                comp.compress(output, body, body_len);
                req.body() = std::move(output);
            } else {
                req.set(http::field::content_type, content_type);
                req.body() = std::string(body, body_len);
                req.prepare_payload();
            }
#else
            req.set(http::field::content_type, content_type);
            req.body() = std::string(body, body_len);
            req.prepare_payload();
#endif
        }

        // 发送请求（带超时）
        {
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool write_completed = false;

#if HKU_ENABLE_HTTP_CLIENT_SSL
            if (conn->ssl_socket) {
                struct WriteOp {
                    ssl::stream<tcp::socket>& stream;
                    http::request<http::string_body>& req;
                    bool& completed_flag;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_write(
                          stream, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // 启动定时器和写操作
                timer.async_wait(
                  [&timer, &write_completed, &conn](const boost::system::error_code& ec) {
                      if (!ec && !write_completed && conn->is_open()) {
                          conn->lowest_layer().cancel();
                      }
                  });

                auto write_op = WriteOp{*conn->ssl_socket, req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消（operation_aborted 表示被 cancel() 取消）
                if (write_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP write timeout");
                }

                if (write_ec) {
                    HKU_THROW("HTTP write failed: {}", write_ec.message());
                }
            } else {
#endif
                struct WriteOp {
                    tcp::socket& sock;
                    http::request<http::string_body>& req;
                    bool& completed_flag;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] =
                          co_await http::async_write(sock, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // 启动定时器和写操作
                timer.async_wait([&write_completed, &conn](const boost::system::error_code& ec) {
                    if (!ec && !write_completed) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto write_op = WriteOp{conn->socket.value(), req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消（operation_aborted 表示被 cancel() 取消）
                if (write_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP write timeout");
                }

                if (write_ec) {
                    HKU_THROW("HTTP write failed: {}", write_ec.message());
                }
#if HKU_ENABLE_HTTP_CLIENT_SSL
            }
#endif
        }

        // 读取响应（带超时）
        beast::flat_buffer buffer;
        http::response<http::string_body> res;

        {
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool read_completed = false;
            boost::system::error_code captured_ec;

#if HKU_ENABLE_HTTP_CLIENT_SSL
            if (conn->ssl_socket) {
                struct ReadOp {
                    ssl::stream<tcp::socket>& stream;
                    beast::flat_buffer& buffer;
                    http::response<http::string_body>& response;
                    bool& completed_flag;
                    boost::system::error_code& captured_ec;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_read(
                          stream, buffer, response, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // 启动定时器和读操作
                timer.async_wait(
                  [&timer, &read_completed, &conn](const boost::system::error_code& ec) {
                      if (!ec && !read_completed && conn->is_open()) {
                          conn->lowest_layer().cancel();
                      }
                  });

                auto read_op = ReadOp{*conn->ssl_socket, buffer, res, read_completed, captured_ec};
                co_await read_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消（operation_aborted 表示被 cancel() 取消）
                if (captured_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP read timeout");
                }

                if (captured_ec) {
                    HKU_THROW("HTTP read failed: {}", captured_ec.message());
                }
            } else {
#endif
                struct ReadOp {
                    tcp::socket& sock;
                    beast::flat_buffer& buffer;
                    http::response<http::string_body>& response;
                    bool& completed_flag;
                    boost::system::error_code& captured_ec;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_read(
                          sock, buffer, response, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // 启动定时器和读操作
                timer.async_wait([&read_completed, &conn](const boost::system::error_code& ec) {
                    if (!ec && !read_completed) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto read_op =
                  ReadOp{conn->socket.value(), buffer, res, read_completed, captured_ec};
                co_await read_op.run();

                // 取消定时器
                timer.cancel();

                // 检查是否因超时而取消（operation_aborted 表示被 cancel() 取消）
                if (captured_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP read timeout");
                }

                if (captured_ec) {
                    HKU_THROW("HTTP read failed: {}", captured_ec.message());
                }
#if HKU_ENABLE_HTTP_CLIENT_SSL
            }
#endif
        }

        // 填充响应对象
        response.m_status = res.result_int();
        response.m_reason = std::string(res.reason());

#if HKU_ENABLE_HTTP_CLIENT_ZIP
        // 正确获取 Content-Encoding 头部
        auto encoding_it = res.find("Content-Encoding");
        if (encoding_it != res.end() && encoding_it->value() == "gzip") {
            response.m_body = gzip::decompress(res.body().data(), res.body().size());
        } else {
            response.m_body = std::move(res.body());
        }
#else
        response.m_body = std::move(res.body());
#endif

        for (auto it = res.begin(); it != res.end(); ++it) {
            response.m_headers.emplace(std::string(it->name_string()), std::string(it->value()));
        }

        // 不关闭连接，让连接池自动管理

    } catch (const boost::system::system_error& e) {
        HKU_ERROR("HTTP request system error! {}", e.what());
        throw;
    } catch (const std::exception& e) {
        HKU_ERROR("HTTP request failed! {}", e.what());
        throw;
    }

    co_return response;
}

net::awaitable<AsioHttpStreamResponse> AsioHttpClient::async_requestStream(
  const std::string& method, const std::string& path, const HttpParams& params,
  const HttpHeaders& headers, const char* body, size_t body_len, const std::string& content_type,
  const HttpChunkCallback& chunk_callback) {
    HKU_CHECK(m_is_valid_url, "Invalid url: {}", m_url);
    HKU_CHECK(chunk_callback != nullptr, "Chunk callback must not be null");

    // 确保 io_context 已设置（默认构造函数可能没有初始化）
    if (m_ctx == nullptr) {
        auto exec = co_await net::this_coro::executor;
        m_ctx = &static_cast<net::io_context&>(exec.context());
        HKU_CHECK(m_ctx != nullptr, "Cannot get io_context from execution context");
    }

#if !HKU_ENABLE_HTTP_CLIENT_SSL
    HKU_CHECK(!m_is_https,
              "HTTPS is not supported. Please enable SSL support with --http_client_ssl=y");
#endif

    // 构建完整的 URI
    std::string uri = _buildURI(path, params);

    AsioHttpStreamResponse response;

    try {
        // 从连接池获取连接（自动处理 DNS 缓存和连接复用）
        auto [conn, is_new] = co_await _getConnection();
        HKU_CHECK(conn != nullptr, "Failed to get connection from pool");

        // 创建 HTTP 请求
        http::request<http::string_body> req;
        req.method(http::string_to_verb(method));
        req.target(uri);
        req.version(11);

        for (const auto& [key, value] : m_default_headers) {
            req.set(key, value);
        }

        for (const auto& [key, value] : headers) {
            req.set(key, value);
        }

        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::host, m_host);
        // 注意：不使用 "close"，允许连接复用

        if (body != nullptr && body_len > 0) {
#if HKU_ENABLE_HTTP_CLIENT_ZIP
            auto content_type = req["Content-Type"];
            if (content_type == "gzip") {
                gzip::Compressor comp(Z_DEFAULT_COMPRESSION);
                std::string output;
                comp.compress(output, body, body_len);
                req.body() = std::move(output);
            } else {
                req.set(http::field::content_type, content_type);
                req.body() = std::string(body, body_len);
                req.prepare_payload();
            }
#else
            req.set(http::field::content_type, content_type);
            req.body() = std::string(body, body_len);
            req.prepare_payload();
#endif
        }

        // 发送请求 - 使用事件驱动方式
        {
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool write_completed = false;

#if HKU_ENABLE_HTTP_CLIENT_SSL
            if (conn->ssl_socket) {
                struct WriteOp {
                    ssl::stream<tcp::socket>& stream;
                    http::request<http::string_body>& req;
                    bool& completed_flag;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_write(
                          stream, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                auto write_op = WriteOp{*conn->ssl_socket, req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // 检查是否因超时而取消
                if (!write_completed && write_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP write timeout");
                }

                if (write_ec) {
                    HKU_THROW("HTTP write failed: {}", write_ec.message());
                }
            } else {
#endif
                struct WriteOp {
                    tcp::socket& sock;
                    http::request<http::string_body>& req;
                    bool& completed_flag;

                    net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                        auto [ec, bytes] =
                          co_await http::async_write(sock, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                auto write_op = WriteOp{conn->socket.value(), req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // 检查是否因超时而取消
                if (!write_completed && write_ec == boost::asio::error::operation_aborted) {
                    HKU_THROW_EXCEPTION(HttpTimeoutException, "HTTP write timeout");
                }

                if (write_ec) {
                    HKU_THROW("HTTP write failed: {}", write_ec.message());
                }
#if HKU_ENABLE_HTTP_CLIENT_SSL
            }
#endif
        }

        // 流式读取响应 - 使用 buffer_body
        beast::flat_buffer buffer;
        http::response_parser<http::buffer_body> parser;

        // 设置缓冲区大小（8KB 块）
        constexpr size_t BUFFER_SIZE = 8192;
        std::vector<char> chunk_buffer(BUFFER_SIZE);

        parser.get().body().data = chunk_buffer.data();
        parser.get().body().size = BUFFER_SIZE;

        {
            // 读取响应头 - 使用事件驱动方式
            {
                // 设置超时定时器
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                // 启动定时器，超时则取消底层 socket
                timer.async_wait([&conn](const boost::system::error_code& ec) {
                    if (!ec) {
                        conn->lowest_layer().cancel();
                    }
                });

#if HKU_ENABLE_HTTP_CLIENT_SSL
                if (conn->ssl_socket) {
                    struct ReadHeaderOp {
                        ssl::stream<tcp::socket>& stream;
                        beast::flat_buffer& buffer;
                        http::response_parser<http::buffer_body>& parser;

                        net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read_header(
                              stream, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_header_op = ReadHeaderOp{*conn->ssl_socket, buffer, parser};
                    auto [read_ec, bytes_transferred] = co_await read_header_op.run();

                    // 取消定时器
                    timer.cancel();

                    if (read_ec && read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read header failed: {}", read_ec.message());
                    }
                } else {
#endif
                    struct ReadHeaderOp {
                        tcp::socket& sock;
                        beast::flat_buffer& buffer;
                        http::response_parser<http::buffer_body>& parser;

                        net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read_header(
                              sock, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_header_op = ReadHeaderOp{conn->socket.value(), buffer, parser};
                    auto [read_ec, bytes_transferred] = co_await read_header_op.run();

                    // 取消定时器
                    timer.cancel();

                    if (read_ec && read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read header failed: {}", read_ec.message());
                    }
#if HKU_ENABLE_HTTP_CLIENT_SSL
                }
#endif
            }

            // 填充响应对象
            response.m_status = static_cast<int>(parser.get().result_int());
            response.m_reason = std::string(parser.get().reason());
            for (auto it = parser.get().begin(); it != parser.get().end(); ++it) {
                response.m_headers.emplace(std::string(it->name_string()),
                                           std::string(it->value()));
            }

            // 循环读取响应体数据块
            while (!parser.is_done()) {
                std::size_t bytes_transferred = 0;
                boost::system::error_code read_ec;

                // 设置超时定时器（每次读取块都重置）
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                // 启动定时器，超时则取消底层 socket
                timer.async_wait([&conn](const boost::system::error_code& ec) {
                    if (!ec) {
                        conn->lowest_layer().cancel();
                    }
                });

#if HKU_ENABLE_HTTP_CLIENT_SSL
                if (conn->ssl_socket) {
                    struct ReadOp {
                        ssl::stream<tcp::socket>& stream;
                        beast::flat_buffer& buffer;
                        http::response_parser<http::buffer_body>& parser;

                        net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read(
                              stream, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_op = ReadOp{*conn->ssl_socket, buffer, parser};
                    auto [ec, bytes] = co_await read_op.run();
                    read_ec = ec;
                    bytes_transferred = bytes;

                    // 取消定时器
                    timer.cancel();

                    if (read_ec && read_ec != http::error::need_buffer &&
                        read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read body failed: {}", read_ec.message());
                    }
                } else {
#endif
                    struct ReadOp {
                        tcp::socket& sock;
                        beast::flat_buffer& buffer;
                        http::response_parser<http::buffer_body>& parser;

                        net::awaitable<std::pair<boost::system::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read(
                              sock, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_op = ReadOp{conn->socket.value(), buffer, parser};
                    auto [ec, bytes] = co_await read_op.run();
                    read_ec = ec;
                    bytes_transferred = bytes;

                    // 取消定时器
                    timer.cancel();

                    if (read_ec && read_ec != http::error::need_buffer &&
                        read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read body failed: {}", read_ec.message());
                    }
#if HKU_ENABLE_HTTP_CLIENT_SSL
                }
#endif

                // 调用回调处理数据块
                if (bytes_transferred > 0) {
                    response.m_total_bytes_read += bytes_transferred;
                    chunk_callback(chunk_buffer.data(), bytes_transferred);
                }

                // 重置缓冲区供下一次读取使用
                parser.get().body().data = chunk_buffer.data();
                parser.get().body().size = BUFFER_SIZE;
            }
        }

        // 不关闭连接，让连接池自动管理（连接会被归还到池中）

    } catch (const boost::system::system_error& e) {
        HKU_ERROR("HTTP stream request system error! {}", e.what());
        throw;
    } catch (const std::exception& e) {
        HKU_ERROR("HTTP stream request failed! {}", e.what());
        throw;
    }

    co_return response;
}

// ============================================================================
// 同步方法实现 - 阻塞直到异步操作完成
// ============================================================================

AsioHttpResponse AsioHttpClient::request(const std::string& method, const std::string& path,
                                         const HttpParams& params, const HttpHeaders& headers,
                                         const char* body, size_t body_len,
                                         const std::string& content_type) {
    HKU_ASSERT(m_ctx);
    if (m_ctx->stopped()) {
        m_ctx->restart();
    }

    auto future =
      co_spawn(*m_ctx, async_request(method, path, params, headers, body, body_len, content_type),
               boost::asio::use_future);  // 使用use_future代替detached以更好地管理future

    return future.get();
}

AsioHttpStreamResponse AsioHttpClient::requestStream(
  const std::string& method, const std::string& path, const HttpParams& params,
  const HttpHeaders& headers, const char* body, size_t body_len, const std::string& content_type,
  const HttpChunkCallback& chunk_callback) {
    HKU_ASSERT(m_ctx);
    if (m_ctx->stopped()) {
        m_ctx->restart();
    }

    auto future =
      co_spawn(*m_ctx,
               async_requestStream(method, path, params, headers, body, body_len, content_type,
                                   chunk_callback),
               boost::asio::use_future);  // 使用use_future代替detached以更好地管理future

    return future.get();
}

}  // namespace hku
