/*
 * MoneyManagerBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#ifndef MONEYMANAGERBASE_H_
#define MONEYMANAGERBASE_H_

#include "../../utilities/Parameter.h"
#include "../../trade_manage/TradeManager.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * 资金管理基类
 * @ingroup MoneyManager
 */
class HKU_API MoneyManagerBase {
    PARAMETER_SUPPORT

public:
    MoneyManagerBase();
    MoneyManagerBase(const string& name);
    virtual ~MoneyManagerBase();

    /** 获取名称 */
    string name() const {
        return m_name;
    }

    /** 设置名称 */
    void name(const string& name) {
        m_name = name;
    }

    /** 复位 */
    void reset();

    /**
     * 设定交易账户
     * @param tm 指定的交易账户
     */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    void setKType(KQuery::KType ktype) {
        m_ktype = ktype;
    }

    typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
    /** 克隆操作 */
    MoneyManagerPtr clone();

    /** 接收实际交易变化情况，一般存在多次增减仓的情况才需要重载 */
    virtual void buyNotify(const TradeRecord&);

    /** 接收实际交易变化情况，一般存在多次增减仓的情况才需要重载 */
    virtual void sellNotify(const TradeRecord&);

    /**
     * 获取指定交易对象可卖出的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 新的交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     * @note 默认实现返回Null<size_t>() 卖出全部; 多次减仓才需要实现该接口
     */
    size_t getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk);

    /**
     * 获取指定交易对象可卖空的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    size_t getSellShortNumber(const Datetime& datetime,
            const Stock& stock, price_t price, price_t risk);

    /**
     * 获取指定交易对象空头回补的买入数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 承担的交易风险，如果为Null<price_t>，表示不设损失上限
     */
    size_t getBuyShortNumber(const Datetime& datetime,
            const Stock& stock, price_t price, price_t risk);

    /**
     * 获取指定交易对象可买入的数量
     * @param datetime 交易日期
     * @param stock 交易对象
     * @param price 交易价格
     * @param risk 交易承担的风险，如果为0，表示全部损失，即市值跌至0元
     */
    size_t getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk);


    virtual size_t _getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) = 0;

    virtual size_t _getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk);

    virtual size_t _getSellShortNumber(const Datetime& datetime,
            const Stock& stock, price_t price, price_t risk);

    virtual size_t _getBuyShortNumber(const Datetime& datetime,
            const Stock& stock, price_t price, price_t risk);

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual MoneyManagerPtr _clone() = 0;

protected:
    string m_name;
    TradeManagerPtr m_tm;
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
        // m_ktype、m_tm都是系统运行时临时设置，不需要序列化
        //ar & BOOST_SERIALIZATION_NVP(m_ktype);
        //ar & BOOST_SERIALIZATION_NVP(m_tm);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        string name;
        ar & boost::serialization::make_nvp("m_name", name);
        m_name = UTF8ToGB(name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};


#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MoneyManagerBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public MoneyManagerBase {
 *     MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MoneyManager
 */
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase); \
    }
#else
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif


/**
 * 客户程序都应使用该指针类型
 * @ingroup MoneyManager
 */
typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;


#define MONEY_MANAGER_IMP(classname) public:\
    virtual MoneyManagerPtr _clone() {\
        return MoneyManagerPtr(new classname());\
    }\
    virtual size_t _getBuyNumber(const Datetime& datetime, const Stock& stock,\
                price_t price, price_t risk);


HKU_API std::ostream & operator<<(std::ostream &, const MoneyManagerBase&);
HKU_API std::ostream & operator<<(std::ostream &, const MoneyManagerPtr&);


inline void MoneyManagerBase::reset() {
    _reset();
}

} /* namespace hku */
#endif /* MONEYMANAGERBASE_H_ */
