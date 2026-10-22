/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-23
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <sstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/multifactor/ScoreRecord.h>

using namespace hku;

/**
 * @defgroup test_ScoreRecord test_ScoreRecord
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Count the flush (sync) calls, so that the "no flush per record" contract is observable
class CountingBuf : public std::stringbuf {
public:
    int sync() override {
        ++m_sync_count;
        return std::stringbuf::sync();
    }

    int syncCount() const {
        return m_sync_count;
    }

private:
    int m_sync_count = 0;
};

}  // namespace

TEST_CASE("test_ScoreRecord_operator_out") {
    Stock stk = StockManager::instance()["sz000001"];
    std::string code = stk.market_code();
    ScoreRecord record(stk, 1.23456);

    /** @arg a single record is printed with 4 fixed decimals, followed by exactly one newline */
    std::ostringstream out;
    out << record;
    CHECK_EQ(out.str(), "(" + code + ", 1.2346)\n");

    /** @arg the caller's precision and format flags are restored after the output */
    std::ostringstream reused;
    reused.precision(2);
    reused << std::defaultfloat;
    auto flags_before = reused.flags();
    reused << record;
    CHECK_EQ(reused.precision(), 2);
    CHECK_EQ(reused.flags(), flags_before);

    /** @arg the record itself is not flushed, only an explicit std::endl flushes */
    CountingBuf buf;
    std::ostream stream(&buf);
    stream << record;
    CHECK_EQ(buf.syncCount(), 0);
    stream << std::endl;
    CHECK_EQ(buf.syncCount(), 1);
}

TEST_CASE("test_ScoreRecord_list_operator_out") {
    Stock stk = StockManager::instance()["sz000001"];
    std::string code = stk.market_code();

    /** @arg an empty list prints nothing */
    ScoreRecordList empty;
    std::ostringstream out0;
    out0 << empty;
    CHECK_EQ(out0.str(), "");

    /** @arg a short list prints every record on its own line */
    ScoreRecordList short_list;
    short_list.emplace_back(stk, 1.0);
    short_list.emplace_back(stk, 2.0);
    std::ostringstream out1;
    out1 << short_list;
    CHECK_EQ(out1.str(), "0: (" + code + ", 1.0000)\n1: (" + code + ", 2.0000)\n");

    /** @arg a list of 10 records prints the first 5 and the last 5 around the ellipsis */
    ScoreRecordList long_list;
    for (int i = 0; i < 10; i++) {
        long_list.emplace_back(stk, i);
    }
    std::ostringstream out2;
    out2 << long_list;
    std::string expected;
    for (int i = 0; i < 5; i++) {
        expected += std::to_string(i) + ": (" + code + ", " + std::to_string(i) + ".0000)\n";
    }
    expected += "......\n";
    for (int i = 5; i < 10; i++) {
        expected += std::to_string(i) + ": (" + code + ", " + std::to_string(i) + ".0000)\n";
    }
    CHECK_EQ(out2.str(), expected);

    /** @arg no record in the list triggers a flush */
    CountingBuf buf;
    std::ostream stream(&buf);
    stream << long_list;
    CHECK_EQ(buf.syncCount(), 0);

    /** @arg the caller's precision is restored */
    std::ostringstream reused;
    reused.precision(2);
    reused << short_list;
    CHECK_EQ(reused.precision(), 2);
}

TEST_CASE("test_ScoreRecord_date_list_operator_out") {
    Stock stk = StockManager::instance()["sz000001"];
    std::string code = stk.market_code();
    std::string line = "0: (" + code + ", 1.0000)\n";
    ScoreRecordList recs;
    recs.emplace_back(stk, 1.0);

    /** @arg a date list with at most 2 cross-sections prints each one with its header */
    vector<ScoreRecordList> two{recs, recs};
    std::ostringstream out1;
    out1 << two;
    CHECK_EQ(out1.str(), "========= 0 =========\n" + line + "========= 1 =========\n" + line);

    /** @arg more than 2 cross-sections print only the first and the last one */
    vector<ScoreRecordList> three{recs, recs, recs};
    std::ostringstream out2;
    out2 << three;
    CHECK_EQ(out2.str(), "========= 0 =========\n" + line + "......\n......\n" +
                           "========= 2 =========\n" + line);

    /** @arg an empty date list prints nothing */
    vector<ScoreRecordList> empty;
    std::ostringstream out3;
    out3 << empty;
    CHECK_EQ(out3.str(), "");
}

/** @} */
