/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "ICycle.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICycle)
#endif

namespace hku {

ICycle::ICycle() : IndicatorImp("CYCLE", 1) {
    _initParams();
}

ICycle::ICycle(const KData& k) : IndicatorImp("CYCLE", 1) {
    _initParams();
    setParam<KData>("kdata", k);
    ICycle::_calculate(Indicator());
}

ICycle::~ICycle() {}

void ICycle::_initParams() {
    setParam<int>("adjust_cycle", 1);              // 调仓周期
    setParam<string>("adjust_mode", "query");      // 调仓模式
    setParam<bool>("delay_to_trading_day", true);  // 延迟至交易日
}

void ICycle::_checkParam(const string& name) const {
    if ("adjust_mode" == name || "adjust_cycle" == name) {
        if (!haveParam("adjust_mode") || !haveParam("adjust_cycle")) {
            // 同时判断两个参数时，可能一个参数还未设定
            return;
        }
        string adjust_mode = getParam<string>("adjust_mode");
        to_lower(adjust_mode);
        int adjust_cycle = getParam<int>("adjust_cycle");
        if ("query" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("day" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1);
        } else if ("week" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 5);
        } else if ("month" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 31);
        } else if ("quarter" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 92);
        } else if ("year" == adjust_mode) {
            HKU_ASSERT(adjust_cycle >= 1 && adjust_cycle <= 366);
        } else {
            HKU_THROW("Invalid adjust_mode: {}!", adjust_mode);
        }
    }
}

static void calculate_no_delay(const DatetimeList& datelist, int adjust_cycle, const string& mode,
                               PriceList& buf) {
    if ("week" == mode) {
        Datetime cur_cycle_end = datelist.front().nextWeek();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.dayOfWeek() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextWeek();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            buf[i] = adjust;
        }
    } else if ("month" == mode) {
        Datetime cur_cycle_end = datelist.front().nextMonth();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.day() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextMonth();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            buf[i] = adjust;
        }
    } else if ("quarter" == mode) {
        Datetime cur_cycle_end = datelist.front().nextQuarter();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.day() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextQuarter();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            buf[i] = adjust;
        }
    } else if ("year" == mode) {
        Datetime cur_cycle_end = datelist.front().nextYear();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            bool adjust = (date.dayOfYear() == adjust_cycle);
            if (adjust) {
                cur_cycle_end = date.nextYear();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }
            buf[i] = adjust;
        }
    }
}

static void calculate_delay(const DatetimeList& datelist, int adjust_cycle, const string& mode,
                            PriceList& buf) {
    std::set<Datetime> adjust_date_set;
    if ("week" == mode) {
        Datetime cur_cycle_end = datelist.front().nextWeek();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfWeek() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextWeek();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            buf[i] = adjust;
        }

    } else if ("month" == mode) {
        Datetime cur_cycle_end = datelist.front().nextMonth();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfMonth() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextMonth();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            buf[i] = adjust;
        }

    } else if ("quarter" == mode) {
        Datetime cur_cycle_end = datelist.front().nextQuarter();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfQuarter() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextQuarter();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            buf[i] = adjust;
        }

    } else if ("year" == mode) {
        Datetime cur_cycle_end = datelist.front().nextYear();
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            const auto& date = datelist[i];
            Datetime adjust_date = date.startOfYear() + Days(adjust_cycle - 1);
            bool adjust = false;
            if (date == adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            } else if (adjust_date_set.find(adjust_date) == adjust_date_set.end() &&
                       date > adjust_date) {
                adjust = true;
                adjust_date_set.emplace(adjust_date);
            }

            if (adjust) {
                cur_cycle_end = date.nextYear();
            }
            if (cur_cycle_end >= datelist.back()) {
                cur_cycle_end = datelist.back() + Seconds(1);
            }

            buf[i] = adjust;
        }
    }
}

void ICycle::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    DatetimeList datelist = k.getStock().getTradingCalendar(k.getQuery());
    HKU_IF_RETURN(datelist.empty(), void());

    int adjust_cycle = getParam<int>("adjust_cycle");
    string adjust_mode = getParam<string>("adjust_mode");
    bool delay_to_trading_day = getParam<bool>("delay_to_trading_day");

    PriceList buf(datelist.size());
    if ("query" == adjust_mode || "day" == adjust_mode) {
        size_t cur_adjust_ix = 0;
        for (size_t i = 0, total = datelist.size(); i < total; i++) {
            bool adjust = false;
            if (i == cur_adjust_ix) {
                adjust = true;
                cur_adjust_ix += adjust_cycle;
            }
            buf[i] = adjust;
        }

    } else if (delay_to_trading_day) {
        calculate_delay(datelist, adjust_cycle, adjust_mode, buf);
    } else {
        calculate_no_delay(datelist, adjust_cycle, adjust_mode, buf);
    }

    Indicator tmpind = ALIGN(PRICELIST(buf, datelist), k);
    const auto* src = tmpind.data();
    auto* dst = this->data();
    HKU_ASSERT(tmpind.size() == total);
    memcpy(dst, src, sizeof(value_t) * total);
}

Indicator HKU_API CYCLE(int adjust_cycle, const string& adjust_mode, bool delay_to_trading_day) {
    auto p = make_shared<ICycle>();
    p->setParam<int>("adjust_cycle", adjust_cycle);
    p->setParam<string>("adjust_mode", adjust_mode);
    p->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    return Indicator(p);
}

Indicator HKU_API CYCLE(const KData& k, int adjust_cycle, const string& adjust_mode,
                        bool delay_to_trading_day) {
    auto p = make_shared<ICycle>(k);
    p->setParam<int>("adjust_cycle", adjust_cycle);
    p->setParam<string>("adjust_mode", adjust_mode);
    p->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    return p->calculate();
}

} /* namespace hku */
