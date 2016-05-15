/*
 * Environment.h
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include "../../KQuery.h"
#include "../../utilities/Parameter.h"
#include "../../utilities/util.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 环境判定策略基类
 * @note 外部环境应该和具体的交易对象没有关系
 * @ingroup Environment
 */
class HKU_API EnvironmentBase {
    PARAMETER_SUPPORT

public:
    EnvironmentBase();
    EnvironmentBase(const string& name);
    virtual ~EnvironmentBase();

    /** 获取名称 */
    string name() const;

    /** 设置名称 */
    void name(const string& name);

    /** 复位 */
    void reset();

    void setKType(KQuery::KType ktype) { m_ktype = ktype; }

    typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
    /** 克隆操作 */
    EnvironmentPtr clone();

    /**
     * 判断指定日期的外部环境是否有效
     * @param market 市场简称
     * @param datetime 指定日期
     * @return true 有效 | false 无效
     */
    virtual bool isValid(const string& market, const Datetime& datetime) = 0;

    /** 子类复位接口 */
    virtual void _reset() = 0;

    /** 子类克隆接口 */
    virtual EnvironmentPtr _clone() = 0;

protected:
    string m_name;
    KQuery::KType m_ktype;

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
        //ev可能多个系统共享，保留m_ktype可能用于差错
        ar & BOOST_SERIALIZATION_NVP(m_ktype);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("m_name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_ktype);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(EnvironmentBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public EnvironmentBase {
 *     ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Environment
 */
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase); \
    }
#else
#define ENVIRONMENT_NO_PRIVATE_MEMBER_SERIALIZATION
#endif


/**
 * 客户程序都应使用该指针类型
 * @ingroup Environment
 */
typedef shared_ptr<EnvironmentBase> EnvironmentPtr;
typedef shared_ptr<EnvironmentBase> EVPtr;


/**
 * 输出Environment信息，如：Environment(name, params[...])
 * @ingroup Environment
 */
HKU_API std::ostream& operator <<(std::ostream &os, const EnvironmentPtr&);
HKU_API std::ostream& operator <<(std::ostream &os, const EnvironmentBase&);


inline string EnvironmentBase::name() const {
    return m_name;
}

inline void EnvironmentBase::name(const string& name) {
    m_name = name;
}

inline void EnvironmentBase::reset() {
    m_ktype = KQuery::DAY;
    _reset();
}

} /* namespace hku */
#endif /* ENVIRONMENT_H_ */
