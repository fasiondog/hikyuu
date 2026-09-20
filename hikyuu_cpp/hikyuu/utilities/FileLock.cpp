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

// 进程内互斥量注册表：POSIX 记录锁以 "进程" 为持有单位，同进程内重复加锁不会失败，
// 因此需要进程内互斥量保证相同路径的加锁串行，并避免解锁时误释放同一进程的其他锁。
// 不考虑过期清理。
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

// 确保锁文件所在目录存在（Windows 的 OPEN_ALWAYS 不会自动创建父目录）
void createParentDir(const std::string& filename) {
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos || pos == 0) {
        return;
    }
    createDir(filename.substr(0, pos));
}

}  // namespace

// 规范化锁文件路径作为注册表键：统一分隔符、折叠重复分隔符与 "."、去掉末尾分隔符，
// 使 "a.lock"、"./a.lock"、"a.lock/"、"a//b.lock" 等写法映射到同一个键，避免
// “同一 inode 两个键” 导致进程内互斥静默失效。
// 其他不合理字符等，最终在调用 OS open 时统一报错，此处不做校验拦截。
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
    // 切分路径分量，忽略空分量与 "."，不折叠 ".."
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

    // 被移动后的对象 m_filename 为空、m_localLock 未关联互斥量，不可再加锁
    if (m_filename.empty()) {
        return false;
    }

    // 先占进程内互斥量：POSIX 记录锁无法拦截同一进程内的重复加锁
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
    // 锁整个文件范围：偏移 0，长度 MAXDWORD
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

    // O_CREAT: 锁文件不存在时自动创建；不使用 O_EXCL，也不截断已有内容
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
    fl.l_len = 0;  // 0 表示锁至文件末尾（即整文件）

    int ret = 0;
    do {
        ret = ::fcntl(fd, F_SETLK, &fl);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1) {
        // EACCES/EAGAIN 表示已被其他进程持有，属正常竞争，不记为错误
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
