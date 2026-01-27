/*
 * IInSum.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2024-5-21
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/thread.h"
#include "IInSum.h"
#include "../Indicator.h"
#include "../crt/ALIGN.h"
#include "../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IInSum)
#endif

namespace hku {

IInSum::IInSum() : IndicatorImp("INSUM", 1) {
    setParam<KQuery>("query", KQuery(0, 0));
    setParam<Block>("block", Block());
    setParam<int>("mode", 0);
    setParam<string>("market", "SH");
    setParam<bool>("ignore_context", false);
    setParam<bool>("fill_null", false);
}

IInSum::~IInSum() {}

void IInSum::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    } else if ("mode" == name) {
        int mode = getParam<int>("mode");
        HKU_ASSERT(mode == 0 || mode == 1 || mode == 2 || mode == 3 || mode == 4 || mode == 5);
    }
}

static IndicatorList getAllIndicators(const Block& block, const KQuery& query,
                                      const DatetimeList& dates, const Indicator& ind,
                                      bool fill_null) {
    auto stks = block.getStockList();
    return global_parallel_for_index(
      0, stks.size(), [nind = ind.clone(), fill_null, &stks, &query, &dates](size_t index) {
          auto k = stks[index].getKData(query);
          return ALIGN(nind, dates, fill_null)(k).getResult(0);
      });
}

static void insum_cum(const IndicatorList& inds, Indicator::value_t* dst, size_t len) {
    for (const auto& value : inds) {
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();
        for (size_t i = 0; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (std::isnan(dst[i])) {
                    dst[i] = data[i];
                } else {
                    dst[i] += data[i];
                }
            }
        }
    }
}

static void insum_mean(const IndicatorList& inds, Indicator::value_t* dst, size_t len) {
    vector<size_t> count(len, 0);
    for (const auto& value : inds) {
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();
        for (size_t i = 0; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (std::isnan(dst[i])) {
                    dst[i] = data[i];
                } else {
                    dst[i] += data[i];
                }
                count[i]++;
            }
        }
    }

    for (size_t i = 0; i < len; i++) {
        if (!std::isnan(dst[i])) {
            dst[i] = dst[i] / count[i];
        }
    }
}

static void insum_max(const IndicatorList& inds, Indicator::value_t* dst, size_t len) {
    for (const auto& value : inds) {
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();
        for (size_t i = 0; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (std::isnan(dst[i])) {
                    dst[i] = data[i];
                } else if (data[i] > dst[i]) {
                    dst[i] = data[i];
                }
            }
        }
    }
}

static void insum_min(const IndicatorList& inds, Indicator::value_t* dst, size_t len) {
    for (const auto& value : inds) {
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();
        // 遍历所有数据data,dst为空则等于data[i],data[i]小于原来值则为data[i]
        for (size_t i = 0; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (std::isnan(dst[i])) {
                    dst[i] = data[i];
                } else if (data[i] < dst[i]) {
                    dst[i] = data[i];
                }
            }
        }
    }
}

// 排名按降序，指标值最高的排名为1
static void insum_rank_desc(const IndicatorList& inds, Indicator::value_t* dst,
                            const Indicator& ind, size_t len) {
    size_t discard = ind.discard();
    for (size_t i = discard; i < len; i++) {
        if (std::isnan(dst[i])) {
            dst[i] = 1;  // 相当于初始化
        }
    }
    for (const auto& value : inds) {  // 单个ind
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();    // 对比股的数据
        const auto* data_ind = ind.data();  // 本股数据

        for (size_t i = discard; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (data[i] > data_ind[i]) {
                    dst[i]++;
                }
            }
        }
    }
}

// 排名按升序，指标值最低的排名为1，指标值越高排名值越高
static void insum_rank_asc(const IndicatorList& inds, Indicator::value_t* dst, const Indicator& ind,
                           size_t len) {
    size_t discard = ind.discard();
    for (size_t i = discard; i < len; i++) {
        if (std::isnan(dst[i])) {
            dst[i] = 1;  // 相当于初始化
        }
    }
    for (const auto& value : inds) {  // 单个ind
        if (value.empty()) {
            continue;
        }
        if (value.size() != len) {
            HKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                     value.getContext().getStock().market_code(), value.size(), len);
            continue;
        }
        const auto* data = value.data();    // 对比股的数据
        const auto* data_ind = ind.data();  // 本股数据

        for (size_t i = discard; i < len; i++) {
            if (!std::isnan(data[i])) {
                if (data[i] < data_ind[i]) {  // 如果比dst_tmp值小,则排名+1,如果比dst_tmp值大,则不变
                    dst[i]++;
                }
            }
        }
    }
}

void IInSum::_calculate(const Indicator& ind) {
    const Block block = getParam<const Block&>("block");
    bool ignore_context = getParam<bool>("ignore_context");
    const KData& k = getContext();
    KQuery q;
    DatetimeList dates;
    if (!ignore_context && !k.empty()) {
        q = k.getQuery();
        dates = k.getDatetimeList();
    } else {
        q = getParam<KQuery>("query");
        if (q != KQuery(0, 0)) {
            dates = StockManager::instance().getTradingCalendar(q, getParam<string>("market"));
        }
    }

    size_t total = dates.size();
    m_discard = 0;
    _readyBuffer(total, 1);
    HKU_IF_RETURN(total == 0, void());

    int mode = getParam<int>("mode");
    // 模式4/5依赖上下文
    if (mode == 4 || mode == 5) {
        if (ind.size() == 0) {
            m_discard = total;
            return;
        }
    }

    auto inds = getAllIndicators(block, q, dates, ind, getParam<bool>("fill_null"));
    auto* dst = this->data();

    if (0 == mode) {
        insum_cum(inds, dst, total);
    } else if (1 == mode) {
        insum_mean(inds, dst, total);
    } else if (2 == mode) {
        insum_max(inds, dst, total);
    } else if (3 == mode) {
        insum_min(inds, dst, total);
    } else if (4 == mode) {
        // 指标值越大排名值越低，即指标最大的值对应排名值为1
        auto nind = ind;
        if (ind.size() != total) {
            nind = ALIGN(ind, dates, getParam<bool>("fill_null"));
            HKU_CHECK(nind.size() == total, "ind size: {}  != total: {}", ind.size(), total);
        }
        insum_rank_desc(inds, dst, nind, total);
    } else if (5 == mode) {
        // 指标值越高排名值越高，即指标值最低的排名值为1
        auto nind = ind;
        if (ind.size() != total) {
            nind = ALIGN(ind, dates, getParam<bool>("fill_null"));
            HKU_CHECK(nind.size() == total, "ind size: {}  != total: {}", ind.size(), total);
        }
        insum_rank_asc(inds, dst, nind, total);
    } else {
        HKU_ERROR("Not support mode: {}", mode);
    }

    for (size_t i = m_discard; i < total; i++) {
        if (!std::isnan(dst[i])) {
            break;
        }
        m_discard++;
    }
}

Indicator HKU_API INSUM(const Block& block, int mode, bool fill_null) {
    IndicatorImpPtr p = make_shared<IInSum>();
    p->setParam<Block>("block", block);
    p->setParam<int>("mode", mode);
    p->setParam<bool>("ignore_context", false);
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p);
}

Indicator HKU_API INSUM(const Block& block, const KQuery& query, const Indicator& ind, int mode,
                        bool fill_null) {
    IndicatorImpPtr p = make_shared<IInSum>();
    p->setParam<KQuery>("query", query);
    p->setParam<Block>("block", block);
    p->setParam<int>("mode", mode);
    p->setParam<bool>("ignore_context", false);
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p)(ind);
}

Indicator HKU_API INSUM(const Block& block, const Indicator& ind, int mode, bool fill_null) {
    return INSUM(block, KQuery(0, 0), ind, mode, fill_null);
}

} /* namespace hku */
