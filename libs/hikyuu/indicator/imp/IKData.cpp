/*
 * IKData.cpp
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#include "IKData.h"
#include <boost/algorithm/string.hpp>

namespace hku {

IKData::IKData(const KData& kdata, ValueType valueType)
: IndicatorImp(0, 1), m_valueType(valueType) {
    size_t total = kdata.size();
    switch (valueType) {
    case KDATA:
        m_result_num = 6;
        for (size_t i = 1; i < 6; ++i) {
            m_pBuffer[i] = new PriceList();
        }
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].openPrice, 0);
            _append(kdata[i].highPrice, 1);
            _append(kdata[i].lowPrice, 2);
            _append(kdata[i].closePrice, 3);
            _append(kdata[i].transAmount, 4);
            _append(kdata[i].transCount, 5);
        }
        break;

    case OPEN:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].openPrice);
        }
        break;

    case HIGH:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].highPrice);
        }
        break;

    case LOW:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].lowPrice);
        }
        break;

    case CLOSE:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].closePrice);
        }
        break;

    case AMOUNT:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].transAmount);
        }
        break;

    case COUNT:
        for (size_t i = 0; i < total; ++i) {
            _append(kdata[i].transCount);
        }
        break;

    default:
        HKU_INFO("Unkown ValueType of KData [IKData::IKData]");
        break;
    }
}


IKData::~IKData() {

}


string IKData::name() const {
    if (m_valueType == OPEN) {
        return "OPEN";
    } else if (m_valueType == HIGH) {
        return "HIGH";
    } else if (m_valueType == LOW) {
        return "LOW";
    } else if (m_valueType == CLOSE) {
        return "CLOSE";
    } else if (m_valueType == AMOUNT) {
        return "AMO";
    } else if (m_valueType == COUNT) {
        return "VOL";
    }

    return "Unknow";
}


Indicator HKU_API KDATA(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::KDATA)));
}

Indicator HKU_API OPEN(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::OPEN)));
}

Indicator HKU_API HIGH(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::HIGH)));
}

Indicator HKU_API LOW(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::LOW)));
}

Indicator HKU_API CLOSE(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::CLOSE)));
}

Indicator HKU_API TRANSAMOUNT(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::AMOUNT)));
}

Indicator HKU_API TRANSCOUNT(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::COUNT)));
}

Indicator HKU_API AMO(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::AMOUNT)));
}

Indicator HKU_API VOL(const KData& kdata) {
    return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::COUNT)));
}

Indicator HKU_API KDATA_PART(const KData& kdata, const string& part) {
    string part_name(part);
    boost::to_upper(part_name);
    if ("KDATA" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::KDATA)));
    } else if ("OPEN" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::OPEN)));
    } else if ("HIGH" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::HIGH)));
    } else if ("LOW" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::LOW)));
    } else if ("CLOSE" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::CLOSE)));
    } else if ("TRANSAMOUNT" == part_name || "AMO" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::AMOUNT)));
    } else if ("TRANSCOUNT" == part_name || "VOL" == part_name) {
        return Indicator(IndicatorImpPtr(new IKData(kdata, IKData::COUNT)));
    }

    return Indicator();
}

} /* namespace hku */
