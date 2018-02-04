/*
 * AllocateMoney.h
 *
 *  Created on: 2018年1月30日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_
#define TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_

#include "../../utilities/Parameter.h"
#include "../allocatefunds/SystemWeight.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

class HKU_API AllocateFundsBase: public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT

public:
    AllocateFundsBase();
    AllocateFundsBase(const string& name);
    virtual ~AllocateFundsBase();

    /** 获取交易账户 */
    TMPtr getTM();

    /** 设定交易账户 */
    void setTM(const TMPtr&);

    /** 复位 */
    void reset();

    typedef shared_ptr<AllocateFundsBase> AFPtr;
    /** 克隆操作 */
    AFPtr clone();

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual AFPtr _clone() {return AFPtr(); }

    SystemWeightList getAllocateMoney(const SystemList&);

private:
    string m_name;
    TMPtr  m_tm;

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
            ar & BOOST_SERIALIZATION_NVP(m_tm);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("name", m_name);
            ar & BOOST_SERIALIZATION_NVP(m_params);
            ar & BOOST_SERIALIZATION_NVP(m_tm);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    #endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;

inline TMPtr AllocateFundsBase::getTM() {
    return m_tm;
}

inline void AllocateFundsBase::setTM(const TMPtr& tm) {
    m_tm = tm;
}

inline void AllocateFundsBase::reset() {
    _reset();
}

} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_ */
