/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "DataType.h"

namespace hku {

/**
 * Data structure of the 10-year Chinese treasury bond
 * @ingroup StockManage
 */
struct HKU_API ZhBond10 {
    Datetime date;  ///< Date
    price_t value;  ///< Yield
};

typedef vector<ZhBond10> ZhBond10List;

/**
 * Output ZhBond10: ZhBond10(date, value)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const ZhBond10&);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ZhBond10> : ostream_formatter {};
#endif