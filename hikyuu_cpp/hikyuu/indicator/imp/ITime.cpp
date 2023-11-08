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

ITime::ITime() : IndicatorImp("TIME", 1) {
    setParam<string>("type", "time");
}

ITime::~ITime() {}

ITime::ITime(const KData& k, const string& type) : IndicatorImp("TIME", 1) {
    string type_name(type);
    to_upper(type_name);
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

    string type_name = getParam<string>("kpart");
    if ("TIME" == type_name) {
        m_name = "TIME";
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            _set(d.hour() * 10000 + d.minute() * 100 + d.second(), i);
        }

    } else if ("DATE" == type_name) {
        m_name = "DATE";
        for (size_t i = 0; i < total; i++) {
            const auto& d = ds[i];
            _set((d.year() - 1900) * 10000 + d.month() * 100 + d.day(), i);
        }

    } else if ("YEAR" == type_name) {
        m_name = "YEAR";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].year(), i);
        }

    } else if ("MONTH" == type_name) {
        m_name = "MONTH";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].month(), i);
        }

    } else if ("WEEK" == type_name) {
        m_name = "WEEK";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].dayOfWeek(), i);
        }

    } else if ("DAY" == type_name) {
        m_name = "DAY";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].day(), i);
        }

    } else if ("HOUR" == type_name) {
        m_name = "HOUR";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].hour(), i);
        }

    } else if ("MINUTE" == type_name) {
        m_name = "MINUTE";
        for (size_t i = 0; i < total; i++) {
            _set(ds[i].minute(), i);
        }
    }
}

}  // namespace hku