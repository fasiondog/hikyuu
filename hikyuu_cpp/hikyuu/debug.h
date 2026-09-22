/*
 * Copytright (C) hikyuu.org
 *
 *  Created on: 2020-2-6
 *      Author: fasiondog
 *
 * Purpose: memory leak detection; a cpp file includes this header so that the file name and other
 *          information can be printed when a memory leak is detected
 */

#pragma once

#if ENABLE_MSVC_LEAK_DETECT && defined(_MSC_VER) && (defined(_DEBUG) || defined(DEBUG))
#ifndef MSVC_LEAKER_DETECT
#define MSVC_LEAKER_DETECT
#endif
#endif

// clang-format off
// MSVC memory leak detection
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