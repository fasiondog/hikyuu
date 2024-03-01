/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#include "ZhBond10.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const ZhBond10& bond) {
    string strip(", ");
    os << std::fixed;
    os.precision(4);
    os << "ZhBond10(Datetime(" << bond.date.ymd() << ")" << strip << bond.value << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

}  // namespace hku