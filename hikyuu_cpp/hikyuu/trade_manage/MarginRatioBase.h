/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-08-30
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"
#include "hikyuu/utilities/util.h"
#include "hikyuu/utilities/Parameter.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif

namespace hku {

/**
 * @brief 保证金比例算法基类
 * @ingroup TradeMargin
 */
class HKU_API MarginRatioBase {
    PARAMETER_SUPPORT

public:
    MarginRatioBase() = default;
    MarginRatioBase(const string& name) : m_name(name) {}
    virtual ~MarginRatioBase() = default;

    typedef shared_ptr<MarginRatioBase> MarginRatioPtr;

    /** 克隆操作 */
    MarginRatioPtr clone();

    /** 获取名称 */
    const string& name() const {
        return m_name;
    }

    /** 继承子类必须实现私有变量的克隆接口 */
    virtual MarginRatioPtr _clone() = 0;

private:
    string m_name;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MarginRatioBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class DrivedMargin: public MarginRatioBase {
 *     MARGIN_RATIO_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     DrivedMargin();
 *     ...
 * };
 * @endcode
 * @ingroup TradeMargin
 */
#define MARGIN_RATIO_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MarginRatioBase); \
    }
#else
#define MARGIN_RATIO_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * 交易保证金比率算法指针
 * @ingroup TradeMargin
 */
typedef shared_ptr<MarginRatioBase> MarginRatioPtr;

HKU_API std::ostream& operator<<(std::ostream&, const MarginRatioBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MarginRatioPtr&);

}  // namespace hku
