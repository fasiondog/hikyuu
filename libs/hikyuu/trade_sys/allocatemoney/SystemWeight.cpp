/*
 * SystemWeight.cpp
 *
 *  Created on: 2018年1月29日
 *      Author: fasiondog
 */

#include "../allocatemoney/SystemWeight.h"

namespace hku {

SystemWeight::SystemWeight(): m_weight(100) {

}

SystemWeight::SystemWeight(const SystemPtr& sys, price_t weight)
: m_sys(sys), m_weight(weight) {

}

SystemWeight::~SystemWeight() {

}

} /* namespace hku */
