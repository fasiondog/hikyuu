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

    DataDriverFactory::init();
    StockManager::instance();
    IndicatorImp::initDynEngine();
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
    while (!sm.dataReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

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
    IndicatorImp::releaseDynEngine();

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