/*
 * SystemWeight.h
 *
 *  Created on: 2018-1-29
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_

#include "../system/System.h"

namespace hku {

/**
 * System weight, the valid range of the weight is [0.0, 1.0]
 * @details It is used to give the corresponding weight of a system to the asset allocation
 * algorithm
 * @ingroup Selector
 */
struct HKU_API SystemWeight {
    SystemPtr sys;
    double weight{1.0};

    SystemWeight() = default;
    SystemWeight(const SystemPtr& sys_, double weight_) : sys(sys_), weight(weight_) {}
    SystemWeight(const SystemWeight&) = default;
    SystemWeight(SystemWeight&& sw) : sys(std::move(sw.sys)), weight(sw.weight) {}
    SystemWeight& operator=(const SystemWeight& other) = default;
    SystemWeight& operator=(SystemWeight&& other);

private:
//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(sys);
        ar& BOOST_SERIALIZATION_NVP(weight);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(sys);
        ar& BOOST_SERIALIZATION_NVP(weight);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef vector<SystemWeight> SystemWeightList;

HKU_API std::ostream& operator<<(std::ostream&, const SystemWeight&);

inline bool operator==(const SystemWeight& d1, const SystemWeight& d2) {
    return d1.sys == d2.sys && std::abs(d1.weight - d2.weight) < 0.0001;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SystemWeight> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_ */
