/*
 * IKData.cpp
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#include "IKData.h"
#include <boost/algorithm/string.hpp>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IKData)
#endif


namespace hku {

IKData::IKData(): IndicatorImp("KDATA") {
    setParam<string>("kpart", "KDATA");
}

IKData::IKData(const KData& kdata, const string& part)
: IndicatorImp() {
    string part_name(part);
    to_upper(part_name);
    setParam<string>("kpart", part_name);
    setParam<KData>("kdata", kdata);
    _calculate(Indicator());
}


IKData::~IKData() {

}

bool IKData::check() {
    string part = getParam<string>("kpart");
    if ("KDATA" == part || "OPEN" == part || "HIGH" == part || "LOW" == part
            || "CLOSE" == part || "AMO" == part || "VOL" == part) {
        return true;
    }

    return false;
}

//支持KDATA Indicator作为参数
void IKData::_calculate(const Indicator& ind) {
    if (!isLeaf() && !ind.empty()) {
        HKU_WARN("The input is ignored because {} depends on the context!", 
            getParam<string>("kpart"));
    }

    KData kdata = getContext();
    size_t total = kdata.size();
    if (total == 0) {
        return;
    }
    
    string part_name = getParam<string>("kpart");
    
    if ("KDATA" == part_name) {
        m_name = "KDATA";
        _readyBuffer(total, 6);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].openPrice, i, 0);
            _set(kdata[i].highPrice, i, 1);
            _set(kdata[i].lowPrice, i, 2);
            _set(kdata[i].closePrice, i, 3);
            _set(kdata[i].transAmount, i, 4);
            _set(kdata[i].transCount, i, 5);
        }

    } else if ("OPEN" == part_name) {
        m_name = "OPEN";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].openPrice, i);
        }

    } else if ("HIGH" == part_name) {
        m_name = "HIGH";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].highPrice, i);
        }
    } else if ("LOW" == part_name) {
        m_name = "LOW";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].lowPrice, i);
        }

    } else if ("CLOSE" == part_name) {
        m_name = "CLOSE";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].closePrice, i);
        }

    } else if ("AMO" == part_name) {
        m_name = "AMO";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].transAmount, i);
        }

    } else if ("VOL" == part_name) {
        m_name = "VOL";
        _readyBuffer(total, 1);
        for (size_t i = 0; i < total; ++i) {
            _set(kdata[i].transCount, i);
        }

    } else {
        m_name = "Unknow";
        m_discard = total;
        HKU_INFO("Unkown ValueType of KData");
    }
}

Indicator HKU_API KDATA(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "KDATA"));
}

Indicator HKU_API OPEN(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "OPEN"));
}

Indicator HKU_API HIGH(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "HIGH"));
}

Indicator HKU_API LOW(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "LOW"));
}

Indicator HKU_API CLOSE(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "CLOSE"));
}

Indicator HKU_API AMO(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "AMO"));
}

Indicator HKU_API VOL(const KData& kdata) {
    return Indicator(make_shared<IKData>(kdata, "VOL"));
}

Indicator HKU_API KDATA_PART(const KData& kdata, const string& part) {
    return Indicator(make_shared<IKData>(kdata, part));
}


//-----------------------------------------------------------
Indicator HKU_API KDATA() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "KDATA");
    p->name("KDATA");
    return p->calculate();
}

Indicator HKU_API OPEN() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "OPEN");
    p->name("OPEN");
    return p->calculate();
}

Indicator HKU_API HIGH() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "HIGH");
    p->name("HIGH");
    return p->calculate();
}

Indicator HKU_API LOW() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "LOW");
    p->name("LOW");
    return p->calculate();
}

Indicator HKU_API CLOSE() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "CLOSE");
    p->name("CLOSE");
    return p->calculate();
}

Indicator HKU_API AMO() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "AMO");
    p->name("AMO");
    return p->calculate();
}

Indicator HKU_API VOL() {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", "VOL");
    p->name("VOL");
    return p->calculate();
}

Indicator HKU_API KDATA_PART(const string& part) {
    IndicatorImpPtr p = make_shared<IKData>();
    p->setParam<string>("kpart", part);
    p->name("KDATA_PART");
    return p->calculate();
}
} /* namespace hku */
