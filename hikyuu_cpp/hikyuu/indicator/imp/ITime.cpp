/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#include "ITime.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITime)
#endif

namespace hku {

ITime::ITime() : IndicatorImp("TIME") {
    setParam<string>("type", "time");
}

ITime::~ITime() {}

ITime::ITime(const KData& k, const string& type) : IndicatorImp() {
    string type_name(type);
    to_upper(type_name);
    m_name = type_name;
    setParam<string>("type", type_name);
    setParam<KData>("kdata", k);
    ITime::_calculate(Indicator());
}

bool ITime::check() {
    string part = getParam<string>("type");
    return "TIME" == part || "DATE" == part || "YEAR" == part || "MONTH" == part ||
           "WEEK" == part || "DAY" == part || "HOUR" == part || "MINUTE" == part;
}

void ITime::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    DatetimeList ds = kdata.getDatetimeList();

    _readyBuffer(total, 1);

    string type_name = getParam<string>("type");
    if ("TIME" == type_name) {
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            _set(d.hour() * 10000 + d.minute() * 100 + d.second(), i);
        }

    } else if ("DATE" == type_name) {
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            _set((d.year() - 1900) * 10000 + d.month() * 100 + d.day(), i);
        }

    } else if ("YEAR" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].year(), i);
        }

    } else if ("MONTH" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].month(), i);
        }

    } else if ("WEEK" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].dayOfWeek(), i);
        }

    } else if ("DAY" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].day(), i);
        }

    } else if ("HOUR" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].hour(), i);
        }

    } else if ("MINUTE" == type_name) {
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].minute(), i);
        }
    }
}

Indicator HKU_API DATE(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "DATE"));
}

Indicator HKU_API TIME(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "TIME"));
}

Indicator HKU_API YEAR(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "YEAR"));
}

Indicator HKU_API MONTH(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "MONTH"));
}

Indicator HKU_API WEEK(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "WEEK"));
}

Indicator HKU_API DAY(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "DAY"));
}

Indicator HKU_API HOUR(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "HOUR"));
}

Indicator HKU_API MINUTE(const KData& kdata) {
    return Indicator(make_shared<ITime>(kdata, "MINUTE"));
}

//-----------------------------------------------------------
Indicator HKU_API DATE() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "DATE");
    p->name("DATE");
    return p->calculate();
}

Indicator HKU_API TIME() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "TIME");
    p->name("TIME");
    return p->calculate();
}

Indicator HKU_API YEAR() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "YEAR");
    p->name("YEAR");
    return p->calculate();
}

Indicator HKU_API MONTH() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "MONTH");
    p->name("MONTH");
    return p->calculate();
}

Indicator HKU_API WEEK() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "WEEK");
    p->name("WEEK");
    return p->calculate();
}

Indicator HKU_API DAY() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "DAY");
    p->name("DAY");
    return p->calculate();
}

Indicator HKU_API HOUR() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "HOUR");
    p->name("HOUR");
    return p->calculate();
}

Indicator HKU_API MINUTE() {
    IndicatorImpPtr p = make_shared<ITime>();
    p->setParam<string>("type", "MINUTE");
    p->name("MINUTE");
    return p->calculate();
}

}  // namespace hku