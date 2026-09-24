/*
 * TradeRequest.h
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADEREQUEST_H_
#define TRADEREQUEST_H_

#include "../../trade_manage/TradeRecord.h"
#include "../../serialization/KRecord_serialization.h"

namespace hku {

/**
 * Trade request record
 * @ingroup System
 */
class HKU_API TradeRequest {
public:
    TradeRequest() = default;
    void clear() noexcept;

    bool valid{false};
    BUSINESS business{BUSINESS_INVALID};
    Datetime datetime;
    price_t stoploss{0.0};
    price_t goal{0.0};
    double number{0.0};             // The planned buy / sell quantity, using the close price of the
                                    // moment when the request is issued; it avoids the slow manual
                                    // It can be set through the system parameters whether to use it
    SystemPart from{PART_INVALID};  // Records SystemBase::Part
    string remark;
    int count{0};  // The number of the consecutive delays due to the failed operation
    KRecord krecord;

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(valid);
        string business_name(getBusinessName(business));
        ar& bs::make_nvp<string>("business", business_name);
        uint64_t datetime_num = datetime.number();
        ar& bs::make_nvp("datetime", datetime_num);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goal);
        ar& BOOST_SERIALIZATION_NVP(number);
        string from_name(getSystemPartName(from));
        ar& bs::make_nvp<string>("from", from_name);
        ar& BOOST_SERIALIZATION_NVP(remark);
        ar& BOOST_SERIALIZATION_NVP(count);
        ar& BOOST_SERIALIZATION_NVP(krecord);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(valid);
        string business_name;
        ar& bs::make_nvp<string>("business", business_name);
        business = getBusinessEnum(business_name);
        uint64_t datetime_num;
        ar& bs::make_nvp("datetime", datetime_num);
        datetime = Datetime(datetime_num);
        ar& BOOST_SERIALIZATION_NVP(stoploss);
        ar& BOOST_SERIALIZATION_NVP(goal);
        ar& BOOST_SERIALIZATION_NVP(number);
        string from_name;
        ar& bs::make_nvp<string>("from", from_name);
        from = getSystemPartEnum(from_name);
        ar& BOOST_SERIALIZATION_NVP(remark);
        ar& BOOST_SERIALIZATION_NVP(count);
        ar& BOOST_SERIALIZATION_NVP(krecord);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

HKU_API std::ostream& operator<<(std::ostream& os, const TradeRequest& tr);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TradeRequest> : ostream_formatter {};
#endif

#endif /* TRADEREQUEST_H_ */
