/*
 * ';.kmuhbvcdxIndicatorImp.h
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#ifndef INDICATORIMP_H_
#define INDICATORIMP_H_

#include "../KData.h"
#include "../utilities/Parameter.h"
#include "../utilities/util.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

#define MAX_RESULT_NUM 10

class HKU_API Indicator;

/**
 * 指标实现类，定义新指标时，应从此类继承
 * @ingroup Indicator
 */
class HKU_API IndicatorImp {
    PARAMETER_SUPPORT

public:
    /**
     * 默认构造函数
     * @note 未初始化内部结果集缓存，新指标不应该使用该构造函数，仅用于Indicator保护
     */
    IndicatorImp();

    /**
     * 构造函数，仅指定抛弃的点数和结果集的个数
     * @param discard
     * @param result_num
     * @note 一般用于未关联相关数据时，获取指标需抛弃的数据个数; 新指标对应的构造函数
     * 中，不应该包含相应的指标计算。部分特殊指标如IPriceList实现时，使用该构造函数，
     * 此时，放置结果数据时，应使用 append 函数
     */
    IndicatorImp(int discard, size_t result_num);

    /**
     * 构造函数，关联数据，并指定抛弃的点数和结果集个数
     * @param indicator 带计算的数据，即输入
     * @param discard 结果中抛弃的数据个数
     * @param result_num 结果集的个数
     * @note 新指标构造函数中如果调用该构造函数，内部结果集缓存已经准备好，此时计算结果
     * 时，应使用 set 函数放置结果，勿使用 append
     */
    IndicatorImp(const Indicator&, int discard, size_t result_num);
    virtual ~IndicatorImp();

    size_t getResultNumber() const {
        return m_result_num;
    }

    size_t discard() const {
        return m_discard;
    }

    size_t size() const {
        return m_pBuffer[0] ? m_pBuffer[0]->size() : 0;
    }

    price_t get(size_t pos, size_t num = 0) {
        return (*m_pBuffer[num])[pos];
    }

    /** 以PriceList方式获取指定的输出集 */
    PriceList getResultAsPriceList(size_t result_num);

    /** 以Indicator的方式获取指定的输出集，该方式包含了discard的信息 */
    Indicator getResult(size_t result_num);

    /**
     * 使用IndicatorImp(const Indicator&...)构造函数后，计算结果使用该函数
     */
    void _set(price_t val, size_t pos, size_t num = 0) {
        (*m_pBuffer[num])[pos] = val;
    }

    /**
     * 未使用IndicatorImp(const Indicator&...)构造函数时，使用该函数放置结果
     */
    void _append(price_t val, size_t num = 0) {
        m_pBuffer[num]->push_back(val);
    }

    /** 返回形如：Name(param1=val,param2=val,...) */
    string long_name() const;

    // ===================
    //  子类接口
    // ===================
    virtual string name() const;

    typedef shared_ptr<IndicatorImp> IndicatorImpPtr;
    virtual IndicatorImpPtr operator()(const Indicator& ind) {
        return IndicatorImpPtr(new IndicatorImp(ind, m_discard, m_result_num));
    }


protected:
    int m_discard;
    size_t m_result_num;
    PriceList *m_pBuffer[MAX_RESULT_NUM];

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        string name_str(GBToUTF8(name()));
        ar & bs::make_nvp<string>("name", name_str);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        for (size_t i = 0; i < m_result_num; ++i) {
            std::stringstream buf;
            buf << "result_" << i;
            ar & bs::make_nvp<PriceList>(buf.str().c_str(), *m_pBuffer[i]);
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        namespace bs = boost::serialization;
        string name;
        ar & bs::make_nvp<string>("name", name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        for (size_t i = 0; i < m_result_num; ++i) {
            m_pBuffer[i] = new PriceList();
            std::stringstream buf;
            buf << "result_" << i;
            ar & bs::make_nvp<PriceList>(buf.str().c_str(), *m_pBuffer[i]);
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(IndicatorImp)
#endif

#if HKU_SUPPORT_SERIALIZATION
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp); \
    }
#else
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef shared_ptr<IndicatorImp> IndicatorImpPtr;

HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImp&);
HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImpPtr&);

} /* namespace hku */
#endif /* INDICATORIMP_H_ */
