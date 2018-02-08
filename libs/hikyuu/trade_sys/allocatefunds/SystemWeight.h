/*
 * SystemWeight.h
 *
 *  Created on: 2018年1月29日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_

#include "../system/System.h"

namespace hku {

class HKU_API SystemWeight {
public:
    SystemWeight();
    SystemWeight(const SystemPtr& sys, price_t weight);
    virtual ~SystemWeight();

    void setSYS(const SystemPtr& sys);
    SystemPtr getSYS() const;

    void setWeight(price_t weight);
    price_t getWeight() const;

public:
    SystemPtr sys;
    price_t weight;

private:

    //============================================
    // 序列化支持
    //============================================
    #if HKU_SUPPORT_SERIALIZATION
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const {
            ar & BOOST_SERIALIZATION_NVP(sys);
            ar & BOOST_SERIALIZATION_NVP(weight);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(sys);
            ar & BOOST_SERIALIZATION_NVP(weight);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    #endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef vector<SystemWeight> SystemWeightList;

HKU_API std::ostream & operator<<(std::ostream &, const SystemWeight&);

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_ */
