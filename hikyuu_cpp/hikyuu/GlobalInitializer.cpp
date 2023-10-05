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

#include "config.h"
#if HKU_ENABLE_HDF5_KDATA
#include <H5public.h>
#endif

#include "Log.h"
#include "hikyuu.h"
#include "GlobalInitializer.h"
#include "StockManager.h"
#include "global/GlobalTaskGroup.h"
#include "global/GlobalSpotAgent.h"
#include "global/schedule/scheduler.h"
#include "indicator/IndicatorImp.h"
#include "global/sysinfo.h"
#include "debug.h"

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

    fmt::print("Initialize hikyuu_{} ...\n", getVersionWithBuild());

    initLogger();
#if defined(_DEBUG) || defined(DEBUG)
    set_log_level(TRACE);
#else
    set_log_level(INFO);
#endif

#if HKU_ENABLE_SEND_FEEDBACK && HKU_OS_WINDOWS
    sendFeedback();
#endif

    DataDriverFactory::init();
    StockManager::instance();
    IndicatorImp::initDynEngine();
    getGlobalSpotAgent();
}

void GlobalInitializer::clean() {
    releaseGlobalTaskGroup();
    releaseScheduler();
    releaseGlobalSpotAgent();

    IndicatorImp::releaseDynEngine();
    StockManager::quit();
    DataDriverFactory::release();

#if HKU_ENABLE_HDF5_KDATA
    H5close();
#endif

#if USE_SPDLOG_LOGGER
    spdlog::drop_all();
#endif

#ifdef MSVC_LEAKER_DETECT
    // MSVC 内存泄露检测，输出至 VS 的输出窗口
    _CrtDumpMemoryLeaks();
#endif
}

} /* namespace hku */