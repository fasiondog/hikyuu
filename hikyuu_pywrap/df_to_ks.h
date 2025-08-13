/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-12
 *      Author: fasiondog
 */

#pragma once

#include <hikyuu/KRecord.h>
#include "pybind_utils.h"

namespace hku {

KRecordList df_to_krecords(const py::object& df, const StringList& cols);

}