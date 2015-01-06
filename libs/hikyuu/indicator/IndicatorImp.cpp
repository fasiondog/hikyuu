/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "../Log.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const IndicatorImp& imp) {
    os << "Indicator(" << imp.name() << ", " << imp.getParameter() << ")";
    return os;
}


HKU_API std::ostream & operator<<(std::ostream& os, const IndicatorImpPtr& imp) {
    if (!imp) {
        os << "Indicator(NULL)";
    } else {
        os << *imp;
    }
    return os;
}


IndicatorImp::IndicatorImp(): m_discard(0), m_result_num(0) {
    for (int i = 0; i < MAX_RESULT_NUM; ++i) {
        m_pBuffer[i] = NULL;
    }
}


IndicatorImp::IndicatorImp(int discard, size_t result_num)
: m_discard(discard), m_result_num(result_num) {
    if (discard < 0) {
        m_discard = 0;
        HKU_ERROR("Discard is a negative number, it's set to 0! [IndicatorImp::IndicatorImp]");
    }
    for (int i = 0; i < MAX_RESULT_NUM; ++i) {
        m_pBuffer[i] = NULL;
    }

    if (m_result_num > MAX_RESULT_NUM) {
        m_result_num = MAX_RESULT_NUM;
        HKU_ERROR("Only support " << MAX_RESULT_NUM << " results! [IndicatorImp::IndicatorImp]");
    }

    for (size_t i = 0; i < m_result_num; ++i) {
        m_pBuffer[i] = new PriceList();
    }
}


IndicatorImp::
IndicatorImp(const Indicator& indicator, int discard, size_t result_num)
: m_result_num(result_num) {
    if (discard < 0) {
        m_discard = 0;
        HKU_ERROR("Discard is a negative number, it's set to 0! [IndicatorImp::IndicatorImp]");
    } else {
        //传入的indicator有可能存在discard
        m_discard = indicator.discard() + discard;
    }

    for (int i = 0; i < MAX_RESULT_NUM; ++i) {
        m_pBuffer[i] = NULL;
    }

    if (m_result_num > MAX_RESULT_NUM) {
        m_result_num = MAX_RESULT_NUM;
        HKU_WARN("Only support " << MAX_RESULT_NUM << " results! [IndicatorImp::IndicatorImp]");
    }

    size_t total = indicator.size();
    price_t null_price = Null<price_t>();
    for (size_t i = 0; i < m_result_num; ++i) {
        m_pBuffer[i] = new PriceList(total, null_price);
    }
}


IndicatorImp::~IndicatorImp() {
    for (size_t i = 0; i < m_result_num; ++i) {
        delete m_pBuffer[i];
    }
}


string IndicatorImp::long_name() const {
    //return name() + "(" + m_params.getValueList() + ")";
    return name() + "(" + m_params.getNameValueList() + ")";
}


string IndicatorImp::name() const {
    return "IndicatorImp";
}


PriceList IndicatorImp::getResultAsPriceList(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return PriceList();
    }

    return PriceList(*m_pBuffer[result_num]);
}


Indicator IndicatorImp::getResult(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return Indicator();
    }

    IndicatorImpPtr imp(new IndicatorImp(discard(), 1));
    size_t total = size();
    for (size_t i = discard(); i < total; ++i) {
        imp->_append(get(i, result_num), 0);
    }
    return Indicator(imp);
}

} /* namespace hku */
