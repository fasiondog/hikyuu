/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#include "AsioHttpClient.h"
#include "hikyuu/utilities/net.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/os.h"
#include "url.h"

#include <sstream>
#include <cstring>
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

namespace beast = boost::beast;
namespace http = boost::beast::http;
using tcp = net::ip::tcp;

#if HKU_ENABLE_HTTP_CLIENT_SSL
namespace ssl = boost::asio::ssl;
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
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

json AsioHttpResponse::json() const {
    auto content_type = getHeader("Content-Type");
    if (content_type.find("application/json") != std::string::npos) {
        return json::parse(m_body);
    }
    if (content_type.find("application/msgpack") != std::string::npos) {
        return json::from_msgpack(m_body);
    }
    if (content_type.find("application/cbor") != std::string::npos) {
        return json::from_cbor(m_body);
    }
    return json::parse(m_body);
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

// The HttpConnection class definition - a reusable connection for the connection pool
struct HttpConnection {
    using SocketType = tcp::socket;

    std::vector<tcp::endpoint> endpoints;                  // The DNS resolution result cache
    std::chrono::steady_clock::time_point last_used_time;  // Last used time
    int m_version = 0;                                     // Version number

    // The socket is created when the connection is acquired
    std::optional<SocketType> socket;

    // The version management interface
    int getVersion() const {
        return m_version;
    }

    void setVersion(int version) {
        m_version = version;
    }

#if HKU_ENABLE_HTTP_CLIENT_SSL
    std::optional<ssl::stream<tcp::socket>> ssl_socket;

    explicit HttpConnection(const Parameter& params)
    : last_used_time(std::chrono::steady_clock::now()) {}

    ~HttpConnection() {
        close();
    }

    void close() {
        if (ssl_socket) {
            net::error_code ec;
            ssl_socket->lowest_layer().close(ec);
            ssl_socket.reset();
        }
        if (socket) {
            net::error_code ec;
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
    explicit HttpConnection(const Parameter& params)
    : last_used_time(std::chrono::steady_clock::now()) {}

    ~HttpConnection() {
        close();
    }

    void close() {
        if (socket) {
            net::error_code ec;
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
    ssl::context ssl_ctx;

    SslContext() : ssl_ctx(ssl::context::tls_client) {
        ssl_ctx.set_default_verify_paths();
        ssl_ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 |
                            ssl::context::no_sslv3 | ssl::context::no_tlsv1 |
                            ssl::context::no_tlsv1_1);
        // Set the minimum TLS version with the native OpenSSL API
        SSL_CTX_set_min_proto_version(ssl_ctx.native_handle(), TLS1_2_VERSION);
    }

    /**
     * @brief Set the custom CA certificate file
     * @param ca_file the CA certificate file path (the PEM format)
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
    // Create a work guard to prevent the io_context from exiting when there is no task
    m_work_guard = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(
      m_own_ctx->get_executor());

#if HKU_ENABLE_HTTP_CLIENT_SSL
    m_ssl_ctx = std::make_unique<SslContext>();
#endif

    // Initialize the connection pool
    m_connection_pool = std::make_unique<ResourceAsioVersionPool<HttpConnection, std::mutex>>(
      Parameter(), max_concurrency);

    // Use the internal io_context and start the worker thread pool to run the event loop
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

        // Create a work guard to prevent the io_context from exiting when there is no task
        m_work_guard = std::make_unique<net::executor_work_guard<net::io_context::executor_type>>(
          m_own_ctx->get_executor());

        // Initialize the connection pool parameters (AsioHttpClient may use multiple threads, so
        // std::mutex is used uniformly for safety)
        m_connection_pool = std::make_unique<ResourceAsioVersionPool<HttpConnection, std::mutex>>(
          Parameter(), max_concurrency);

        // Start the background thread pool to run the io_context
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
  m_ctx(&ctx),  // Use the external io_context without taking the ownership
  m_worker_threads() {
    _parseUrl();

    if (m_is_valid_url && m_ctx) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
        m_ssl_ctx = std::make_unique<SslContext>();
#endif

        // Initialize the connection pool parameters (an external io_context is used and the caller
        // guarantees the thread safety, so std::mutex is used conservatively here)
        m_connection_pool = std::make_unique<ResourceAsioVersionPool<HttpConnection, std::mutex>>(
          Parameter(), max_concurrency);
    }
}

AsioHttpClient::~AsioHttpClient() {
    if (m_own_ctx) {
        m_work_guard.reset();

        if (!m_own_ctx->stopped()) {
            m_own_ctx->stop();
        }

        m_connection_pool.reset();

        // Wait for all the worker threads to finish
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
        // Update the connection pool parameters when the timeout changes, the version is increased
        // automatically
        if (m_connection_pool) {
            Parameter pool_param;
            m_connection_pool->setParameter(std::move(pool_param));
        }
    }
}

void AsioHttpClient::setUrl(const std::string& url) {
    m_url = url;

    // Save the old host and port for the comparison
    std::string old_host = m_host;
    std::string old_port = m_port;

    // Parse the URL
    _parseUrl();

    // The connection pool is not updated when the parsing fails
    if (!m_is_valid_url) {
        return;
    }

    // Check whether the host or the port changed; if so update the connection pool parameters (the
    // version is increased automatically)
    bool host_changed = (old_host != m_host || old_port != m_port);

    if (host_changed && m_connection_pool) {
        Parameter pool_param;
        // Set the new parameters; the resource pool increases the version and releases the idle old
        // version connections automatically
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
        host.resize(pos);
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
        host.resize(pos);
    }

    m_base_path = std::move(base_path);
    m_host = std::move(host);
    m_port = std::to_string(port);
}

// The URI construction helper method
std::string AsioHttpClient::_buildURI(const std::string& path, const HttpParams& params) {
    std::ostringstream uri_stream;

    // Handle base_path: it is not added when it is empty or just "/"
    if (!m_base_path.empty() && m_base_path != "/") {
        uri_stream << m_base_path;
    }

    if (!path.empty()) {
        // Judge whether the separator / needs to be added
        bool need_separator = false;
        if (!m_base_path.empty() && m_base_path != "/") {
            // When base_path is not empty and is not a single slash, check whether the separator
            // needs to be added
            need_separator = (m_base_path.back() != '/' && path.front() != '/');
        } else {
            // When base_path is empty or a single slash, it is needed only when path does not start
            // with '/'
            need_separator = (path.front() != '/');
        }

        if (need_separator) {
            uri_stream << '/';
        }

        // URL encode the path segment by segment (keeping / as a path separator, not encoded)
        // For example: "ipquery/index" -> encoded as "ipquery/index"
        //      "/api/v1/users" -> encoded as "/api/v1/users"
        std::string_view path_view = path;
        size_t pos = 0;

        while (pos < path_view.size()) {
            // Find the position of the next /
            size_t slash_pos = path_view.find('/', pos);

            if (slash_pos == std::string_view::npos) {
                // The last segment (or the whole path has no /)
                std::string segment = std::string(path_view.substr(pos));
                if (!segment.empty()) {
                    uri_stream << url_escape(segment.c_str());
                }
                break;
            } else {
                // Extract and encode the current segment (excluding the /)
                if (slash_pos > pos) {
                    std::string segment = std::string(path_view.substr(pos, slash_pos - pos));
                    uri_stream << url_escape(segment.c_str());
                }
                // Keep the original / separator
                uri_stream << '/';
                pos = slash_pos + 1;
            }
        }

        // Handle the trailing /
        if (!path.empty() && path.back() == '/') {
            uri_stream << '/';
        }
    }

    // Add the query parameters
    bool first = true;
    for (const auto& [key, value] : params) {
        uri_stream << (first ? "?" : "&");
        // URL encode the key and the value separately
        uri_stream << url_escape(key.c_str()) << "=" << url_escape(value.c_str());
        first = false;
    }

    return uri_stream.str();
}

// The asynchronous DNS resolution method
net::awaitable<std::vector<tcp::endpoint>> AsioHttpClient::_resolveDNS() {
    // First judge whether the host is an IP address; if so construct the endpoint and return
    // directly, avoiding an unnecessary DNS query
    net::error_code ec;
    auto addr = net::ip::make_address(m_host, ec);
    if (!ec) {
        // The host is a valid IP address, construct the endpoint directly
        std::vector<tcp::endpoint> endpoints;
        uint16_t port_num = static_cast<uint16_t>(std::stoi(m_port));
        if (addr.is_v4()) {
            endpoints.emplace_back(tcp::endpoint(addr.to_v4(), port_num));
        } else if (addr.is_v6()) {
            endpoints.emplace_back(tcp::endpoint(addr.to_v6(), port_num));
        }
        co_return endpoints;
    }

#if HKU_OS_OSX || HKU_OS_IOS
    // macOS uses the native getaddrinfo way (the beast resolution has a known issue and would hang)
    struct addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;  // Query the address types supported by this machine only

    int ret = getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &res);
    HKU_CHECK(ret == 0, "DNS resolve failed! {}:{}", m_host, m_port);

    std::vector<tcp::endpoint> dns_endpoints;
    for (struct addrinfo* ai = res; ai != nullptr; ai = ai->ai_next) {
        if (ai->ai_family == AF_INET) {
            auto* sin = reinterpret_cast<sockaddr_in*>(ai->ai_addr);
            net::ip::address_v4::bytes_type v4_bytes;
            memcpy(&v4_bytes, &(sin->sin_addr.s_addr), sizeof(v4_bytes));
            dns_endpoints.push_back(
              tcp::endpoint(net::ip::make_address_v4(v4_bytes), ntohs(sin->sin_port)));
        } else if (ai->ai_family == AF_INET6) {
            auto* sin6 = reinterpret_cast<sockaddr_in6*>(ai->ai_addr);
            net::ip::address_v6::bytes_type v6_bytes;
            memcpy(&v6_bytes, &(sin6->sin6_addr.s6_addr), sizeof(v6_bytes));
            dns_endpoints.push_back(
              tcp::endpoint(net::ip::make_address_v6(v6_bytes), ntohs(sin6->sin6_port)));
        }
    }

    freeaddrinfo(res);
    HKU_CHECK(!dns_endpoints.empty(), "DNS resolve failed! {}:{}", m_host, m_port);
    co_return dns_endpoints;

#else
    // The other platforms use the Boost.ASIO asynchronous DNS resolution
    // Note: the resolver must have the same lifetime as op, avoiding a dangling reference and a
    // hang
    auto resolver = std::make_shared<tcp::resolver>(*m_ctx);

    struct ResolveOp {
        std::shared_ptr<tcp::resolver> resolver;  // Changed to a shared_ptr to extend the lifetime
        std::string host, port;
        tcp::resolver::results_type endpoints;
        net::error_code ec;
        bool done = false;

        ResolveOp(std::shared_ptr<tcp::resolver> r, const std::string& h, const std::string& p)
        : resolver(r), host(h), port(p) {}

        net::awaitable<net::error_code> run() {
            auto [e, eps] =
              co_await resolver->async_resolve(host, port, net::as_tuple(net::use_awaitable));
            ec = e;
            endpoints = std::move(eps);
            done = true;
            co_return e;
        }
    };

    auto op = std::make_shared<ResolveOp>(resolver, m_host, m_port);

    // Start the timer and the DNS resolution
    auto timer = net::steady_timer{*m_ctx};
    timer.expires_after(m_timeout);

    timer.async_wait([resolver, op](const net::error_code& ec) {
        if (!ec && !op->done) {
            // Cancel all the asynchronous operations of the resolver on a timeout
            // The resolver is a shared_ptr now, ensuring the lifetime safety
            resolver->cancel();
        }
    });

    auto resolve_result = co_await op->run();

    // Cancel the timer
    timer.cancel();

    // Check whether it was cancelled due to a timeout (operation_aborted means it was cancelled by
    // cancel())
    if (resolve_result == boost::asio::error::operation_aborted) {
        HKU_THROW_EXCEPTION(HttpTimeoutException, "DNS resolve timeout");
    }

    if (resolve_result) {
        HKU_THROW("DNS resolve failed: {}", resolve_result.message());
    }

    // Convert to the endpoint list
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

// Get a connected connection from the connection pool (with the DNS cache)
net::awaitable<std::pair<std::shared_ptr<HttpConnection>, bool>> AsioHttpClient::_getConnection() {
    HKU_ASSERT(m_connection_pool != nullptr);

    // Get a connection from the pool (the resource pool checks the version automatically and the
    // old version connections are eliminated)
    auto conn_result = co_await m_connection_pool->asyncGet();
    if (!conn_result) {
        HKU_THROW_EXCEPTION(HttpTimeoutException, "Failed to get connection from pool: {}",
                            conn_result.error());
    }
    auto conn_ptr = std::move(conn_result.value());
    HKU_CHECK(conn_ptr != nullptr, "Failed to get connection from pool");

    bool is_new_connection = false;

    // Check whether the connection needs to be recreated
    if (!conn_ptr->is_open()) {
        // The connection is closed and needs to be recreated
        is_new_connection = true;

        // Re-resolve the DNS when the DNS cache is empty or expired (5 minutes)
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
            // The DNS resolution (with a timeout)
            conn_ptr->endpoints = co_await _resolveDNS();
        }

        // Close the old connection (if there is one)
        conn_ptr->close();

        if (m_is_https) {
#if HKU_ENABLE_HTTP_CLIENT_SSL
            // Connect to the server
            bool connected = false;
            for (const auto& endpoint : conn_ptr->endpoints) {
                conn_ptr->ssl_socket.emplace(*m_ctx, m_ssl_ctx->ssl_ctx);
                SSL_set_tlsext_host_name(conn_ptr->ssl_socket->native_handle(), m_host.c_str());

                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool connect_completed = false;
                net::error_code captured_ec;

                struct ConnectOp {
                    tcp::socket* socket;
                    tcp::endpoint endpoint;
                    bool& completed_flag;
                    net::error_code& captured_ec;

                    net::awaitable<net::error_code> run() {
                        auto [ec] = co_await socket->async_connect(
                          endpoint, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec ? ec
                                     : (socket->is_open()
                                          ? net::error_code()
                                          : net::error::make_error_code(net::error::not_connected));
                    }
                };

                // Start the timer and the connection operation
                timer.async_wait([&connect_completed, &conn_ptr](const net::error_code& ec) {
                    if (!ec && !connect_completed && conn_ptr->ssl_socket.has_value()) {
                        conn_ptr->ssl_socket->lowest_layer().cancel();
                    }
                });

                ConnectOp connect_op{&conn_ptr->ssl_socket->next_layer(), endpoint,
                                     connect_completed, captured_ec};
                co_await connect_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout
                if (captured_ec == boost::asio::error::operation_aborted) {
                    break;  // Do not try the other endpoints after a timeout
                }

                // Check whether the connection succeeded
                if (!captured_ec && conn_ptr->ssl_socket->lowest_layer().is_open()) {
                    connected = true;
                    break;
                }

                // The connection failed, reset it and continue trying the next endpoint
                conn_ptr->ssl_socket.reset();
            }

            if (!connected) {
                HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host,
                                    m_port);
            }

            // Set the socket options
            conn_ptr->ssl_socket->lowest_layer().set_option(tcp::no_delay(true));

            // The SSL handshake (with a timeout)
            {
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool handshake_completed = false;
                net::error_code captured_ec;

                struct SslHandshakeOp {
                    ssl::stream<tcp::socket>* stream;
                    bool& completed_flag;
                    net::error_code& captured_ec;

                    net::awaitable<net::error_code> run() {
                        auto [ec] = co_await stream->async_handshake(
                          ssl::stream_base::client, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec;
                    }
                };

                // Start the timer and the handshake operation
                timer.async_wait([&handshake_completed, &conn_ptr](const net::error_code& ec) {
                    if (!ec && !handshake_completed && conn_ptr->ssl_socket.has_value()) {
                        conn_ptr->ssl_socket->lowest_layer().cancel();
                    }
                });

                SslHandshakeOp handshake_op{&conn_ptr->ssl_socket.value(), handshake_completed,
                                            captured_ec};
                co_await handshake_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout
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
                // An ordinary HTTP connection (SSL is enabled but HTTP is used currently)
                conn_ptr->socket.emplace(*m_ctx);

                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                bool connect_completed = false;
                net::error_code captured_ec;

                struct ConnectOp {
                    tcp::socket* socket;
                    tcp::endpoint endpoint;
                    bool& completed_flag;
                    net::error_code& captured_ec;

                    net::awaitable<net::error_code> run() {
                        auto [ec] = co_await socket->async_connect(
                          endpoint, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return ec ? ec
                                     : (socket->is_open()
                                          ? net::error_code()
                                          : net::error::make_error_code(net::error::not_connected));
                    }
                };

                // Start the timer and the connection operation
                timer.async_wait([&connect_completed, &conn_ptr](const net::error_code& ec) {
                    if (!ec && !connect_completed && conn_ptr->socket.has_value()) {
                        conn_ptr->socket->cancel();
                    }
                });

                ConnectOp connect_op{&conn_ptr->socket.value(), endpoint, connect_completed,
                                     captured_ec};
                co_await connect_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout
                if (captured_ec == boost::asio::error::operation_aborted) {
                    break;  // Do not try the other endpoints after a timeout
                }

                // Check whether the connection succeeded
                if (!captured_ec && conn_ptr->socket->is_open()) {
                    connected = true;
                    break;
                }

                // The connection failed without a timeout, continue trying the next endpoint

                // Close and reset the socket for the next attempt
                net::error_code ec;
                conn_ptr->socket->close(ec);
                conn_ptr->socket.reset();
            }

            if (!connected) {
                HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host,
                                    m_port);
            }

            // Set the socket options
            conn_ptr->socket->set_option(tcp::no_delay(true));
        }

        // Update the last used time
        conn_ptr->last_used_time = std::chrono::steady_clock::now();
    }
    // The existing connection is reused with no extra operation; the connection pool manages its
    // lifetime automatically It is enough to update last_used_time after every use
    conn_ptr->last_used_time = std::chrono::steady_clock::now();

    // A reused connection does not need the socket options to be set here again,
    // because they were already set in _connect or when the connection was established before.
    // When a connection is reused its socket state is preserved.

    co_return std::make_pair(conn_ptr, is_new_connection);
}

// Create the socket (a variant is used to store an ordinary socket or an SSL socket)
struct AsioHttpClient::SocketVariant {
    std::optional<tcp::socket> plain;
#if HKU_ENABLE_HTTP_CLIENT_SSL
    std::optional<ssl::stream<tcp::socket>> ssl;

    void close(net::error_code& ec) {
        if (plain) {
            plain->close(ec);
            plain.reset();
        }
        if (ssl) {
            ssl->lowest_layer().close(ec);
            ssl.reset();
        }
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
    void close(net::error_code& ec) {
        if (plain) {
            plain->close(ec);
            plain.reset();
        }
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
    // Connect (with a timeout)
    net::error_code connect_ec;
    bool connected = false;

    for (const auto& endpoint : dns_endpoints) {
        socket_variant.close(connect_ec);

        {
            // Create an ordinary socket first
            socket_variant.plain.emplace(*m_ctx);

            // Use the event driven asynchronous connection with a timeout
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool connect_completed = false;
            net::error_code captured_ec;

            struct ConnectOp {
                tcp::socket* socket;
                tcp::endpoint endpoint;
                bool& completed_flag;
                net::error_code& captured_ec;

                net::awaitable<net::error_code> run() {
                    auto [ec] =
                      co_await socket->async_connect(endpoint, net::as_tuple(net::use_awaitable));
                    completed_flag = true;
                    captured_ec = ec;
                    co_return ec ? ec
                                 : (socket->is_open()
                                      ? net::error_code()
                                      : net::error::make_error_code(net::error::not_connected));
                }
            };

            // Start the timer and the connection operation
            timer.async_wait([&connect_completed, &socket_variant](const net::error_code& ec) {
                if (!ec && !connect_completed && socket_variant.plain.has_value()) {
                    socket_variant.plain->cancel();
                }
            });

            ConnectOp connect_op{&socket_variant.plain.value(), endpoint, connect_completed,
                                 captured_ec};

            // Wait for the connection to complete
            co_await connect_op.run();

            // Cancel the timer
            timer.cancel();

            // Check whether it was cancelled due to a timeout
            if (captured_ec == boost::asio::error::operation_aborted) {
                continue;  // Try the next endpoint
            }

            // Check whether the connection succeeded
            if (!captured_ec && socket_variant.plain->is_open()) {
                connected = true;
                break;
            }

            // The connection failed, close the socket
            socket_variant.close(connect_ec);
        }
    }

    if (!connected) {
        HKU_THROW_EXCEPTION(HttpTimeoutException, "Connect timeout to {}:{}", m_host, m_port);
    }

    // Set the socket options
    socket_variant.socket().set_option(tcp::no_delay(true));

#if HKU_ENABLE_HTTP_CLIENT_SSL
    // For HTTPS perform the SSL handshake (with a timeout)
    if (m_is_https) {
        // Move to the SSL socket
        socket_variant.ssl.emplace(std::move(*socket_variant.plain), m_ssl_ctx->ssl_ctx);
        socket_variant.plain.reset();

        // Set the SNI (Server Name Indication)
        SSL_set_tlsext_host_name(socket_variant.ssl->native_handle(), m_host.c_str());

        // Use the event driven SSL handshake with a timeout
        auto timer = net::steady_timer{*m_ctx};
        timer.expires_after(m_timeout);

        bool handshake_completed = false;
        net::error_code captured_ec;

        struct SslHandshakeOp {
            ssl::stream<tcp::socket>* stream;
            bool& completed_flag;
            net::error_code& captured_ec;

            net::awaitable<net::error_code> run() {
                auto [ec] = co_await stream->async_handshake(ssl::stream_base::client,
                                                             net::as_tuple(net::use_awaitable));
                completed_flag = true;
                captured_ec = ec;
                co_return ec;
            }
        };

        // Start the timer and the handshake operation
        timer.async_wait([&handshake_completed, &socket_variant](const net::error_code& ec) {
            if (!ec && !handshake_completed && socket_variant.ssl.has_value()) {
                socket_variant.ssl->lowest_layer().cancel();
            }
        });

        SslHandshakeOp handshake_op{&socket_variant.ssl.value(), handshake_completed, captured_ec};
        co_await handshake_op.run();

        // Cancel the timer
        timer.cancel();

        // Check whether it was cancelled due to a timeout
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

    // Make sure the io_context is set (the default constructor may not initialize it)
    if (m_ctx == nullptr) {
        auto exec = co_await net::this_coro::executor;
        m_ctx = &static_cast<net::io_context&>(exec.context());
    }

#if !HKU_ENABLE_HTTP_CLIENT_SSL
    HKU_CHECK(!m_is_https,
              "HTTPS is not supported. Please enable SSL support with --http_client_ssl=y");
#endif

    // Build the complete URI
    std::string uri = _buildURI(path, params);

    AsioHttpResponse response;

    try {
        // Get a connection from the connection pool (the DNS cache and the connection reuse are
        // handled automatically)
        auto [conn, is_new] = co_await _getConnection();
        HKU_CHECK(conn != nullptr, "Failed to get connection from pool");

        // Create the HTTP request
        http::request<http::string_body> req;
        req.method(http::string_to_verb(method));
        req.target(uri);
        req.version(11);  // HTTP/1.1

        // Add the default headers
        for (const auto& [key, value] : m_default_headers) {
            req.set(key, value);
        }

        // Add the extra headers
        for (const auto& [key, value] : headers) {
            req.set(key, value);
        }

        // Add the User-Agent
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::host, m_host);
        // Note: "close" is not used, allowing the connection reuse

        // Add the request body
        if (body != nullptr && body_len > 0) {
#if HKU_ENABLE_HTTP_CLIENT_ZIP
            req.set(http::field::content_type, content_type);
            auto content_encoding = req["Content-Encoding"];
            if (content_encoding == "gzip") {
                gzip::Compressor comp(Z_DEFAULT_COMPRESSION);
                std::string output;
                comp.compress(output, body, body_len);
                req.body() = std::move(output);
            } else {
                req.body() = std::string(body, body_len);
            }
            req.prepare_payload();
#else
            req.set(http::field::content_type, content_type);
            req.body() = std::string(body, body_len);
            req.prepare_payload();
#endif
        }

        // Send the request (with a timeout)
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

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_write(
                          stream, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // Start the timer and the write operation
                timer.async_wait([&write_completed, &conn](const net::error_code& ec) {
                    if (!ec && !write_completed && conn->is_open()) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto write_op = WriteOp{*conn->ssl_socket, req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout (operation_aborted means it was
                // cancelled by cancel())
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

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] =
                          co_await http::async_write(sock, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // Start the timer and the write operation
                timer.async_wait([&write_completed, &conn](const net::error_code& ec) {
                    if (!ec && !write_completed) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto write_op = WriteOp{conn->socket.value(), req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout (operation_aborted means it was
                // cancelled by cancel())
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

        // Read the response (with a timeout)
        beast::flat_buffer buffer;
        http::response<http::string_body> res;

        {
            auto timer = net::steady_timer{*m_ctx};
            timer.expires_after(m_timeout);

            bool read_completed = false;
            net::error_code captured_ec;

#if HKU_ENABLE_HTTP_CLIENT_SSL
            if (conn->ssl_socket) {
                struct ReadOp {
                    ssl::stream<tcp::socket>& stream;
                    beast::flat_buffer& buffer;
                    http::response<http::string_body>& response;
                    bool& completed_flag;
                    net::error_code& captured_ec;

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_read(
                          stream, buffer, response, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // Start the timer and the read operation
                timer.async_wait([&read_completed, &conn](const net::error_code& ec) {
                    if (!ec && !read_completed && conn->is_open()) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto read_op = ReadOp{*conn->ssl_socket, buffer, res, read_completed, captured_ec};
                co_await read_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout (operation_aborted means it was
                // cancelled by cancel())
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
                    net::error_code& captured_ec;

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_read(
                          sock, buffer, response, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        captured_ec = ec;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                // Start the timer and the read operation
                timer.async_wait([&read_completed, &conn](const net::error_code& ec) {
                    if (!ec && !read_completed) {
                        conn->lowest_layer().cancel();
                    }
                });

                auto read_op =
                  ReadOp{conn->socket.value(), buffer, res, read_completed, captured_ec};
                co_await read_op.run();

                // Cancel the timer
                timer.cancel();

                // Check whether it was cancelled due to a timeout (operation_aborted means it was
                // cancelled by cancel())
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

        // Fill the response object
        response.m_status = res.result_int();
        response.m_reason = std::string(res.reason());

#if HKU_ENABLE_HTTP_CLIENT_ZIP
        // Get the Content-Encoding header correctly
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

        // Do not close the connection, let the connection pool manage it

    } catch (const net::system_error&) {
        // HKU_DEBUG("HTTP request system error! {}", e.what());
        throw;
    } catch (const std::exception&) {
        // HKU_DEBUG("HTTP request failed! {}", e.what());
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

    // Make sure the io_context is set (the default constructor may not initialize it)
    if (m_ctx == nullptr) {
        auto exec = co_await net::this_coro::executor;
        m_ctx = &static_cast<net::io_context&>(exec.context());
    }

#if !HKU_ENABLE_HTTP_CLIENT_SSL
    HKU_CHECK(!m_is_https,
              "HTTPS is not supported. Please enable SSL support with --http_client_ssl=y");
#endif

    // Build the complete URI
    std::string uri = _buildURI(path, params);

    AsioHttpStreamResponse response;

    try {
        // Get a connection from the connection pool (the DNS cache and the connection reuse are
        // handled automatically)
        auto [conn, is_new] = co_await _getConnection();
        HKU_CHECK(conn != nullptr, "Failed to get connection from pool");

        // Create the HTTP request
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
        // Note: "close" is not used, allowing the connection reuse

        if (body != nullptr && body_len > 0) {
#if HKU_ENABLE_HTTP_CLIENT_ZIP
            req.set(http::field::content_type, content_type);
            auto content_encoding = req["Content-Type"];
            if (content_encoding == "gzip") {
                gzip::Compressor comp(Z_DEFAULT_COMPRESSION);
                std::string output;
                comp.compress(output, body, body_len);
                req.body() = std::move(output);
            } else {
                req.body() = std::string(body, body_len);
            }
            req.prepare_payload();
#else
            req.set(http::field::content_type, content_type);
            req.body() = std::string(body, body_len);
            req.prepare_payload();
#endif
        }

        // Send the request - use the event driven way
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

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] = co_await http::async_write(
                          stream, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                auto write_op = WriteOp{*conn->ssl_socket, req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // Check whether it was cancelled due to a timeout
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

                    net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                        auto [ec, bytes] =
                          co_await http::async_write(sock, req, net::as_tuple(net::use_awaitable));
                        completed_flag = true;
                        co_return std::make_pair(ec, bytes);
                    }
                };

                auto write_op = WriteOp{conn->socket.value(), req, write_completed};
                auto [write_ec, bytes_transferred] = co_await write_op.run();

                // Check whether it was cancelled due to a timeout
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

        // Read the response in a streaming way - use buffer_body
        beast::flat_buffer buffer;
        http::response_parser<http::buffer_body> parser;

        // Set the buffer size (8KB chunks)
        constexpr size_t BUFFER_SIZE = 8192;
        std::vector<char> chunk_buffer(BUFFER_SIZE);

        parser.get().body().data = chunk_buffer.data();
        parser.get().body().size = BUFFER_SIZE;

        {
            // Read the response header - use the event driven way
            {
                // Set the timeout timer
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                // Start the timer and cancel the underlying socket on a timeout
                timer.async_wait([&conn](const net::error_code& ec) {
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

                        net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read_header(
                              stream, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_header_op = ReadHeaderOp{*conn->ssl_socket, buffer, parser};
                    auto [read_ec, bytes_transferred] = co_await read_header_op.run();

                    // Cancel the timer
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

                        net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read_header(
                              sock, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_header_op = ReadHeaderOp{conn->socket.value(), buffer, parser};
                    auto [read_ec, bytes_transferred] = co_await read_header_op.run();

                    // Cancel the timer
                    timer.cancel();

                    if (read_ec && read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read header failed: {}", read_ec.message());
                    }
#if HKU_ENABLE_HTTP_CLIENT_SSL
                }
#endif
            }

            // Fill the response object
            response.m_status = static_cast<int>(parser.get().result_int());
            response.m_reason = std::string(parser.get().reason());
            for (auto it = parser.get().begin(); it != parser.get().end(); ++it) {
                response.m_headers.emplace(std::string(it->name_string()),
                                           std::string(it->value()));
            }

            // Read the response body data chunks in a loop
            while (!parser.is_done()) {
                std::size_t bytes_transferred = 0;
                net::error_code read_ec;

                // Set the timeout timer (reset on every chunk read)
                auto timer = net::steady_timer{*m_ctx};
                timer.expires_after(m_timeout);

                // Start the timer and cancel the underlying socket on a timeout
                timer.async_wait([&conn](const net::error_code& ec) {
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

                        net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read(
                              stream, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_op = ReadOp{*conn->ssl_socket, buffer, parser};
                    auto [ec, bytes] = co_await read_op.run();
                    read_ec = ec;
                    bytes_transferred = bytes;

                    // Cancel the timer
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

                        net::awaitable<std::pair<net::error_code, std::size_t>> run() {
                            auto [ec, bytes] = co_await http::async_read(
                              sock, buffer, parser, net::as_tuple(net::use_awaitable));
                            co_return std::make_pair(ec, bytes);
                        }
                    };

                    auto read_op = ReadOp{conn->socket.value(), buffer, parser};
                    auto [ec, bytes] = co_await read_op.run();
                    read_ec = ec;
                    bytes_transferred = bytes;

                    // Cancel the timer
                    timer.cancel();

                    if (read_ec && read_ec != http::error::need_buffer &&
                        read_ec != http::error::end_of_stream) {
                        HKU_THROW("HTTP read body failed: {}", read_ec.message());
                    }
#if HKU_ENABLE_HTTP_CLIENT_SSL
                }
#endif

                // Call the callback to process the data chunk
                if (bytes_transferred > 0) {
                    response.m_total_bytes_read += bytes_transferred;
                    chunk_callback(chunk_buffer.data(), bytes_transferred);
                }

                // Reset the buffer for the next read
                parser.get().body().data = chunk_buffer.data();
                parser.get().body().size = BUFFER_SIZE;
            }
        }

        // Do not close the connection, let the connection pool manage it (it is returned to the
        // pool)

    } catch (const net::system_error&) {
        // HKU_DEBUG("HTTP stream request system error! {}", e.what());
        throw;
    } catch (const std::exception&) {
        // HKU_DEBUG("HTTP stream request failed! {}", e.what());
        throw;
    }

    co_return response;
}

// ============================================================================
// The synchronous method implementation - it blocks until the asynchronous operation completes
// ============================================================================

AsioHttpResponse AsioHttpClient::request(const std::string& method, const std::string& path,
                                         const HttpParams& params, const HttpHeaders& headers,
                                         const char* body, size_t body_len,
                                         const std::string& content_type) {
    // Validate the URL in advance, avoiding discovering the problem after entering the asynchronous
    // coroutine
    HKU_CHECK(m_is_valid_url, "Invalid url: {}", m_url);
    HKU_ASSERT(m_ctx);

    // Make sure the io_context is running
    if (m_ctx->stopped()) {
        m_ctx->restart();
    }

    // Convert the coroutine result into a std::future with use_future
    auto future =
      co_spawn(*m_ctx, async_request(method, path, params, headers, body, body_len, content_type),
               boost::asio::use_future);

    // Get the result; an exception thrown in the coroutine is rethrown here
    return future.get();
}

AsioHttpStreamResponse AsioHttpClient::requestStream(
  const std::string& method, const std::string& path, const HttpParams& params,
  const HttpHeaders& headers, const char* body, size_t body_len, const std::string& content_type,
  const HttpChunkCallback& chunk_callback) {
    // Validate the URL and the callback function in advance
    HKU_CHECK(m_is_valid_url, "Invalid url: {}", m_url);
    HKU_CHECK(chunk_callback != nullptr, "Chunk callback must not be null");
    HKU_ASSERT(m_ctx);

    // Make sure the io_context is running
    if (m_ctx->stopped()) {
        m_ctx->restart();
    }

    // Convert the coroutine result into a std::future with use_future
    auto future = co_spawn(*m_ctx,
                           async_requestStream(method, path, params, headers, body, body_len,
                                               content_type, chunk_callback),
                           boost::asio::use_future);

    // Get the result; an exception thrown in the coroutine is rethrown here
    return future.get();
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

}  // namespace hku
