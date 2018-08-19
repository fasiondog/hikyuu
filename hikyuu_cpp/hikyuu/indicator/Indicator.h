/*
 * Indicator.h
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#ifndef INDICATOR_H_
#define INDICATOR_H_

#include "IndicatorImp.h"

namespace hku {

#define IND_EQ_THRESHOLD 0.000001 ///<判断浮点数相等的阈值,两者差值小于此数

/**
 * 指标类，具体由IndicatorImp实现，实现新指标时应继承IndicatorImp
 * @details 实现新指标时，应继承IndicatorImp类，并定义相关的指标生成函数，如:
 * <pre>
 * class Ma: public IndicatorImp {
 * public:
 *     MA(size_t);
 *     MA(const Indicator&, size_t);
 *
 *     virtual string name() { return "MA"; }
 *     virtual string toString() { retun "Indicator(MA)"; }
 * };
 *
 * Indicator HKU_API MA(size_t n = 30) {
 *     return Indicator(IndicatorImpPtr(new MA(n)));
 * }
 *
 * Indicator HKU_API MA(const Indicator& indicator, size_t n = 30);
 * </pre>
 * @ingroup Indicator
 */
class HKU_API Indicator {
    HKU_API friend std::ostream & operator<<(std::ostream &, const Indicator&);

public:
    Indicator() {}
    Indicator(const IndicatorImpPtr& imp);
    Indicator(const Indicator&);
    virtual ~Indicator();

    Indicator& operator=(const Indicator&);

    /** 使用已有参数计算新值，返回全新的Indicator */
    Indicator operator()(const Indicator& ind);

    /** 指标名称 */
    string name() const;
    void name(const string& name);

    /** 返回形如：Name(param1_val,param2_val,...) */
    string long_name() const;

    /** 结果中需抛弃的个数 */
    size_t discard() const;

    /** 设置抛弃的个数，如果小于原有的discard则无效 */
    void setDiscard(size_t discard);

    /** 返回有几个结果集输出 */
    size_t getResultNumber() const;

    /** 判断是否为空 **/
    bool empty() const;

    /** 获取大小 **/
    size_t size() const;

    /**
     * 只获取第一个结果集中相应位置输出，等同于get(pos, 0)
     * @note 不做下标越界检查，也不抛出异常
     */
    price_t operator[](size_t pos) const {
        return m_imp->get(pos, 0);
    }

    /**
     * 获取第num个结果集中指定位置的数据
     * @param pos 结果集中的位置
     * @param num 第几个结果集
     * @note 不做下标越界检查，不会抛出异常
     */
    price_t get(size_t pos, size_t num = 0) const {
        return m_imp->get(pos, num);
    }

    Indicator getResult(size_t num) const {
        return m_imp->getResult(num);
    }

    PriceList getResultAsPriceList(size_t num) const {
        return m_imp->getResultAsPriceList(num);
    }

    template <typename ValueType>
    void setParam(const string& name, const ValueType& value) {
        if (m_imp) {
            m_imp->setParam<ValueType>(name, value);
        }
    }

    template <typename ValueType>
    ValueType getParam(const string& name) const {
        if (!m_imp) {
            throw std::out_of_range("out_of_range in Parameter::get : " + name);
        }
        return m_imp->getParam<ValueType>(name);
    }

    IndicatorImpPtr getImp() const { return m_imp; }

protected:
    IndicatorImpPtr m_imp;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(m_imp);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};


/**
 * Indicator实例相加，两者的size必须相等，否在返回空
 * @return 1) 相加的两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 * @ingroup Indicator
 */
HKU_API Indicator operator+(const Indicator&, const Indicator&);
HKU_API Indicator operator+(const Indicator&, price_t);
HKU_API Indicator operator+(price_t, const Indicator&);

/**
 * Indicator实例相减，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 * @ingroup Indicator
 */
HKU_API Indicator operator-(const Indicator&, const Indicator&);
HKU_API Indicator operator-(const Indicator&, price_t);
HKU_API Indicator operator-(price_t, const Indicator&);

/**
 * Indicator实例相乘，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 * @ingroup Indicator
 */
HKU_API Indicator operator*(const Indicator&, const Indicator&);
HKU_API Indicator operator*(const Indicator&, price_t);
HKU_API Indicator operator*(price_t, const Indicator&);

/**
 * Indicator实例相除，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如被除中存在为零的数据，则对应位置结果为Null<price_t>()
 * @ingroup Indicator
 */
HKU_API Indicator operator/(const Indicator&, const Indicator&);
HKU_API Indicator operator/(const Indicator&, price_t);
HKU_API Indicator operator/(price_t, const Indicator&);

/**
 * Indicator实例相等，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果相同位置的值相等为1.0, 否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator==(const Indicator&, const Indicator&);
HKU_API Indicator operator==(const Indicator&, price_t);
HKU_API Indicator operator==(price_t, const Indicator&);

/**
 * Indicator实例不相等，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果相同位置的值不相等为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator!=(const Indicator&, const Indicator&);
HKU_API Indicator operator!=(const Indicator&, price_t);
HKU_API Indicator operator!=(price_t, const Indicator&);

/**
 * Indicator实例大于操作，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值大于ind2相同位置的值则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator>(const Indicator&, const Indicator&);
HKU_API Indicator operator>(const Indicator&, price_t);
HKU_API Indicator operator>(price_t, const Indicator&);

/**
 * Indicator实例小于操作，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值小于ind2相同位置的值则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator<(const Indicator&, const Indicator&);
HKU_API Indicator operator<(const Indicator&, price_t);
HKU_API Indicator operator<(price_t, const Indicator&);

/**
 * Indicator实例大于操作，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值大于等于ind2相同位置的值则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator>=(const Indicator&, const Indicator&);
HKU_API Indicator operator>=(const Indicator&, price_t);
HKU_API Indicator operator>=(price_t, const Indicator&);

/**
 * Indicator实例小于操作，两者的size必须相等，否在返回空
 * @return 1) 两个实例的size必须相等，否在返回空实例
 *         2）如果两个实例的resultNumber不等，则取最小的resultNumber
 *         3）如果ind1相同位置的值小于等于ind2相同位置的值则为1.0，否则为0.0
 * @ingroup Indicator
 */
HKU_API Indicator operator<=(const Indicator&, const Indicator&);
HKU_API Indicator operator<=(const Indicator&, price_t);
HKU_API Indicator operator<=(price_t, const Indicator&);

} /* namespace hku */
#endif /* INDICATOR_H_ */
