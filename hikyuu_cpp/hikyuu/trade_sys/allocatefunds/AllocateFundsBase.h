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

    string name() const;
    void name(const string& name);

    /**
     * Portfolio获取实际获得资产分配的系统策略实例
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @param hold_list 当前分配过资金的系统实例
     * @return
     */
    SystemList getAllocatedSystemList(const Datetime& date,
                                      const SystemList& se_list,
                                      const SystemList& hold_list);

    /** 获取交易账户 */
    TMPtr getTM();

    /** 设定交易账户 */
    void setTM(const TMPtr&);

    KQuery getQuery();
    void setQuery(KQuery query);

    /** 获取不参与资产分配的保留比例 */
    double getReservePercent();

    /**
     * 设置不参与资产分配的保留比例，该比例在执行reset时会被置为0
     * @param percent 取值范围[0,1]，小于0将被强制置为0， 大于1将被置为1
     */
    void setReserverPercent(double p);

    bool changed(Datetime date);

    /** 复位 */
    void reset();

    typedef shared_ptr<AllocateFundsBase> AFPtr;
    /** 克隆操作 */
    AFPtr clone();

    /** 子类复位接口 */
    virtual void _reset() {}

    /** 子类克隆私有变量接口 */
    virtual AFPtr _clone() = 0;

    /**
     * 子类分配权重接口，获取实际分配资产的系统实例及其权重
     * @details 实际调用子类接口 _allocateWeight，并根据允许的最大持仓系统数参数对子类返回的
     *          系统实例及权重列表进行了截断处理
     * @param date 指定日期
     * @param se_list 系统实例选择器选出的系统实例
     * @param hold_list 当前持仓的系统实例
     * @return
     */

    virtual SystemWeightList _allocateWeight(const Datetime& date,
                                             const SystemList& se_list) = 0;

    void _getAllocatedSystemList_adjust_hold(
            const Datetime& date,
            const SystemList& se_list,
            const SystemList& hold_list,
            SystemList& out_sys_list);
    void _getAllocatedSystemList_not_adjust_hold(
            const Datetime& date,
            const SystemList& se_list,
            const SystemList& hold_list,
            SystemList& out_sys_list);

private:
    string m_name;
    KQuery m_query;
    int    m_count;
    Datetime m_pre_date;
    TMPtr  m_tm;

    double m_reserve_percent; //保留资产比例，不参与资产分配

    list<SystemWeight> m_wait_for_allocate_list;

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
            ar & BOOST_SERIALIZATION_NVP(m_query);
            ar & BOOST_SERIALIZATION_NVP(m_count);
            ar & BOOST_SERIALIZATION_NVP(m_pre_date);
            ar & BOOST_SERIALIZATION_NVP(m_reserve_percent);
            ar & BOOST_SERIALIZATION_NVP(m_tm);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            ar & boost::serialization::make_nvp("name", m_name);
            ar & BOOST_SERIALIZATION_NVP(m_params);
            ar & BOOST_SERIALIZATION_NVP(m_query);
            ar & BOOST_SERIALIZATION_NVP(m_count);
            ar & BOOST_SERIALIZATION_NVP(m_pre_date);
            ar & BOOST_SERIALIZATION_NVP(m_reserve_percent);
            ar & BOOST_SERIALIZATION_NVP(m_tm);
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
 * class Drived: public AllocateFundsBase {
 *     ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
    }
#else
#define ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATEFUNDS_IMP(classname) public:\
    virtual AFPtr _clone() {\
        return AFPtr(new classname());\
    }\
    virtual SystemWeightList _allocateWeight(const Datetime&, const SystemList&);


typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;
typedef shared_ptr<AllocateFundsBase> AFPtr;


HKU_API std::ostream & operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream & operator<<(std::ostream&, const AFPtr&);


inline string AllocateFundsBase::name() const {
    return m_name;
}

inline void AllocateFundsBase::name(const string& name) {
    m_name = name;
}


inline TMPtr AllocateFundsBase::getTM() {
    return m_tm;
}

inline void AllocateFundsBase::setTM(const TMPtr& tm) {
    m_tm = tm;
}

inline KQuery AllocateFundsBase::getQuery() {
    return m_query;
}

inline void AllocateFundsBase::setQuery(KQuery query) {
    m_query = query;
}

inline double AllocateFundsBase::getReservePercent() {
    return m_reserve_percent;
}


} /* namespace hku */

#endif /* TRADE_SYS_ALLOCATEFUNDS_ALLOCATEFUNDSBASE_H_ */
