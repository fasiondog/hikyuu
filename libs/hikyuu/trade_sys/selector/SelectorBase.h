/*
 * SelectorBase.h
 *
 *  Created on: 2016年2月21日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_SELECTOR_SELECTORBASE_H_
#define TRADE_SYS_SELECTOR_SELECTORBASE_H_

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

/**
 * 交易对象选择模块
 * @ingroup Selector
 */
class HKU_API SelectorBase {
    PARAMETER_SUPPORT

public:
    SelectorBase();
    virtual ~SelectorBase();

    void addStock(const Stock& stock);
    void addStockList(const StockList&);

    StockList getRawStockList() const { return m_stock_list; }

    void reset();

    void clearStockList();

    typedef shared_ptr<SelectorBase> SelectorPtr;
    SelectorPtr clone();

    virtual string name() const {
        return "SelectorBase";
    }

    virtual StockList getSelectedStock(Datetime date);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆接口 */
    virtual SelectorPtr _clone() = 0;


protected:
    StockList m_stock_list;

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
        ar & BOOST_SERIALIZATION_NVP(m_stock_list);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_stock_list);
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


#define SELECTOR_IMP(classname, str_name) public:\
    virtual string name() const { \
        return(str_name);\
    }\
    virtual SelectorPtr _clone() {\
        return SelectorPtr(new classname());\
    }


/**
 * 客户程序都应使用该指针类型
 * @ingroup Selector
 */
typedef shared_ptr<SelectorBase> SelectorPtr;

HKU_API std::ostream & operator<<(std::ostream&, const SelectorBase&);
HKU_API std::ostream & operator<<(std::ostream&, const SelectorPtr&);

} /* namespace hku */

#endif /* TRADE_SYS_SELECTOR_SELECTORBASE_H_ */
