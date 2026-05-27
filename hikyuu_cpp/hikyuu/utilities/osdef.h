/**
 *  Copyright (c) 2021 hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/*
 * CPU 架构宏定义
 */
#if defined(__arm__) || defined(_M_ARM)
#define HKU_ARCH_ARM 1
#define HKU_ARCH_ARM64 0
#define HKU_ARCH_X86 0
#define HKU_ARCH_X64 0
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HKU_ARCH_ARM 0
#define HKU_ARCH_ARM64 1
#define HKU_ARCH_X86 0
#define HKU_ARCH_X64 0
#elif defined(__x86_64__) || defined(_M_X64) || defined(_WIN64)
#define HKU_ARCH_ARM 0
#define HKU_ARCH_ARM64 0
#define HKU_ARCH_X86 0
#define HKU_ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86) || defined(_WIN32)
#define HKU_ARCH_ARM 0
#define HKU_ARCH_ARM64 0
#define HKU_ARCH_X86 1
#define HKU_ARCH_X64 0
#else
#define HKU_ARCH_ARM 0
#define HKU_ARCH_ARM64 0
#define HKU_ARCH_X86 0
#define HKU_ARCH_X64 0
#endif

/*
 * 操作系统宏定义
 */
#if defined(_WIN32) || defined(_WIN64)
#define HKU_OS_WINDOWS 1
#define HKU_OS_LINUX 0
#define HKU_OS_ANDROID 0
#define HKU_OS_OSX 0
#define HKU_OS_IOS 0
#elif defined(__ANDROID__)
#define HKU_OS_WINDOWS 0
#define HKU_OS_LINUX 0
#define HKU_OS_ANDROID 1
#define HKU_OS_OSX 0
#define HKU_OS_IOS 0
#elif defined(__linux__)
#define HKU_OS_WINDOWS 0
#define HKU_OS_LINUX 1
#define HKU_OS_ANDROID 0
#define HKU_OS_OSX 0
#define HKU_OS_IOS 0
#elif TARGET_OS_OSX
#define HKU_OS_WINDOWS 0
#define HKU_OS_LINUX 0
#define HKU_OS_ANDROID 0
#define HKU_OS_OSX 1
#define HKU_OS_IOS 0
#elif TARGET_OS_IOS || TARGET_OS_IPHONE
#define HKU_OS_WINDOWS 0
#define HKU_OS_LINUX 0
#define HKU_OS_ANDROID 0
#define HKU_OS_OSX 0
#define HKU_OS_IOS 1
#else
#define HKU_OS_WINDOWS 0
#define HKU_OS_LINUX 0
#define HKU_OS_ANDROID 0
#define HKU_OS_OSX 0
#define HKU_OS_IOS 0
#endif

// IOS 模拟器
#if HKU_OS_IOS && TARGET_OS_SIMULATOR
#define HKU_OS_IOS_SIMULATOR 1
#else
#define HKU_OS_IOS_SIMULATOR 0
#endif
