/*
 * SelectorBase.h
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SELECTOR_SELECTORBASE_H_
#define TRADE_SYS_SELECTOR_SELECTORBASE_H_

#include "../system/System.h"
#include "../../KData.h"
#include "../../utilities/Parameter.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 交易对象选择模块
 * @ingroup Selector
 */
class HKU_API SelectorBase: public enable_shared_from_this<SelectorBase>  {
    PARAMETER_SUPPORT

public:
    SelectorBase();
    SelectorBase(const string& name);
    virtual ~SelectorBase();

    string name() const;
    void name(const string& name);

    void addStock(const Stock& stock, const SystemPtr& protoSys);
    void addStockList(const StockList& stkList, const SystemPtr& protoSys);

    SystemList getAllSystemList() const { return m_sys_list; }

    bool changed(Datetime date);

    void reset();

    void clear();

    typedef shared_ptr<SelectorBase> SelectorPtr;
    SelectorPtr clone();

    virtual SystemList getSelectedSystemList(Datetime date) = 0;

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual SelectorPtr _clone() = 0;

protected:
    string m_name;
    int m_count;
    Datetime m_pre_date;
    SystemList m_sys_list;

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
        ar & BOOST_SERIALIZATION_NVP(m_count);
        ar & BOOST_SERIALIZATION_NVP(m_pre_date);
        ar & BOOST_SERIALIZATION_NVP(m_sys_list);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        ar & boost::serialization::make_nvp("name", m_name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_count);
        ar & BOOST_SERIALIZATION_NVP(m_pre_date);
        ar & BOOST_SERIALIZATION_NVP(m_sys_list);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SelectorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public SelectorBase {
 *     SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase); \
    }
#else
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif


#define SELECTOR_IMP(classname) public:\
    virtual SelectorPtr _clone() {\
        return SelectorPtr(new classname());\
    }\
    virtual SystemList getSelectedSystemList(Datetime date);


/**
 * 客户程序都应使用该指针类型
 * @ingroup Selector
 */
typedef shared_ptr<SelectorBase> SelectorPtr;
typedef shared_ptr<SelectorBase> SEPtr;

HKU_API std::ostream & operator<<(std::ostream&, const SelectorBase&);
HKU_API std::ostream & operator<<(std::ostream&, const SelectorPtr&);


inline string SelectorBase::name() const {
    return m_name;
}

inline void SelectorBase::name(const string& name) {
    m_name = name;
}


} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_SELECTORBASE_H_ */
