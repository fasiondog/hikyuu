/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#endif

namespace hku {

struct HKU_API MarginRecord {
    MarginRecord() = default;
    MarginRecord(const MarginRecord&) = default;
    MarginRecord(double initRatio, double maintainRatio)
    : initRatio(initRatio), maintainRatio(maintainRatio) {}

    double initRatio = 1.0;      // 初始保证金
    double maintainRatio = 1.0;  // 维持保证金

    string toString() const {
        return fmt::format("MarginRecord(initRatio: {:<.2f}, maintainRatio: {:<.2f}", initRatio,
                           maintainRatio);
    }

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(initRatio);
        ar& BOOST_SERIALIZATION_NVP(maintainRatio);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

inline std::ostream& operator<<(std::ostream& os, const MarginRecord& mr) {
    os << mr.toString();
    return os;
}

inline bool operator==(const MarginRecord& d1, const MarginRecord& d2) {
    return std::fabs(d1.initRatio - d2.initRatio) < 0.0001 &&
           std::fabs(d1.maintainRatio - d2.maintainRatio) < 0.0001;
}

}  // namespace hku