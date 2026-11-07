/*
 * test_StockTimeNormalize.cpp
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <algorithm>
#include <memory>
#include <string>
#include <hikyuu/Stock.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/DataDriverFactory.h>
#include <hikyuu/data_driver/KDataDriver.h>
#include <hikyuu/utilities/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_StockTimeNormalize test_hikyuu_StockTimeNormalize
 * @ingroup test_hikyuu_base_suite
 * @{
 */

namespace {

/** What the fake driver observed and what it will hand out */
struct FakeState {
    KRecordList records;            // records stored in the market's local wall clock
    Datetime last_start, last_end;  // the date range of the last date query it received
    bool date_query{false};         // whether any date query arrived at all
};

using FakeStatePtr = std::shared_ptr<FakeState>;

/**
 * A driver that simulates the local wall clock storage
 * @details Checks the Stock buffer-fill funnels: the driver receives local-expression date queries
 * and its local records must leave Stock in the unified (Beijing) expression
 */
class FakeLocalDriver : public KDataDriver {
public:
    explicit FakeLocalDriver(const string& name)
    : KDataDriver(name), m_state(std::make_shared<FakeState>()) {}

    KDataDriverPtr _clone() override {
        // The copy shares the same state, so a clone's writes are visible to the prototype holder
        return std::make_shared<FakeLocalDriver>(*this);
    }

    bool isIndexFirst() override {
        return true;
    }

    bool canParallelLoad() override {
        return false;
    }

    size_t getCount(const string&, const string&, const KQuery::KType&) override {
        return m_state->records.size();
    }

    bool getIndexRangeByDate(const string&, const string&, const KQuery& query, size_t& out_start,
                             size_t& out_end) override {
        m_state->date_query = true;
        m_state->last_start = query.startDatetime();
        m_state->last_end = query.endDatetime();
        out_start = 0;
        out_end = m_state->records.size();
        return true;
    }

    KRecordList getKRecordList(const string&, const string&, const KQuery& query) override {
        if (query.queryType() == KQuery::DATE) {
            m_state->date_query = true;
            m_state->last_start = query.startDatetime();
            m_state->last_end = query.endDatetime();
        }
        return m_state->records;
    }

    const FakeStatePtr& state() const {
        return m_state;
    }

private:
    FakeStatePtr m_state;
};

/**
 * Registers a fake driver prototype for one test case and hands out stocks bound to it
 * @note The pool is cached by name and a prototype cannot be re-registered, so each case uses
 * its own name and removes it again
 */
class FakeDriverFixture {
public:
    explicit FakeDriverFixture(const string& name) : m_name(name) {
        auto proto = std::make_shared<FakeLocalDriver>(name);
        m_state = proto->state();
        DataDriverFactory::regKDataDriver(proto);
    }

    ~FakeDriverFixture() {
        DataDriverFactory::removeKDataDriver(m_name);
    }

    Stock stock(const string& market) {
        Stock stk(market, "FAKE", "fake local storage stock", STOCKTYPE_A, true,
                  Datetime(202601010000LL), Datetime(209901010000LL), 1.0, 1.0, 2, 1, 1000000);
        Parameter param;
        param.set<string>("type", m_name);
        stk.setKDataDriver(DataDriverFactory::getKDataDriverPool(param));
        return stk;
    }

    const FakeStatePtr& state() const {
        return m_state;
    }

private:
    string m_name;
    FakeStatePtr m_state;
};

KRecord bar(const Datetime& dt, price_t close) {
    return KRecord(dt, close, close, close, close, close * 100, 100);
}

}  // namespace

/** @par Test points */
TEST_CASE("test_Stock_timeNormalize") {
    FakeDriverFixture fixture("fake_local_a");
    auto state = fixture.state();

    // NASDAQ: tz -300 / dst 1, so local 09:45 is Beijing 21:45 in daylight, 22:45 in standard
    MarketInfo nasdaq = StockManager::instance().getMarketInfo("NASDAQ");
    REQUIRE_UNARY(nasdaq != Null<MarketInfo>());
    REQUIRE_EQ(nasdaq.tzOffsetMin(), -300);
    REQUIRE_EQ(nasdaq.dstRule(), 1);
    const Datetime local_summer_bar(202607010945LL);
    const Datetime unified_summer_bar(202607012145LL);
    const Datetime local_winter_bar(202601050945LL);
    const Datetime unified_winter_bar(202601052245LL);

    Stock stk = fixture.stock("NASDAQ");
    REQUIRE_UNARY(!stk.isNull());

    /** @arg Non-buffered date query: the driver receives local bounds, the records leave unified */
    state->records = {bar(local_summer_bar, 10.0), bar(Datetime(202607011000LL), 11.0)};
    state->date_query = false;
    auto ks =
      stk.getKRecordList(KQueryByDate(unified_summer_bar, Datetime(202607020000LL), KQuery::MIN15));
    REQUIRE_UNARY(state->date_query);
    CHECK_EQ(state->last_start, local_summer_bar);
    CHECK_EQ(state->last_end, Datetime(202607011200LL));
    REQUIRE_EQ(ks.size(), (size_t)2);
    CHECK_EQ(ks[0].datetime, unified_summer_bar);
    CHECK_EQ(ks[1].datetime, Datetime(202607012200LL));

    /** @arg An index query needs no input conversion but the output is still unified */
    auto by_pos = stk.getKRecord(0, KQuery::MIN15);
    CHECK_EQ(by_pos.datetime, unified_summer_bar);

    /** @arg A query by datetime converts both the range and the returned bar */
    by_pos = stk.getKRecord(unified_summer_bar, KQuery::MIN15);
    CHECK_EQ(by_pos.datetime, unified_summer_bar);

    /** @arg The index range query converts the range only: indices stay in the driver's space */
    state->date_query = false;
    size_t out_start = 0, out_end = 0;
    CHECK_UNARY(
      stk.getIndexRange(KQueryByDate(unified_summer_bar, Datetime(202607020000LL), KQuery::MIN15),
                        out_start, out_end));
    CHECK_UNARY(state->date_query);
    CHECK_EQ(state->last_start, local_summer_bar);
    CHECK_EQ(out_start, (size_t)0);
    CHECK_EQ(out_end, (size_t)2);

    /** @arg Preloaded buffer: converted exactly once (winter bar first, keeping order) */
    state->records = {bar(local_winter_bar, 12.0), bar(local_summer_bar, 10.0)};
    stk.loadKDataToBuffer(KQuery::MIN15);
    REQUIRE_UNARY(stk.isBuffer(KQuery::MIN15));
    auto buf = stk.getKRecordListFromBuffer(KQuery::MIN15);
    REQUIRE_EQ(buf.size(), (size_t)2);
    CHECK_EQ(buf[0].datetime, unified_winter_bar);
    CHECK_EQ(buf[1].datetime, unified_summer_bar);
    // The conversion must not break the ordering (the premise of the buffer search and getPos)
    CHECK_UNARY(buf[0].datetime < buf[1].datetime);

    /** @arg Buffered path: the driver is not touched a second time and no second shift appears */
    state->date_query = false;
    ks = stk.getKRecordList(
      KQueryByDate(Datetime(202607012100LL), Datetime(202607020000LL), KQuery::MIN15));
    CHECK_UNARY(!state->date_query);
    REQUIRE_EQ(ks.size(), (size_t)1);
    CHECK_EQ(ks[0].datetime, unified_summer_bar);

    /** @arg A bar closing on the next Beijing day crosses too (use Datetime, not number()) */
    stk.releaseKDataBuffer(KQuery::MIN15);
    state->records = {bar(Datetime(202607011545LL), 10.0)};
    ks = stk.getKRecordList(
      KQueryByDate(Datetime(202607020000LL), Datetime(202607030000LL), KQuery::MIN15));
    REQUIRE_EQ(ks.size(), (size_t)1);
    CHECK_EQ(ks[0].datetime, Datetime(202607020345LL));

    /** @arg An empty driver result stays empty, it is never turned into a phantom record */
    stk.releaseKDataBuffer(KQuery::MIN15);
    state->records = KRecordList();
    ks = stk.getKRecordList(
      KQueryByDate(Datetime(202607020000LL), Datetime(202607030000LL), KQuery::MIN15));
    CHECK_UNARY(ks.empty());
    CHECK_UNARY(!stk.getKRecord(Datetime(202607020000LL), KQuery::MIN15).isValid());

    /** @arg Day and higher types are never converted: the trading-day label comes out unchanged */
    const Datetime day_label(202607010000LL);
    state->records = {bar(day_label, 10.0)};
    ks = stk.getKRecordList(KQueryByDate(day_label, Datetime(202607020000LL), KQuery::DAY));
    REQUIRE_EQ(ks.size(), (size_t)1);
    CHECK_EQ(ks[0].datetime, day_label);
    // The range handed to the driver must not be shifted either (labels keep their own day)
    CHECK_EQ(state->last_start, day_label);

    DataDriverFactory::removeKDataDriver("fake_local_a");
}

/** @par Test points */
TEST_CASE("test_Stock_timeNormalize_identityMarket") {
    FakeDriverFixture fixture("fake_local_b");
    auto state = fixture.state();

    const Datetime bar_dt(202607010945LL);

    /** @arg Identity market (A share): the range and the records stay byte for byte unchanged */
    Stock cn = fixture.stock("SH");
    REQUIRE_UNARY(!cn.isNull());
    state->records = {bar(bar_dt, 10.0)};
    state->date_query = false;
    auto ks = cn.getKRecordList(KQueryByDate(bar_dt, Datetime(202607020000LL), KQuery::MIN15));
    CHECK_UNARY(state->date_query);
    CHECK_EQ(state->last_start, bar_dt);
    CHECK_EQ(state->last_end, Datetime(202607020000LL));
    REQUIRE_EQ(ks.size(), (size_t)1);
    CHECK_EQ(ks[0].datetime, bar_dt);

    /** @arg An unregistered market is treated as identity: nothing shifts into an unknown zone */
    Stock unknown = fixture.stock("NOSUCH");
    state->records = {bar(bar_dt, 10.0)};
    ks = unknown.getKRecordList(KQueryByDate(bar_dt, Datetime(202607020000LL), KQuery::MIN15));
    REQUIRE_EQ(ks.size(), (size_t)1);
    CHECK_EQ(ks[0].datetime, bar_dt);

    DataDriverFactory::removeKDataDriver("fake_local_b");
}

/** @par Test points */
TEST_CASE("test_Stock_timeNormalize_realtimeGate") {
    FakeDriverFixture fixture("fake_local_c");
    auto state = fixture.state();
    Stock stk = fixture.stock("NASDAQ");

    // The buffer holds one daylight bar, read back as the unified 2016-07-01 21:45
    state->records = {bar(Datetime(201607010945LL), 10.0)};
    stk.loadKDataToBuffer(KQuery::MIN15);
    REQUIRE_EQ(stk.getKRecordListFromBuffer(KQuery::MIN15).size(), (size_t)1);

    /** @arg The holiday gate looks at the market's own local date: a US weekend is blocked even
     * though the incoming unified instant is a Beijing Saturday evening */
    stk.realtimeUpdate(bar(Datetime(201607022200LL), 13.0), KQuery::MIN15);  // Saturday New York
    CHECK_EQ(stk.getKRecordListFromBuffer(KQuery::MIN15).size(), (size_t)1);

    /** @arg The same clock time one day earlier is a Friday locally, so it passes the gate */
    stk.realtimeUpdate(bar(Datetime(201607012200LL), 13.0), KQuery::MIN15);  // Friday New York
    auto after = stk.getKRecordListFromBuffer(KQuery::MIN15);
    REQUIRE_EQ(after.size(), (size_t)2);
    CHECK_EQ(after.back().datetime, Datetime(201607012200LL));

    /** @arg A null instant is refused before any conversion, not turned into a midnight bar */
    stk.realtimeUpdate(bar(Datetime(), 15.0), KQuery::MIN15);
    CHECK_EQ(stk.getKRecordListFromBuffer(KQuery::MIN15).size(), (size_t)2);

    DataDriverFactory::removeKDataDriver("fake_local_c");
}

/** @par Test points */
TEST_CASE("test_Stock_timeNormalize_monotonic") {
    FakeDriverFixture fixture("fake_local_d");
    auto state = fixture.state();
    Stock stk = fixture.stock("NASDAQ");

    /** @arg A whole US session of local minutes stays strictly increasing after the conversion */
    KRecordList local;
    for (int hh = 9; hh <= 15; hh++) {
        int start_mm = (hh == 9 ? 45 : 0);
        for (int mm = start_mm; mm < 60; mm += 15) {
            if (hh == 15 && mm > 45) {
                break;
            }
            local.push_back(bar(Datetime(202607010000LL + hh * 100 + mm), 10.0));
        }
    }
    REQUIRE_UNARY(local.size() > 10);
    state->records = local;
    auto ks = stk.getKRecordList(
      KQueryByDate(Datetime(202607020000LL), Datetime(202607030000LL), KQuery::MIN15));
    REQUIRE_EQ(ks.size(), local.size());
    auto out_of_order = std::adjacent_find(
      ks.begin(), ks.end(),
      [](const KRecord& a, const KRecord& b) { return !(a.datetime < b.datetime); });
    CHECK_UNARY(out_of_order == ks.end());
    /** @arg Session edges: local 09:45 -> Beijing 21:45, local 15:45 -> Beijing 03:45 next day */
    CHECK_EQ(ks.front().datetime, Datetime(202607012145LL));
    CHECK_EQ(ks.back().datetime, Datetime(202607020345LL));

    DataDriverFactory::removeKDataDriver("fake_local_d");
}

/** @par Test points */
TEST_CASE("test_Stock_dayLabels_tableDriven") {
    FakeDriverFixture fixture("fake_local_e");
    auto state = fixture.state();
    Stock cn = fixture.stock("SH");
    Stock us = fixture.stock("NASDAQ");
    REQUIRE_UNARY(!cn.isNull());
    REQUIRE_UNARY(!us.isNull());

    const vector<KQuery::KType> label_ktypes = {KQuery::DAY,     KQuery::WEEK,     KQuery::MONTH,
                                                KQuery::QUARTER, KQuery::HALFYEAR, KQuery::YEAR};
    for (const auto& ktype : label_ktypes) {
        /** @arg Day..Year labels are identical between the identity and the US market, and the
         * driver receives the query bounds unshifted */
        KRecordList records;
        for (int i = 0; i < 30; i++) {
            records.push_back(bar(Datetime(202601050000LL) + Days(i), 10.0 + i));
        }
        state->records = records;
        state->date_query = false;
        KRecordList cn_records = cn.getKRecordList(
          KQueryByDate(Datetime(202601010000LL), Datetime(202602010000LL), ktype));
        KRecordList us_records = us.getKRecordList(
          KQueryByDate(Datetime(202601010000LL), Datetime(202602010000LL), ktype));
        REQUIRE_EQ(cn_records.size(), (size_t)30);
        REQUIRE_EQ(us_records.size(), (size_t)30);
        for (size_t i = 0; i < cn_records.size(); i++) {
            CHECK_EQ(cn_records[i].datetime, records[i].datetime);
            CHECK_EQ(us_records[i], cn_records[i]);
        }
        CHECK_EQ(state->last_start, Datetime(202601010000LL));
        CHECK_EQ(state->last_end, Datetime(202602010000LL));
    }

    DataDriverFactory::removeKDataDriver("fake_local_e");
}

/** @} */
