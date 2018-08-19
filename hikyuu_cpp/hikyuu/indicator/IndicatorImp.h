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

#define MAX_RESULT_NUM 6

class HKU_API Indicator;

/**
 * 指标实现类，定义新指标时，应从此类继承
 * @ingroup Indicator
 */
class HKU_API IndicatorImp {
    PARAMETER_SUPPORT

public:
    /** 默认构造函数   */
    IndicatorImp();
    IndicatorImp(const string& name);
    IndicatorImp(const string& name, size_t result_num);

    virtual ~IndicatorImp();

    size_t getResultNumber() const {
        return m_result_num;
    }

    size_t discard() const {
        return m_discard;
    }

    void setDiscard(size_t discard);

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
     * 使用IndicatorImp(const Indicator&...)构造函数后，计算结果使用该函数,
     * 未做越界保护
     */
    void _set(price_t val, size_t pos, size_t num = 0) {
#if CHECK_ACCESS_BOUND
        if ((m_pBuffer[num] == NULL) || pos>= m_pBuffer[num]->size()) {
            throw(std::out_of_range("Try to access value out of bounds! "
                        + name() + " [IndicatorImp::_set]"));
        }
        (*m_pBuffer[num])[pos] = val;
#else
        (*m_pBuffer[num])[pos] = val;
#endif
    }

    /**
     * 准备内存
     * @param len 长度，如果长度大于MAX_RESULT_NUM将抛出异常std::invalid_argument
     * @param result_num 结果集数量
     * @return true 成功 | false 失败
     */
    void _readyBuffer(size_t len, size_t result_num);

    string name() const { return m_name; }
    void name(const string& name) { m_name = name; }

    /** 返回形如：Name(param1=val,param2=val,...) */
    string long_name() const;

    void calculate(const Indicator& data);

    // ===================
    //  子类接口
    // ===================
    virtual bool check() { return false;}

    virtual void _calculate(const Indicator& data) {}

    typedef shared_ptr<IndicatorImp> IndicatorImpPtr;
    virtual IndicatorImpPtr operator()(const Indicator& ind);

protected:
    string m_name;
    size_t m_discard;
    size_t m_result_num;
    PriceList *m_pBuffer[MAX_RESULT_NUM];

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        string name_str(GBToUTF8(m_name));
        ar & bs::make_nvp<string>("m_name", name_str);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        int act_result_num = 0;
        size_t i = 0;
        while (i < m_result_num) {
            if (m_pBuffer[i++])
                act_result_num++;
        }
        ar & BOOST_SERIALIZATION_NVP(act_result_num);

        for (size_t i = 0; i < act_result_num; ++i) {
            std::stringstream buf;
            buf << "result_" << i;
            ar & bs::make_nvp<PriceList>(buf.str().c_str(), *m_pBuffer[i]);
        }
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar & BOOST_SERIALIZATION_NVP(m_name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        int act_result_num = 0;
        ar & BOOST_SERIALIZATION_NVP(act_result_num);
        for (size_t i = 0; i < act_result_num; ++i) {
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

#define INDICATOR_IMP(classname) public: \
    virtual bool check(); \
    virtual void _calculate(const Indicator& data); \
    virtual IndicatorImpPtr operator()(const Indicator& ind) { \
        IndicatorImpPtr p = make_shared<classname>(); \
        p->setParameter(m_params); \
        p->calculate(ind); \
        return p; \
    }


typedef shared_ptr<IndicatorImp> IndicatorImpPtr;

HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImp&);
HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImpPtr&);

} /* namespace hku */
#endif /* INDICATORIMP_H_ */
