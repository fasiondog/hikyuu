/*
 * SystemWeight.h
 *
 *  Created on: 2018年1月29日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_
#define TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_

#include "../system/System.h"

namespace hku {

/**
 * 系统权重，权重有效范围为 [0.0, 1.0]
 * @details 用于指定系统对应权重告知资产分配算法
 * @ingroup AllocateFunds
 */
class HKU_API SystemWeight {
public:
    /** 默认构造函数，缺省权重为1.0 */
    SystemWeight() : m_weight(1.0) {}

    /**
     * @brief Construct a new System Weight object
     *
     * @param sys 对应的系统
     * @param weight 对应的权重，须在 [0.0, 1.0] 之间
     */
    SystemWeight(const SystemPtr& sys, double weight) : m_sys(sys), m_weight(1.0) {
        HKU_CHECK_THROW(weight >= 0.0 && weight <= 1.0, std::out_of_range,
                        "weigth ({}) is out of range [0, 1]!", weight);
        m_weight = weight;
    }

    SystemWeight(const SystemWeight&) = default;
    SystemWeight(SystemWeight&& sw) : m_sys(std::move(sw.m_sys)), m_weight(sw.m_weight) {}

    SystemWeight& operator=(const SystemWeight& other) = default;
    SystemWeight& operator=(SystemWeight&& other);

    /** 析构函数 */
    virtual ~SystemWeight() {}

    /** 修改对应的系统 */
    void setSYS(const SystemPtr& sys) {
        m_sys = sys;
    }

    /** 获取对应的系统 */
    const SystemPtr& getSYS() const {
        return m_sys;
    }

    /** 设置权重值，须在[0,1]之间 */
    void setWeight(double weight) {
        HKU_CHECK_THROW(weight >= 0.0 && weight <= 1.0, std::out_of_range,
                        "weigth ({}) is out of range [0, 1]!", weight);
        m_weight = weight;
    }

    /** 获取权重值 */
    double getWeight() const {
        return m_weight;
    }

public:
    SystemPtr m_sys;
    double m_weight;

private:
//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_sys);
        ar& BOOST_SERIALIZATION_NVP(m_weight);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_sys);
        ar& BOOST_SERIALIZATION_NVP(m_weight);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

typedef vector<SystemWeight> SystemWeightList;

HKU_API std::ostream& operator<<(std::ostream&, const SystemWeight&);

inline bool operator==(const SystemWeight& d1, const SystemWeight& d2) {
    return d1.getSYS() == d2.getSYS() && d1.getWeight() == d2.getWeight();
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SystemWeight> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_ALLOCATEFUNDS_SYSTEMWEIGHT_H_ */
