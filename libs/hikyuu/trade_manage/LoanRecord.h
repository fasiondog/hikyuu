/*
 * LoanRecord.h
 *
 *  Created on: 2013-5-24
 *      Author: fasiondog
 */

#ifndef LOANRECORD_H_
#define LOANRECORD_H_

#include "../DataType.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#endif

namespace hku {

/**
 * 借款记录（融资记录）
 * @ingroup TradeManagerClass
 */
class HKU_API LoanRecord {
public:
    LoanRecord(): datetime(Null<Datetime>()), value(0.0) {}
    LoanRecord(const Datetime& datetime, price_t value):
        datetime(datetime), value(value) {}

    Datetime datetime;
    price_t value;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        hku::hku_uint64 date_number = datetime.number();
        ar & bs::make_nvp("datetime", date_number);
        ar & BOOST_SERIALIZATION_NVP(value);
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        namespace bs = boost::serialization;
        hku::hku_uint64 date_number;
        ar & bs::make_nvp("datetime", date_number);
        datetime = Datetime(date_number);
        ar & BOOST_SERIALIZATION_NVP(value);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

typedef vector<LoanRecord> LoanRecordList;

HKU_API std::ostream & operator<<(std::ostream &, const LoanRecord&);

} /* namespace hku */
#endif /* LOANRECORD_H_ */
