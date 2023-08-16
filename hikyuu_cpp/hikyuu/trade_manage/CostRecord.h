/*
 * CostRecord.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#pragma once
#ifndef COSTRECORD_H_
#define COSTRECORD_H_

#include "../DataType.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#endif

namespace hku {

/**
 * 成本记录
 * @details 总成本 = 佣金 + 印花税 + 过户费 + 其他费用
 * @note 该结构主要用于存放成本记录结果，一般当做struct直接使用，
 *       该类本身不对总成本进行计算，也不保证上面的公式成立
 * @ingroup TradeCost
 */
class HKU_API CostRecord {
public:
    CostRecord();
    CostRecord(price_t commission, price_t stamptax, price_t transferfee, price_t others,
               price_t total);

    price_t commission;  /**< 佣金     */
    price_t stamptax;    /**< 印花税   */
    price_t transferfee; /**< 过户费   */
    price_t others;      /**< 其他费用 */
    price_t total;       /**< 总成本   */

    string toString() const;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(commission);
        ar& BOOST_SERIALIZATION_NVP(stamptax);
        ar& BOOST_SERIALIZATION_NVP(transferfee);
        ar& BOOST_SERIALIZATION_NVP(others);
        ar& BOOST_SERIALIZATION_NVP(total);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/**
 * 输出成本信息
 * @ingroup TradeCost
 */
HKU_API std::ostream& operator<<(std::ostream& os, const CostRecord&);

bool HKU_API operator==(const CostRecord& d1, const CostRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::CostRecord> : ostream_formatter {};
#endif

#endif /* COSTRECORD_H_ */
