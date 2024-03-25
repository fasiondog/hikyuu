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
    setParam<string>("type", "TIME");
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

void ITime::_checkParam(const string& name) const {
    if ("type" == name) {
        string param_type = getParam<string>("type");
        HKU_CHECK("TIME" == param_type || "DATE" == param_type || "YEAR" == param_type ||
                    "MONTH" == param_type || "WEEK" == param_type || "DAY" == param_type ||
                    "HOUR" == param_type || "MINUTE" == param_type,
                  "Invalid param type: {}", param_type);
    }
}

void ITime::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    DatetimeList ds = kdata.getDatetimeList();

    _readyBuffer(total, 1);
    auto* dst = this->data();

    string type_name = getParam<string>("type");
    if ("TIME" == type_name) {
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            dst[i] = d.hour() * 10000 + d.minute() * 100 + d.second();
        }

    } else if ("DATE" == type_name) {
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            dst[i] = (d.year() - 1900) * 10000 + d.month() * 100 + d.day();
        }

    } else if ("YEAR" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].year();
        }

    } else if ("MONTH" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].month();
        }

    } else if ("WEEK" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].dayOfWeek();
        }

    } else if ("DAY" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].day();
        }

    } else if ("HOUR" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].hour();
        }

    } else if ("MINUTE" == type_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = ds[i].minute();
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