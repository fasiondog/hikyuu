/*
 * SystemWeight.cpp
 *
 *  Created on: 2018年1月29日
 *      Author: fasiondog
 */

#include "../allocatefunds/SystemWeight.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream & os, const SystemWeight& sw) {
    os << std::fixed;
    os.precision(4);

    string name("NULL");
    string stk_name("(Stock(NULL))");
    if (sw.getSYS()) {
       name = sw.getSYS()->name();

       Stock stk = sw.getSYS()->getStock();
       if (!stk.isNull()) {
           stk_name = "(Stock(" + stk.market_code() + "))";
       }
    }

    os << "SystemWeight(sys: " << name << stk_name
       << ",  weight: " << sw.getWeight()
       << ")";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

SystemWeight::SystemWeight(): m_weight(100) {

}

SystemWeight::SystemWeight(const SystemPtr& sys, price_t weight)
: m_sys(sys), m_weight(weight) {

}

SystemWeight::~SystemWeight() {

}

} /* namespace hku */
