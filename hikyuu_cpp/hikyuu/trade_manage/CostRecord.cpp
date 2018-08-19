/*
 * CostRecord.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "CostRecord.h"

namespace hku {

CostRecord::CostRecord()
: commission(0.0), stamptax(0.0), transferfee(0.0), others(0.0), total(0.0) {

}

CostRecord::CostRecord(price_t commission, price_t stamptax,
        price_t transferfee, price_t others, price_t total)
: commission(commission), stamptax(stamptax), transferfee(transferfee),
  others(others), total(total){

}

HKU_API std::ostream& operator <<(std::ostream &os, const CostRecord& record) {
    os << record.toString();
    return os;
}

std::string CostRecord::toString() const {
    std::stringstream os;
    os << std::fixed;
    os.precision(2);
    os << "CostRecord(commision: " << commission << ", stamptax: " << stamptax
       << ", transferfee: " << transferfee << ", others: " << others
       << ", total: " << total << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}


bool HKU_API operator==(const CostRecord& d1, const CostRecord&d2) {
    if (std::fabs(d1.commission - d2.commission) < 0.0001
            && std::fabs(d1.stamptax - d2.stamptax) < 0.0001
            && std::fabs(d1.transferfee - d2.transferfee) < 0.0001
            && std::fabs(d1.others - d2.others) < 0.0001
            && std::fabs(d1.total - d2.total) < 0.0001) {
        return true;
    }
    return false;
}

} /* namespace hku */
