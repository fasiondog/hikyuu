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
    if (sw.sys) {
       name = sw.sys->name();
    }

    os << "SystemWeight(sys: " << name
       << ",  weight: " << sw.weight
       << ")"<< std::endl;

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
