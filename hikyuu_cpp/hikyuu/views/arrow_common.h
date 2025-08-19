/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-15
 *      Author: fasiondog
 */

#pragma once

#include <arrow/api.h>
#include "hikyuu/config.h"
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/Log.h"

namespace hku {

/**
 * @ingroup Views
 * @{
 */

#if HKU_USE_LOW_PRECISION
#define HKU_ARROW_PRICE_FIELD arrow::float32()
#define HKU_ARROW_PRICE_BUILDER arrow::FloatBuilder
#else
#define HKU_ARROW_PRICE_FIELD arrow::float64()
#define HKU_ARROW_PRICE_BUILDER arrow::DoubleBuilder
#endif

/**
 * @brief 打印 arrow table
 * @param table
 * @param max_rows
 */
void printArrowTable(const std::shared_ptr<arrow::Table>& table, int max_rows = 10);

/** @} */
}  // namespace hku