/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-16
 *      Author: fasiondog
 */

#include <string>
#include <arrow/api.h>

namespace hku {

void printArrowTable(const std::shared_ptr<arrow::Table>& table, int max_rows = 10);

}