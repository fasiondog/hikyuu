/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_ASIO_HTTP_CLIENT_H
#define HKU_UTILS_ASIO_HTTP_CLIENT_H

#include "hikyuu/utilities/config.h"
#if !HKU_ENABLE_HTTP_CLIENT
#error "Don't enable http client, please config with --http_client=y"
#endif

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <nlohmann/json.hpp>
#include <boost/beast.hpp>
#include "hikyuu/utilities/net.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/Parameter.h"
#include "HttpException.h"
#include "hikyuu/utilities/ResourceAsioPool.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

using json = nlohmann::json;

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = net::tcp;

using HttpHeaders = std::map<std::string, std::string>;
using HttpParams = std::map<std::string, std::string>;

/**
 * @brief HTTP data chunk callback function type
 *
 * It is used for the streaming response processing and is called every time a data chunk is
 * received.
 * It is suitable for the scenarios such as the large file download and the realtime data stream,
 * avoiding loading everything into the memory at once.
 *
 * @param data the data chunk pointer
 * @param size the data chunk size (the number of the bytes)
 *
 * @note The callback function should stay non-blocking, avoiding the time-consuming operations
 * @note An exception should not be thrown in the callback, otherwise the request is terminated
 */
using HttpChunkCallback = std::function<void(const char* data, size_t size)>;

class HKU_UTILS_API AsioHttpClient;

// Forward declaration of HttpConnection
struct HttpConnection;

/**
 * @brief HTTP response class
 *
 * It encapsulates the complete HTTP response information, including the status code, the response
 * headers and the response body.
 * It is suitable for the traditional scenario of loading the whole response body at once.
 *
 * @note For a large response body (>100MB) or a memory limited scenario, AsioHttpStreamResponse is
 *       recommended
 */
class HKU_UTILS_API AsioHttpResponse final {
    friend class HKU_UTILS_API AsioHttpClient;

public:
    AsioHttpResponse() = default;
    ~AsioHttpResponse() = default;

    AsioHttpResponse(const AsioHttpResponse&) = default;
    AsioHttpResponse& operator=(const AsioHttpResponse&) = default;

    AsioHttpResponse(AsioHttpResponse&& rhs);
    AsioHttpResponse& operator=(AsioHttpResponse&& rhs) noexcept;

    /**
     * @brief Get the response body content
     * @return the complete response body string
     */
    const std::string& body() const noexcept {
        return m_body;
    }

    /**
     * @brief Parse the response body into a JSON object
     * @return the JSON object
     * @throws nlohmann::json::exception when the response body is not a valid JSON
     */
    hku::json json() const;

    /**
     * @brief Get the HTTP status code
     * @return the HTTP status code (such as 200, 404, 500, etc.)
     */
    int status() const noexcept {
        return m_status;
    }

    /**
     * @brief Get the HTTP status description
     * @return the status description text (such as "OK", "Not Found", etc.)
     */
    const std::string& reason() const noexcept {
        return m_reason;
    }

    /**
     * @brief Get the value of the given response header field
     * @param key the response header key name (case insensitive)
     * @return the response header value; an empty string is returned when it does not exist
     */
    std::string getHeader(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        return it != m_headers.end() ? it->second : std::string();
    }

    /**
     * @brief Get the response body length
     * @return the value of Content-Length; 0 is returned when it is not set or the parsing fails
     */
    size_t getContentLength() const noexcept {
        auto it = m_headers.find("Content-Length");
        if (it != m_headers.end() && !it->second.empty()) {
            try {
                return std::stoull(it->second);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }

private:
    int m_status{0};
    std::string m_reason;
    std::string m_body;
    std::map<std::string, std::string> m_headers;
};

/**
 * @brief Streaming HTTP response class
 *
 * It is used for the streaming download of a large response body, avoiding loading everything into
 * the memory at once.
 * Both the Content-Length and the Transfer-Encoding: chunked modes are supported.
 *
 * ## Applicable scenarios
 * - Large file download (>100MB)
 * - Realtime data stream processing
 * - Server-Sent Events (SSE)
 * - Memory limited environment
 *
 * ## Inapplicable scenarios
 * - Random access to the response data is needed
 * - The scenario where the complete data is needed for the processing (such as a JSON parsing)
 *
 * ## Performance comparison
 * - The memory usage of the traditional way = the file size
 * - The memory usage of the streaming way is about 8KB (a constant buffer)
 *
 * @note A streaming request is also limited by the configured timeout
 * @note HttpTimeoutException and the other network exceptions need to be caught
 */
class HKU_UTILS_API AsioHttpStreamResponse final {
    friend class HKU_UTILS_API AsioHttpClient;

public:
    AsioHttpStreamResponse() = default;
    ~AsioHttpStreamResponse() = default;

    AsioHttpStreamResponse(const AsioHttpStreamResponse&) = default;
    AsioHttpStreamResponse& operator=(const AsioHttpStreamResponse&) = default;

    AsioHttpStreamResponse(AsioHttpStreamResponse&& rhs);
    AsioHttpStreamResponse& operator=(AsioHttpStreamResponse&& rhs) noexcept;

    /**
     * @brief Get the HTTP status code
     * @return the HTTP status code (such as 200, 404, 500, etc.)
     */
    int status() const noexcept {
        return m_status;
    }

    /**
     * @brief Get the HTTP status description
     * @return the status description text (such as "OK", "Not Found", etc.)
     */
    const std::string& reason() const noexcept {
        return m_reason;
    }

    /**
     * @brief Get the value of the given response header field
     * @param key the response header key name (case insensitive)
     * @return the response header value; an empty string is returned when it does not exist
     */
    std::string getHeader(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        return it != m_headers.end() ? it->second : std::string();
    }

    /**
     * @brief Get the total length of the response body
     * @return the value of Content-Length; 0 is returned when it is not set or the parsing fails
     */
    size_t getContentLength() const noexcept {
        auto it = m_headers.find("Content-Length");
        if (it != m_headers.end() && !it->second.empty()) {
            try {
                return std::stoull(it->second);
            } catch (...) {
                return 0;
            }
        }
        return 0;
    }

    /**
     * @brief Judge whether it is a chunked transfer encoding
     * @return true means Transfer-Encoding: chunked is used and false means Content-Length is used
     */
    bool isChunked() const noexcept {
        auto it = m_headers.find("Transfer-Encoding");
        return it != m_headers.end() && it->second == "chunked";
    }

    /**
     * @brief Get the total number of the bytes already read
     * @return the accumulated number of the bytes read and passed to the callback function
     */
    uint64_t totalBytesRead() const noexcept {
        return m_total_bytes_read;
    }

private:
    int m_status{0};
    std::string m_reason;
    std::map<std::string, std::string> m_headers;
    uint64_t m_total_bytes_read{0};
};

/**
 * @brief High performance HTTP client based on Boost.Beast/Asio
 *
 * It supports the HTTP/1.1 and HTTPS protocols and provides the synchronous and asynchronous
 * request modes. It uses the connection pool technology to improve the performance and supports the
 * streaming response for the large file download.
 *
 * ## Basic features
 * - Underlying network library: Boost.Beast (HTTP/1.1) + Boost.Asio (the asynchronous I/O)
 * - Coroutine support: the C++20 coroutines, the return type is boost::asio::awaitable<T>
 * - Connection pool: it manages the HTTP persistent connections automatically and supports the
 * version detection
 * - SSL/TLS: the optional HTTPS support (OpenSSL is required)
 * - Timeout control: the timeout of every stage of the DNS resolution, the connection, the sending
 * and the receiving
 * - Streaming: Content-Length and Transfer-Encoding: chunked are supported
 *
 * ## Running modes
 * - **The internal io_context mode**: the default constructor and the constructor with a URL create
 * an independent io_context
 *   and start the background threads to run the event loop, the user does not need to manage it
 *   manually
 * - **The external io_context mode**: it is injected through the constructor with an external
 * io_context parameter,
 *   the lifetime is fully controlled from outside and multiple clients can share the same
 * io_context
 *
 * ## Asynchronous operation modes
 * - **The wait-for-completion mode** (async_*): it is suitable for the scenarios needing a return
 * value, an exception handling or a guaranteed order
 * - **The fire-and-forget mode**: it is suitable for the scenarios not caring about the result,
 * such as the background logging and the monitoring reporting (to be implemented)
 *
 * ## Resource management
 * - std::unique_ptr<net::io_context> is used to manage the internal io_context
 * - An executor_work_guard is created to prevent the io_context from exiting when there is no task
 * - At the destruction the work_guard is released first, then it waits for the worker threads to be
 * finished, and finally the io_context is stopped
 * - The move operations are disabled to ensure the resource safety
 *
 * ## Build configuration
 * ```bash
 * # Enable the HTTP client
 * xmake f --http_client=y
 *
 * # Enable the HTTPS support (OpenSSL is required)
 * xmake f --http_client_ssl=y
 * ```
 *
 * ## Usage example
 * @code
 * // A simple GET request (synchronous)
 * AsioHttpClient client("https://api.example.com");
 * auto response = client.get("/users");
 * std::cout << response.body() << std::endl;
 *
 * // An asynchronous GET request (the C++20 coroutine)
 * co_await client.async_get("/users");
 *
 * // A streaming download of a large file
 * std::ofstream file("download.bin", std::ios::binary);
 * co_await client.async_getStream("/largefile",
 *     [&file](const char* data, size_t size) {
 *         file.write(data, size);
 *     });
 *
 * // A custom CA certificate (HTTPS)
 * client.setCaFile("/path/to/ca.pem");
 * @endcode
 *
 * @note This class cannot be moved or copied, because it manages the lifetimes of the background
 *       threads and the io_context
 * @note When an external io_context is used, its lifetime must be longer than the client
 * @see AsioHttpResponse the complete response class
 * @see AsioHttpStreamResponse the streaming response class
 */
class HKU_UTILS_API AsioHttpClient {
public:
    using executor_type = boost::asio::any_io_executor;

    /// @brief Default timeout (ms)
    static constexpr int32_t DEFAULT_TIMEOUT_MS = 30000;  // 30 seconds

    /// @brief Maximum timeout (ms), it is used when a value <= 0 is passed
    static constexpr int32_t MAX_TIMEOUT_MS = 60000;  // 60 seconds

    /**
     * @brief Constructor (the internal io_context mode)
     *
     * It creates the internal io_context and starts the background threads to run the event loop.
     *
     * @param thread_count the number of the worker threads, 1 by default
     * @param max_concurrency the maximum number of the concurrent connections of the connection
     * pool, 0 means unlimited
     */
    explicit AsioHttpClient(int32_t thread_count = 1, size_t max_concurrency = 0);

    /**
     * @brief Constructor (the internal io_context mode with a URL)
     *
     * It creates the internal io_context, starts the background threads and sets the target URL.
     *
     * @param url the target URL (e.g. "https://api.example.com:8080/v1")
     * @param timeout the timeout (ms); MAX_TIMEOUT_MS is used when it is <= 0
     * @param thread_count the number of the worker threads, 1 by default
     * @param max_concurrency the maximum number of the concurrent connections of the connection
     * pool, 0 means unlimited
     */
    explicit AsioHttpClient(const std::string& url, int32_t timeout = DEFAULT_TIMEOUT_MS,
                            int32_t thread_count = 1, size_t max_concurrency = 0);

    /**
     * @brief Constructor (the external io_context mode)
     *
     * It uses the io_context provided from outside and does not own it.
     * It is suitable for the scenario where multiple clients share the same event loop.
     *
     * @param ctx the external io_context reference, its lifetime is managed by the caller
     * @param url the target URL (e.g. "https://api.example.com:8080/v1")
     * @param timeout the timeout (ms); MAX_TIMEOUT_MS is used when it is <= 0
     * @param max_concurrency the maximum number of the concurrent connections of the connection
     * pool, 0 means unlimited
     *
     * @note The external io_context must be run and maintained by the caller
     */
    explicit AsioHttpClient(net::io_context& ctx, const std::string& url,
                            int32_t timeout = DEFAULT_TIMEOUT_MS, size_t max_concurrency = 0);

    /**
     * @brief Destructor
     *
     * The resources are released safely in the following order:
     * 1. Release the executor_work_guard, allowing the io_context to exit naturally
     * 2. Wait for all the worker threads to be finished
     * 3. Explicitly stop the io_context
     *
     * @note It ensures that all the asynchronous operations are finished safely, without a forced
     *       interruption
     */
    virtual ~AsioHttpClient();

    // The copy operations are disabled
    AsioHttpClient(const AsioHttpClient&) = delete;
    AsioHttpClient& operator=(const AsioHttpClient&) = delete;

    // The move operations are disabled, because managing the lifetimes of the background threads
    // and the io_context is unsafe
    AsioHttpClient(AsioHttpClient&&) = delete;
    AsioHttpClient& operator=(AsioHttpClient&&) = delete;

    /**
     * @brief Check whether the client is valid
     * @return true means the URL has been set correctly and a request can be initiated
     */
    bool valid() const noexcept {
        return !m_url.empty();
    }

    /**
     * @brief Get the currently set URL
     * @return the complete URL string
     */
    const std::string& url() const noexcept {
        return m_url;
    }

    /**
     * @brief Set the target URL
     *
     * It parses the URL and extracts the information such as the protocol, the host, the port and
     * the path. The connection pool parameters are updated automatically.
     *
     * @param url the complete URL (e.g. "https://api.example.com:8080/v1/users")
     *
     * @note An error flag is set when the URL parsing fails
     */
    void setUrl(const std::string& url);

    /**
     * @brief Set the timeout
     *
     * It controls the timeout of every stage of the DNS resolution, the connection, the sending and
     * the receiving.
     *
     * @param ms the timeout (ms)
     *
     * @note When ms <= 0, MAX_TIMEOUT_MS (60000 ms) is used automatically as the default value
     * @note The timeout setting takes effect for all the subsequent requests
     */
    void setTimeout(int32_t ms);

    /**
     * @brief Get the current timeout
     * @return the timeout (ms)
     */
    int32_t getTimeout() const noexcept {
        return static_cast<int32_t>(m_timeout.count());
    }

    /**
     * @brief Get the executor of the io_context
     *
     * It is used to start a custom coroutine or an asynchronous operation on the io_context managed
     * by AsioHttpClient. It follows the executor exposure convention and does not expose the
     * internal implementation details.
     *
     * @return net::any_io_executor the executor of the io_context
     *
     * @example
     * @code
     * // Execute a custom task in the event loop of the client
     * co_spawn(client.get_executor(), []() -> net::awaitable<void> {
     *     co_await some_async_operation();
     * }, net::detached);
     * @endcode
     */
    executor_type get_executor() const noexcept {
        return m_ctx->get_executor();
    }

    /**
     * @brief Set the default request headers (the move semantics)
     * @param headers the request header map, it is moved into the internal storage
     */
    void setDefaultHeaders(std::map<std::string, std::string>&& headers) {
        m_default_headers = std::move(headers);
    }

    /**
     * @brief Set the custom CA certificate file path
     *
     * It is used to verify the server certificate during an HTTPS connection.
     * Configuration priority: the custom CA certificate > the system default certificate store.
     *
     * @param filename the CA certificate file path (the PEM format)
     *
     * @note It must be set before the HTTPS connection is established
     * @note An exception is thrown at the connection when the file does not exist or its format is
     *       wrong
     * @see HttpAsyncClient the asynchronous communication, resource management and timeout control
     * specification, item 7
     */
    void setCaFile(const std::string& filename);

    /**
     * @brief Set the default request headers (the copy semantics)
     * @param headers the request header map, it is copied into the internal storage
     */
    void setDefaultHeaders(const HttpHeaders& headers) {
        m_default_headers = headers;
    }

    // ==================== Asynchronous request methods ====================
    // They return net::awaitable and need to be called with co_await in a coroutine

    /**
     * @brief General asynchronous HTTP request
     *
     * The underlying asynchronous request implementation, it supports all the HTTP methods and the
     * custom parameters.
     * It handles the whole process of the DNS resolution, the connection establishment, the request
     * sending and the response receiving automatically.
     *
     * @param method the HTTP method (GET, POST, PUT, DELETE, etc.)
     * @param path the request path (e.g. "/api/users")
     * @param params the URL query parameters (a GET request) or the form parameters (a POST form)
     * @param headers the extra HTTP request headers
     * @param body the request body pointer (a POST/PUT request)
     * @param body_len the request body length
     * @param content_type the content type (e.g. "application/json", "text/plain")
     * @return AsioHttpResponse the complete HTTP response
     *
     * @throws HttpTimeoutException on a timeout
     * @throws boost::system::system_error on a network error
     *
     * @note A boost::system::system_error exception is thrown on a timeout or a network error
     * @note The concrete error cause can be determined by catching system_error and checking its
     *       error_code
     * @note operation_aborted means it was cancelled due to a timeout
     */
    net::awaitable<AsioHttpResponse> async_request(const std::string& method,
                                                   const std::string& path,
                                                   const HttpParams& params,
                                                   const HttpHeaders& headers, const char* body,
                                                   size_t body_len,
                                                   const std::string& content_type);

    /**
     * @brief Asynchronous GET request (without parameters)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_get(const std::string& path,
                                               const HttpHeaders& headers = {}) {
        co_return co_await async_request("GET", path, {}, headers, nullptr, 0, "");
    }

    /**
     * @brief Asynchronous GET request (with the query parameters)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_get(const std::string& path, const HttpParams& params,
                                               const HttpHeaders& headers) {
        co_return co_await async_request("GET", path, params, headers, nullptr, 0, "");
    }

    /**
     * @brief Asynchronous POST request (with the parameters and the request body)
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param len the request body length
     * @param content_type the content type
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers, const char* body,
                                                size_t len, const std::string& content_type) {
        co_return co_await async_request("POST", path, params, headers, body, len, content_type);
    }

    /**
     * @brief Asynchronous POST request (the request body only)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param len the request body length
     * @param content_type the content type
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const char* body, size_t len,
                                                const std::string& content_type) {
        co_return co_await async_request("POST", path, {}, headers, body, len, content_type);
    }

    /**
     * @brief Asynchronous POST request (the string content)
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param content the request body string
     * @param content_type the content type, "text/plain" by default
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers,
                                                const std::string& content,
                                                const std::string& content_type = "text/plain") {
        co_return co_await async_post(path, params, headers, content.data(), content.size(),
                                      content_type);
    }

    /**
     * @brief Asynchronous POST request (the string content, without parameters)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param content the request body string
     * @param content_type the content type, "text/plain" by default
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const std::string& content,
                                                const std::string& content_type = "text/plain") {
        co_return co_await async_post(path, {}, headers, content, content_type);
    }

    /**
     * @brief Asynchronous POST request (the JSON data, with the parameters)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     *
     * @note Content-Type is set to "application/json" automatically
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers, const json& body) {
        co_return co_await async_post(path, params, headers, body.dump(), "application/json");
    }

    /**
     * @brief Asynchronous POST request (the JSON data, with the request headers)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const json& body) {
        co_return co_await async_post(path, {}, headers, body);
    }

    /**
     * @brief Asynchronous POST request (the JSON data, the simplified version)
     * @param path request path
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const json& body) {
        co_return co_await async_post(path, {}, body);
    }

    /**
     * @brief Streaming asynchronous HTTP request (it supports the large file download)
     *
     * It uses a callback function to process the response data chunks, avoiding loading everything
     * into the memory at once. Both the Content-Length and the Transfer-Encoding: chunked modes are
     * supported automatically.
     *
     * @param method the HTTP method (GET, POST, etc.)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function, it is called every time data is
     *                       received
     * @return AsioHttpStreamResponse the streaming response object
     *
     * @note The callback function should stay non-blocking, avoiding the time-consuming operations
     * @note An exception should not be thrown in the callback, otherwise the request is terminated
     * @note Pay attention to the thread safety of the shared data
     */
    net::awaitable<AsioHttpStreamResponse> async_requestStream(
      const std::string& method, const std::string& path, const HttpParams& params,
      const HttpHeaders& headers, const char* body, size_t body_len,
      const std::string& content_type, const HttpChunkCallback& chunk_callback);

    /**
     * @brief Streaming asynchronous GET request (with the parameters)
     *
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    net::awaitable<AsioHttpStreamResponse> async_getStream(
      const std::string& path, const HttpParams& params, const HttpHeaders& headers,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("GET", path, params, headers, nullptr, 0, "",
                                               chunk_callback);
    }

    /**
     * @brief Streaming asynchronous GET request (without parameters)
     *
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    net::awaitable<AsioHttpStreamResponse> async_getStream(
      const std::string& path, const HttpHeaders& headers,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("GET", path, {}, headers, nullptr, 0, "",
                                               chunk_callback);
    }

    /**
     * @brief Streaming asynchronous POST request (with the parameters and the request body)
     *
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    net::awaitable<AsioHttpStreamResponse> async_postStream(
      const std::string& path, const HttpParams& params, const HttpHeaders& headers,
      const char* body, size_t body_len, const std::string& content_type,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("POST", path, params, headers, body, body_len,
                                               content_type, chunk_callback);
    }

    /**
     * @brief Streaming asynchronous POST request (the request body only)
     *
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    net::awaitable<AsioHttpStreamResponse> async_postStream(
      const std::string& path, const HttpHeaders& headers, const char* body, size_t body_len,
      const std::string& content_type, const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("POST", path, {}, headers, body, body_len,
                                               content_type, chunk_callback);
    }

    // ==================== Synchronous request methods ====================
    // They block until the request is finished and use the asynchronous implementation internally

    /**
     * @brief General synchronous HTTP request
     *
     * A blocking request, it uses the asynchronous implementation internally and waits for its
     * completion.
     * It is suitable for a non-coroutine environment or the scenario needing a synchronous call.
     *
     * @param method the HTTP method (GET, POST, PUT, DELETE, etc.)
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @return AsioHttpResponse the complete HTTP response
     *
     * @throws HttpTimeoutException on a timeout
     * @throws boost::system::system_error on a network error
     */
    AsioHttpResponse request(const std::string& method, const std::string& path,
                             const HttpParams& params, const HttpHeaders& headers, const char* body,
                             size_t body_len, const std::string& content_type);

    /**
     * @brief Synchronous GET request (without parameters)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse get(const std::string& path, const HttpHeaders& headers = {}) {
        return request("GET", path, {}, headers, nullptr, 0, "");
    }

    /**
     * @brief Synchronous GET request (with the query parameters)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse get(const std::string& path, const HttpParams& params,
                         const HttpHeaders& headers) {
        return request("GET", path, params, headers, nullptr, 0, "");
    }

    /**
     * @brief Synchronous POST request (with the parameters and the request body)
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param len the request body length
     * @param content_type the content type
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const char* body, size_t len,
                          const std::string& content_type) {
        return request("POST", path, params, headers, body, len, content_type);
    }

    /**
     * @brief Synchronous POST request (the request body only)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param len the request body length
     * @param content_type the content type
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers, const char* body,
                          size_t len, const std::string& content_type) {
        return request("POST", path, {}, headers, body, len, content_type);
    }

    /**
     * @brief Synchronous POST request (the string content, with the parameters)
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param content the request body string
     * @param content_type the content type, "text/plain" by default
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const std::string& content,
                          const std::string& content_type = "text/plain") {
        return post(path, params, headers, content.data(), content.size(), content_type);
    }

    /**
     * @brief Synchronous POST request (the string content, without parameters)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param content the request body string
     * @param content_type the content type, "text/plain" by default
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers,
                          const std::string& content,
                          const std::string& content_type = "text/plain") {
        return post(path, {}, headers, content, content_type);
    }

    /**
     * @brief Synchronous POST request (the JSON data, with the parameters)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const json& body) {
        return post(path, params, headers, body.dump(), "application/json");
    }

    /**
     * @brief Synchronous POST request (the JSON data, with the request headers)
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers, const json& body) {
        return post(path, {}, headers, body);
    }

    /**
     * @brief Synchronous POST request (the JSON data, the simplified version)
     * @param path request path
     * @param body the JSON object
     * @return AsioHttpResponse the complete HTTP response
     */
    AsioHttpResponse post(const std::string& path, const json& body) {
        return post(path, {}, body);
    }

    /**
     * @brief Synchronous streaming HTTP request (it supports the large file download)
     *
     * A blocking streaming request, it uses a callback function to process the response data
     * chunks. Both the Content-Length and the Transfer-Encoding: chunked modes are supported
     * automatically.
     *
     * @param method the HTTP method (GET, POST, etc.)
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function, it is called every time data is
     *                       received
     * @return AsioHttpStreamResponse the streaming response object
     *
     * @note The callback function should stay non-blocking, avoiding the time-consuming operations
     * @note An exception should not be thrown in the callback, otherwise the request is terminated
     * @note Pay attention to the thread safety of the shared data
     */
    AsioHttpStreamResponse requestStream(const std::string& method, const std::string& path,
                                         const HttpParams& params, const HttpHeaders& headers,
                                         const char* body, size_t body_len,
                                         const std::string& content_type,
                                         const HttpChunkCallback& chunk_callback);

    /**
     * @brief Synchronous streaming GET request (with the parameters)
     *
     * @param path request path
     * @param params the URL query parameters
     * @param headers the extra HTTP request headers
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    AsioHttpStreamResponse getStream(const std::string& path, const HttpParams& params,
                                     const HttpHeaders& headers,
                                     const HttpChunkCallback& chunk_callback) {
        return requestStream("GET", path, params, headers, nullptr, 0, "", chunk_callback);
    }

    /**
     * @brief Synchronous streaming GET request (without parameters)
     *
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    AsioHttpStreamResponse getStream(const std::string& path, const HttpHeaders& headers,
                                     const HttpChunkCallback& chunk_callback) {
        return requestStream("GET", path, {}, headers, nullptr, 0, "", chunk_callback);
    }

    /**
     * @brief Synchronous streaming POST request (with the parameters and the request body)
     *
     * @param path request path
     * @param params the URL query parameters or the form parameters
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    AsioHttpStreamResponse postStream(const std::string& path, const HttpParams& params,
                                      const HttpHeaders& headers, const char* body, size_t body_len,
                                      const std::string& content_type,
                                      const HttpChunkCallback& chunk_callback) {
        return requestStream("POST", path, params, headers, body, body_len, content_type,
                             chunk_callback);
    }

    /**
     * @brief Synchronous streaming POST request (the request body only)
     *
     * @param path request path
     * @param headers the extra HTTP request headers
     * @param body the request body pointer
     * @param body_len the request body length
     * @param content_type the content type
     * @param chunk_callback the data chunk callback function
     * @return AsioHttpStreamResponse the streaming response object
     */
    AsioHttpStreamResponse postStream(const std::string& path, const HttpHeaders& headers,
                                      const char* body, size_t body_len,
                                      const std::string& content_type,
                                      const HttpChunkCallback& chunk_callback) {
        return requestStream("POST", path, {}, headers, body, body_len, content_type,
                             chunk_callback);
    }

private:
    /**
     * @brief Parse the URL
     *
     * It extracts the information such as the protocol, the host, the port and the path.
     * It is responsible for the parsing logic only and does not modify the external state.
     */
    void _parseUrl() noexcept;

    /**
     * @brief DNS resolution
     *
     * It resolves the host name into an IP address list asynchronously.
     *
     * @return the IP endpoint list
     */
    net::awaitable<std::vector<tcp::endpoint>> _resolveDNS();

    struct SocketVariant;

    /**
     * @brief Establish the TCP connection
     *
     * It tries to connect to the endpoint list obtained from the DNS resolution.
     *
     * @param socket_variant the socket variant (TCP or SSL)
     * @param dns_endpoints the endpoint list obtained from the DNS resolution
     */
    net::awaitable<void> _connect(SocketVariant& socket_variant,
                                  const std::vector<tcp::endpoint>& dns_endpoints);

    /**
     * @brief Get a connected socket from the connection pool
     *
     * The connection pool reuse mechanism with the version check.
     *
     * @return pair<the connection object, whether it is newly created>; true means it is a newly
     *         created connection
     */
    net::awaitable<std::pair<std::shared_ptr<HttpConnection>, bool>> _getConnection();

    /**
     * @brief Build the complete URI (the path + the query parameters)
     *
     * It is responsible for combining the base path, the resource path and the query parameters
     * into a legal URI. It handles the logic such as the URL encoding of the path segments and the
     * deduplication of the slashes automatically.
     *
     * @param path the resource path (e.g. "api/v1/users" or "/api/v1/users")
     * @param params the query parameter map
     * @return the complete constructed URI string
     */
    std::string _buildURI(const std::string& path, const HttpParams& params);

private:
#if HKU_ENABLE_HTTP_CLIENT_SSL
    struct SslContext;
    std::unique_ptr<SslContext> m_ssl_ctx;  // SSL context (used when SSL is enabled only)
#endif

    bool m_is_valid_url{false};                               // Whether the URL is valid
    bool m_is_https{false};                                   // Whether the HTTPS protocol is used
    std::string m_url;                                        // The complete URL
    std::string m_base_path;                                  // The base path part of the URL
    std::string m_host;                                       // Host name
    std::string m_port;                                       // Port number
    std::chrono::milliseconds m_timeout{DEFAULT_TIMEOUT_MS};  // Timeout
    std::map<std::string, std::string> m_default_headers;     // Default request headers
    std::string m_ca_file;                                    // Custom CA certificate file path

    // Connection pool related members
    std::unique_ptr<ResourceAsioVersionPool<HttpConnection, std::mutex>> m_connection_pool;

    // io_context management
    std::unique_ptr<net::io_context> m_own_ctx;  // Internal io_context
    net::io_context* m_ctx{nullptr};             // The io_context currently used
    std::vector<std::thread> m_worker_threads;   // The thread pool running the io_context in the
                                                 // background
    std::unique_ptr<net::executor_work_guard<net::io_context::executor_type>>
      m_work_guard;  // Prevents the io_context from exiting when there is no task
};

}  // namespace hku

#endif