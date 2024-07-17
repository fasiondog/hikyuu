/*
 * SpendTimer.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-8-3
 *      Author: fasiondog
 */

#include <tuple>
#include "SpendTimer.h"

namespace hku {

#define MAX_SPEND_MSG_LEN 1024

#if HKU_CLOSE_SPEND_TIME
bool SpendTimer::ms_closed = true;
#else
bool SpendTimer::ms_closed = false;
#endif

static std::tuple<double, std::string> transUnit(std::chrono::duration<double> sec) {
    std::string unit;
    double duration = 0.0;
    if (sec.count() < 0.000001) {
        unit = "ns";
        duration = sec.count() * 1000000000;
    } else if (sec.count() < 0.001) {
        unit = "us";
        duration = sec.count() * 1000000;
    } else if (sec.count() < 1) {
        unit = "ms";
        duration = sec.count() * 1000;
    } else if (sec.count() > 60) {
        unit = " m";
        duration = sec.count() / 60;
    } else if (sec.count() > 86400) {
        unit = " h";
        duration = sec.count() / 360;
    } else {
        unit = " s";
        duration = sec.count();
    }
    return std::make_tuple(duration, unit);
}

SpendTimer::~SpendTimer() {
    if (ms_closed) {
        return;
    }
    show();
    size_t total = m_keep_seconds.size();
    if (total > 0) {
        m_keep_seconds.push_back(std::chrono::steady_clock::now() - m_pre_keep_time);
        double duration = 0.0;
        std::string unit;
        for (size_t i = 0; i < total; i++) {
            std::tie(duration, unit) = transUnit(m_keep_seconds[i]);
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_INFO, "HKU", "%6zu keep: %7.3f %s - %s\n", i, duration,
                                unit.c_str(), m_keep_desc[i].c_str());

#if defined(HIKYUU_ENABLE_ANDROID_SHELL_OUTPUT) && HIKYUU_ENABLE_ANDROID_SHELL_OUTPUT
            printf("%6zu keep: %7.3f %s - %s\n", i, duration, unit.c_str(), m_keep_desc[i].c_str());
#endif

#else
            // printf("%5zu keep: %7.3f %s - %s\n", i, duration, unit.c_str(),
            // m_keep_desc[i].c_str());
            std::cout << std::setw(5) << " keep: " << i << std::setw(7) << std::setprecision(3)
                      << duration << " " << unit << " - " << m_keep_desc[i] << std::endl;
#endif /* __ANDROID__ */
        }
    }
}

void SpendTimer::keep(const std::string& description) {
    auto now = std::chrono::steady_clock::now();
    m_keep_seconds.push_back(now - m_pre_keep_time);
    m_keep_desc.push_back(description);
    m_pre_keep_time = now;
}

void SpendTimer::show() const {
    std::chrono::duration<double> sec = std::chrono::steady_clock::now() - m_start_time;
    double duration = 0.0;
    char outmsg[MAX_SPEND_MSG_LEN];
    std::memset(outmsg, 0, MAX_SPEND_MSG_LEN);

    if (m_cycle > 1) {
        duration = sec.count() * 1000;
        snprintf(
          outmsg, MAX_SPEND_MSG_LEN,
          "+------------------------------------------------------------------------------\n"
          "| Benchmark %s %s (%s:%d)\n"
          "+------------------------------------------------------------------------------\n"
          "| average time (ms): %.3f\n"
          "|   total time (ms): %.3f\n"
          "|   run cycle count: %d\n"
          "+------------------------------------------------------------------------------\n",
          m_id.c_str(), m_msg.c_str(), m_filename.c_str(), m_lineno, duration / m_cycle, duration,
          m_cycle);
    } else {
        std::string unit;
        std::tie(duration, unit) = transUnit(sec);
        snprintf(outmsg, MAX_SPEND_MSG_LEN, "spend time: %7.3f %s | %s %s (%s:%d)\n", duration,
                 unit.c_str(), m_id.c_str(), m_msg.c_str(), m_filename.c_str(), m_lineno);
    }

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "HKU", "%s", outmsg);

#if defined(HIKYUU_ENABLE_ANDROID_SHELL_OUTPUT) && HIKYUU_ENABLE_ANDROID_SHELL_OUTPUT
    printf("%s", outmsg);
#endif

#else
    // printf("%s", outmsg);
    std::cout << outmsg;
#endif /* __ANDROID__ */
}

double SpendTimer::value() const {
    std::chrono::duration<double> sec = std::chrono::steady_clock::now() - m_start_time;
    return sec.count();
}

void HKU_UTILS_API close_spend_time() {  // NOSONAR
    SpendTimer::ms_closed = true;
}

void HKU_UTILS_API open_spend_time() {  // NOSONAR
    SpendTimer::ms_closed = false;
}

bool HKU_UTILS_API get_spend_time_status() {
    return !SpendTimer::ms_closed;
}

}  // namespace hku