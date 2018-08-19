/*
 * POS.cpp
 *
 *  Created on: 2015年3月17日
 *      Author: fasiondog
 */

#include "PRICELIST.h"
#include "POS.h"

namespace hku {

Indicator HKU_API POS(const Block& block, KQuery query,
        SignalPtr sg) {
    Indicator result;
    StockManager& sm = StockManager::instance();

    //计算每日股票总数
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
            if (stk_iter->startDatetime() <= dateList[i]
                  && dateList[i] <= stk_iter->lastDatetime()) {
                numberPerDay[i]++;
            }
        }
    }

    //计算每日持仓的股票数
    vector<size_t> position(dayTotal);
    for (auto stk_iter = block.begin(); stk_iter != block.end(); ++stk_iter) {
        KData kdata = stk_iter->getKData(query);
        if (kdata.empty())
            continue;
        sg->setTO(kdata);
        bool isHold = false;
        size_t n_dis = 0;
        for (size_t i = 0; i < dayTotal; ++i) {
            if (isHold) {
                if (sg->shouldSell(dateList[i])) {
                    isHold = false;
                } else {
                    position[i]++;
                }

            } else {
                if (sg->shouldBuy(dateList[i])) {
                    position[i]++;
                    isHold = true;
                }
            }
        }
    }

    PriceList tmp_result(dayTotal, Null<price_t>());
    for (auto i = 0; i < dayTotal; ++i) {
        tmp_result[i] = numberPerDay[i]
                  ? (double)position[i]/(double)numberPerDay[i] : 1.0;
    }

    result = PRICELIST(tmp_result);
    result.name("POS");
    return result;
}

} /* namespace hku */


