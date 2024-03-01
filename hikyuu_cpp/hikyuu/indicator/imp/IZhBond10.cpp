/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#include "hikyuu/StockManager.h"
#include "IZhBond10.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IZhBond10)
#endif

namespace hku {

IZhBond10::IZhBond10() : IndicatorImp("ZHBOND10") {
    // 在没有上下文时使用该参数，否则使用上下文中的日期, 数据应该是按升序排列的
    setParam<DatetimeList>("dates", DatetimeList());
    setParam<double>("default", 4.0);
}

IZhBond10::IZhBond10(const DatetimeList& dates, double default_val) : IndicatorImp("ZHBOND10") {
    setParam<DatetimeList>("dates", dates);
    setParam<double>("default", default_val);
}

IZhBond10::~IZhBond10() {}

bool IZhBond10::check() {
    return true;
}

void IZhBond10::_calculate(const Indicator& data) {
    DatetimeList dates;
    auto k = data.getContext();
    if (!k.empty()) {
        dates = k.getDatetimeList();
    } else {
        k = this->getContext();
        if (k.empty()) {
            dates = getParam<DatetimeList>("dates");
        } else {
            dates = k.getDatetimeList();
        }
    }

    size_t total = dates.size();
    HKU_IF_RETURN(0 == total, void());

    _readyBuffer(total, 1);

    value_t default_val = (value_t)getParam<double>("default");
    auto* dst = this->data();

    // 需要获取全部数据，在指定日期超过已有数据时，取最后一条数据的值作为当前值
    const auto& bonds = StockManager::instance().getZhBond10();
    size_t bonds_size = bonds.size();
    if (0 == bonds_size) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = default_val;
        }
        return;
    }

    size_t bondix = 0;
    for (size_t i = 0; i < total; i++) {
        bool found = false;
        for (size_t j = bondix; j < bonds_size; j++) {
            if (bonds[j].date > dates[i]) {
                found = true;
                bondix = j;
                break;
            }
        }
        if (found) {
            if (bondix != 0) {
                dst[i] = bonds[bondix - 1].value;
            } else {
                dst[i] = default_val;
            }
        } else {
            dst[i] = bonds[bonds_size - 1].value;
        }
    }
}

Indicator HKU_API ZHBOND10(double default_val) {
    auto p = make_shared<IZhBond10>();
    p->setParam<double>("default", default_val);
    return Indicator(p);
}

Indicator HKU_API ZHBOND10(const DatetimeList& dates, double default_val) {
    auto p = make_shared<IZhBond10>(dates);
    p->setParam<double>("default", default_val);
    p->calculate();
    return Indicator(p);
}

Indicator HKU_API ZHBOND10(const KData& k, double default_val) {
    auto p = make_shared<IZhBond10>();
    p->setParam<double>("default", default_val);
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku