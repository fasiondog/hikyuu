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
    setParam<KData>("kdata", kdata);
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


} /* namespace hku */
