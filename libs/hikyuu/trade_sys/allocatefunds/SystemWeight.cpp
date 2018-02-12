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
    if (sw.sys) {
       name = sw.sys->name();

       Stock stk = sw.sys->getStock();
       if (!stk.isNull()) {
           stk_name = "(Stock(" + stk.market_code() + "))";
       }
    }

    os << "SystemWeight(sys: " << name << stk_name
       << ",  weight: " << sw.weight
       << ")";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

SystemWeight::SystemWeight(): weight(100) {

}

SystemWeight::SystemWeight(const SystemPtr& sys, price_t weight)
: sys(sys), weight(weight) {

}

SystemWeight::~SystemWeight() {

}

} /* namespace hku */
