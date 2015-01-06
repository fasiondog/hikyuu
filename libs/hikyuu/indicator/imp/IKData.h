/*
 * IKData.h
 *
 *  Created on: 2013-2-11
 *      Author: fasiondog
 */

#ifndef IKDATA_H_
#define IKDATA_H_

#include "../Indicator.h"

namespace hku {

class IKData: public IndicatorImp {
public:
    enum ValueType {
        KDATA,
        OPEN,
        HIGH,
        LOW,
        CLOSE,
        AMOUNT,
        COUNT
    };

public:
    IKData() {} //仅用于序列化需要默认构造函数
    IKData(const KData&, ValueType valueType);
    virtual ~IKData();

    virtual string name() const;

private:
    ValueType m_valueType;

//===================================
// 序列化实现
//===================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar & BOOST_SERIALIZATION_NVP(m_valueType);
    }
#endif
};

} /* namespace hku */
#endif /* IKDATA_H_ */
