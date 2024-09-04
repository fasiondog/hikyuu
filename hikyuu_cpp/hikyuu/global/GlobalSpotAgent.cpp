/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-21
 *     Author: fasiondog
 */

#include <hikyuu/GlobalInitializer.h>
#include "../GlobalInitializer.h"
#include "GlobalSpotAgent.h"
#include "../StockManager.h"

namespace hku {

SpotAgent* g_spot_agent = nullptr;

SpotAgent* getGlobalSpotAgent() {
    if (!g_spot_agent) {
        g_spot_agent = new SpotAgent();
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
    HKU_IF_RETURN(stk.isNull() || !stk.isBuffer(KQuery::DAY), void());
    HKU_IF_RETURN(!stk.isTransactionTime(spot.datetime), void());
    KRecord krecord(spot.datetime.startOfDay(), spot.open, spot.high, spot.low, spot.close,
                    spot.amount, spot.volume);
    stk.realtimeUpdate(krecord, KQuery::DAY);
}

static void updateStockDayUpData(const SpotRecord& spot, KQuery::KType ktype) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull() || !stk.isBuffer(ktype), void());
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

    // 重新计算交易金额、交易量
    Datetime spot_start_of_phase = startOfPhase(&spot_day);
    KRecordList klist =
      stk.getKRecordList(KQuery(spot_start_of_phase, spot_end_of_phase + TimeDelta(1), ktype));
    price_t sum_amount = 0.0, sum_volume = 0.0;
    for (const auto& k : klist) {
        sum_amount += k.transAmount;
        sum_volume += k.transCount;
    }

    price_t amount =
      spot.amount > sum_amount ? spot.amount - sum_amount : (sum_amount == 0.0 ? spot.amount : 0.0);
    price_t spot_volume = spot.volume;
    price_t volume =
      spot_volume > sum_volume ? spot_volume - sum_volume : (sum_volume == 0.0 ? spot_volume : 0.0);
    KRecord krecord(spot_end_of_phase, spot.open, spot.high, spot.low, spot.close, amount, volume);
    stk.realtimeUpdate(krecord, ktype);
}

static void updateStockMinData(const SpotRecord& spot, KQuery::KType ktype) {
    Stock stk = StockManager::instance().getStock(getSpotMarketCode(spot));
    HKU_IF_RETURN(stk.isNull() || !stk.isBuffer(ktype), void());
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
    Datetime today = minute.startOfDay();
    // 非24小时交易品种，且时间和当天零时相同认为无分钟线级别数据
    HKU_IF_RETURN(stk.type() != STOCKTYPE_CRYPTO && minute == today, void());

    Datetime end_minute = minute - (minute - today) % gap + gap;

    // 处理闭市时最后一条记录
    MarketInfo market_info = StockManager::instance().getMarketInfo(stk.market());
    Datetime close1 = today + market_info.closeTime1();
    Datetime close2 = today + market_info.closeTime2();
    Datetime open2 = today + market_info.openTime2();
    if (!close2.isNull() && end_minute > close2) {
        end_minute = close2;
    } else if (!open2.isNull() && !close1.isNull() && end_minute < open2 && end_minute > close1) {
        end_minute = close1;
    }

    // 计算当前之前的累积成交金额、成交量
    KRecordList klist = stk.getKRecordList(KQuery(today, end_minute, ktype));
    price_t sum_amount = 0.0, sum_volume = 0.0;
    for (const auto& k : klist) {
        sum_amount += k.transAmount;
        sum_volume += k.transCount;
    }

    price_t amount =
      spot.amount > sum_amount ? spot.amount - sum_amount : (sum_amount == 0.0 ? spot.amount : 0.0);
    price_t spot_volume = spot.volume * 100.;  // spot 传过来的是手数
    price_t volume =
      spot_volume > sum_volume ? spot_volume - sum_volume : (sum_volume == 0.0 ? spot_volume : 0.0);
    KRecord krecord(end_minute, spot.open, spot.high, spot.low, spot.close, amount, volume);
    stk.realtimeUpdate(krecord, ktype);
}

void HKU_API startSpotAgent(bool print, size_t worker_num, const string& addr) {
    StockManager& sm = StockManager::instance();
    auto& agent = *getGlobalSpotAgent();
    HKU_CHECK(!agent.isRunning(), "The agent is running, please stop first!");

    if (addr.empty()) {
        SpotAgent::setQuotationServer(
          sm.getHikyuuParameter().tryGet<string>("quotation_server", "ipc:///tmp/hikyuu_real.ipc"));
    } else {
        SpotAgent::setQuotationServer(addr);
    }

    agent.setServerAddr(addr);
    agent.setWorkerNum(worker_num);
    agent.setPrintFlag(print);

    // 防止调用 stopSpotAgent 后重新 startSpotAgent
    static std::atomic_bool g_init_spot_agent{false};
    if (!g_init_spot_agent) {
        const auto& preloadParam = sm.getPreloadParameter();
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
            agent.addProcess(
              std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::QUARTER));
        }

        if (preloadParam.tryGet<bool>("halfyear", false)) {
            agent.addProcess(
              std::bind(updateStockDayUpData, std::placeholders::_1, KQuery::HALFYEAR));
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
    }

    agent.start();
}

void HKU_API stopSpotAgent() {
    auto& agent = *getGlobalSpotAgent();
    agent.stop();
}

}  // namespace hku