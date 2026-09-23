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
 * 跨进程文件锁
 *
 * 底层实现：POSIX 使用 fcntl(F_SETLK) 记录锁，Windows 使用 LockFileEx，
 * 均为内核维护的跨进程互斥锁，进程退出（含异常崩溃）时由内核自动释放，
 * 因此不会出现早期 "O_CREAT|O_EXCL" 实现那种锁文件残留导致的永久死锁。
 *
 * @note
 *  - POSIX 记录锁为建议锁，且以 "进程" 为持有单位：同一进程内重复加锁不会失败，
 *    且关闭该文件的任一 fd 会释放该进程在此文件上的全部锁。故本类额外维护进程内
 *    互斥量，保证同一进程内相同路径的加锁串行，并避免误释放其他持有的锁。
 *    进程内互斥依赖 "同一份实现镜像"：若同进程内同时存在多份本实现拷贝（如插件与
 *    主程序各自静态链接），进程内互斥会失效，跨进程锁也会因同进程 fcntl 语义而失效。
 *  - 锁文件（filename）不存在时会被自动创建，且不会被删除。请勿对锁文件执行
 *    unlink/rename，否则锁所依附的 inode 不再唯一，跨进程互斥会被破坏。
 *  - 参与互斥的所有进程必须使用完全相同的 filename（本类会对注册表键做路径规范化，
 *    但仍建议统一使用绝对路径）。
 *  - 本类仅可作为函数局部对象或受控生命周期对象使用，禁止作为静态/全局对象：
 *    进程内互斥量注册表在进程退出时销毁，静态 FileLock 可能在注册表析构后被析构。
 *  - 对象被移动后（filename() 为空）仅可再被析构或赋值，不可继续调用 tryLock()。
 *
 * @code
 *     hku::FileLock lock(fmt::format("{}.lock", filename));
 *     if (lock.tryLock()) {
 *         // ... 受保护的操作
 *     }   // lock 析构时自动解锁
 * @endcode
 */
class HKU_UTILS_API FileLock {
public:
    /**
     * 构造函数，仅记录锁文件名并登记进程内互斥量，不进行加锁
     * @param filename 锁文件名（建议使用独立于数据文件的旁路文件）
     */
    explicit FileLock(std::string filename);

    /** 析构函数，若仍持有锁则自动释放 */
    ~FileLock();

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    FileLock(FileLock&& other) noexcept;
    FileLock& operator=(FileLock&& other) noexcept;

    /**
     * 尝试加锁（非阻塞）
     * @return true 加锁成功，false 已被其他进程或本进程其他线程持有
     */
    bool tryLock() noexcept;

    /**
     * 等待加锁，最多尝试 maxAttempts 次，每次间隔 waitTimeMs 毫秒
     * @param maxAttempts 最大尝试次数
     * @param waitTimeMs 每次尝试的间隔毫秒数
     * @return true 加锁成功，false 超出尝试次数仍未获得锁
     * @note 最后一次尝试结束后不再空等
     */
    bool waitLock(int maxAttempts = 10, int waitTimeMs = 100) noexcept;

    /** 解锁（未持锁时为无操作） */
    void unlock() noexcept;

    /** 当前是否已持有锁 */
    bool isLocked() const noexcept {
        return m_locked;
    }

    /** 获取锁文件名 */
    const std::string& filename() const noexcept {
        return m_filename;
    }

private:
    bool lockFile() noexcept;
    void unlockFile() noexcept;

private:
    std::string m_filename;
    bool m_locked{false};
    std::unique_lock<std::mutex> m_localLock; // 保证进程内互斥

#if HKU_OS_WINDOWS
    void* m_handle{nullptr};  // 仅存放有效句柄，不记录 INVALID_HANDLE_VALUE
#else
    int m_handle{-1};
#endif
};

}  // namespace hku

#endif  // HIKYUU_UTILITIES_FILELOCK_H_
