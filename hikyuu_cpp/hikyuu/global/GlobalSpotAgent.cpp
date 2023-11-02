/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-21
 *     Author: fasiondog
 */

#include "../GlobalInitializer.h"
#include "GlobalSpotAgent.h"
#include "../StockManager.h"

namespace hku {

SpotAgent* g_spot_agent = nullptr;

SpotAgent* getGlobalSpotAgent() {
    if (!g_spot_agent) {
        g_spot_agent = new SpotAgent;
    }
    return g_spot_agent;
}

void releaseGlobalSpotAgent() {
    if (g_spot_agent) {
        HKU_TRACE("relase spot agent");
        delete g_spot_agent;
        g_spot_agent = nullptr;
    }
}

static string getSpotMarketCode(const SpotRecord& spot) {
    std::stringstream market_code_buf;
    market_code_buf << spot.market << spot.code;
    return market_code_buf.str();
}

static void updateStockDayData(const SpotRecord& spot) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());
    HKU_IF_RETURN(!stk.isTransactionTime(spot.datetime), void());
    KRecord krecord(Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day()),
                    spot.open, spot.high, spot.low, spot.close, spot.amount, spot.volume);
    stk.realtimeUpdate(krecord, KQuery::DAY);
}

static void updateStockDayUpData(const SpotRecord& spot, KQuery::KType ktype) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());
    HKU_IF_RETURN(!stk.isTransactionTime(spot.datetime), void());

    std::function<Datetime(Datetime*)> endOfPhase;
    std::function<Datetime(Datetime*)> startOfPhase;

    if (KQuery::WEEK == ktype) {
        endOfPhase = &Datetime::endOfWeek;
        startOfPhase = &Datetime::startOfWeek;
    } else if (KQuery::MONTH == ktype) {
        endOfPhase = &Datetime::endOfMonth;
        startOfPhase = &Datetime::startOfMonth;
    } else if (KQuery::QUARTER == ktype) {
        endOfPhase = &Datetime::endOfQuarter;
        startOfPhase = &Datetime::startOfQuarter;
    } else if (KQuery::HALFYEAR == ktype) {
        endOfPhase = &Datetime::endOfHalfyear;
        startOfPhase = &Datetime::startOfHalfyear;
    } else if (KQuery::YEAR == ktype) {
        endOfPhase = &Datetime::endOfYear;
        startOfPhase = &Datetime::startOfYear;
    } else {
        HKU_THROW("Invalid ktype: {}", ktype);
    }

    Datetime spot_day = Datetime(spot.datetime.year(), spot.datetime.month(), spot.datetime.day());
    Datetime spot_end_of_phase = endOfPhase(&spot_day);
    if (KQuery::WEEK == ktype) {
        spot_end_of_phase = spot_end_of_phase - TimeDelta(2);  // 周五日期
    }
    size_t total = stk.getCount(ktype);

    // 没有历史数据，则直接更新并返回
    if (total == 0) {
        stk.realtimeUpdate(KRecord(spot_end_of_phase, spot.open, spot.high, spot.low, spot.close,
                                   spot.amount, spot.volume),
                           ktype);
        return;
    }

    KRecord last_record = stk.getKRecord(total - 1, ktype);
    if (spot_end_of_phase > last_record.datetime) {
        // 如果当前的日期大于最后记录的日期，则为新增数据，直接更新并返回
        stk.realtimeUpdate(KRecord(spot_end_of_phase, spot.open, spot.high, spot.low, spot.close,
                                   spot.amount, spot.volume),
                           ktype);

    } else if (spot_end_of_phase == last_record.datetime) {
        // 如果当前日期等于最后记录的日期，则需要重新计算最高价、最低价、交易金额、交易量
        Datetime spot_start_of_phase = startOfPhase(&spot_day);
        KRecordList klist =
          stk.getKRecordList(KQuery(spot_start_of_phase, spot_end_of_phase + TimeDelta(1), ktype));
        price_t amount = 0.0, volume = 0.0;
        for (auto& k : klist) {
            amount += k.transAmount;
            volume += k.transCount;
        }
        stk.realtimeUpdate(
          KRecord(spot_end_of_phase, last_record.openPrice,
                  spot.high > last_record.highPrice ? spot.high : last_record.highPrice,
                  spot.low < last_record.lowPrice ? spot.low : last_record.lowPrice, spot.close,
                  amount, volume),
          ktype);
    } else {
        // 不应该出现的情况：当前日期小于最后记录的日期
        HKU_WARN(
          "Ignore, What should not happen, the current date is less than the last recorded date!");
    }
}

static void updateStockMinData(const SpotRecord& spot, KQuery::KType ktype) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull(), void());
    HKU_IF_RETURN(!stk.isTransactionTime(spot.datetime), void());

    TimeDelta gap;
    if (KQuery::MIN == ktype) {
        gap = TimeDelta(0, 0, 1);
    } else if (KQuery::MIN5 == ktype) {
        gap = TimeDelta(0, 0, 5);
    } else if (KQuery::MIN15 == ktype) {
        gap = TimeDelta(0, 0, 15);
    } else if (KQuery::MIN30 == ktype) {
        gap = TimeDelta(0, 0, 30);
    } else if (KQuery::MIN60 == ktype) {
        gap = TimeDelta(0, 0, 60);
    } else if (KQuery::MIN3 == ktype) {
        gap = TimeDelta(0, 0, 3);
    } else if (KQuery::HOUR2 == ktype) {
        gap = TimeDelta(0, 2);
    } else if (KQuery::HOUR4 == ktype) {
        gap = TimeDelta(0, 4);
    } else if (KQuery::HOUR6 == ktype) {
        gap = TimeDelta(0, 6);
    } else if (KQuery::HOUR12 == ktype) {
        gap = TimeDelta(0, 12);
    } else {
        HKU_THROW("Invalid ktype: {}", ktype);
    }

    Datetime minute = spot.datetime;
    minute = minute - (minute - minute.startOfDay()) % gap;
    KRecordList klist = stk.getKRecordList(KQuery(minute, minute + gap, ktype));
    price_t sum_amount = 0.0, sum_volume = 0.0;
    for (auto& k : klist) {
        sum_amount += k.transAmount;
        sum_volume += k.transCount;
    }

    price_t amount = spot.amount > sum_amount ? spot.amount - sum_amount : spot.amount;
    price_t volume = spot.volume > sum_volume ? spot.volume - sum_volume : spot.volume;
    KRecord krecord(minute, spot.open, spot.high, spot.low, spot.close, amount, volume);
    stk.realtimeUpdate(krecord, ktype);
}

void HKU_API startSpotAgent(bool print) {
    auto& agent = *getGlobalSpotAgent();
    HKU_CHECK(!agent.isRunning(), "The agent is running, please stop first!");

    agent.setPrintFlag(print);

    const auto& preloadParam = StockManager::instance().getPreloadParameter();
    if (preloadParam.tryGet<bool>("min", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN));
    }

    if (preloadParam.tryGet<bool>("day", false)) {
        agent.addProcess(updateStockDayData);
    }

    if (preloadParam.tryGet<bool>("week", false)) {
        agent.addProcess(std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::WEEK));
    }

    if (preloadParam.tryGet<bool>("month", false)) {
        agent.addProcess(std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::MONTH));
    }

    if (preloadParam.tryGet<bool>("quarter", false)) {
        agent.addProcess(std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::QUARTER));
    }

    if (preloadParam.tryGet<bool>("halfyear", false)) {
        agent.addProcess(std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::HALFYEAR));
    }

    if (preloadParam.tryGet<bool>("year", false)) {
        agent.addProcess(std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::YEAR));
    }

    if (preloadParam.tryGet<bool>("min5", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN5));
    }

    if (preloadParam.tryGet<bool>("min15", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN15));
    }

    if (preloadParam.tryGet<bool>("min30", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN30));
    }

    if (preloadParam.tryGet<bool>("min60", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN60));
    }
    if (preloadParam.tryGet<bool>("min3", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::MIN3));
    }

    if (preloadParam.tryGet<bool>("hour2", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::HOUR2));
    }

    if (preloadParam.tryGet<bool>("hour4", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::HOUR4));
    }

    if (preloadParam.tryGet<bool>("hour6", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::HOUR6));
    }

    if (preloadParam.tryGet<bool>("hour12", false)) {
        agent.addProcess(std::bind(updateStockMinData, std::placeholders::_1, KQuery::HOUR12));
    }

    agent.start();
}

void HKU_API stopSpotAgent() {
    auto& agent = *getGlobalSpotAgent();
    agent.stop();
}

}  // namespace hku