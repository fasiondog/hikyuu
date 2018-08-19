/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <stdexcept>
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


IndicatorImp::IndicatorImp()
: m_name("IndicatorImp"), m_discard(0), m_result_num(0) {
    memset(m_pBuffer, NULL, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name)
: m_name(name), m_discard(0), m_result_num(0) {
    memset(m_pBuffer, NULL, sizeof(PriceList*) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string& name, size_t result_num)
: m_name(name), m_discard(0) {
    memset(m_pBuffer, NULL, sizeof(PriceList*) * MAX_RESULT_NUM);
    m_result_num = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
}

void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
    if (result_num > MAX_RESULT_NUM) {
        throw(std::invalid_argument("result_num oiverload MAX_RESULT_NUM! "
                "[IndicatorImp::_readyBuffer]" + name()));
        return;
    }

    price_t null_price = Null<price_t>();

    for (size_t i = 0; i < result_num; ++i) {
        if (!m_pBuffer[i]) {
            m_pBuffer[i] = new PriceList(len, null_price);

        } else {
            m_pBuffer[i]->clear();
            m_pBuffer[i]->reserve(len);
            for (size_t j = 0; j < len; ++j) {
                m_pBuffer[i]->push_back(null_price);
            }
        }
    }

    for (size_t i = result_num; i < m_result_num; ++i) {
        delete m_pBuffer[i];
        m_pBuffer[i] = NULL;
    }

    m_result_num = result_num;
}

IndicatorImp::~IndicatorImp() {
    for (size_t i = 0; i < m_result_num; ++i) {
        delete m_pBuffer[i];
    }
}


IndicatorImpPtr IndicatorImp::operator()(const Indicator& ind) {
    HKU_INFO("This indicator not support operator()! " << *this
            << " [IndicatorImp::operator()]");
    //保证对齐
    IndicatorImpPtr result = make_shared<IndicatorImp>();
    size_t total = ind.size();
    result->_readyBuffer(total, m_result_num);
    result->setDiscard(total);
    return result;
}

void IndicatorImp::setDiscard(size_t discard) {
    size_t tmp_discard = discard > size() ? size() : discard;
    if (tmp_discard > m_discard) {
        price_t null_price = Null<price_t>();
        for (size_t i = 0; i < m_result_num; ++i) {
            for (size_t j = m_discard; j < tmp_discard; ++j) {
                _set(null_price, j, i);
            }
        }
        m_discard = tmp_discard;
    }
}

string IndicatorImp::long_name() const {
    return name() + "(" + m_params.getNameValueList() + ")";
}

PriceList IndicatorImp::getResultAsPriceList(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return PriceList();
    }

    return (*m_pBuffer[result_num]);
}


Indicator IndicatorImp::getResult(size_t result_num) {
    if (result_num >= m_result_num || m_pBuffer[result_num] == NULL) {
        return Indicator();
    }

    IndicatorImpPtr imp = make_shared<IndicatorImp>();
    size_t total = size();
    imp->_readyBuffer(total, 1);
    imp->setDiscard(discard());
    for (size_t i = discard(); i < total; ++i) {
        imp->_set(get(i, result_num), i);
    }
    return Indicator(imp);
}

void IndicatorImp::calculate(const Indicator& data) {
    _readyBuffer(data.size(), m_result_num);
    if (check()) {
        _calculate(data);
    } else {
        HKU_WARN("Invalid param! " << long_name());
    }
}

} /* namespace hku */
