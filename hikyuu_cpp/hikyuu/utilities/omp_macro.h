/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-16
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if defined(_OPENMP)
#include <omp.h>
#define HKU_OMP_PARALLEL_FOR _Pragma("omp parallel for")
#define OMP_SAFETY_PARALLEL_FOR \
    _Pragma("omp parallel for num_threads(omp_get_max_threads()) if (!omp_in_parallel())")
#define HKU_OMP_CHECK_THRESHOLD(guard, threshold)      \
    if ((guard) > (threshold) && !omp_in_parallel()) { \
        omp_set_num_threads(omp_get_max_threads());    \
    }
#else
#define HKU_OMP_PARALLEL_FOR
#define OMP_SAFETY_PARALLEL_FOR
#define HKU_OMP_CHECK_THRESHOLD(guard, threshold)
#endif