/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20230222 added by fasiondog
 */

#include "simd_check.h"
#include "hikyuu/Log.h"

#if !HKU_OS_IOS
#if HKU_ARCH_X86 || HKU_ARCH_X64
#include <cpu_features/cpuinfo_x86.h>
#endif

#if HKU_ARCH_ARM
#include <cpu_features/cpuinfo_arm.h>
#endif

#if HKU_ARCH_ARM64
#include <cpu_features/cpuinfo_aarch64.h>
#endif

using namespace cpu_features;
#endif  // #if HKU_OS_IOS

namespace hku {

#if HKU_OS_IOS

void HKU_API checkUnavailableButEnabledCpuInstructions() {}

#else

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#endif

void HKU_API checkUnavailableButEnabledCpuInstructions() {
#if HKU_ARCH_X86 || HKU_ARCH_X64
    X86Info x86 = GetX86Info();

#ifdef HKU_ENABLE_SSE2
    HKU_TRACE("enable SSE2: {}", x86.features.sse2 ? 1 : 0);
    HKU_CHECK(
      x86.features.sse2,
      "The target was compiled to use SSE2 instructions, but these aren't available on your "
      "machine.");
#endif

#ifdef HKU_ENABLE_SSE3
    HKU_TRACE("enable SSE3: {}", x86.features.sse3 ? 1 : 0);
    HKU_CHECK(
      x86.features.sse3,
      "The target was compiled to use SSE3 instructions, but these aren't available on your "
      "machine.");
#endif

#ifdef HKU_ENABLE_SSE41
    HKU_TRACE("enable SSE41: {}", x86.features.sse4_1 ? 1 : 0);
    HKU_CHECK(
      x86.features.sse4_1,
      "The target was compiled to use SSE3 instructions, but these aren't available on your "
      "machine.");
#endif

#ifdef HKU_ENABLE_AVX
    HKU_TRACE("enable AVX: {}", x86.features.avx ? 1 : 0);
    HKU_CHECK(x86.features.avx,
              "The target was compiled to use AVX instructions, but these aren't available on your "
              "machine.");
#endif

#ifdef HKU_ENABLE_AVX2
    HKU_TRACE("enable AVX2: {}", x86.features.avx2 ? 1 : 0);
    HKU_CHECK(
      x86.features.avx2,
      "The target was compiled to use AVX2 instructions, but these aren't available on your "
      "machine.");
#endif
#endif

#if HKU_ARCH_ARM
    ArmInfo arm = GetArmInfo();  // NOSONAR
#ifdef HKU_ENABLE_NEON
    HKU_TRACE("enable neon: {}", arm.features.neon ? 1 : 0);
    HKU_CHECK(
      arm.features.neon,
      "The target was compiled to use NEON instructions, but these aren't available on your "
      "machine.");
    HKU_TRACE("enable vfp: {}", arm.features.vfp ? 1 : 0);
    HKU_TRACE("enable vfpv3: {}", arm.features.vfpv3 ? 1 : 0);
    HKU_TRACE("enable vfpv4: {}", arm.features.vfpv4 ? 1 : 0);
    HKU_TRACE("enable vfpd32: {}", arm.features.vfpd32 ? 1 : 0);
#endif
#endif
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif

}  // namespace hku