/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-08
 *     Author: fasiondog
 */

#pragma once

#include <forward_list>
#include <unordered_map>
#include <functional>
#include "hikyuu/utilities/datetime/Datetime.h"
#include "hikyuu/utilities/Log.h"
#include "thread/ThreadPool.h"
#include "cppdef.h"

namespace hku {

/**
 * 定时管理与调度
 * @ingroup Utilities
 */
class TimerManager {
public:
    TimerManager(const TimerManager&) = delete;
    TimerManager(TimerManager&) = delete;
    TimerManager(TimerManager&&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;
    TimerManager& operator=(TimerManager&) = delete;
    TimerManager& operator=(TimerManager&&) = delete;

    /**
     * 构造函数
     * @param work_num 定时任务执行线程池线程数量
     */
    explicit TimerManager(size_t work_num = 1)
    : m_stop(true), m_current_timer_id(-1), m_work_num(work_num) {
        start();
    }

    /** 析构函数 */
    ~TimerManager() {
        stop();
        for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
            delete iter->second;
        }
    }

    /** 启动调度, 可在停止后重新启动 */
    void start() {
        // 已经在执行状态，直接返回
        HKU_IF_RETURN(!m_stop, void());

        // 设置执行状态
        m_stop = false;

        std::unique_lock<std::mutex> lock(m_mutex);

        std::priority_queue<IntervalS> new_queue;
        m_queue.swap(new_queue);
        if (!m_tg) {
#if CPP_STANDARD >= CPP_STANDARD_14
            m_tg = std::make_unique<ThreadPool>(m_work_num);
#else
            m_tg = std::unique_ptr<ThreadPool>(new ThreadPool(m_work_num));
#endif
        }

        /*
         * 根据已有 timer 重建执行队列，并删除已无效的 timer
         */

        std::forward_list<int> invalid_timers;  // 记录已无效的 timer
        for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
            int time_id = iter->first;
            const Timer* timer = iter->second;
            Datetime now = Datetime::now();

            // 记录已失效的 timer id
            if (timer->m_repeat_num <= 0 || (timer->m_end_date != Datetime::max() &&
                                             timer->m_end_date + timer->m_end_time < now)) {
                invalid_timers.push_front(time_id);
                continue;
            }

            IntervalS s;
            s.m_timer_id = time_id;
            if (timer->m_start_time < TimeDelta()) {
                Datetime first_start_time = timer->m_start_date + timer->m_end_time;
                if (first_start_time >= now) {
                    s.m_time_point = first_start_time;
                } else {
                    if (timer->m_repeat_num <= 1) {
                        invalid_timers.push_front(time_id);
                        continue;
                    }
                    s.m_time_point = now.startOfDay() + timer->m_end_time;
                    if (s.m_time_point < now) {
                        s.m_time_point = s.m_time_point + TimeDelta(1);
                    }
                }

            } else {
                s.m_time_point = timer->m_start_date >= now.startOfDay()
                                   ? timer->m_start_date + timer->m_start_time + timer->m_duration
                                   : now + timer->m_duration;
                if (timer->m_start_time != timer->m_end_time) {
                    Datetime point_date = s.m_time_point.startOfDay();
                    TimeDelta point = s.m_time_point - point_date;
                    if (point < timer->m_start_time) {
                        s.m_time_point = point_date + timer->m_start_time;
                    } else if (point > timer->m_end_time) {
                        s.m_time_point = point_date + timer->m_start_time + TimeDelta(1);
                    } else {
                        TimeDelta gap = point - timer->m_start_time;
                        if (gap % timer->m_duration != TimeDelta()) {
                            int x = int(gap / timer->m_duration) + 1;
                            s.m_time_point =
                              point_date + timer->m_start_time + timer->m_duration * double(x);
                        }
                    }
                }
            }

            m_queue.push(s);
        }

        // 清除已无效的 timer
        for (auto id : invalid_timers) {
            _removeTimer(id);
        }

        lock.unlock();
        m_cond.notify_all();

        m_detect_thread = std::thread([this]() { detectThread(); });
    }

    /** 终止调度 */
    void stop() {
        if (!m_stop) {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::priority_queue<IntervalS> queue;
            m_queue.swap(queue);
            m_stop = true;
            lock.unlock();
            m_cond.notify_all();
        }

        if (m_detect_thread.joinable()) {
            m_detect_thread.join();
        }

        if (m_tg) {
            m_tg->stop();
            m_tg.reset();
        }
    }

    /** 获取当前定时任务数量 */
    size_t size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_timers.size();
    }

    /** 当前是否为空 */
    bool empty() {
        return size() == 0;
    }

    /** 返回当前停止状态 */
    bool stopped() const {
        return m_stop;
    }

    /**
     * 增加计划任务, 添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param start_date 允许运行的起始日期
     * @param end_date 允许运行的结束日期
     * @param start_time 允许运行的起始时间
     * @param end_time 允许运行的结束时间
     * @param repeat_num 重复次数，必须大于0，等于std::numeric_limits<int>::max()时表示无限循环
     * @param delay 间隔时间，需大于 TimeDelta(0)
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addFunc(Datetime start_date, Datetime end_date, TimeDelta start_time, TimeDelta end_time,
                int repeat_num, TimeDelta duration, F&& f, Args&&... args) {
        HKU_CHECK(!start_date.isNull(), "Invalid start_date!");
        HKU_CHECK(!end_date.isNull(), "Invalid end_date!");
        Datetime start = start_date.startOfDay();
        Datetime end = end_date.startOfDay();
        HKU_CHECK(end >= start, "end_date({}) need > start_date({})!", end, start);
        HKU_CHECK(start_time >= TimeDelta(0) && start_time <= TimeDelta(0, 23, 59, 59, 999, 999),
                  "Invalid start_time: {}", start_time.repr());
        HKU_CHECK(end_time >= TimeDelta(0) && end_time <= TimeDelta(0, 23, 59, 59, 999, 999),
                  "Invalid end_time: {}", end_time.repr());
        HKU_CHECK(end_time >= start_time, "end_time({}) need >= start_time({})!", end_time,
                  start_time);
        HKU_CHECK(repeat_num > 0, "Invalid repeat_num: {}", repeat_num);
        HKU_CHECK(duration > TimeDelta(0), "Invalid duration: {}", duration.repr());
        return _addFunc(start, end, start_time, end_time, repeat_num, duration, std::forward<F>(f),
                        std::forward<Args>(args)...);
    }

    /**
     * 增加重复定时任务，添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param repeat_num 重复次数，必须大于0，等于std::numeric_limits<int>::max()时表示无限循环
     * @param delay 间隔时间，需大于 TimeDelta(0)
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addDurationFunc(int repeat_num, TimeDelta duration, F&& f, Args&&... args) {
        HKU_CHECK(repeat_num > 0, "Invalid repeat_num: {}, must > 0", repeat_num);
        HKU_CHECK(duration > TimeDelta(), "Invalid duration: {}, must > TimeDelta(0)!",
                  duration.repr());
        return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(), TimeDelta(), repeat_num,
                        duration, std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * 增加延迟运行任务（只执行一次）, 添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param delay 延迟时间，需大于 TimeDelta(0)
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addDelayFunc(TimeDelta delay, F&& f, Args&&... args) {
        HKU_CHECK(delay > TimeDelta(), "Invalid delay: {}, must > TimeDelta(0)!", delay);
        return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(), TimeDelta(), 1, delay,
                        std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * 在指定时刻执行任务（只执行一次）, 添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param time_point 指定的运行时刻（包含具体的日、时、分、秒...）
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addFuncAtTime(Datetime time_point, F&& f, Args&&... args) {
        Datetime now = Datetime::now();
        HKU_CHECK(time_point > now, "You want run at {}, but now is {}", time_point, now);
        Datetime point_date = time_point.startOfDay();
        TimeDelta point = time_point - point_date;
        return _addFunc(time_point.startOfDay(), Datetime::max(), TimeDelta(-1), point, 1,
                        TimeDelta(), std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * 在日内指定时刻执行任务, 添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param start_date 允许执行的开始日期
     * @param end_date 允许执行的结束日期
     * @param time 指定运行的日内时刻
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addFuncAtTimeEveryDay(Datetime start_date, Datetime end_date, TimeDelta time, F&& f,
                              Args&&... args) {
        HKU_CHECK(!start_date.isNull() && !end_date.isNull(),
                  "Invalid start_date({}) or end_date({})!", start_date, end_date);
        HKU_CHECK(time >= TimeDelta() && time <= TimeDelta(0, 23, 59, 59, 999, 999),
                  "Invalid time {}", time.repr());
        Datetime start = start_date.startOfDay();
        Datetime end = end_date.startOfDay();
        HKU_CHECK(end >= start, "Invalid range of date! ({} - {})", start, end);
        return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(-1), time,
                        std::numeric_limits<int>::max(), TimeDelta(), std::forward<F>(f),
                        std::forward<Args>(args)...);
    }

    /**
     * 每日在指定时刻执行任务, 添加失败时抛出异常
     * @tparam F 任务类型
     * @tparam Args 任务参数
     * @param time 指定运行的日内时刻
     * @param f 待执行的延迟任务
     * @param args 任务具体参数
     * @return timer id
     */
    template <typename F, typename... Args>
    int addFuncAtTimeEveryDay(TimeDelta time, F&& f, Args&&... args) {
        return addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), time, std::forward<F>(f),
                                     std::forward<Args>(args)...);
    }

    /**
     * 移除定时任务
     * @param timerid 定时器id
     */
    void removeTimer(int timerid) {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto iter = m_timers.find(timerid);
        if (iter != m_timers.end()) {
            iter->second->m_repeat_num = 0;
        }
    }

private:
    void _removeTimer(int id) {
        delete m_timers[id];
        m_timers.erase(id);
    }

    void detectThread() {
        while (!m_stop) {
            Datetime now = Datetime::now();
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_queue.empty()) {
                m_cond.wait(lock);
                continue;
            }

            IntervalS s = m_queue.top();
            if (s.m_time_point == Datetime::min()) {
                break;  // 结束检测线程，用于 dll 能够安全退出，因为atomic在dll退出时可能无效
            }

            TimeDelta diff = s.m_time_point - now;
            if (diff > TimeDelta()) {
                m_cond.wait_for(lock, std::chrono::duration<int64_t, std::micro>(diff.ticks()));
                continue;
            }

            m_queue.pop();

            // 重新获取当前时间
            now = Datetime::now();

            auto timer_iter = m_timers.find(s.m_timer_id);
            if (timer_iter == m_timers.end()) {
                continue;
            }

            auto timer = timer_iter->second;
            m_tg->submit(timer->m_func);

            if (timer->m_repeat_num != std::numeric_limits<int>::max()) {
                timer->m_repeat_num--;
            }

            if (timer->m_repeat_num <= 0) {
                _removeTimer(s.m_timer_id);
                continue;
            }

            // 计算下一次执行的时间点
            Datetime today = now.startOfDay();
            if (timer->m_start_time >= TimeDelta()) {
                // 非指定时刻执行的定时器
                s.m_time_point = s.m_time_point + timer->m_duration;
                if (s.m_time_point < now) {
                    // 系统时间发生向前调整
                    s.m_time_point = now;
                }

                // 如果限定了当日可执行的时间段，且下一执行时刻超出了当日的限定时间
                if (timer->m_start_time != timer->m_end_time &&
                    s.m_time_point > today + timer->m_end_time) {
                    s.m_time_point = today + timer->m_start_time + TimeDelta(1);
                }

            } else {
                // 指定了每日运行时刻的定时器
                s.m_time_point =
                  s.m_time_point + (today - s.m_time_point.startOfDay() + TimeDelta(1));
            }

            if (timer->m_end_date != Datetime::max() &&
                s.m_time_point > timer->m_end_date + timer->m_end_time) {
                _removeTimer(s.m_timer_id);
                continue;
            }

            // 将下一运行时间推入队列
            m_queue.push(s);
        }
    }

    // 分配 timer_id
    int getNewTimerId() {
        int max_int = std::numeric_limits<int>::max();
        HKU_WARN_IF_RETURN(m_timers.size() >= size_t(max_int), -1, "Timer queue is full!");

        if (m_current_timer_id >= max_int) {
            m_current_timer_id = 0;
        } else {
            m_current_timer_id++;
        }

        while (true) {
            if (m_timers.find(m_current_timer_id) != m_timers.end()) {
                if (m_current_timer_id >= max_int) {
                    m_current_timer_id = 0;
                } else {
                    m_current_timer_id++;
                }
            } else {
                break;
            }
        }
        return m_current_timer_id;
    }

private:
    class Timer {
    public:
        void operator()() {
            m_func();
        }

        Datetime m_start_date = Datetime::min().startOfDay();  // 允许执行的起始日期（包含该日期）
        Datetime m_end_date = Datetime::max().startOfDay();  // 允许执行的终止日期（包含该日期）
        /*
         * 注：如果 m_start_time < TimeDelta(0), 则 m_end_time 代表每日指定的运行时刻，此时忽略
         * m_duration
         */
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

    template <typename F, typename... Args>
    int _addFunc(Datetime start_date, Datetime end_date, TimeDelta start_time, TimeDelta end_time,
                 int repeat_num, TimeDelta duration, F&& f, Args&&... args) {
        Datetime now = Datetime::now();
        Datetime today = now.startOfDay();
        HKU_CHECK(end_date >= today, "Invalid end_date {}, because today is {}", end_date, today);
        if (end_date != Datetime::max()) {
            HKU_CHECK(end_date + end_time >= now,
                      "Invalid param! You want end time is {}, but now is {}", end_date + end_time,
                      now);
        }

        Timer* timer = new Timer;
        timer->m_start_date = start_date;
        timer->m_end_date = end_date;
        timer->m_start_time = start_time;
        timer->m_end_time = end_time;
        timer->m_repeat_num = repeat_num;
        timer->m_duration = duration;
        timer->m_func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        IntervalS s;
        if (start_time < TimeDelta()) {
            Datetime first_start_time = start_date + end_time;
            if (first_start_time >= now) {
                s.m_time_point = first_start_time;
            } else {
                HKU_CHECK(repeat_num > 1, "The time has expired! expect time {}, but now is {}",
                          first_start_time, now);
                s.m_time_point = today + end_time;
                if (s.m_time_point < now) {
                    s.m_time_point = s.m_time_point + TimeDelta(1);
                }
            }

        } else {
            s.m_time_point =
              start_date >= today ? start_date + start_time + duration : now + duration;
            if (timer->m_start_time != timer->m_end_time) {
                Datetime point_date = s.m_time_point.startOfDay();
                TimeDelta point = s.m_time_point - point_date;
                if (point < timer->m_start_time) {
                    s.m_time_point = point_date + timer->m_start_time;
                } else if (point > timer->m_end_time) {
                    s.m_time_point = point_date + timer->m_start_time + TimeDelta(1);
                } else {
                    TimeDelta gap = point - timer->m_start_time;
                    if (gap % timer->m_duration != TimeDelta()) {
                        int x = int(gap / timer->m_duration) + 1;
                        s.m_time_point =
                          point_date + timer->m_start_time + timer->m_duration * double(x);
                    }
                }
            }
        }

        std::unique_lock<std::mutex> lock(m_mutex);
        int id = getNewTimerId();
        if (id < 0) {
            delete timer;
            lock.unlock();
            HKU_THROW("Failed to get new id, maybe too timers!");
        }

        m_timers[id] = timer;
        s.m_timer_id = id;
        // HKU_TRACE("s.m_time_point: {}", s.m_time_point.repr());
        m_queue.push(s);
        lock.unlock();
        m_cond.notify_all();
        return id;
    }

private:
    std::priority_queue<IntervalS> m_queue;
    std::atomic_bool m_stop;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::thread m_detect_thread;

    std::unordered_map<int, Timer*> m_timers;
    int m_current_timer_id;
    size_t m_work_num;  // 任务执行线程池线程数量
    std::unique_ptr<ThreadPool> m_tg;
};

}  // namespace hku