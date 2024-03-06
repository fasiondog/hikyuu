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
 * 10年期中国国债数据结构
 * @ingroup StockManage
 */
struct HKU_API ZhBond10 {
    Datetime date;  ///< 日期
    price_t value;  ///< 收益率
};

typedef vector<ZhBond10> ZhBond10List;

/**
 * 输出 ZhBond10 ZhBond10(date, value)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const ZhBond10&);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ZhBond10> : ostream_formatter {};
#endif