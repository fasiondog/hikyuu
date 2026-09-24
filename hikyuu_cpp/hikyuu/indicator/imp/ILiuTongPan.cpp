/*
 * ILiuTongPang.cpp
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#include "ILiuTongPan.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILiuTongPan)
#endif

namespace hku {

ILiuTongPan::ILiuTongPan() : IndicatorImp("LIUTONGPAN", 1) {
    m_need_context = true;
}

ILiuTongPan::~ILiuTongPan() {}

void ILiuTongPan::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    // Set the discard to everything first, it is updated later
    m_discard = total;

    Stock stock = k.getStock();
    auto* kdata = k.data();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();

    StockWeightList sw_list = stock.getWeight(Datetime::min(), lastdate + Days(1));
    HKU_IF_RETURN(sw_list.empty(), void());

    // Find the first ex-rights/ex-dividend record whose outstanding shares are not 0
    price_t pre_free_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->freeCount() > 0) {
            pre_free_count = sw_iter->freeCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }

    // Return directly when there is no ex-rights/ex-dividend data with outstanding shares, or when
    // the date of that record is later than the last K-line date
    HKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, void());

    auto* dst = this->data();
    size_t pos = 0;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (free_count <= 0.0) {
            continue;  // Ignore the ex-rights/ex-dividend record whose outstanding shares are 0
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            if (kdata[pos].datetime >= pre_sw_date) {
                dst[pos] = pre_free_count;
            }
            pos++;
        }

        pre_free_count = free_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        dst[pos] = pre_free_count;
    }

    // Update the discard
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API LIUTONGPAN() {
    return make_shared<ILiuTongPan>()->calculate();
}

Indicator HKU_API LIUTONGPAN(const KData& k) {
    auto p = make_shared<ILiuTongPan>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
