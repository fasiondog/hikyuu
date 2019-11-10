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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

#ifndef HKU_API
#define HKU_API
#endif

#ifdef __ANDROID__
#include <android/log.h>
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

#if HIKYUU_CLOSE_SPEND_TIME
#define SPEND_TIME(id)
#define SPEND_TIME_MSG(id, msg)
#define close_spend_time()
#define open_spend_time()

#else /* #if HIKYUU_CLOSE_SPEND_TIME */
#define SPEND_TIME(id)                                                    \
    std::stringstream sptmsg_buf_##id(std::stringstream::out);            \
    sptmsg_buf_##id << #id << " (" << __FILE__ << ":" << __LINE__ << ")"; \
    SpendTimer test_spend_timer_##id(sptmsg_buf_##id.str());
#define SPEND_TIME_MSG(id, msg)                                                          \
    std::stringstream sptmsg_buf_##id(std::stringstream::out);                           \
    sptmsg_buf_##id << #id << ": " << msg << " (" << __FILE__ << ":" << __LINE__ << ")"; \
    SpendTimer test_spend_timer_##id(sptmsg_buf_##id.str());

class HKU_API SpendTimer {
public:
    SpendTimer() : m_msg(""), m_start_time(std::chrono::steady_clock::now()) {}

    explicit SpendTimer(const std::string& msg)
    : m_msg(msg), m_start_time(std::chrono::steady_clock::now()) {}

    virtual ~SpendTimer() {
        if (m_closed) {
            return;
        }
        std::chrono::duration<double> sec = std::chrono::steady_clock::now() - m_start_time;
        std::ostringstream buf;
        buf.fill(' ');
        buf.precision(m_precision);
        if (sec.count() < 0.000001) {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() * 1000000000
                << " ns | " << m_msg;
        } else if (sec.count() < 0.001) {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() * 1000000
                << " us | " << m_msg;
        } else if (sec.count() < 1) {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() * 1000
                << " ms | " << m_msg;
        } else if (sec.count() > 60) {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() / 60
                << "  m | " << m_msg;
        } else if (sec.count() > 86400) {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() / 360
                << "  h | " << m_msg;
        } else {
            buf << "spend time: " << std::setw(m_width) << std::right << sec.count() << "  s | "
                << m_msg;
        }

#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_INFO, "HIKYUU", buf.str().c_str());
#if YIHUA_WITH_ANDROID_SHELL_OUTPUT
        std::cout << buf.str() << std::endl;
#endif
#else  /* __ANDROID__ */
        std::cout << buf.str() << std::endl;
#endif /* __ANDROID__ */
    }

private:
    std::string m_msg;
    std::chrono::time_point<std::chrono::steady_clock> m_start_time;

    static bool m_closed;
    static const int m_width = 7;
    static constexpr int m_precision = m_width - 1;
    friend void HKU_API close_spend_time();
    friend void HKU_API open_spend_time();
};

inline void HKU_API close_spend_time() {
    SpendTimer::m_closed = true;
}

inline void HKU_API open_spend_time() {
    SpendTimer::m_closed = false;
}

#endif /* HIKYUU_CLOSE_SPEND_TIME */

} /* namespace hku */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* HIKYUU_UTILITIES_SPENDTIMER_H_ */