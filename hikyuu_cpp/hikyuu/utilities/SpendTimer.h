/*
 * SpendTimer.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-6-15
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_SPENDTIMER_H_
#define HIKYUU_UTILITIES_SPENDTIMER_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <fmt/format.h>

#ifndef HKU_API
#define HKU_API
#endif
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifndef HKU_CLOSE_SPEND_TIME
#define HKU_CLOSE_SPEND_TIME 1
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4834)
#endif

#ifdef __GNUC__
// 关闭 using  _Base::_Base; 这行代码产生的警告
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

namespace hku {

/**
 * @ingroup Utilities
 * @addtogroup SpendTimer Spend timer 耗时统计
 * @details 辅助统计代码块执行时间相关的工具宏，编译时可通过指定宏 HKU_CLOSE_SPEND_TIME=1
 * 进行关闭，或 HKU_CLOSE_SPEND_TIME=0 开启耗时统计
 * @code
 *     {
 *        SPEND_TIME(test, "run test");
 *        ...
 *     }
 * @endcode
 * @{
 */
#if HKU_CLOSE_SPEND_TIME
#define SPEND_TIME(id)
#define SPEND_TIME_MSG(id, ...)
#define SPEND_TIME_KEEP(id, ...)
#define SPEND_TIME_SHOW(id)
#define SPEND_TIME_VALUE(id)
#define BENCHMARK_TIME(id, cycle)
#define BENCHMARK_TIME_MSG(id, cycle, ...)
#define SPEND_TIME_CONTROL(open)
#define OPEN_SPEND_TIME
#define CLOSE_SPEND_TIME

#else /* #if HKU_CLOSE_SPEND_TIME */
/**
 * 代码执行耗时计时器
 * @param id 自定义耗时计时器id
 */
#define SPEND_TIME(id) hku::SpendTimer test_spend_timer_##id(#id, __FILE__, __LINE__);

/**
 * 代码执行耗时计时器，附带输出信息
 * @param id 自定义耗时计时器id
 * @param ... 输出信息
 */
#define SPEND_TIME_MSG(id, ...)                     \
    std::string msg_##id(fmt::format(__VA_ARGS__)); \
    hku::SpendTimer test_spend_timer_##id(#id, msg_##id.c_str(), __FILE__, __LINE__);

/** 秒表计时 */
#define SPEND_TIME_KEEP(id, ...) test_spend_timer_##id.keep(fmt::format(__VA_ARGS__));

/** 显示当前耗时 */
#define SPEND_TIME_SHOW(id) test_spend_timer_##id.show();

/** 获取启动到当前的耗时秒数 */
#define SPEND_TIME_VALUE(id) test_spend_timer_##id.value()

/**
 * 用于动态控制当前代码块及其子块中的耗时计时器，主要用于测试代码中关闭和开启部分耗时统计
 */
#define SPEND_TIME_CONTROL(open) hku::SpendTimerGuad spend_timer_guard_##open(open);

/** 全局开启消息耗时打印 */
#define OPEN_SPEND_TIME hku::open_spend_time()

/** 全局关闭消息耗时打印 */
#define CLOSE_SPEND_TIME hku::close_spend_time()

/**
 * Bechmark耗时计时器
 * @param id 自定义耗时计时器id
 * @param cycle  循环运行的次数（仅用于统计打印耗时输出）
 */
#define BENCHMARK_TIME(id, cycle)                                   \
    hku::SpendTimer test_spend_timer_##id(#id, __FILE__, __LINE__); \
    test_spend_timer_##id.setCycle(cycle);

/**
 * Bechchmark耗时计时器，附带输出信息
 * @param id 自定义耗时计时器id
 * @param cycle  循环运行的次数（仅用于统计打印耗时输出）
 * @param ... 输出信息
 */
#define BENCHMARK_TIME_MSG(id, cycle, ...)                                            \
    std::string msg_##id(fmt::format(__VA_ARGS__));                                   \
    hku::SpendTimer test_spend_timer_##id(#id, msg_##id.c_str(), __FILE__, __LINE__); \
    test_spend_timer_##id.setCycle(cycle);

#endif /* HKU_CLOSE_SPEND_TIME */

/**
 * 程序执行耗时统计计时器，辅助计算代码执行时间
 * @note 不建议直接使用，应使用相关工具宏
 * @see SPEND_TIME, SPEND_TIME_MSG, SPEND_TIMG_CONTROL
 */
class HKU_API SpendTimer {
public:
    /** 构造函数，记录当前系统时间 */
    explicit SpendTimer()
    : m_cycle(1),
      m_msg(""),
      m_lineno(0),
      m_start_time(std::chrono::steady_clock::now()),
      m_pre_keep_time(m_start_time) {}

    /**
     * 构造函数，记录当前系统时间
     * @param id 计时器id
     * @param filename 当前文件名，对应 __FILE__
     * @param lineno 当前行号，对应 __LINE__
     */
    explicit SpendTimer(const char *id, const char *filename, int lineno)
    : m_cycle(1),
      m_id(id),
      m_msg(""),
      m_filename(filename),
      m_lineno(lineno),
      m_start_time(std::chrono::steady_clock::now()),
      m_pre_keep_time(m_start_time) {}

    /**
     * 构造函数，记录当前系统时间
     * @param id 计时器id
     * @param msg 附加输出信息
     * @param filename 当前文件名，对应 __FILE__
     * @param lineno 当前行号，对应 __LINE__
     */
    explicit SpendTimer(const char *id, const char *msg, const char *filename, int lineno)
    : m_cycle(1),
      m_id(id),
      m_msg(msg),
      m_filename(filename),
      m_lineno(lineno),
      m_start_time(std::chrono::steady_clock::now()),
      m_pre_keep_time(m_start_time) {}

    /** 析构函数，计算从构造至析构所消耗的时间，并打印输出 */
    virtual ~SpendTimer();

    /**
     * @brief 获取从启动至当前时间发生的耗时，单位秒
     * @return std::chrono::duration<double>
     */
    std::chrono::duration<double> duration() const {
        return std::chrono::steady_clock::now() - m_start_time;
    }

    /** 打印计时器启动到现在的耗时，同时返回当前耗时的秒数 */
    void show() const;

    /** 获取启动到当前耗时的秒数 */
    double value() const;

    /**
     * @brief 秒针计时，每 keep 一下，记录当前时间到上次keep的间隔时间
     * @param description 描述说明
     * @note 首次 keep 为当前时间至启动时的时间间隔
     */
    void keep(const std::string &description);

    /**
     * @brief 获取秒表计时列表
     * @return const std::vector<std::chrono::duration<double>>&
     */
    const std::vector<std::chrono::duration<double>> &getKeepDurations() const {
        return m_keep_seconds;
    }

    /**
     * bencmark测试时告知循环的次数
     * @param cycle 循环次数，仅用于统计耗时打印输出
     */
    void setCycle(int cycle) {
        m_cycle = cycle;
    }

public:
    /** 获取当前耗时打印开关状态 */
    static bool isClosed() {
        return ms_closed;
    }

private:
    int m_cycle;  // bencmark测试时，告知循环的次数
    std::string m_id;
    std::string m_msg;
    std::string m_filename;
    int m_lineno;
    std::chrono::time_point<std::chrono::steady_clock> m_start_time;
    std::chrono::time_point<std::chrono::steady_clock> m_pre_keep_time;
    std::vector<std::chrono::duration<double>> m_keep_seconds;
    std::vector<std::string> m_keep_desc;

    static bool ms_closed;
    friend void HKU_API close_spend_time();
    friend void HKU_API open_spend_time();
    friend bool HKU_API get_spend_time_status();
};

/** 全局关闭耗时打印输出 */
void HKU_API close_spend_time();

/** 全局开启耗时打印输出 */
void HKU_API open_spend_time();

/** 获取全局耗时打印输出状态：true - 开启，false - 关闭 */
bool HKU_API get_spend_time_status();

/**
 * 耗时计时器开启关闭状态看守，记录之前的耗时开关状态，并置为指定状态，释放时恢复原状态
 */
class SpendTimerGuad {
public:
    /**
     * 构造函数，记录当前状态，并根据 open 指示开启或关闭耗时统计
     * @param open 是否开启耗时统计
     */
    explicit SpendTimerGuad(bool open) : m_open(open), m_old_open(false) {
        m_old_open = !SpendTimer::isClosed();
        if (m_open == m_old_open) {
            return;
        }
        if (m_open) {
            open_spend_time();
        } else {
            close_spend_time();
        }
    }

    /** 析构函数，退出当前指定状态，恢复原状态 */
    ~SpendTimerGuad() {
        if (m_open == m_old_open) {
            return;
        }
        if (m_old_open) {
            open_spend_time();
        } else {
            close_spend_time();
        }
    }

private:
    bool m_open;
    bool m_old_open;
};

/** @} */

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* HIKYUU_UTILITIES_SPENDTIMER_H_ */