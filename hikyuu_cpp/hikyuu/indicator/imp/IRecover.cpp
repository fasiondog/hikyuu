/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240317 added by fasiondog
 */

#include "IKData.h"
#include "IRecover.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRecover)
#endif

namespace hku {

IRecover::IRecover() : IndicatorImp("RECOVER") {
    setParam<int>("recover_type", KQuery::NO_RECOVER);
}

IRecover::IRecover(int recoverType) {
    setParam<int>("recover_type", recoverType);
}

IRecover::IRecover(const KData& kdata, int recoverType) : IndicatorImp("RECOVER") {
    setParam<int>("recover_type", recoverType);
    setParam<KData>("kdata", kdata);
}

IRecover::~IRecover() {}

void IRecover::_checkParam(const string& name) const {
    if ("recover_type" == name) {
        int recover_type = getParam<int>("recover_type");
        HKU_ASSERT(recover_type >= KQuery::NO_RECOVER &&
                   recover_type < KQuery::INVALID_RECOVER_TYPE);
    }
}

void IRecover::checkInputIndicator(const Indicator& ind) {
    HKU_CHECK(dynamic_cast<IKData*>(ind.getImp().get()) != nullptr,
              "Only the following indicators are accepted: OPEN|HIGH|CLOSE|LOW");
    string part = ind.getParam<string>("kpart");
    HKU_CHECK(part == "CLOSE" || part == "OPEN" || part == "HIGH" || part == "LOW",
              "Only the following indicators are accepted: OPEN|HIGH|CLOSE|LOW");
}

void IRecover::_calculate(const Indicator& ind) {
    auto kdata = ind.getContext();
    auto query = kdata.getQuery();

    KQuery::RecoverType recover_type =
      static_cast<KQuery::RecoverType>(getParam<int>("recover_type"));
    m_name = fmt::format("RECOVER_{}", KQuery::getRecoverTypeName(recover_type));

    query.recoverType(recover_type);
    KData new_k = kdata.getStock().getKData(query);
    HKU_ASSERT(new_k.size() == ind.size());

    size_t total = new_k.size();
    _readyBuffer(total, 1);

    string part_name = ind.getParam<string>("kpart");
    const auto* data = new_k.data();
    auto* dst = this->data();
    if ("CLOSE" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].closePrice;
        }

    } else if ("OPEN" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].openPrice;
        }

    } else if ("HIGH" == part_name) {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].highPrice;
        }

    } else {
        for (size_t i = 0; i < total; i++) {
            dst[i] = data[i].lowPrice;
        }
    }
}

Indicator HKU_API RECOVER_FORWARD() {
    return Indicator(make_shared<IRecover>(KQuery::FORWARD));
}

Indicator HKU_API RECOVER_BACKWARD() {
    return Indicator(make_shared<IRecover>(KQuery::BACKWARD));
}

Indicator HKU_API RECOVER_EQUAL_FORWARD() {
    return Indicator(make_shared<IRecover>(KQuery::EQUAL_FORWARD));
}

Indicator HKU_API RECOVER_EQUAL_BACKWARD() {
    return Indicator(make_shared<IRecover>(KQuery::EQUAL_BACKWARD));
}

Indicator HKU_API RECOVER_FORWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_FORWARD()(ind);
}

Indicator HKU_API RECOVER_BACKWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_BACKWARD()(ind);
}

Indicator HKU_API RECOVER_EQUAL_FORWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_EQUAL_FORWARD()(ind);
}

Indicator HKU_API RECOVER_EQUAL_BACKWARD(const Indicator& ind) {
    IRecover::checkInputIndicator(ind);
    return RECOVER_EQUAL_BACKWARD()(ind);
}

}  // namespace hku