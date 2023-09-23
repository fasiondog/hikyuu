/*
 * Copytright (C) hikyuu.org
 *
 *  Created on: 2020-2-6
 *      Author: fasiondog
 *
 * 用途：用于内存泄露检测，cpp 文件包含此头文件以便内存泄露检测时可打印文件名等信息
 */

#pragma once

#if defined(_MSC_VER) && (defined(_DEBUG) || defined(DEBUG)) && defined(ENABLE_LEAK_DETECT)
#ifndef MSVC_LEAKER_DETECT
#define MSVC_LEAKER_DETECT
#endif
#endif

// clang-format off
// MSVC 内存泄露检测
#ifdef MSVC_LEAKER_DETECT
    #define _CRTDBG_MAP_ALLOC
    #include "crtdbg.h"

    #ifndef DEBUG_CLIENTBLOCK
    #define DEBUG_CLIENTBLOCK new (_CLIENT_BLOCK, __FILE__, __LINE__)
    #define new DEBUG_CLIENTBLOCK
    #endif /* #ifndef DEBUG_CLIENTBLOCK */

#else /* #if MSVC_LEAKER_DETECT */
    #define DEBUG_CLIENTBLOCK
#endif /* #if MSVC_LEAKER_DETECT */
// clang-format on