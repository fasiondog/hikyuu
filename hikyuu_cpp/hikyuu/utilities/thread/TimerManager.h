/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-08
 *     Author: fasiondog
 */

#pragma once

#include <ThreadPool.h>

namespace hku {

class TimerManager {
public:
    struct Timer {
        std::chrono::time_point<std::chrono::high_resolution_clock> m_time_point;
        std::function<void()> m_func;
        int repeated_id;
        bool operator<(const Timer& other) const {
            return m_time_point > other.m_time_point;
        }
    };

    TimerManager() : m_stop(true) {}
    ~TimerManager() = default;

    void start() {
        m_stop = false;
        std::thread([this]() { detect_thread(); }).detach();
    }

    void stop() {
        m_stop = true;
        m_cond.notify_all();
        m_tg.stop();
    }

    template <typename R, typename P, typename F, typename... Args>
    void add_func_after_duration(const std::chrono::duration<R, P>& time, F&& f, Args&&... args) {
        Timer t;
        t.m_time_point = std::chrono::high_resolution_clock::now() + time;
        t.m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
        m_cond.notify_all();
    }

    template <typename F, typename... Args>
    void add_func_at_time_point(
      const std::chrono::time_point<std::chrono::high_resolution_clock>& time_point, F&& f,
      Args&&... args) {
        Timer t;
        t.m_time_point = time_point;
        t.m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
        m_cond.notify_all();
    }

private:
    void detect_thread() {
        while (!m_stop) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_queue.empty()) {
                m_cond.wait(lock);
                continue;
            }

            auto timer = m_queue.top();
            auto diff = timer.m_time_point - std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() > 0) {
                m_cond.wait_for(lock, diff);
                continue;
            } else {
                m_queue.pop();
                lock.unlock();
                m_tg.submit(timer.m_func);
            }
        }
    }

private:
    std::priority_queue<Timer> m_queue;
    std::atomic_bool m_stop;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    ThreadPool m_tg;
};

}  // namespace hku