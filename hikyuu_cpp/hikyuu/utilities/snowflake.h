/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  The code comes from: https://github.com/sniper00/snowflake-cpp
 *  Thanks sniper00
 *
 *  Create on: 2021-04-13
 *     Author: fasiondog
 */

#pragma once

#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <mutex>

namespace hku {

class snowflake_nonlock {
public:
    void lock() {}
    void unlock() {}
};

template <int64_t Twepoch, typename Lock = snowflake_nonlock>
class snowflake {
    using lock_type = Lock;
    static constexpr int64_t TWEPOCH = Twepoch;
    static constexpr int64_t WORKER_ID_BITS = 5L;
    static constexpr int64_t DATACENTER_ID_BITS = 5L;
    static constexpr int64_t MAX_WORKER_ID = (1 << WORKER_ID_BITS) - 1;
    static constexpr int64_t MAX_DATACENTER_ID = (1 << DATACENTER_ID_BITS) - 1;
    static constexpr int64_t SEQUENCE_BITS = 12L;
    static constexpr int64_t WORKER_ID_SHIFT = SEQUENCE_BITS;
    static constexpr int64_t DATACENTER_ID_SHIFT = SEQUENCE_BITS + WORKER_ID_BITS;
    static constexpr int64_t TIMESTAMP_LEFT_SHIFT =
      SEQUENCE_BITS + WORKER_ID_BITS + DATACENTER_ID_BITS;
    static constexpr int64_t SEQUENCE_MASK = (1 << SEQUENCE_BITS) - 1;

    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    time_point start_time_point_ = std::chrono::steady_clock::now();
    int64_t start_millsecond_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();

    int64_t last_timestamp_ = -1;
    int64_t workerid_ = 0;
    int64_t datacenterid_ = 0;
    int64_t sequence_ = 0;
    lock_type lock_;

public:
    snowflake() = default;

    snowflake(const snowflake&) = delete;

    snowflake& operator=(const snowflake&) = delete;

    void init(int64_t workerid, int64_t datacenterid) {
        std::lock_guard<lock_type> lock(lock_);
        if (workerid > MAX_WORKER_ID || workerid < 0) {
            throw std::runtime_error("worker Id can't be greater than 31 or less than 0");
        }

        if (datacenterid > MAX_DATACENTER_ID || datacenterid < 0) {
            throw std::runtime_error("datacenter Id can't be greater than 31 or less than 0");
        }

        workerid_ = workerid;
        datacenterid_ = datacenterid;
    }

    int64_t nextid() {
        std::lock_guard<lock_type> lock(lock_);
        // std::chrono::steady_clock  cannot decrease as physical time moves forward
        auto timestamp = millsecond();
        if (last_timestamp_ == timestamp) {
            sequence_ = (sequence_ + 1) & SEQUENCE_MASK;
            if (sequence_ == 0) {
                timestamp = wait_next_millis(last_timestamp_);
            }
        } else {
            sequence_ = 0;
        }

        last_timestamp_ = timestamp;

        return ((timestamp - TWEPOCH) << TIMESTAMP_LEFT_SHIFT) |
               (datacenterid_ << DATACENTER_ID_SHIFT) | (workerid_ << WORKER_ID_SHIFT) | sequence_;
    }

private:
    int64_t millsecond() const {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() - start_time_point_);
        return start_millsecond_ + diff.count();
    }

    int64_t wait_next_millis(int64_t last) const {
        auto timestamp = millsecond();
        while (timestamp <= last) {
            timestamp = millsecond();
        }
        return timestamp;
    }
};

}  // namespace hku