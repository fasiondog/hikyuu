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
 * Timer management and scheduling
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
     * Constructor
     * @param work_num the number of the threads in the thread pool executing the timer tasks
     */
    explicit TimerManager(size_t work_num = 1)
    : m_stop(true),
      m_current_timer_id(-1),
      m_work_num(work_num),
      m_tg(nullptr),
      m_use_extend_tg(false) {
        HKU_ASSERT(work_num >= 1);
        start();
    }

    /**
     * Constructor with the given thread pool, so as to share the other thread pools
     * @note Please guarantee by yourself that the lifetime of tg is always valid while TimerManager
     *       is alive
     * @param tg the given task group thread pool
     */
    explicit TimerManager(ThreadPool* tg)
    : m_stop(true), m_current_timer_id(-1), m_work_num(1), m_tg(tg), m_use_extend_tg(true) {
        HKU_ASSERT(m_tg);
        start();
    }

    /** Destructor */
    ~TimerManager() {
        stop();
        for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
            delete iter->second;
        }
    }

    /** Start the scheduling, it can be restarted after a stop */
    void start() {
        // It is already in the executing state, return directly
        HKU_IF_RETURN(!m_stop, void());

        // Set the executing state
        m_stop = false;

        std::unique_lock<std::mutex> lock(m_mutex);

        std::priority_queue<IntervalS> new_queue;
        m_queue.swap(new_queue);
        if (!m_tg) {
            m_tg = new ThreadPool(m_work_num);
        }

        /*
         * Rebuild the execution queue according to the existing timers and delete the invalid ones
         */

        std::forward_list<int> invalid_timers;  // Records the invalid timers
        for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
            int time_id = iter->first;
            const Timer* timer = iter->second;
            Datetime now = Datetime::now();

            // Record the invalid timer id
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

        // Clear the invalid timers
        for (auto id : invalid_timers) {
            _removeTimer(id);
        }

        lock.unlock();
        m_cond.notify_all();

        m_detect_thread = std::thread([this]() { detectThread(); });
    }

    /** Terminate the scheduling */
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

        if (!m_use_extend_tg && m_tg) {
            m_tg->stop();
            delete m_tg;
            m_tg = nullptr;
        }
    }

    /** Get the current number of the timer tasks */
    size_t size() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_timers.size();
    }

    /** Whether it is currently empty */
    bool empty() {
        return size() == 0;
    }

    /** Return the current stop state */
    bool stopped() const {
        return m_stop;
    }

    /**
     * Add a scheduled task; an exception is thrown when the addition fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param start_date the start date allowed to run
     * @param end_date the end date allowed to run
     * @param start_time the start time allowed to run
     * @param end_time the end time allowed to run
     * @param repeat_num the number of the repetitions, it must be greater than 0; it means an
     * infinite loop when it equals std::numeric_limits<int>::max()
     * @param duration the interval, it needs to be greater than TimeDelta(0)
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
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
     * Add a repeated timer task; an exception is thrown when the addition fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param repeat_num the number of the repetitions, it must be greater than 0; it means an
     * infinite loop when it equals std::numeric_limits<int>::max()
     * @param duration the interval, it needs to be greater than TimeDelta(0)
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
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
     * Add a delayed task (executed once only); an exception is thrown when the addition fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param delay the delay time, it needs to be greater than TimeDelta(0)
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
     * @return timer id
     */
    template <typename F, typename... Args>
    int addDelayFunc(TimeDelta delay, F&& f, Args&&... args) {
        HKU_CHECK(delay > TimeDelta(), "Invalid delay: {}, must > TimeDelta(0)!", delay);
        return _addFunc(Datetime::min(), Datetime::max(), TimeDelta(), TimeDelta(), 1, delay,
                        std::forward<F>(f), std::forward<Args>(args)...);
    }

    /**
     * Execute the task at the given moment (once only); an exception is thrown when the addition
     * fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param time_point the given running moment (including the concrete day, hour, minute, second
     *                   ...)
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
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
     * Execute the task at the given time within the day; an exception is thrown when the addition
     * fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param start_date the start date allowed to be executed
     * @param end_date the end date allowed to be executed
     * @param time the given running time within the day
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
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
     * Execute the task at the given time every day; an exception is thrown when the addition fails
     * @tparam F the task type
     * @tparam Args the task parameters
     * @param time the given running time within the day
     * @param f the delayed task to be executed
     * @param args the concrete task parameters
     * @return timer id
     */
    template <typename F, typename... Args>
    int addFuncAtTimeEveryDay(TimeDelta time, F&& f, Args&&... args) {
        return addFuncAtTimeEveryDay(Datetime::min(), Datetime::max(), time, std::forward<F>(f),
                                     std::forward<Args>(args)...);
    }

    /**
     * Remove a timer task
     * @param timerid timer id
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
                break;  // End the detection thread so that the dll can exit safely, because the
                        // atomic may be invalid when the dll exits
            }

            TimeDelta diff = s.m_time_point - now;
            if (diff > TimeDelta()) {
                m_cond.wait_for(lock, std::chrono::duration<int64_t, std::micro>(diff.ticks()));
                continue;
            }

            m_queue.pop();

            // Get the current time again
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

            // Calculate the time point of the next execution
            Datetime today = now.startOfDay();
            if (timer->m_start_time >= TimeDelta()) {
                // The timer not executed at the given moment
                s.m_time_point = s.m_time_point + timer->m_duration;
                if (s.m_time_point < now) {
                    // The system time is adjusted forward
                    s.m_time_point = now;
                }

                // If the executable time range of the day is limited and the next execution moment
                // exceeds the limit of the day
                if (timer->m_start_time != timer->m_end_time &&
                    s.m_time_point > today + timer->m_end_time) {
                    s.m_time_point = today + timer->m_start_time + TimeDelta(1);
                }

            } else {
                // The timer with the given daily running time
                s.m_time_point =
                  s.m_time_point + (today - s.m_time_point.startOfDay() + TimeDelta(1));
            }

            if (timer->m_end_date != Datetime::max() &&
                s.m_time_point > timer->m_end_date + timer->m_end_time) {
                _removeTimer(s.m_timer_id);
                continue;
            }

            // Push the next running time into the queue
            m_queue.push(s);
        }
    }

    // Allocate the timer_id
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

        Datetime m_start_date = Datetime::min().startOfDay();  // The start date allowed to be
                                                               // executed (inclusive)
        Datetime m_end_date = Datetime::max().startOfDay();  // The end date allowed to be executed
                                                             // (inclusive)
        /*
         * Note: if m_start_time < TimeDelta(0), m_end_time represents the given daily running time,
         *       and
         * m_duration
         */
        TimeDelta m_start_time;  // The start time of the day allowed to be executed (inclusive)
        TimeDelta m_end_time;    // The end time of the day allowed to be executed (inclusive)
        TimeDelta m_duration;    // The delay or the interval
        int m_repeat_num = 1;    // The number of the repetitions, max means an infinite loop
        std::function<void()> m_func;
    };

    struct IntervalS {
        Datetime m_time_point;  // The exact time point of the execution
        int m_timer_id = -1;    // The corresponding Timer, a negative value is invalid
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
    size_t m_work_num;  // The number of the threads in the task execution thread pool
    ThreadPool* m_tg{nullptr};
    bool m_use_extend_tg{false};
};

}  // namespace hku