/*
 * SystemWeight.cpp
 *
 *  Created on: 2018年1月29日
 *      Author: fasiondog
 */

#include "SystemWeight.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const SystemWeight& sw) {
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

    os << "SystemWeight(sys: " << name << stk_name << ",  weight: " << sw.weight << ")";

    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os;
}

SystemWeight& SystemWeight::operator=(SystemWeight&& other) {
    if (this != &other) {
        sys = std::move(other.sys);
        weight = other.weight;
    }
    return *this;
}

} /* namespace hku */
