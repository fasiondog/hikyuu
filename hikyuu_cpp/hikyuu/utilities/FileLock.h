/*
 * FileLock.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-20
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_FILELOCK_H_
#define HIKYUU_UTILITIES_FILELOCK_H_

#include <string>
#include <mutex>
#include "hikyuu/utilities/osdef.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * Cross-process file lock
 *
 * Underlying implementation: POSIX uses the fcntl(F_SETLK) record lock and Windows uses LockFileEx;
 * both are cross-process mutual exclusion locks maintained by the kernel and are released
 * automatically by the kernel when the process exits (including an abnormal crash),
 * so the permanent deadlock caused by the residual lock file in the early "O_CREAT|O_EXCL"
 * implementation does not happen.
 *
 * @note
 *  - The POSIX record lock is an advisory lock and is held per "process": locking repeatedly within
 *    the same process does not fail,
 *    and closing any fd of this file releases all the locks of this process on this file. Therefore
 *    this class additionally maintains an in-process
 *    mutex to guarantee that the locking of the same path within the same process is serialized,
 * and to avoid mistakenly releasing the other held locks. The in-process mutual exclusion relies on
 * "a single copy of the implementation image": if multiple copies of this implementation exist in
 * the same process at the same time (such as the plugin and the main program statically linking it
 * separately), the in-process mutual exclusion becomes invalid and the cross-process lock also
 * becomes invalid because of the fcntl semantics of the same process.
 *  - The lock file (filename) is created automatically when it does not exist and is never deleted.
 *    Do not perform
 *    unlink/rename on the lock file, otherwise the inode the lock depends on is no longer unique
 * and the cross-process mutual exclusion is broken.
 *  - All the processes participating in the mutual exclusion must use exactly the same filename
 * (this class normalizes the path for the registry key, but it is still recommended to use the
 * absolute path uniformly).
 *  - This class can be used as a local function object or a controlled lifetime object only; it
 * must not be used as a static/global object: the in-process mutex registry is destroyed when the
 * process exits, and a static FileLock may be destructed after the registry has been destructed.
 *  - After the object has been moved (filename() is empty) it can only be destructed or assigned
 *    again, tryLock() must not be called further.
 *
 * @code
 *     hku::FileLock lock(fmt::format("{}.lock", filename));
 *     if (lock.tryLock()) {
 *         // ... the protected operations
 *     }   // It is unlocked automatically when lock is destructed
 * @endcode
 */
class HKU_UTILS_API FileLock {
public:
    /**
     * Constructor, it only records the lock file name and registers the in-process mutex, without
     * locking
     * @param filename the lock file name (it is recommended to use a bypass file independent of the
     *                 data files)
     */
    explicit FileLock(std::string filename);

    /** Destructor, it releases the lock automatically if it is still held */
    ~FileLock();

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& other) noexcept;
    FileLock& operator=(FileLock&& other) noexcept;

    /**
     * Try to lock (non-blocking)
     * @return true the lock is acquired successfully, false it is already held by another process
     * or another thread of this process
     */
    bool tryLock() noexcept;

    /**
     * Wait for the lock; it tries at most maxAttempts times with an interval of waitTimeMs
     * milliseconds every time
     * @param maxAttempts the maximum number of the attempts
     * @param waitTimeMs the interval in milliseconds between the attempts
     * @return true the lock is acquired successfully, false the lock is still not acquired after
     * the number of the attempts is exceeded
     * @note It does not keep waiting after the last attempt
     */
    bool waitLock(int maxAttempts = 10, int waitTimeMs = 100) noexcept;

    /** Unlock (it is a no-op when the lock is not held) */
    void unlock() noexcept;

    /** Whether the lock is currently held */
    bool isLocked() const noexcept {
        return m_locked;
    }

    /** Get the lock file name */
    const std::string& filename() const noexcept {
        return m_filename;
    }

private:
    bool lockFile() noexcept;
    void unlockFile() noexcept;

private:
    std::string m_filename;
    bool m_locked{false};
    std::unique_lock<std::mutex> m_localLock;  // Guarantees the in-process mutual exclusion

#if HKU_OS_WINDOWS
    void* m_handle{nullptr};  // Only a valid handle is stored, INVALID_HANDLE_VALUE is not recorded
#else
    int m_handle{-1};
#endif
};

}  // namespace hku

#endif  // HIKYUU_UTILITIES_FILELOCK_H_
