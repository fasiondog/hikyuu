/*
 * IPriceList.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "IPriceList.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IPriceList)
#endif

namespace hku {

IPriceList::IPriceList() : IndicatorImp("PRICELIST", 1) {
    setParam<PriceList>("data", PriceList());
    setParam<int>("discard", 0);
}

IPriceList::IPriceList(const PriceList& data, int in_discard) : IndicatorImp("PRICELIST", 1) {
    setParam<PriceList>("data", data);
    setParam<int>("discard", in_discard);
}

IPriceList::~IPriceList() {}

void IPriceList::_checkParam(const string& name) const {
    if ("discard" == name) {
        HKU_ASSERT(getParam<int>("discard") >= 0);
    }
}

void IPriceList::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(), "The PRICELIST indicator is not leaf node");

    DatetimeList align_dates =
      haveParam("align_date_list") ? getParam<DatetimeList>("align_date_list") : DatetimeList();
    PriceList x = getParam<PriceList>("data");
    int x_discard = getParam<int>("discard");
    size_t x_total = x.size();

    auto k = getContext();
    size_t total = x_total;
    if (k.size() > 0) {
        total = k.size();
    }

    _readyBuffer(total, 1);

    if (k != Null<KData>() && align_dates.size() > 0) {
        // 如果本身是时间序列，则使用时间进行对齐
        auto tmp = ALIGN(PRICELIST(x, align_dates, x_discard), k);
        HKU_ASSERT(tmp.size() == total);
        auto* dst = this->data();
        auto* src = x.data();
        for (size_t i = tmp.discard(); i < total; ++i) {
            dst[i] = src[i];
        }
        m_discard = tmp.discard();
        return;
    }

    // 如果指定了上下文，则按上下文数值右对齐，保证和上下文等长
    if (x_discard >= x_total) {
        m_discard = total;
        return;
    }

    size_t x_start = x_discard;
    auto* dst = this->data();
    if (x_total < total) {
        dst = dst + total + x_discard - x_total;
    } else if (x_total > total) {
        x_start = x_total - total;
        dst = dst - x_start;
        if (x_discard > x_start) {
            x_start = x_discard;
            dst = dst + x_discard - x_start;
        }
    }

    for (size_t i = x_start; i < x_total; ++i) {
        dst[i] = x[i];
    }
    m_discard = total + x_start - x_total;
    return;
}

Indicator HKU_API PRICELIST(const PriceList& data, int discard) {
    return make_shared<IPriceList>(data, discard)->calculate();
}

Indicator HKU_API PRICELIST(const PriceList& data, const DatetimeList& ds, int discard) {
    auto ret = PRICELIST(data, discard);
    HKU_CHECK(data.size() == ds.size(),
              "The data length must be the same as the length of the reference date list");
    ret.setParam<DatetimeList>("align_date_list", ds);
    return ret;
}

Indicator HKU_API PRICELIST() {
    auto p = make_shared<IPriceList>();
    return Indicator(p);
}

} /* namespace hku */
