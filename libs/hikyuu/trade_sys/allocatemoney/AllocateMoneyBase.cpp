/*
 * AllocateMoney.cpp
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#include "AllocateMoneyBase.h"

namespace hku {

AllocateMoneyBase::AllocateMoneyBase(): m_name("AllocateMoneyBase") {

}

AllocateMoneyBase::AllocateMoneyBase(const string& name)
: m_name("AllocateMoneyBase") {

}

AllocateMoneyBase::~AllocateMoneyBase() {

}

PriceList AllocateMoneyBase::getAllocateMoney(const Datetime&, const SystemWeightList) {
    return PriceList();
}

} /* namespace hku */
