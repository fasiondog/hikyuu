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
#include <boost/asio.hpp>
#include <boost/beast.hpp>
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
namespace net = boost::asio;
using tcp = net::ip::tcp;

using HttpHeaders = std::map<std::string, std::string>;
using HttpParams = std::map<std::string, std::string>;

/**
 * @brief HTTP 数据块回调函数类型
 *
 * 用于流式响应处理，每次接收到数据块时调用。
 * 适用于大文件下载、实时数据流等场景，避免一次性加载到内存。
 *
 * @param data 数据块指针
 * @param size 数据块大小（字节数）
 *
 * @note 回调函数应保持非阻塞，避免执行耗时操作
 * @note 不应在回调中抛出异常，否则会终止请求
 */
using HttpChunkCallback = std::function<void(const char* data, size_t size)>;

class HKU_UTILS_API AsioHttpClient;

// HttpConnection 前向声明
struct HttpConnection;

/**
 * @brief HTTP 响应类
 *
 * 封装完整的 HTTP 响应信息，包括状态码、响应头、响应体等。
 * 适用于传统的一次性加载整个响应体的场景。
 *
 * @note 对于大响应体（>100MB）或内存受限场景，建议使用 AsioHttpStreamResponse
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
     * @brief 获取响应体内容
     * @return 完整的响应体字符串
     */
    const std::string& body() const noexcept {
        return m_body;
    }

    /**
     * @brief 将响应体解析为 JSON 对象
     * @return JSON 对象
     * @throws nlohmann::json::exception 当响应体不是合法 JSON 时
     */
    hku::json json() const {
        return json::parse(m_body);
    }

    /**
     * @brief 获取 HTTP 状态码
     * @return HTTP 状态码（如 200, 404, 500 等）
     */
    int status() const noexcept {
        return m_status;
    }

    /**
     * @brief 获取 HTTP 状态描述
     * @return 状态描述文本（如 "OK", "Not Found" 等）
     */
    std::string reason() const noexcept {
        return m_reason;
    }

    /**
     * @brief 获取指定的响应头字段值
     * @param key 响应头键名（不区分大小写）
     * @return 响应头值，若不存在则返回空字符串
     */
    std::string getHeader(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        return it != m_headers.end() ? it->second : std::string();
    }

    /**
     * @brief 获取响应体长度
     * @return Content-Length 的值，若未设置或解析失败则返回 0
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
 * @brief 流式 HTTP 响应类
 *
 * 用于处理大响应体的流式下载，避免一次性加载到内存。
 * 支持 Content-Length 和 Transfer-Encoding: chunked 两种模式。
 *
 * ## 适用场景
 * - 大文件下载（>100MB）
 * - 实时数据流处理
 * - Server-Sent Events (SSE)
 * - 内存受限环境
 *
 * ## 不适用场景
 * - 需要随机访问响应数据
 * - 需要完整数据才能处理的场景（如 JSON 解析）
 *
 * ## 性能对比
 * - 传统方式内存占用 = 文件大小
 * - 流式方式内存占用 ≈ 8KB（恒定缓冲区）
 *
 * @note 流式请求同样受配置的超时时间限制
 * @note 需捕获 HttpTimeoutException 和其他网络异常
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
     * @brief 获取 HTTP 状态码
     * @return HTTP 状态码（如 200, 404, 500 等）
     */
    int status() const noexcept {
        return m_status;
    }

    /**
     * @brief 获取 HTTP 状态描述
     * @return 状态描述文本（如 "OK", "Not Found" 等）
     */
    std::string reason() const noexcept {
        return m_reason;
    }

    /**
     * @brief 获取指定的响应头字段值
     * @param key 响应头键名（不区分大小写）
     * @return 响应头值，若不存在则返回空字符串
     */
    std::string getHeader(const std::string& key) const noexcept {
        auto it = m_headers.find(key);
        return it != m_headers.end() ? it->second : std::string();
    }

    /**
     * @brief 获取响应体总长度
     * @return Content-Length 的值，若未设置或解析失败则返回 0
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
     * @brief 判断是否为分块传输编码
     * @return true 表示使用 Transfer-Encoding: chunked，false 表示使用 Content-Length
     */
    bool isChunked() const noexcept {
        auto it = m_headers.find("Transfer-Encoding");
        return it != m_headers.end() && it->second == "chunked";
    }

    /**
     * @brief 获取已读取的总字节数
     * @return 累计已读取并传递给回调函数的字节数
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
 * @brief 基于 Boost.Beast/Asio 的高性能 HTTP 客户端
 *
 * 支持 HTTP/1.1 和 HTTPS 协议，提供同步和异步两种请求模式。
 * 采用连接池技术提升性能，支持流式响应处理大文件下载。
 *
 * ## 基础特性
 * - 底层网络库：Boost.Beast (HTTP/1.1) + Boost.Asio (异步 I/O)
 * - 协程支持：C++20 coroutines，返回类型为 boost::asio::awaitable<T>
 * - 连接池：自动管理 HTTP 长连接，支持版本检测
 * - SSL/TLS：可选的 HTTPS 支持（需 OpenSSL）
 * - 超时控制：DNS 解析、连接、发送、接收各阶段超时
 * - 流式处理：支持 Content-Length 和 Transfer-Encoding: chunked
 *
 * ## 运行模式
 * - **内部 io_context 模式**：默认构造函数和带 URL 的构造函数会创建独立的 io_context
 *   并启动后台线程运行事件循环，用户无需手动管理
 * - **外部 io_context 模式**：通过带外部 io_context 参数的构造函数注入，
 *   由外部完全控制生命周期，多个客户端可共享同一 io_context
 *
 * ## 异步操作模式
 * - **等待完成模式**（async_*）：适用于需获取返回值、处理异常或保证顺序的场景
 * - **即发即忘模式**：适用于后台日志、监控上报等不关心结果的场景（待实现）
 *
 * ## 资源管理
 * - 使用 std::unique_ptr<net::io_context> 管理内部 io_context
 * - 创建 executor_work_guard 防止 io_context 在无任务时退出
 * - 析构时先释放 work_guard，再等待工作线程完成，最后停止 io_context
 * - 禁用移动操作，确保资源安全
 *
 * ## 构建配置
 * ```bash
 * # 启用 HTTP 客户端
 * xmake f --http_client=y
 *
 * # 启用 HTTPS 支持（需要 OpenSSL）
 * xmake f --http_client_ssl=y
 * ```
 *
 * ## 使用示例
 * @code
 * // 简单 GET 请求（同步）
 * AsioHttpClient client("https://api.example.com");
 * auto response = client.get("/users");
 * std::cout << response.body() << std::endl;
 *
 * // 异步 GET 请求（C++20 协程）
 * co_await client.async_get("/users");
 *
 * // 流式下载大文件
 * std::ofstream file("download.bin", std::ios::binary);
 * co_await client.async_getStream("/largefile",
 *     [&file](const char* data, size_t size) {
 *         file.write(data, size);
 *     });
 *
 * // 自定义 CA 证书（HTTPS）
 * client.setCaFile("/path/to/ca.pem");
 * @endcode
 *
 * @note 该类不可移动或拷贝，因为管理着后台线程和 io_context 的生命周期
 * @note 使用外部 io_context 时需确保其生命周期长于客户端
 * @see AsioHttpResponse 完整响应类
 * @see AsioHttpStreamResponse 流式响应类
 */
class HKU_UTILS_API AsioHttpClient {
public:
    using executor_type = net::any_io_executor;

    /// @brief 默认超时时间（毫秒）
    static constexpr int32_t DEFAULT_TIMEOUT_MS = 30000;  // 30 秒

    /// @brief 最大超时时间（毫秒），当传入<=0 时使用此值
    static constexpr int32_t MAX_TIMEOUT_MS = 60000;  // 60 秒

    /**
     * @brief 构造函数（内部 io_context 模式）
     *
     * 创建内部的 io_context 并启动后台线程运行事件循环。
     *
     * @param thread_count 工作线程数量，默认为 1
     * @param max_concurrency 连接池最大并发连接数，0 表示无限制
     */
    explicit AsioHttpClient(int32_t thread_count = 1, size_t max_concurrency = 0);

    /**
     * @brief 构造函数（带 URL 的内部 io_context 模式）
     *
     * 创建内部的 io_context 并启动后台线程，同时设置目标 URL。
     *
     * @param url 目标 URL（如 "https://api.example.com:8080/v1"）
     * @param timeout 超时时间（毫秒），若<=0 则使用 MAX_TIMEOUT_MS
     * @param thread_count 工作线程数量，默认为 1
     * @param max_concurrency 连接池最大并发连接数，0 表示无限制
     */
    explicit AsioHttpClient(const std::string& url, int32_t timeout = DEFAULT_TIMEOUT_MS,
                            int32_t thread_count = 1, size_t max_concurrency = 0);

    /**
     * @brief 构造函数（外部 io_context 模式）
     *
     * 使用外部提供的 io_context，不拥有其所有权。
     * 适用于多个客户端共享同一事件循环的场景。
     *
     * @param ctx 外部 io_context 引用，由调用方管理生命周期
     * @param url 目标 URL（如 "https://api.example.com:8080/v1"）
     * @param timeout 超时时间（毫秒），若<=0 则使用 MAX_TIMEOUT_MS
     * @param max_concurrency 连接池最大并发连接数，0 表示无限制
     *
     * @note 外部 io_context 必须由调用方负责运行和维护
     */
    explicit AsioHttpClient(net::io_context& ctx, const std::string& url,
                            int32_t timeout = DEFAULT_TIMEOUT_MS, size_t max_concurrency = 0);

    /**
     * @brief 析构函数
     *
     * 按以下顺序安全释放资源：
     * 1. 释放 executor_work_guard，允许 io_context 自然退出
     * 2. 等待所有工作线程完成
     * 3. 显式停止 io_context
     *
     * @note 确保所有异步操作安全完成，不会强制中断
     */
    virtual ~AsioHttpClient();

    // 禁用拷贝操作
    AsioHttpClient(const AsioHttpClient&) = delete;
    AsioHttpClient& operator=(const AsioHttpClient&) = delete;

    // 禁用移动操作，因为管理后台线程和 io_context 的生命周期不安全
    AsioHttpClient(AsioHttpClient&&) = delete;
    AsioHttpClient& operator=(AsioHttpClient&&) = delete;

    /**
     * @brief 检查客户端是否有效
     * @return true 表示 URL 已正确设置，可以发起请求
     */
    bool valid() const noexcept {
        return !m_url.empty();
    }

    /**
     * @brief 获取当前设置的 URL
     * @return 完整的 URL 字符串
     */
    const std::string& url() const noexcept {
        return m_url;
    }

    /**
     * @brief 设置目标 URL
     *
     * 解析 URL 并提取协议、主机、端口、路径等信息。
     * 会自动更新连接池参数。
     *
     * @param url 完整的 URL（如 "https://api.example.com:8080/v1/users"）
     *
     * @note URL 解析失败时会设置错误标志
     */
    void setUrl(const std::string& url);

    /**
     * @brief 设置超时时间
     *
     * 控制 DNS 解析、连接、发送、接收各阶段的超时时间。
     *
     * @param ms 超时时间（毫秒）
     *
     * @note 若 ms <= 0，会自动使用 MAX_TIMEOUT_MS（60000 毫秒）作为默认值
     * @note 超时设置对所有后续请求生效
     */
    void setTimeout(int32_t ms);

    /**
     * @brief 获取当前超时时间
     * @return 超时时间（毫秒）
     */
    int32_t getTimeout() const noexcept {
        return static_cast<int32_t>(m_timeout.count());
    }

    /**
     * @brief 获取 io_context 的执行器
     *
     * 用于在 AsioHttpClient 管理的 io_context 上启动自定义协程或异步操作。
     * 遵循执行器暴露规范，不暴露内部实现细节。
     *
     * @return net::any_io_executor io_context 的执行器
     *
     * @example
     * @code
     * // 在客户端的事件循环中执行自定义任务
     * co_spawn(client.get_executor(), []() -> net::awaitable<void> {
     *     co_await some_async_operation();
     * }, net::detached);
     * @endcode
     */
    executor_type get_executor() const noexcept {
        return m_ctx->get_executor();
    }

    /**
     * @brief 设置默认请求头（移动语义）
     * @param headers 请求头映射表，将被移动到内部存储
     */
    void setDefaultHeaders(std::map<std::string, std::string>&& headers) {
        m_default_headers = std::move(headers);
    }

    /**
     * @brief 设置自定义 CA 证书文件路径
     *
     * 用于 HTTPS 连接时验证服务器证书。
     * 配置优先级：自定义 CA 证书 > 系统默认证书库。
     *
     * @param filename CA 证书文件路径（PEM 格式）
     *
     * @note 必须在建立 HTTPS 连接前设置
     * @note 文件不存在或格式错误会在连接时抛出异常
     * @see HttpAsyncClient 异步通信、资源管理与超时控制规范第 7 条
     */
    void setCaFile(const std::string& filename);

    /**
     * @brief 设置默认请求头（拷贝语义）
     * @param headers 请求头映射表，会被拷贝到内部存储
     */
    void setDefaultHeaders(const HttpHeaders& headers) {
        m_default_headers = headers;
    }

    // ==================== 异步请求方法 ====================
    // 返回 net::awaitable，需在协程中使用 co_await 调用

    /**
     * @brief 通用异步 HTTP 请求
     *
     * 底层的异步请求实现，支持所有 HTTP 方法和自定义参数。
     * 自动处理 DNS 解析、连接建立、请求发送、响应接收全过程。
     *
     * @param method HTTP 方法（GET, POST, PUT, DELETE 等）
     * @param path 请求路径（如 "/api/users"）
     * @param params URL 查询参数（GET 请求）或表单参数（POST 表单）
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针（POST/PUT 请求）
     * @param body_len 请求体长度
     * @param content_type 内容类型（如 "application/json", "text/plain"）
     * @return AsioHttpResponse 完整的 HTTP 响应
     *
     * @throws HttpTimeoutException 超时时
     * @throws boost::system::system_error 网络错误时
     *
     * @note 超时或网络错误时会抛出 boost::system::system_error 异常
     * @note 可通过捕获 system_error 并检查 error_code 判断具体错误原因
     * @note operation_aborted 表示因超时被取消
     */
    net::awaitable<AsioHttpResponse> async_request(const std::string& method,
                                                   const std::string& path,
                                                   const HttpParams& params,
                                                   const HttpHeaders& headers, const char* body,
                                                   size_t body_len,
                                                   const std::string& content_type);

    /**
     * @brief 异步 GET 请求（无参数）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_get(const std::string& path,
                                               const HttpHeaders& headers = {}) {
        co_return co_await async_request("GET", path, {}, headers, nullptr, 0, "");
    }

    /**
     * @brief 异步 GET 请求（带查询参数）
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_get(const std::string& path, const HttpParams& params,
                                               const HttpHeaders& headers) {
        co_return co_await async_request("GET", path, params, headers, nullptr, 0, "");
    }

    /**
     * @brief 异步 POST 请求（带参数和请求体）
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param len 请求体长度
     * @param content_type 内容类型
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers, const char* body,
                                                size_t len, const std::string& content_type) {
        co_return co_await async_request("POST", path, params, headers, body, len, content_type);
    }

    /**
     * @brief 异步 POST 请求（仅请求体）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param len 请求体长度
     * @param content_type 内容类型
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const char* body, size_t len,
                                                const std::string& content_type) {
        co_return co_await async_request("POST", path, {}, headers, body, len, content_type);
    }

    /**
     * @brief 异步 POST 请求（字符串内容）
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param content 请求体字符串
     * @param content_type 内容类型，默认为 "text/plain"
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers,
                                                const std::string& content,
                                                const std::string& content_type = "text/plain") {
        co_return co_await async_post(path, params, headers, content.data(), content.size(),
                                      content_type);
    }

    /**
     * @brief 异步 POST 请求（字符串内容，无参数）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param content 请求体字符串
     * @param content_type 内容类型，默认为 "text/plain"
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const std::string& content,
                                                const std::string& content_type = "text/plain") {
        co_return co_await async_post(path, {}, headers, content, content_type);
    }

    /**
     * @brief 异步 POST 请求（JSON 数据，带参数）
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     *
     * @note 自动设置 Content-Type 为 "application/json"
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpParams& params,
                                                const HttpHeaders& headers, const json& body) {
        co_return co_await async_post(path, params, headers, body.dump(), "application/json");
    }

    /**
     * @brief 异步 POST 请求（JSON 数据，带请求头）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const HttpHeaders& headers,
                                                const json& body) {
        co_return co_await async_post(path, {}, headers, body);
    }

    /**
     * @brief 异步 POST 请求（JSON 数据，简化版）
     * @param path 请求路径
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    net::awaitable<AsioHttpResponse> async_post(const std::string& path, const json& body) {
        co_return co_await async_post(path, {}, body);
    }

    /**
     * @brief 流式异步 HTTP 请求（支持大文件下载）
     *
     * 使用回调函数处理响应数据块，避免一次性加载到内存。
     * 自动支持 Content-Length 和 Transfer-Encoding: chunked 两种模式。
     *
     * @param method HTTP 方法 (GET, POST 等)
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数，每次接收到数据时调用
     * @return AsioHttpStreamResponse 流式响应对象
     *
     * @note 回调函数应保持非阻塞，避免执行耗时操作
     * @note 不应在回调中抛出异常，否则会终止请求
     * @note 注意共享数据的线程安全
     */
    net::awaitable<AsioHttpStreamResponse> async_requestStream(
      const std::string& method, const std::string& path, const HttpParams& params,
      const HttpHeaders& headers, const char* body, size_t body_len,
      const std::string& content_type, const HttpChunkCallback& chunk_callback);

    /**
     * @brief 流式异步 GET 请求（带参数）
     *
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    net::awaitable<AsioHttpStreamResponse> async_getStream(
      const std::string& path, const HttpParams& params, const HttpHeaders& headers,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("GET", path, params, headers, nullptr, 0, "",
                                               chunk_callback);
    }

    /**
     * @brief 流式异步 GET 请求（无参数）
     *
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    net::awaitable<AsioHttpStreamResponse> async_getStream(
      const std::string& path, const HttpHeaders& headers,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("GET", path, {}, headers, nullptr, 0, "",
                                               chunk_callback);
    }

    /**
     * @brief 流式异步 POST 请求（带参数和请求体）
     *
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    net::awaitable<AsioHttpStreamResponse> async_postStream(
      const std::string& path, const HttpParams& params, const HttpHeaders& headers,
      const char* body, size_t body_len, const std::string& content_type,
      const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("POST", path, params, headers, body, body_len,
                                               content_type, chunk_callback);
    }

    /**
     * @brief 流式异步 POST 请求（仅请求体）
     *
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    net::awaitable<AsioHttpStreamResponse> async_postStream(
      const std::string& path, const HttpHeaders& headers, const char* body, size_t body_len,
      const std::string& content_type, const HttpChunkCallback& chunk_callback) {
        co_return co_await async_requestStream("POST", path, {}, headers, body, body_len,
                                               content_type, chunk_callback);
    }

    // ==================== 同步请求方法 ====================
    // 阻塞直到请求完成，内部使用异步实现

    /**
     * @brief 通用同步 HTTP 请求
     *
     * 阻塞式请求，内部使用异步实现并等待完成。
     * 适用于非协程环境或需要同步调用的场景。
     *
     * @param method HTTP 方法（GET, POST, PUT, DELETE 等）
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @return AsioHttpResponse 完整的 HTTP 响应
     *
     * @throws HttpTimeoutException 超时时
     * @throws boost::system::system_error 网络错误时
     */
    AsioHttpResponse request(const std::string& method, const std::string& path,
                             const HttpParams& params, const HttpHeaders& headers, const char* body,
                             size_t body_len, const std::string& content_type);

    /**
     * @brief 同步 GET 请求（无参数）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse get(const std::string& path, const HttpHeaders& headers = {}) {
        return request("GET", path, {}, headers, nullptr, 0, "");
    }

    /**
     * @brief 同步 GET 请求（带查询参数）
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse get(const std::string& path, const HttpParams& params,
                         const HttpHeaders& headers) {
        return request("GET", path, params, headers, nullptr, 0, "");
    }

    /**
     * @brief 同步 POST 请求（带参数和请求体）
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param len 请求体长度
     * @param content_type 内容类型
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const char* body, size_t len,
                          const std::string& content_type) {
        return request("POST", path, params, headers, body, len, content_type);
    }

    /**
     * @brief 同步 POST 请求（仅请求体）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param len 请求体长度
     * @param content_type 内容类型
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers, const char* body,
                          size_t len, const std::string& content_type) {
        return request("POST", path, {}, headers, body, len, content_type);
    }

    /**
     * @brief 同步 POST 请求（字符串内容，带参数）
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param content 请求体字符串
     * @param content_type 内容类型，默认为 "text/plain"
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const std::string& content,
                          const std::string& content_type = "text/plain") {
        return post(path, params, headers, content.data(), content.size(), content_type);
    }

    /**
     * @brief 同步 POST 请求（字符串内容，无参数）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param content 请求体字符串
     * @param content_type 内容类型，默认为 "text/plain"
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers,
                          const std::string& content,
                          const std::string& content_type = "text/plain") {
        return post(path, {}, headers, content, content_type);
    }

    /**
     * @brief 同步 POST 请求（JSON 数据，带参数）
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpParams& params,
                          const HttpHeaders& headers, const json& body) {
        return post(path, params, headers, body.dump(), "application/json");
    }

    /**
     * @brief 同步 POST 请求（JSON 数据，带请求头）
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const HttpHeaders& headers, const json& body) {
        return post(path, {}, headers, body);
    }

    /**
     * @brief 同步 POST 请求（JSON 数据，简化版）
     * @param path 请求路径
     * @param body JSON 对象
     * @return AsioHttpResponse 完整的 HTTP 响应
     */
    AsioHttpResponse post(const std::string& path, const json& body) {
        return post(path, {}, body);
    }

    /**
     * @brief 同步流式 HTTP 请求（支持大文件下载）
     *
     * 阻塞式流式请求，使用回调函数处理响应数据块。
     * 自动支持 Content-Length 和 Transfer-Encoding: chunked 两种模式。
     *
     * @param method HTTP 方法 (GET, POST 等)
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数，每次接收到数据时调用
     * @return AsioHttpStreamResponse 流式响应对象
     *
     * @note 回调函数应保持非阻塞，避免执行耗时操作
     * @note 不应在回调中抛出异常，否则会终止请求
     * @note 注意共享数据的线程安全
     */
    AsioHttpStreamResponse requestStream(const std::string& method, const std::string& path,
                                         const HttpParams& params, const HttpHeaders& headers,
                                         const char* body, size_t body_len,
                                         const std::string& content_type,
                                         const HttpChunkCallback& chunk_callback);

    /**
     * @brief 同步流式 GET 请求（带参数）
     *
     * @param path 请求路径
     * @param params URL 查询参数
     * @param headers 额外的 HTTP 请求头
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    AsioHttpStreamResponse getStream(const std::string& path, const HttpParams& params,
                                     const HttpHeaders& headers,
                                     const HttpChunkCallback& chunk_callback) {
        return requestStream("GET", path, params, headers, nullptr, 0, "", chunk_callback);
    }

    /**
     * @brief 同步流式 GET 请求（无参数）
     *
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    AsioHttpStreamResponse getStream(const std::string& path, const HttpHeaders& headers,
                                     const HttpChunkCallback& chunk_callback) {
        return requestStream("GET", path, {}, headers, nullptr, 0, "", chunk_callback);
    }

    /**
     * @brief 同步流式 POST 请求（带参数和请求体）
     *
     * @param path 请求路径
     * @param params URL 查询参数或表单参数
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
     */
    AsioHttpStreamResponse postStream(const std::string& path, const HttpParams& params,
                                      const HttpHeaders& headers, const char* body, size_t body_len,
                                      const std::string& content_type,
                                      const HttpChunkCallback& chunk_callback) {
        return requestStream("POST", path, params, headers, body, body_len, content_type,
                             chunk_callback);
    }

    /**
     * @brief 同步流式 POST 请求（仅请求体）
     *
     * @param path 请求路径
     * @param headers 额外的 HTTP 请求头
     * @param body 请求体指针
     * @param body_len 请求体长度
     * @param content_type 内容类型
     * @param chunk_callback 数据块回调函数
     * @return AsioHttpStreamResponse 流式响应对象
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
     * @brief 解析 URL
     *
     * 提取协议、主机、端口、路径等信息。
     * 仅负责解析逻辑，不修改外部状态。
     */
    void _parseUrl() noexcept;

    /**
     * @brief DNS 解析
     *
     * 异步解析主机名为 IP 地址列表。
     *
     * @return IP 端点列表
     */
    net::awaitable<std::vector<tcp::endpoint>> _resolveDNS();

    struct SocketVariant;

    /**
     * @brief 建立 TCP 连接
     *
     * 尝试连接到 DNS 解析得到的端点列表。
     *
     * @param socket_variant Socket 变体（TCP 或 SSL）
     * @param dns_endpoints DNS 解析得到的端点列表
     */
    net::awaitable<void> _connect(SocketVariant& socket_variant,
                                  const std::vector<tcp::endpoint>& dns_endpoints);

    /**
     * @brief 从连接池获取已连接的 socket
     *
     * 带版本检查的连接池复用机制。
     *
     * @return pair<连接对象，是否新创建> 若为 true 表示是新创建的连接
     */
    net::awaitable<std::pair<std::shared_ptr<HttpConnection>, bool>> _getConnection();

    /**
     * @brief 构建完整的 URI（路径 + 查询参数）
     *
     * 负责将基础路径、资源路径和查询参数组合成合法的 URI。
     * 自动处理路径分段 URL 编码、斜杠拼接去重等逻辑。
     *
     * @param path 资源路径（如 "api/v1/users" 或 "/api/v1/users"）
     * @param params 查询参数映射
     * @return 构建好的完整 URI 字符串
     */
    std::string _buildURI(const std::string& path, const HttpParams& params);

private:
#if HKU_ENABLE_HTTP_CLIENT_SSL
    struct SslContext;
    std::unique_ptr<SslContext> m_ssl_ctx;  // SSL 上下文（仅在启用 SSL 时使用）
#endif

    bool m_is_valid_url{false};                               // URL 是否有效
    bool m_is_https{false};                                   // 是否使用 HTTPS 协议
    std::string m_url;                                        // 完整的 URL
    std::string m_base_path;                                  // URL 的基础路径部分
    std::string m_host;                                       // 主机名
    std::string m_port;                                       // 端口号
    std::chrono::milliseconds m_timeout{DEFAULT_TIMEOUT_MS};  // 超时时间
    std::map<std::string, std::string> m_default_headers;     // 默认请求头
    std::string m_ca_file;                                    // 自定义 CA 证书文件路径

    // 连接池相关成员
    std::unique_ptr<ResourceAsioVersionPool<HttpConnection, std::mutex>> m_connection_pool;

    // io_context 管理
    std::unique_ptr<net::io_context> m_own_ctx;  // 内部 io_context
    net::io_context* m_ctx{nullptr};             // 当前使用的 io_context
    std::vector<std::thread> m_worker_threads;   // 后台运行 io_context 的线程池
    std::unique_ptr<net::executor_work_guard<net::io_context::executor_type>>
      m_work_guard;  // 防止 io_context 在无任务时退出
};

}  // namespace hku

#endif