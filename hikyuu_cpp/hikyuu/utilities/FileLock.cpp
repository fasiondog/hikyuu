/*
 * FileLock.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-20
 *      Author: fasiondog
 */

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <cerrno>
#include <cstring>

#include "hikyuu/utilities/FileLock.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/osdef.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace hku {

namespace {

// The in-process mutex registry: a POSIX record lock is held per process, so locking the same
// path repeatedly inside one process would not fail; therefore an in-process mutex is needed to
// serialize the locking of the same path and to avoid releasing another lock of the same process
// by mistake at the unlock. The expiration cleanup is not considered.
std::mutex& localMutex(const std::string& key) {
    static std::mutex s_mutex;
    static std::map<std::string, std::unique_ptr<std::mutex>> s_locks;
    std::lock_guard<std::mutex> guard(s_mutex);
    auto& mtx = s_locks[key];
    if (!mtx) {
        mtx = std::make_unique<std::mutex>();
    }
    return *mtx;
}

// Make sure the directory of the lock file exists (OPEN_ALWAYS on Windows does not create the
// parent directory automatically)
void createParentDir(const std::string& filename) {
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos || pos == 0) {
        return;
    }
    createDir(filename.substr(0, pos));
}

}  // namespace

// Normalize the lock file path as the registry key: unify the separators, collapse the repeated
// separators and ".", remove the trailing separator, so that "a.lock", "./a.lock", "a.lock/"
// and "a//b.lock" map to the same key, avoiding "two keys for the same inode" making the
// in-process mutex fail silently. The other unreasonable characters are reported uniformly when
std::string HKU_UTILS_API normalizeLockKey(std::string_view filename) {
#if HKU_OS_WINDOWS
    constexpr std::string_view sep = "/\\";
#else
    constexpr std::string_view sep = "/";
#endif

    std::string key;
    if (!filename.empty() && filename.find_first_of(sep) == 0) {
        key = "/";
    }
    // Split the path components, ignoring the empty ones and ".", without collapsing ".."
    for (size_t beg = 0, end = 0;; beg = end + 1) {
        end = filename.find_first_of(sep, beg);
        const std::string_view part =
          filename.substr(beg, end == std::string_view::npos ? std::string_view::npos : end - beg);
        if (!part.empty() && part != ".") {
            if (!key.empty() && key.back() != '/') {
                key.push_back('/');
            }
            key += part;
        }
        if (end == std::string_view::npos) {
            break;
        }
    }
    return key.empty() ? std::string(filename) : key;
}

FileLock::FileLock(std::string filename)
: m_filename(std::move(filename)),
  m_localLock(localMutex(normalizeLockKey(m_filename)), std::defer_lock) {}

FileLock::~FileLock() {
    unlock();
}

FileLock::FileLock(FileLock&& other) noexcept
: m_filename(std::move(other.m_filename)),
  m_locked(other.m_locked),
  m_localLock(std::move(other.m_localLock)),
  m_handle(other.m_handle) {
    other.m_locked = false;
#if HKU_OS_WINDOWS
    other.m_handle = nullptr;
#else
    other.m_handle = -1;
#endif
}

FileLock& FileLock::operator=(FileLock&& other) noexcept {
    if (this != &other) {
        unlock();
        m_filename = std::move(other.m_filename);
        m_locked = other.m_locked;
        m_localLock = std::move(other.m_localLock);
        m_handle = other.m_handle;
        other.m_locked = false;
#if HKU_OS_WINDOWS
        other.m_handle = nullptr;
#else
        other.m_handle = -1;
#endif
    }
    return *this;
}

bool FileLock::tryLock() noexcept {
    if (m_locked) {
        return true;
    }

    // After being moved, m_filename is empty and m_localLock has no associated mutex, so no more
    // locking is possible
    if (m_filename.empty()) {
        return false;
    }

    // Take the in-process mutex first: a POSIX record lock cannot intercept a repeated lock inside
    // the same process
    if (!m_localLock.owns_lock()) {
        if (!m_localLock.try_lock()) {
            return false;
        }
    }

    if (!lockFile()) {
        m_localLock.unlock();
        return false;
    }

    m_locked = true;
    return true;
}

bool FileLock::waitLock(int maxAttempts, int waitTimeMs) noexcept {
    for (int i = 0; i < maxAttempts; ++i) {
        if (tryLock()) {
            return true;
        }

        if (i + 1 < maxAttempts && waitTimeMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeMs));
        }
    }
    return false;
}

void FileLock::unlock() noexcept {
    if (m_locked) {
        unlockFile();
        m_locked = false;
    }
    if (m_localLock.owns_lock()) {
        m_localLock.unlock();
    }
}

#if HKU_OS_WINDOWS

bool FileLock::lockFile() noexcept {
    createParentDir(m_filename);

    std::string path = HKU_PATH(m_filename);
    HANDLE handle =
      ::CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        HKU_ERROR("Failed to open lock file: {} ({})", m_filename, ::GetLastError());
        return false;
    }

    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    // Lock the whole file range: the offset 0 and the length MAXDWORD
    if (!::LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD,
                      MAXDWORD, &overlapped)) {
        DWORD err = ::GetLastError();
        if (err != ERROR_LOCK_VIOLATION) {
            HKU_ERROR("Failed to lock file: {} ({})", m_filename, err);
        }
        ::CloseHandle(handle);
        return false;
    }

    m_handle = handle;
    return true;
}

void FileLock::unlockFile() noexcept {
    if (m_handle == nullptr || m_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    HANDLE handle = static_cast<HANDLE>(m_handle);
    OVERLAPPED overlapped;
    std::memset(&overlapped, 0, sizeof(overlapped));
    if (!::UnlockFileEx(handle, 0, MAXDWORD, MAXDWORD, &overlapped)) {
        HKU_WARN("Failed to unlock file: {} ({})", m_filename, ::GetLastError());
    }
    ::CloseHandle(handle);
    m_handle = nullptr;
}

#else

bool FileLock::lockFile() noexcept {
    createParentDir(m_filename);

    // O_CREAT: the lock file is created automatically when it does not exist; O_EXCL is not used
    // and the existing content is not truncated
    int fd = ::open(m_filename.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        HKU_ERROR("Failed to open lock file: {} ({})", m_filename, std::strerror(errno));
        return false;
    }

    struct ::flock fl;
    std::memset(&fl, 0, sizeof(fl));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;  // 0 means locking to the end of the file (i.e. the whole file)

    int ret = 0;
    do {
        ret = ::fcntl(fd, F_SETLK, &fl);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1) {
        // EACCES/EAGAIN means it is held by another process, which is a normal contention and not
        // recorded as an error
        if (errno != EACCES && errno != EAGAIN) {
            HKU_ERROR("Failed to lock file: {} ({})", m_filename, std::strerror(errno));
        }
        ::close(fd);
        return false;
    }

    m_handle = fd;
    return true;
}

void FileLock::unlockFile() noexcept {
    if (m_handle < 0) {
        return;
    }

    struct ::flock fl;
    std::memset(&fl, 0, sizeof(fl));
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (::fcntl(m_handle, F_SETLK, &fl) == -1) {
        HKU_WARN("Failed to unlock file: {} ({})", m_filename, std::strerror(errno));
    }
    ::close(m_handle);
    m_handle = -1;
}

#endif

}  // namespace hku
