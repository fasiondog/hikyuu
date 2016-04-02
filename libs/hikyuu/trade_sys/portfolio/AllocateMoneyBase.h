/*
 * AllocateMoneyBase.h
 *
 *  Created on: 2016年3月23日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_PORTFOLIO_ALLOCATEMONEYBASE_H_
#define TRADE_SYS_PORTFOLIO_ALLOCATEMONEYBASE_H_

#include "../system/System.h"
#include "../../KData.h"
#include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"

#if HKU_SUPPORT_SERIALIZATION
#include "../../serialization/Datetime_serialization.h"
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

class HKU_API AllocateMoneyBase {
    PARAMETER_SUPPORT

public:
    AllocateMoneyBase();
    AllocateMoneyBase(const TradeManagerPtr& tm);
    virtual ~AllocateMoneyBase();

    void setTM(const TradeManagerPtr& tm) { m_tm = tm; }
    TradeManagerPtr getTM() { return m_tm; }

    void reset();

    typedef shared_ptr<AllocateMoneyBase> AllocateMoneyPtr;
    AllocateMoneyPtr clone();

    virtual StockList tryAllocate(Datetime date, const StockList& stocks);

    virtual string name() const {
        return "AllocateMoneyBase";
    }

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual AllocateMoneyPtr _clone() = 0;

protected:
    TradeManagerPtr m_tm;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        string name_str(GBToUTF8(name()));
        ar & boost::serialization::make_nvp("name", name_str);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
            ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(AllocateMoneyBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public AllocateMoneyBase {
 *     ALLOCATEMONEY_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup AllocateMoney
 */
#define ALLOCATEMONEY_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateMoneyBase); \
    }
#else
#define ALLOCATEMONEY_NO_PRIVATE_MEMBER_SERIALIZATION
#endif


#define ALLOCATEMONEY_IMP(classname, str_name) public:\
    virtual string name() const { \
        return(str_name);\
    }\
    virtual AllocateMoneyPtr _clone() {\
        return AllocateMoneyPtr(new classname());\
    }


/**
 * 客户程序都应使用该指针类型
 * @ingroup AllocateMoney
 */
typedef shared_ptr<AllocateMoneyBase> AllocateMoneyPtr;

HKU_API std::ostream & operator<<(std::ostream&, const AllocateMoneyBase&);
HKU_API std::ostream & operator<<(std::ostream&, const AllocateMoneyPtr&);

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_ALLOCATEMONEYBASE_H_ */
