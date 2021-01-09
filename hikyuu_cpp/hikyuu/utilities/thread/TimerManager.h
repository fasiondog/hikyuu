/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-08
 *     Author: fasiondog
 *
 *  参考：https://github.com/fightingwangzq/wzq_utils 实现
 *
 */

#pragma once

#include <forward_list>
#include "../../datetime/Datetime.h"
#include "../../Log.h"
#include "../Null.h"
#include "ThreadPool.h"

namespace hku {

class TimerManager {
public:
    TimerManager() : m_current_timer_id(-1), m_stop(true) {}

    ~TimerManager() {
        stop();
        m_tg->stop();
    }

    void start() {
        if (m_stop) {
            m_stop = false;
            if (!m_tg) {
                m_tg = std::make_unique<ThreadPool>();
            }

            /*
             * 根据已有 timer 重建执行队列，并删除已无效的 timer
             */

            std::forward_list<int> invalid_timers;  // 记录已无效的 timer
            std::unique_lock<std::mutex> lock(m_mutex);
            for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
                int time_id = iter->first;
                Timer* timer = iter->second;

                Datetime now = Datetime::now();
                if (timer->m_repeat_num <= 0 || timer->m_end_date <= now) {
                    invalid_timers.push_front(time_id);
                } else {
                    IntervalS s;
                    s.m_timer_id = time_id;
                    s.m_time_point = now + timer->m_duration;
                    TimeDelta point(0, s.m_time_point.hour(), s.m_time_point.minute(),
                                    s.m_time_point.second(), s.m_time_point.millisecond(),
                                    s.m_time_point.microsecond());
                    if (point < timer->m_start_time) {
                        s.m_time_point = s.m_time_point.startOfDay() + timer->m_start_time;
                    } else if (point > timer->m_end_time) {
                        s.m_time_point =
                          s.m_time_point.startOfDay() + timer->m_start_time + TimeDelta(1);
                    } else if (timer->m_start_time != TimeDelta()) {
                        TimeDelta gap = point - timer->m_start_time;
                        if (gap % timer->m_duration != TimeDelta()) {
                            int x = int(gap / timer->m_duration);
                            s.m_time_point = s.m_time_point.startOfDay() + timer->m_start_time +
                                             timer->m_duration * double(x);
                        }
                    }

                    m_queue.push(s);
                }
            }

            // 清除已无效的 timer
            for (auto id : invalid_timers) {
                removeTimer(id);
            }

            lock.unlock();
            m_cond.notify_all();

            std::thread([this]() { detectThread(); }).detach();
        }
    }

    void stop() {
        if (!m_stop) {
            m_stop = true;
            std::unique_lock<std::mutex> lock(m_mutex);
            std::priority_queue<IntervalS> queue;
            m_queue.swap(queue);
            lock.unlock();
            m_cond.notify_all();
        }
    }

    template <typename R, typename P, typename F, typename... Args>
    void add_func_after_duration(TimeDelta duration, F&& f, Args&&... args) {
        Timer* t = new Timer;
        t->m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        IntervalS s;
        s.m_time_point = Datetime::now() + duration;

        std::unique_lock<std::mutex> lock(m_mutex);
        int id = getNewTimerId();
        HKU_IF_RETURN(id < 0, void());
        m_timers[id] = t;
        m_queue.push(s);
        lock.unlock();
        m_cond.notify_all();
    }

    /*template <typename F, typename... Args>
            void add_func_at_time_point(
              const std::chrono::time_point<std::chrono::high_resolution_clock>& time_point, F&&
       f, Args&&... args) { IntervalS t; t.m_time_point = time_point; t.m_func =
       std::bind(std::forward<F>(f), std::forward<Args>(args)...); std::unique_lock<std::mutex>
       lock(m_mutex); m_queue.push(t); m_cond.notify_all();
            }*/

private:
    void removeTimer(int id) {
        delete m_timers[id];
        m_timers.erase(id);
    }

    void detectThread() {
        while (!m_stop) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_queue.empty()) {
                m_cond.wait(lock);
                continue;
            }

            Datetime now = Datetime::now();
            IntervalS s = m_queue.top();
            TimeDelta diff = s.m_time_point - now;
            if (diff > TimeDelta()) {
                m_cond.wait_for(lock, std::chrono::duration<int64_t, std::micro>(diff.ticks()));
                continue;
            } else {
                m_queue.pop();
                auto timer = m_timers[s.m_timer_id];
                m_tg->submit(*timer);
                timer->m_repeat_num--;
                if (timer->m_repeat_num <= 0) {
                    delete timer;
                    m_timers.erase(s.m_timer_id);
                } else {
                    s.m_time_point = s.m_time_point + timer->m_duration;
                    if (s.m_time_point <= timer->m_end_date) {
                        m_queue.push(s);
                    }
                }
            }
        }
    }

    // 分配 timer_id
    int getNewTimerId() {
        int max_int = std::numeric_limits<int>::max();
        HKU_WARN_IF_RETURN(m_timers.size() >= max_int, -1, "Timer queue is full!");

        int id = m_current_timer_id >= max_int ? 0 : m_current_timer_id;
        while (true) {
            if (m_timers.find(id) != m_timers.end()) {
                if (id >= max_int) {
                    id = 0;
                } else {
                    id++;
                }
            } else {
                break;
            }
        }
        m_current_timer_id = id;
        return id;
    }

private:
    class Timer {
    public:
        void operator()() {
            m_func();
        }

        Datetime m_start_date = Datetime::min().startOfDay();  // 允许执行的起始日期（包含该日期）
        Datetime m_end_date = Datetime::max().startOfDay();  // 允许执行的终止日期（包含该日期）
        TimeDelta m_start_time;  // 允许执行的当日起始时间（包含该时间）
        TimeDelta m_end_time;    // 允许执行的当日结束时间（包含该时间）
        TimeDelta m_duration;    // 延迟时长或间隔时长
        int m_repeat_num = 1;    // 重复执行次数，max标识无限循环
        std::function<void()> m_func;
    };

    struct IntervalS {
        Datetime m_time_point;  // 执行的精确时间点
        int m_timer_id = -1;    // 对应的 Timer, 负数无效
        bool operator<(const IntervalS& other) const {
            return m_time_point > other.m_time_point;
        }
    };

private:
    std::priority_queue<IntervalS> m_queue;
    std::atomic_bool m_stop;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    std::unordered_map<int, Timer*> m_timers;
    int m_current_timer_id;
    std::unique_ptr<ThreadPool> m_tg;
};  // namespace hku

}  // namespace hku