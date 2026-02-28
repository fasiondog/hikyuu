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
#include "indicator/IndicatorImp.h"
#include "global/sysinfo.h"
#include "debug.h"

#if HKU_ENABLE_TA_LIB
#include <ta-lib/ta_libc.h>
#endif

namespace hku {

int GlobalInitializer::m_count = 0;

void GlobalInitializer::init() {
#ifdef MSVC_LEAKER_DETECT
    // MSVC内存泄露检测
    int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flag |= _CRTDBG_LEAK_CHECK_DF;
    flag |= _CRTDBG_ALLOC_MEM_DF;
    _CrtSetDbgFlag(flag);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    // 存在内存泄露时，可在填写 VS 输出的泄露点，VS 调试时可自动跳转
    // 记得重新设回 -1 或注释掉，否则会运行失败
    _CrtSetBreakAlloc(-1);
#endif

    IndicatorImp::initEngine();

#if HKU_OS_WINDOWS
    // 获取进程默认堆
    HANDLE hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        fmt::print("GetProcessHeap failed: {}\n", GetLastError());
    }

    // 启用LFH（关键：lfhFlag固定为2）
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
    StockManager::instance().getPlugin<ExtendIndicatorsPluginInterface>(
      HKU_PLUGIN_EXTEND_INDICATOR);

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

    StockManager &sm = StockManager::instance();
    sm.cancelLoad();

#if HKU_OS_OSX
    // 主动停止异步数据加载任务组，否则 hdf5 在 linux 下会报关闭异常
    auto *tg = sm.getLoadTaskGroup();
    if (tg) {
        tg->stop();
    }
#endif

    sysinfo_clean();
    releaseScheduler();
    releaseGlobalSpotAgent();

#if !HKU_OS_OSX
    // 主动停止异步数据加载任务组，否则 hdf5 在 linux 下会报关闭异常
    auto *tg = sm.getLoadTaskGroup();
    if (tg) {
        tg->stop();
    }
#endif

#if HKU_ENABLE_LEAK_DETECT || defined(MSVC_LEAKER_DETECT)
    // 非内存泄漏检测时，内存让系统自动释放，避免某些场景下 windows 下退出速度过慢
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
    // windows 反而会卡死
    nng_fini();
#endif

#if HKU_ENABLE_HDF5_KDATA
    H5close();
#endif

    spdlog::drop_all();

#ifdef MSVC_LEAKER_DETECT
    // MSVC 内存泄露检测，输出至 VS 的输出窗口
    _CrtDumpMemoryLeaks();
#endif
}

} /* namespace hku */