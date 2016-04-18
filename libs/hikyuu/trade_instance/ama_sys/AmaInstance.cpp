/*
 * AmaInstance.cpp
 *
 *  Created on: 2015年3月12日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/signal/build_in.h>
#include "../../indicator/crt/PRICELIST.h"
#include "AmaInstance.h"

namespace hku {

Indicator HKU_API AmaSpecial(const Block& block, KQuery query,
        Indicator ama) {
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

    vector<size_t> position(dayTotal);
    size_t discard = ama.discard();
    for (auto stk_iter = block.begin(); stk_iter != block.end(); ++stk_iter) {
        KData kdata = stk_iter->getKData(query);
        if (kdata.empty())
            continue;
        SignalPtr sg(SG_Single(ama));
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

            if (dateList[i] >= kdata[0].datetime) {
                if (n_dis < discard) {
                    n_dis++;
                    numberPerDay[i]--;
                }
            }

        }
    }

    PriceList tmp_result(dayTotal, Null<price_t>());
    for (auto i = discard; i < dayTotal; ++i) {
        tmp_result[i] = numberPerDay[i]
                  ? (double)position[i]/(double)numberPerDay[i] : 1.0;
    }

    result = PRICELIST(tmp_result);
    result.name("POS");
    return PRICELIST(result);
}

} /* namespace hku */


