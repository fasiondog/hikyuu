/*
 * BorrowRecord.h
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#ifndef BORROWRECORD_H_
#define BORROWRECORD_H_

#include "../DataType.h"
#include "../serialization/Stock_serialization.h"
#include "../serialization/Datetime_serialization.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/list.hpp>
#endif

namespace hku {

/**
 * 记录当前借入的股票信息
 * @ingroup TradeManagerClass
 */
class HKU_API BorrowRecord {
public:
    BorrowRecord();
    BorrowRecord(const Stock&, size_t number, price_t value);

    Stock stock;
    size_t number;  //借入总数量
    price_t value;  //借入总价值

    struct Data {
        Data(): datetime(Null<Datetime>()), price(0.0), number(0) {}
        Data(const Datetime& datetime, price_t price, size_t number)
        :datetime(datetime), price(price), number(number) {}

        Datetime datetime; //借入时间
        price_t price;     //借入时的单价
        size_t number;     //借入数量

    #if HKU_SUPPORT_SERIALIZATION
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive & ar, const unsigned int version) const {
            hku_uint64 datetime_num = datetime.number();
            ar & boost::serialization::make_nvp("datetime", datetime_num);
            ar & BOOST_SERIALIZATION_NVP(number);
            ar & BOOST_SERIALIZATION_NVP(price);
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            hku_uint64 datetime_num;
            ar & boost::serialization::make_nvp("datetime", datetime_num);
            datetime = Datetime(datetime_num);
            ar & BOOST_SERIALIZATION_NVP(number);
            ar & BOOST_SERIALIZATION_NVP(price);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    #endif
    };

    list<Data> record_list; //当前的借入记录

//序列化支持
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_NVP(stock);
        ar & BOOST_SERIALIZATION_NVP(number);
        ar & BOOST_SERIALIZATION_NVP(value);
        ar & BOOST_SERIALIZATION_NVP(record_list);
    }
#endif
};

typedef vector<BorrowRecord> BorrowRecordList;

HKU_API std::ostream & operator<<(std::ostream &, const BorrowRecord&);

} /* namespace hku */
#endif /* BORROWRECORD_H_ */
