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
#include <H5public.h>
#include <fmt/format.h>
#include "Log.h"
#include "hikyuu.h"
#include "GlobalInitializer.h"
#include "StockManager.h"
#include "agent/SpotAgent.h"
#include "base/GlobalTaskGroup.h"
#include "base/schedule/scheduler.h"
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

    DataDriverFactory::init();
    StockManager::instance();
    SpotAgent::instance();
}

void GlobalInitializer::clean() {
#if !_MSC_VER
    // windows dll 退出时，暂无法安全停止线程，需程序自动回收
    // 会导致对内存泄露误报
    releaseGlobalTaskGroup();
    releaseScheduler();
    SpotAgent::release();
#endif

    StockManager::quit();
    DataDriverFactory::release();
    H5close();

#if USE_SPDLOG_LOGGER
    spdlog::shutdown();
#endif

#ifdef MSVC_LEAKER_DETECT
    // MSVC 内存泄露检测，输出至 VS 的输出窗口
    _CrtDumpMemoryLeaks();
#endif
}

} /* namespace hku */