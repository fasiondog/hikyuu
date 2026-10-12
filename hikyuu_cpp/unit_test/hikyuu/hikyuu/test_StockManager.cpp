/*
 * test_StockManager.cpp
 *
 *  Created on: 2011-11-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/runtimeinfo.h>
#include <hikyuu/utilities/Log.h>
#include "../plugin_valid.h"

using namespace hku;

namespace {
#if 0
// waitDataReady() is expected to return promptly once the loading has failed; it is run in a
// detached thread with a deadline, so that a regression is reported as a failure instead of
// blocking the whole test run
bool waitDataReadyReleased(StockManager& sm) {
    auto done = std::make_shared<std::atomic_bool>(false);
    std::thread([&sm, done]() {
        sm.waitDataReady();
        done->store(true);
    }).detach();
    for (int i = 0; i < 100 && !done->load(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return done->load();
}
#endif

// Only used by the disabled deadlock case at the end of this file
#if 0
// Runs the given function in a detached thread with a deadline. A deadlock leaves the worker
// holding the lock, which would hang the static destruction as well, so on a timeout the process is
// left without running the destructors: the failure stays visible instead of hanging forever
template <typename Func>
void runWithDeadline(Func&& func, int seconds = 10) {
    auto done = std::make_shared<std::atomic_bool>(false);
    std::thread([func = std::forward<Func>(func), done]() {
        func();
        done->store(true);
    }).detach();
    for (int i = 0; i < seconds * 10 && !done->load(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!done->load()) {
        std::cerr << "runWithDeadline: the operation did not finish within " << seconds
                  << "s (deadlock?)" << std::endl;
        std::quick_exit(1);
    }
}
#endif

bool hasBlock(const BlockList& blocks, const string& category, const string& name) {
    for (const auto& blk : blocks) {
        if (blk.category() == category && blk.name() == name) {
            return true;
        }
    }
    return false;
}
}  // namespace

/**
 * @defgroup test_hikyuu_StockManager test_hikyuu_StockManager
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_StockManager_size") {
    StockManager& sm = StockManager::instance();
    /** @arg Check that it matches the security count of the test data */
    CHECK_EQ(sm.size(), 4729);
}

/** @par Test points */
TEST_CASE("test_StockManager_getStock") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    Stock null_stock = Null<Stock>();

    /** @arg Query by "market code" while the market does not exist */
    CHECK_EQ(sm.getStock("z000001"), null_stock);

    /** @arg Query by "market code" while the market exists but the security code does not */
    CHECK_EQ(sm.getStock("sh1000001"), null_stock);

    /** @arg Query an existing stock by "market code" */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.market(), "SH");
    CHECK_EQ(stock.code(), "000001");
    CHECK_EQ(stock.name(), "上证指数");
    HKU_INFO("{}", stock.toString());
    CHECK_EQ(stock.type(), 2);
    CHECK_EQ(stock.valid(), true);
    CHECK_EQ(stock.startDatetime(), Datetime(199012190000));
    CHECK_EQ(stock.lastDatetime(), Null<Datetime>());
    CHECK_EQ(stock.tick(), 0.001);
    CHECK_EQ(stock.tickValue(), 0.001);
    CHECK_EQ(stock.unit(), 1.0);
    CHECK_EQ(stock.precision(), 3);
    CHECK_EQ(stock.atom(), 1);
    CHECK_EQ(stock.minTradeNumber(), 1);
    CHECK_EQ(stock.maxTradeNumber(), 1000000);
}

/** @par Test points */
TEST_CASE("test_StockManager_getMarketInfo") {
    StockManager& sm = StockManager::instance();

    /** @arg Query a market that does not exist */
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo("XXX"));
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo(""));

    /** @arg Query an existing market with an all-uppercase market name */
    MarketInfo marketInfo = sm.getMarketInfo("SH");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");
    CHECK_EQ(marketInfo.code(), "000001");
    CHECK_EQ(marketInfo.name(), "上海证券交易所");
    CHECK_EQ(marketInfo.description(), "上海市场");
    CHECK_EQ(marketInfo.lastDate(), Datetime(201112060000L));
    CHECK_EQ(marketInfo.openTime1(), TimeDelta(0, 9, 30));
    CHECK_EQ(marketInfo.closeTime1(), TimeDelta(0, 11, 30));
    CHECK_EQ(marketInfo.openTime2(), TimeDelta(0, 13, 00));
    CHECK_EQ(marketInfo.closeTime2(), TimeDelta(0, 15, 00));

    /** @arg Query an existing market with an all-lowercase market name */
    marketInfo = sm.getMarketInfo("sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");

    /** @arg Query an existing market with a mixed-case market name */
    marketInfo = sm.getMarketInfo("Sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");
}

/** @par Test points */
TEST_CASE("test_StockManager_getStockTypeInfo") {
    StockManager& sm = StockManager::instance();
    StockTypeInfo stockTypeInfo;

    /** @arg Query a type that does not exist */
    CHECK_EQ(sm.getStockTypeInfo(999), Null<StockTypeInfo>());

    /** @arg Query the first record, i.e. type=0 */
    stockTypeInfo = sm.getStockTypeInfo(0);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 0);
    CHECK_EQ(stockTypeInfo.description(), "Block");
    CHECK_EQ(stockTypeInfo.tick(), 100);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg Query the last record, i.e. type=8 */
    stockTypeInfo = sm.getStockTypeInfo(8);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 8);
    CHECK_EQ(stockTypeInfo.description(), "创业板");
    CHECK_EQ(stockTypeInfo.tick(), 0.01);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg Query information that exists normally */
    stockTypeInfo = sm.getStockTypeInfo(2);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 2);
    CHECK_EQ(stockTypeInfo.description(), "指数");
    CHECK_EQ(stockTypeInfo.tick(), 0.001);
    CHECK_EQ(stockTypeInfo.precision(), 3);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 1);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);
}

/** @par Test points */
TEST_CASE("test_StockManager_getAllMarket") {
    StockManager& sm = StockManager::instance();

    /** @arg Check the Market in the test data */
    StringList result(sm.getAllMarket());
    std::vector<string> want_list{"SH", "SZ"};
    for (auto want : want_list) {
        bool found = false;
        for (auto i = 0; i < result.size(); i++) {
            if (result[i] == want) {
                found = true;
                break;
            }
        }
        CHECK_UNARY(found);
    }
}

/** @par Test points */
TEST_CASE("test_StockManager_getTradingCalendar") {
    StockManager& sm = StockManager::instance();

    /** @arg Unknown market returns an empty list without throwing */
    DatetimeList unknown = sm.getTradingCalendar(KQuery(), "FOO");
    CHECK_EQ(unknown.size(), 0);

    /** @arg Known market returns the same datetime list as its representative index stock */
    MarketInfo sh_info = sm.getMarketInfo("SH");
    Stock sh_index = sm.getStock(sh_info.market() + sh_info.code());
    DatetimeList expect = sh_index.getDatetimeList(KQuery());
    DatetimeList actual = sm.getTradingCalendar(KQuery(), "SH");
    CHECK_EQ(actual.size(), expect.size());

    /** @arg StockList overload merges multiple markets' trading days */
    MarketInfo sz_info = sm.getMarketInfo("SZ");
    StockList stk_list{sm.getStock(sh_info.market() + sh_info.code()),
                       sm.getStock(sz_info.market() + sz_info.code())};
    DatetimeList merged = sm.getTradingCalendar(stk_list, KQuery());
    CHECK_GT(merged.size(), 0);
}

/** @par Test points */
TEST_CASE("test_StockManager_getBlock") {
    if (supportChineseSimple()) {
        StockManager& sm = StockManager::instance();
        Block result = sm.getBlock("地域板块", "陕西");
        CHECK_NE(result.size(), 0);

        BlockList blk_list = sm.getBlockList("地域板块");
        blk_list = sm.getBlockList();
        CHECK_NE(blk_list.size(), 0);
    }
}

/** @par Test points */
TEST_CASE("test_StockManager_getBlockList_inner_block") {
    auto& sm = StockManager::instance();

    /** @arg Filtering by the category of the inner blocks returns them (their keys are
     * "category_name", which never equals a bare category) */
    BlockList blocks_a = sm.getBlockList("A");
    CHECK_UNARY(hasBlock(blocks_a, "A", "ALL"));
    CHECK_UNARY(hasBlock(blocks_a, "A", "沪深"));
    CHECK_UNARY(hasBlock(blocks_a, "A", "SH"));
    CHECK_UNARY(hasBlock(blocks_a, "A", "SZ"));
    for (const auto& blk : blocks_a) {
        CHECK_EQ(blk.category(), "A");
    }

    /** @arg The other inner categories are matched as well */
    CHECK_UNARY(hasBlock(sm.getBlockList("G"), "G", "创业板"));
    CHECK_UNARY(hasBlock(sm.getBlockList("START"), "START", "科创板"));
    CHECK_UNARY(hasBlock(sm.getBlockList("ETF"), "ETF", "ALL"));

    /** @arg The inner category matching is case-insensitive */
    CHECK_UNARY(hasBlock(sm.getBlockList("a"), "A", "ALL"));
    CHECK_UNARY(hasBlock(sm.getBlockList("start"), "START", "科创板"));

    /** @arg An unknown category matches neither a driver block nor an inner block */
    CHECK_EQ(sm.getBlockList("__no_such_category__").size(), 0);

    /** @arg getBlock of the same category+name is a non-empty inner block, and the unfiltered list
     * contains it as well */
    CHECK_NE(sm.getBlock("A", "ALL").size(), 0);
    CHECK_UNARY(hasBlock(sm.getBlockList(), "A", "ALL"));

    /** @arg getAllCategory lists the inner categories too, so they can be discovered */
    StringList categories = sm.getAllCategory();
    CHECK_UNARY(std::find(categories.begin(), categories.end(), "A") != categories.end());
    CHECK_UNARY(std::find(categories.begin(), categories.end(), "G") != categories.end());
    CHECK_UNARY(std::find(categories.begin(), categories.end(), "START") != categories.end());
    CHECK_UNARY(std::find(categories.begin(), categories.end(), "ETF") != categories.end());

    /** @arg getStockBelongs finds the inner block of the given category */
    Stock stk = sm.getStock("sz000001");
    CHECK_UNARY(!stk.isNull());
    CHECK_UNARY(hasBlock(sm.getStockBelongs(stk, "A"), "A", "ALL"));
}

/** @par Test points */
TEST_CASE("test_StockManager_isHoliday") {
    auto& sm = StockManager::instance();
    CHECK_THROWS(sm.isHoliday(Datetime()));
    CHECK_EQ(sm.isHoliday(Datetime(202101010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202101020000LL)), true);  // Saturday
    CHECK_EQ(sm.isHoliday(Datetime(202101030000LL)), true);  // Sunday
    CHECK_EQ(sm.isHoliday(Datetime(202110010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202109300000LL)), false);
}

/** @par Test points */
TEST_CASE("test_StockManager_isTradingHours") {
    auto& sm = StockManager::instance();
    CHECK_THROWS(sm.isTradingHours(Datetime()));
    CHECK_EQ(sm.isTradingHours(Datetime(202101010000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202101020000LL)), false);  // Saturday
    CHECK_EQ(sm.isTradingHours(Datetime(202101030000LL)), false);  // Sunday
    CHECK_EQ(sm.isTradingHours(Datetime(202110010000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300929LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300930LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301000LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301130LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301131LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301259LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301300LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301301LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301500LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301501LL)), false);
    CHECK_THROWS(sm.isTradingHours(Datetime(202109301500LL), "invalid"));
}

/** @par Test points */
TEST_CASE("test_StockManager_getZhBond10") {
    auto& sm = StockManager::instance();
    const auto& result = sm.getZhBond10();
    CHECK_EQ(result.size(), 5536);
    CHECK_EQ(result[0].date, Datetime(20020104));
    CHECK_EQ(result[0].value, doctest::Approx(3.2096));
    CHECK_EQ(result[10].date, Datetime(20020118));
    CHECK_EQ(result[10].value, doctest::Approx(3.2968));
    CHECK_EQ(result[5535].date, Datetime(20240229));
    CHECK_EQ(result[5535].value, doctest::Approx(2.3375));
}

/** @par Test points */
TEST_CASE("test_StockManager_releaseShmServerBaseInfoCache") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto& sm = StockManager::instance();
    const bool org_role = isShmServerRole();
    Stock stk = sm.getStock("sz000001");
    const size_t weight_cnt = stk.getWeight().size();
    const size_t finance_cnt = stk.getHistoryFinance().size();

    /** @arg The call is a no-op in a non-server role (an ordinary standalone / client process): the
     * cache is unaffected */
    setShmServerRole(false);
    sm.releaseShmServerBaseInfoCache();
    CHECK_EQ(stk.getWeight().size(), weight_cnt);
    CHECK_EQ(stk.getHistoryFinance().size(), finance_cnt);

    /** @arg The server role releases the ex-rights/ex-dividend cache: switching back to the
     * standalone mode (where getWeight has no lazy loading fallback) and querying again returns
     * empty, proving that the cache has really been cleared and returned */
    setShmServerRole(true);
    sm.releaseShmServerBaseInfoCache();
    setShmServerRole(false);
    CHECK_EQ(stk.getWeight().size(), 0);

    /** @arg Accessing a released security again in the server role: the on-demand lazy reload heals
     * it and the result matches the one before the release */
    setShmServerRole(true);
    CHECK_EQ(stk.getWeight().size(), weight_cnt);
    CHECK_EQ(stk.getHistoryFinance().size(), finance_cnt);

    // Restore the state: Stock::getWeight has no lazy loading fallback in the standalone / client
    // mode, so the ex-rights/ex-dividend cache must be refilled before the role is restored,
    // avoiding an emptied state affecting the later cases (the historical finance has a fallback)
    for (const auto& stock : sm.getStockList(nullptr)) {
        stock.getWeight();
    }
    setShmServerRole(org_role);
}

/** @par Test points */
TEST_CASE("test_StockManager_loadAllStocks_by_context") {
    auto& sm = StockManager::instance();
    const StrategyContext org_context = sm.getStrategyContext();

    StrategyContext context({"sh000001", "sz000001", "sh999999", "xx000001", "xsh000001"});
    // The stock dictionary is never cleared by the load, so the two valid codes are removed first:
    // only the context-driven loading below can bring them back (sh000001 also via the must-load
    // list, sz000001 only via the context)
    sm.removeStock("SH000001");
    sm.removeStock("SZ000001");
    sm.reloadWith(context);
    sm.joinPreloadThread();

    /** @arg The codes given by the context are loaded */
    CHECK_UNARY(!sm.getStock("sh000001").isNull());
    CHECK_UNARY(!sm.getStock("sz000001").isNull());

    /** @arg A driver miss is skipped: it would otherwise be loaded as a garbage security with an
     * empty market_code */
    CHECK_UNARY(sm.getStock("sh999999").isNull());
    bool has_empty_market_code = false;
    for (const auto& stk : sm.getStockList(nullptr)) {
        if (stk.market_code().empty()) {
            has_empty_market_code = true;
            break;
        }
    }
    CHECK_UNARY_FALSE(has_empty_market_code);

    /** @arg A code whose market abbreviation does not lead the string is rejected (the market is
     * matched at position 0 only) */
    CHECK_UNARY(sm.getStock("xsh000001").isNull());
    CHECK_UNARY(sm.getStock("xx000001").isNull());

    // Restore the full load for the following cases
    sm.reloadWith(org_context);
    sm.joinPreloadThread();
    CHECK_EQ(sm.size(), 4729);
}

/** @par Test points */
TEST_CASE("test_StockManager_getStockList") {
    StockManager& sm = StockManager::instance();

    /** @arg No filter returns every loaded stock */
    StockList all = sm.getStockList(nullptr);
    CHECK_EQ(all.size(), sm.size());

    /** @arg A filter that always returns true returns every stock */
    StockList all_true = sm.getStockList([](const Stock&) { return true; });
    CHECK_EQ(all_true.size(), sm.size());

    /** @arg A filter that always returns false returns an empty list */
    StockList none = sm.getStockList([](const Stock&) { return false; });
    CHECK_EQ(none.size(), 0);

    /** @arg A filter selecting by market keeps only that market's stocks */
    StockList sh = sm.getStockList([](const Stock& s) { return s.market() == "SH"; });
    CHECK_GT(sh.size(), 0);
    for (const auto& s : sh) {
        CHECK_EQ(s.market(), "SH");
    }

    /** @arg The user filter may re-enter the manager (getStock) without deadlock,
     * because the dictionary lock is released before the filter runs */
    StockList reentrant = sm.getStockList([&sm](const Stock& s) {
        return !sm.getStock(s.market_code()).isNull();
    });
    CHECK_EQ(reentrant.size(), sm.size());

    /** @arg Regression guard: a filter that requests the unique lock (addStock)
     * must not self-deadlock; run under a watchdog so a regression fails loudly
     * instead of hanging the whole test process */
    bool tried = false;
    auto done = std::make_shared<std::atomic_bool>(false);
    std::thread([&sm, done, &tried]() {
        sm.getStockList([&sm, &tried](const Stock&) {
            if (!tried) {
                tried = true;
                sm.addStock(sm.getStock("sh000001"));
            }
            return true;
        });
        done->store(true);
    }).detach();
    for (int i = 0; i < 100 && !done->load(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK_UNARY(done->load());
}

// Disabled by default: it performs several full reloads and exercises injected
// failure paths; enable it when load-failure handling or the callback registry changes.
#if 0
/** @par Test points */
TEST_CASE("test_StockManager_load_failed") {
    auto& sm = StockManager::instance();
    sm.joinPreloadThread();

    /** @arg A load that finishes normally leaves the failure flag unset */
    CHECK_UNARY(!sm.loadFailed());
    CHECK_UNARY(sm.dataReady());

    /** @arg An exception thrown before the preload thread is started is published as a failure:
     * nothing would ever set the readiness flag, so the waiters must be released explicitly */
    size_t early_id = registerLoadEventCallback([](LoadEvent event) {
        HKU_IF_RETURN(event != LoadEvent::BASE_DATA_READY, void());
        throw std::runtime_error("injected failure before the preload is started");
    });
    CHECK_THROWS_AS(sm.reload(), std::runtime_error);
    unregisterLoadEventCallback(early_id);
    CHECK_UNARY(sm.loadFailed());
    CHECK_UNARY(sm.dataReady());
    CHECK_UNARY(waitDataReadyReleased(sm));

    /** @arg An exception escaping the preload thread body is caught: it no longer terminates the
     * process, and the failure (together with the readiness) is published */
    size_t preload_id = registerLoadEventCallback([](LoadEvent event) {
        HKU_IF_RETURN(event != LoadEvent::KDATA_PRELOAD_FINISHED, void());
        throw std::runtime_error("injected failure in the preload thread");
    });
    sm.reload();
    sm.joinPreloadThread();
    unregisterLoadEventCallback(preload_id);
    CHECK_UNARY(sm.loadFailed());
    CHECK_UNARY(sm.dataReady());
    CHECK_UNARY(waitDataReadyReleased(sm));

    // Restore the state for the following cases: a clean reload clears the failure flag and refills
    // the preload buffer
    sm.reload();
    sm.joinPreloadThread();
    CHECK_UNARY(!sm.loadFailed());
    CHECK_UNARY(sm.dataReady());
}
#endif

// Disabled by default: it costs a full reload on every run and it can only fail when the dispatch
// path regresses into a deadlock. Enable it when _fireLoadEvent or the callback registry changes.
#if 0
/** @par Test points */
TEST_CASE("test_StockManager_fireLoadEvent_unregister_in_callback") {
    auto& sm = StockManager::instance();

    /** @arg A callback unregistering another one does not self-deadlock: the dispatch runs on a
     * snapshot taken under the lock, so the unregister below may take the same lock */
    size_t outer_id = registerLoadEventCallback([](LoadEvent) {});
    size_t inner_id =
      registerLoadEventCallback([outer_id](LoadEvent) { unregisterLoadEventCallback(outer_id); });
    runWithDeadline([&sm]() {
        sm.reload();
        sm.joinPreloadThread();
    });
    unregisterLoadEventCallback(inner_id);

    CHECK_UNARY(!sm.loadFailed());
    CHECK_UNARY(sm.dataReady());
}

/** @par Test points */
TEST_CASE("test_StockManager_fireLoadEvent_unregister_waits_inflight") {
    auto& sm = StockManager::instance();

    /** @arg unregisterLoadEventCallback waits for the in-flight dispatch holding this callback:
     * when it returns no dispatch is running any more, so the callback context may be destroyed
     * right after (the plugin stop sequence: unregister then destroy) */
    std::atomic<bool> in_callback{false};
    std::atomic<bool> unreg_entered{false};
    std::atomic<bool> context_alive_at_callback{true};
    std::atomic<bool> in_callback_at_unreg_return{true};
    auto ctx = std::make_shared<int>(42);
    std::weak_ptr<int> watcher = ctx;
    size_t slow_id = registerLoadEventCallback([&](LoadEvent event) {
        HKU_IF_RETURN(event != LoadEvent::BASE_DATA_READY, void());
        in_callback.store(true, std::memory_order_release);
        while (!unreg_entered.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        context_alive_at_callback.store(!watcher.expired(), std::memory_order_release);
        in_callback.store(false, std::memory_order_release);
    });
    std::thread unreg_thread([&]() {
        while (!in_callback.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        unreg_entered.store(true, std::memory_order_release);
        unregisterLoadEventCallback(slow_id);
        in_callback_at_unreg_return.store(in_callback.load(std::memory_order_acquire),
                                          std::memory_order_release);
        ctx.reset();  // destroy the context right after unregistering (the plugin stop sequence)
    });
    runWithDeadline([&sm]() { sm.reload(); });
    unreg_thread.join();
    sm.joinPreloadThread();

    CHECK_UNARY(context_alive_at_callback.load());
    CHECK_UNARY_FALSE(in_callback_at_unreg_return.load());
    CHECK_UNARY(watcher.expired());
}
#endif

/** @} */
