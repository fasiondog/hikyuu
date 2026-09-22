/*
 * GlobalInitializer.cpp
 *
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-11-01
 *      Author: fasiondog
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <fmt/format.h>
#include <nng/nng.h>

#include "config.h"
#if HKU_ENABLE_HDF5_KDATA
#include <H5public.h>
#endif

#include "utilities/Log.h"
#include "utilities/os.h"
#include "utilities/thread/algorithm.h"
#include "hikyuu.h"
#include "GlobalInitializer.h"
#include "StockManager.h"
#include "global/GlobalSpotAgent.h"
#include "global/schedule/scheduler.h"
#include "plugin/shmserver.h"
#include "data_driver/ipc/ShmClientHook.h"
#include "indicator/IndicatorImp.h"
#include "global/sysinfo.h"
#include "plugin/interface/plugins.h"
#include "debug.h"

#if HKU_ENABLE_TA_LIB
#include <ta-lib/ta_libc.h>
#endif

namespace hku {

int GlobalInitializer::m_count = 0;

void GlobalInitializer::init() {
#ifdef MSVC_LEAKER_DETECT
    // MSVC memory leak detection
    int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flag |= _CRTDBG_LEAK_CHECK_DF;
    flag |= _CRTDBG_ALLOC_MEM_DF;
    _CrtSetDbgFlag(flag);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    // When a memory leak exists, the leak point can be filled in the VS output and VS can jump to
    // it automatically during the debugging Remember to set it back to -1 or comment it out,
    // otherwise the run would fail
    _CrtSetBreakAlloc(-1);
#endif

    IndicatorImp::initEngine();

#if HKU_OS_WINDOWS
    // Get the default heap of the process
    HANDLE hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        fmt::print("GetProcessHeap failed: {}\n", GetLastError());
    }

    // Enable the LFH (key: lfhFlag is fixed to 2)
    ULONG lfhFlag = 2;
    if (!HeapSetInformation(hHeap, HeapCompatibilityInformation, &lfhFlag, sizeof(lfhFlag))) {
        fmt::print("Enable LFH failed: {}\n", GetLastError());
    }
#endif

#if HKU_USE_LOW_PRECISION
    fmt::print("Initialize hikyuu_{}_low_precision ...\n", getVersionWithBuild());
#else
    fmt::print("Initialize hikyuu_{} ...\n", getVersionWithBuild());
#endif

    if (createDir(fmt::format("{}/.hikyuu", getUserDir()))) {
        initLogger(false, fmt::format("{}/.hikyuu/hikyuu.log", getUserDir()));
    } else {
        initLogger();
    }

    sysinfo_init();

#if HKU_ENABLE_SEND_FEEDBACK
    sendFeedback();
#endif

#if HKU_ENABLE_TA_LIB
    TA_Initialize();
#endif

    size_t cpu_num = std::thread::hardware_concurrency();
    if (cpu_num <= 10) {
        cpu_num = cpu_num * 2;
    } else if (cpu_num <= 64) {
        cpu_num = cpu_num * 3 / 2;
    } else {
        cpu_num = cpu_num * 5 / 4;
    }
    init_global_task_group(cpu_num);

    DataDriverFactory::init();
    StockManager::instance();

    getGlobalSpotAgent();
}

void GlobalInitializer::clean() {
#if HKU_ENABLE_SEND_FEEDBACK
    if (runningInPython() && CanUpgrade()) {
        LatestVersionInfo info = getLatestVersionInfo();
        fmt::print(
          "\n====================================================================\n"
          "The new version of Hikyuu is {}, and you can run the upgrade command:\n"
          "Hikyuu 的最新版本是 {}, 您可以运行升级命令:\n"
          "pip install hikyuu --upgrade\n"
          "{}\n"
          "====================================================================\n\n",
          info.version, info.version, info.remark);
    }
#endif

    reminderLicenseExpiration();

    StockManager &sm = StockManager::instance();
    sm.cancelLoad();
    // Wait for the background preload thread to exit: it must happen before any tg->stop(),
    // eradicating the concurrent access of the preload thread to m_load_tg (C3: TOCTOU/UAF). That
    // thread does not involve nng and exits quickly after the cancel, so the join is safe (the same
    // on Windows, which does not conflict with the existing decision of skipping the nng teardown
    // in its process exit path).
    sm.joinPreloadThread();

#if HKU_OS_OSX
    // Actively stop the asynchronous data loading task group, otherwise hdf5 reports a close
    // exception on linux
    auto *tg = sm.getLoadTaskGroup();
    if (tg) {
        tg->stop();
    }
#endif

    sysinfo_clean();
    releaseScheduler();
    releaseGlobalSpotAgent();

#if !HKU_OS_OSX
    // Actively stop the asynchronous data loading task group, otherwise hdf5 reports a close
    // exception on linux
    auto *tg = sm.getLoadTaskGroup();
    if (tg) {
        tg->stop();
    }
#endif

    // Unregister the shm client forwarding callbacks (if this process is a client): after that the
    // forwarding calls of Stock::realtimeUpdate / getLastUpdateTime return directly, avoiding
    // blocking on an already invalid connection during the exit. In the default build ~StockManager
    // is never executed, so like the server shutdown it must be called explicitly in clean().
    ipc::registerShmClient(ipc::ShmClientForwarders());

    // Explicitly stop the shm data service started in this process (if startShmServer was called):
    // it must happen before the nng_fini below, because the server nng worker holds in-flight
    // receive operations and would crash if the global nng state is torn down first. When it was
    // not started it is a no-op (the facade only checks the plugin pointer of this process and does
    // not trigger a plugin load).
    stopShmServer();

#if HKU_ENABLE_LEAK_DETECT || defined(MSVC_LEAKER_DETECT)
    // Without the memory leak detection the memory is left to the system to release automatically,
    // avoiding a too slow exit on Windows in some scenarios
    StockManager::quit();
#else
    fmt::print("Quit Hikyuu system!\n\n");
#endif

    DataDriverFactory::release();
    IndicatorImp::releaseEngine();

#if HKU_ENABLE_TA_LIB
    TA_Shutdown();
#endif

#if !HKU_OS_WINDOWS
    // on Windows it would hang instead
    nng_fini();
#endif

#if HKU_ENABLE_HDF5_KDATA
    H5close();
#endif

    spdlog::drop_all();

#ifdef MSVC_LEAKER_DETECT
    // The MSVC memory leak detection, it outputs to the output window of VS
    _CrtDumpMemoryLeaks();
#endif
}

} /* namespace hku */