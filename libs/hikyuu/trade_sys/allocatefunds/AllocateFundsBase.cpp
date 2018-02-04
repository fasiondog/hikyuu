/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#include "../allocatefunds/AllocateFundsBase.h"

namespace hku {

AllocateFundsBase::AllocateFundsBase(): m_name("AllocateMoneyBase") {

}

AllocateFundsBase::AllocateFundsBase(const string& name)
: m_name("AllocateMoneyBase") {

}

AllocateFundsBase::~AllocateFundsBase() {

}

AFPtr AllocateFundsBase::clone() {
    AFPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [AllocateMoneyBase::clone]");
        p = AFPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [AllocateMoneyBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    return p;
}

SystemWeightList AllocateFundsBase
::getAllocateMoney(const SystemList& sw_list) {

    return SystemWeightList();
}

} /* namespace hku */
