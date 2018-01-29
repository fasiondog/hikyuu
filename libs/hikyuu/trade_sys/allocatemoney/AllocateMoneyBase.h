/*
 * AllocateMoney.h
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_ALLOCATEMONEY_ALLOCATEMONEYBASE_H_
#define TRADE_SYS_ALLOCATEMONEY_ALLOCATEMONEYBASE_H_

#include "../selector/SystemWeight.h"
#include "../../utilities/Parameter.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

class HKU_API AllocateMoneyBase {
    PARAMETER_SUPPORT

public:
    AllocateMoneyBase();
    AllocateMoneyBase(const string& name);
    virtual ~AllocateMoneyBase();

    PriceList getAllocateMoney(const Datetime&, const SystemWeightList);

private:
    string m_name;

    //============================================
    // 序列化支持
    //============================================
    #if HKU_SUPPORT_SERIALIZATION
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const {
            string name_str(GBToUTF8(m_name));
            ar & boost::serialization::make_nvp("name", name_str);
            ar & BOOST_SERIALIZATION_NVP(m_params);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("name", m_name);
            ar & BOOST_SERIALIZATION_NVP(m_params);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    #endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<AllocateMoneyBase> AllocateMoneyPtr;
typedef shared_ptr<AllocateMoneyBase> AMPtr;

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEMONEY_ALLOCATEMONEYBASE_H_ */
