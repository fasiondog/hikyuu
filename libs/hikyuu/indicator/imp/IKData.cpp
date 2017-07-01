/*
 * IKData.cpp
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#include "IKData.h"
#include <boost/algorithm/string.hpp>

namespace hku {

IKData::IKData(): IndicatorImp("KDATA") {
    setParam<string>("kpart", "KDATA");
}

IKData::IKData(const KData& kdata, const string& part)
: IndicatorImp() {
    string part_name(part);
    boost::to_upper(part_name);
    setParam<string>("kpart", part_name);

    size_t total = kdata.size();
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
        HKU_INFO("Unkown ValueType of KData [IKData::IKData]");
    }
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
    size_t total = ind.size();
    if (total == 0) {
        return;
    }

    string part = getParam<string>("kpart");
    m_discard = ind.discard();

    if ("KDATA" == part) {
        if (ind.getResultNumber() < 6) {
            HKU_ERROR("KDATA result_number must >= 6! input ind is "
                    << ind.getResultNumber() << "IKData::_calculate");
            return;
        }

        _readyBuffer(total, 6);
        for (size_t i = m_discard; i < total; ++i) {
            _set(ind.get(i,0), i, 0);
            _set(ind.get(i,1), i, 1);
            _set(ind.get(i,2), i, 2);
            _set(ind.get(i,3), i, 3);
            _set(ind.get(i,4), i, 4);
            _set(ind.get(i,5), i, 5);
        }
        return;
    }

    int ind_num = ind.getResultNumber();
    int result_num = 0;
    if ("OPEN" == part) {
        result_num = 0;
    } else if ("HIGH" == part) {
        result_num = ind_num >= 1 ? 1 : 0;
    } else if ("LOW" == part) {
        result_num = ind_num >= 2 ? 2 : 0;
    } else if ("CLOSE" == part) {
        result_num = ind_num >= 3 ? 3 : 0;
    } else if ("AMO" == part) {
        result_num = ind_num >= 4 ? 4 : 0;
    } else if ("VOL" == part) {
        result_num = ind_num >= 5 ? 5 : 0;
    } else {
        //无效参数值，退出
        return;
    }

    _readyBuffer(total, 1);
    for (size_t i = m_discard; i < total; ++i) {
        _set(ind.get(i, result_num), i);
    }
}

Indicator HKU_API KDATA(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "KDATA")));
}

Indicator HKU_API OPEN(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "OPEN")));
}

Indicator HKU_API HIGH(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "HIGH")));
}

Indicator HKU_API LOW(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "LOW")));
}

Indicator HKU_API CLOSE(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "CLOSE")));
}

Indicator HKU_API AMO(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "AMO")));
}

Indicator HKU_API VOL(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, "VOL")));
}

Indicator HKU_API KDATA_PART(const KData& kdata, const string& part) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, part)));
}

//-----------------------------------------------------------
Indicator HKU_API KDATA(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "KDATA");
    p->name("KDATA");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API OPEN(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "OPEN");
    p->name("OPEN");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API HIGH(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "HIGH");
    p->name("HIGH");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API LOW(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "LOW");
    p->name("LOW");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API CLOSE(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "CLOSE");
    p->name("CLOSE");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API AMO(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "AMO");
    p->name("AMO");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API VOL(const Indicator& kdata) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "VOL");
    p->name("VOL");
    p->calculate(kdata);
    return Indicator(p);
}

Indicator HKU_API KDATA_PART(const Indicator& kdata, const string& part) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", part);
    p->name("KDATA_PART");
    p->calculate(kdata);
    return Indicator(p);
}

//-----------------------------------------------------------
Indicator HKU_API KDATA() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "KDATA");
    p->name("KDATA");
    return Indicator(p);
}

Indicator HKU_API OPEN() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "OPEN");
    p->name("OPEN");
    return Indicator(p);
}

Indicator HKU_API HIGH() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "HIGH");
    p->name("HIGH");
    return Indicator(p);
}

Indicator HKU_API LOW() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "LOW");
    p->name("LOW");
    return Indicator(p);
}

Indicator HKU_API CLOSE() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "CLOSE");
    p->name("CLOSE");
    return Indicator(p);
}

Indicator HKU_API AMO() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "AMO");
    p->name("AMO");
    return Indicator(p);
}

Indicator HKU_API VOL() {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", "VOL");
    p->name("VOL");
    return Indicator(p);
}

Indicator HKU_API KDATA_PART(const string& part) {
    IndicatorImpPtr p(new IKData());
    p->setParam<string>("kpart", part);
    p->name("KDATA_PART");
    return Indicator(p);
}

} /* namespace hku */
