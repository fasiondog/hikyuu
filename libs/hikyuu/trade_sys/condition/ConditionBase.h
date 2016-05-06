/*
 * ConditionBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#ifndef CONDITIONBASE_H_
#define CONDITIONBASE_H_

#include "../../utilities/Parameter.h"
#include "../../utilities/util.h"
#include "../../KData.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 系统有效条件基类
 * @note 系统有效性和待交易的对象有关也可能没关，所以保留setTradeObj接口
 * @ingroup Condition
 */
class HKU_API ConditionBase {
    PARAMETER_SUPPORT

public:
    ConditionBase();
    ConditionBase(const string& name);
    virtual ~ConditionBase();

    /** 获取名称 */
    const string& name() const;

    /** 复位操作 */
    void reset();

    /** 设置交易对象 */
    void setTO(const KData& kdata);

    typedef shared_ptr<ConditionBase> ConditionPtr;
    /** 克隆操作 */
    ConditionPtr clone();

    /**
     * 指定日期系统是否有效
     * @param datetime 指定日期
     * @return true 有效 | false 失效
     */
    virtual bool isValid(const Datetime& datetime) = 0;

    virtual void _calculate() = 0;

    /** 子类reset接口 */
    virtual void _reset() = 0;

    /** 子类克隆接口 */
    virtual ConditionPtr _clone() = 0;

protected:
    string m_name;
    KData  m_kdata;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        string name(GBToUTF8(m_name));
        ar & boost::serialization::make_nvp("m_name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        // m_kdata是系统运行时临时设置，不需要序列化
        //ar & BOOST_SERIALIZATION_NVP(m_ktype);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("m_name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        // m_kdata是系统运行时临时设置，不需要序列化
        //ar & BOOST_SERIALIZATION_NVP(m_ktype);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};


#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ConditionBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public ConditionBase {
 *     CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Condition
 */
#define CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase); \
    }
#else
#define CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
#endif


/**
 * 客户程序都应使用该指针类型
 * @ingroup Condition
 */
typedef shared_ptr<ConditionBase> ConditionPtr;

HKU_API std::ostream & operator<<(std::ostream &, const ConditionPtr&);
HKU_API std::ostream & operator<<(std::ostream &, const ConditionBase&);


inline const string& ConditionBase::name() const {
    return m_name;
}


inline void ConditionBase::reset() {
    _reset();
}

} /* namespace hku */
#endif /* CONDITIONBASE_H_ */
