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

namespace hku {

/**
 * Cost record
 * @details Total cost = commission + stamp duty + transfer fee + other fees
 * @note This structure is mainly used to store the cost record result; it is generally used
 * directly as a struct. The class itself does not calculate the total cost, and it does not
 * guarantee that the above formula holds
 * @ingroup TradeCost
 */
class HKU_API CostRecord {
public:
    CostRecord();
    CostRecord(price_t commission, price_t stamptax, price_t transferfee, price_t others,
               price_t total);

    price_t commission;  /**< Commission     */
    price_t stamptax;    /**< Stamp duty     */
    price_t transferfee; /**< Transfer fee   */
    price_t others;      /**< Other fees     */
    price_t total;       /**< Total cost     */

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
 * Output the cost information
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
