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
 * 交易请求记录
 * @ingroup System
 */
class HKU_API TradeRequest {
public:
    TradeRequest();
    void clear();

    bool valid;
    BUSINESS business;
    Datetime datetime;
    price_t stoploss;
    price_t goal;
    double number;  //计划的买入/卖出数量，使用发出请求时刻的收盘价，
                    //用于避免实际买入时需用重新计算数量时，人工执行速度较慢
                    //可通过系统参数进行设置，是否使用
    SystemPart from;  //记录SystemBase::Part
    int count;        //因操作失败，连续延迟的次数
    KRecord krecord;

//============================================
// 序列化支持
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
        ar& BOOST_SERIALIZATION_NVP(count);
        ar& BOOST_SERIALIZATION_NVP(krecord);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */
#endif /* TRADEREQUEST_H_ */
