/*
 * POS.cpp
 *
 *  Created on: 2015年3月17日
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/thread.h"
#include "PRICELIST.h"
#include "POS.h"

namespace hku {

Indicator HKU_API POS(const Block& block, KQuery query, SignalPtr sg) {
    Indicator result;
    StockManager& sm = StockManager::instance();

    // 计算每日股票总数
    DatetimeList dateList = sm.getTradingCalendar(query, "SH");

    size_t dayTotal = dateList.size();
    if (dayTotal == 0) {
        result = PRICELIST(PriceList());
        result.name("POS");
        return result;
    }

    vector<size_t> numberPerDay(dayTotal);
    for (size_t i = 0; i < dayTotal; ++i) {
        numberPerDay[i] = 0;
        for (auto stk_iter = block.begin(); stk_iter != block.end(); ++stk_iter) {
            if (stk_iter->startDatetime() <= dateList[i] &&
                dateList[i] <= stk_iter->lastDatetime()) {
                numberPerDay[i]++;
            }
        }
    }

    ThreadPool tg;
    vector<SGPtr> sgs;
    for (auto stk_iter = block.begin(); stk_iter != block.end(); ++stk_iter) {
        auto tmpsg = sg->clone();
        sgs.push_back(tmpsg);
        KData kdata = stk_iter->getKData(query);
        tg.submit([tmpsg, k = std::move(kdata)]() { tmpsg->setTO(k); });
    }
    tg.join();

    // 计算每日持仓的股票数
    vector<size_t> position(dayTotal);
    for (size_t i = 0, total = block.size(); i < total; i++) {
        const auto& xsg = sgs[i];
        bool isHold = false;
        for (size_t j = 0; j < dayTotal; ++j) {
            if (isHold) {
                if (xsg->shouldSell(dateList[j])) {
                    isHold = false;
                } else {
                    position[j]++;
                }

            } else {
                if (xsg->shouldBuy(dateList[j])) {
                    position[j]++;
                    isHold = true;
                }
            }
        }
    }

    PriceList tmp_result(dayTotal, Null<price_t>());
    for (auto i = 0; i < dayTotal; ++i) {
        tmp_result[i] = numberPerDay[i] ? (double)position[i] / (double)numberPerDay[i] : 1.0;
    }

    result = PRICELIST(tmp_result);
    result.name("POS");
    return result;
}

} /* namespace hku */
