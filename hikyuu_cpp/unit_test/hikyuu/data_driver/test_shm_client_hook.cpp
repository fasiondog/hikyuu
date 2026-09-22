/*
 * test_shm_client_hook.cpp
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#include "doctest/doctest.h"

#include <string>
#include <vector>
#include "hikyuu/data_driver/ipc/ShmClientHook.h"
#include "../plugin_valid.h"

using namespace hku;
using namespace hku::ipc;

namespace {

/** A forwarding recorder: it captures the arguments and the returns of the registered callbacks,
 * verifying that the core library keeps plain callbacks only, depending on no shm type */
struct ForwardRecorder {
    std::string last_market_code;
    std::string last_rt_ktype;
    std::string last_lu_ktype;
    std::string last_addr;
    std::string last_pull_ktype;
    std::vector<std::string> last_codes;
    KRecord last_record;
    int rt_count{0};
    int lu_count{0};
    int pull_count{0};
    bool rt_result{true};
    bool pull_result{true};
    Datetime lu_result{202601011200LL};
};

/** Build the callback group registered to the thin forwarding layer (in the real scenario it is
 * built and registered by the shmserver plugin after a successful connect) */
ShmClientForwarders makeForwarders(ForwardRecorder& rec) {
    ShmClientForwarders fwd;
    fwd.realtimeUpdate = [&rec](const std::string& market_code, const KQuery::KType& ktype,
                                const KRecord& record) {
        rec.last_market_code = market_code;
        rec.last_rt_ktype = ktype;
        rec.last_record = record;
        ++rec.rt_count;
        return rec.rt_result;
    };
    fwd.getLastUpdateTime = [&rec](const std::string& market_code, const KQuery::KType& ktype) {
        rec.last_market_code = market_code;
        rec.last_lu_ktype = ktype;
        ++rec.lu_count;
        return rec.lu_result;
    };
    fwd.pullFromBufferServer = [&rec](const std::string& addr,
                                      const std::vector<std::string>& codes,
                                      const KQuery::KType& ktype) {
        rec.last_addr = addr;
        rec.last_codes = codes;
        rec.last_pull_ktype = ktype;
        ++rec.pull_count;
        return rec.pull_result;
    };
    return fwd;
}

}  // namespace

/**
 * @defgroup test_shm_client_hook_suite test_shm_client_hook_suite
 * @ingroup DataDriver
 * @{
 */

/** @par Test points */
TEST_CASE("test_ShmClientHook_forward") {
    HKU_IF_RETURN(!pluginValid(), void());

    ForwardRecorder rec;
    auto fwd = makeForwarders(rec);

    /** @arg Without a registered client (the standalone mode / no plugin): a safe no-op forwarding
     */
    CHECK_UNARY(!shmClient());
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {"SH600000"}, KQuery::MIN));
    CHECK_UNARY(forwardGetLastUpdateTime("SH600000", KQuery::DAY) == Datetime::min());
    CHECK_EQ(rec.rt_count, 0);

    /** @arg After the registration the three forwarding paths pass the arguments and the returns
     * through as they are */
    registerShmClient(fwd);
    CHECK_UNARY(shmClient());

    KRecord record;
    record.datetime = Datetime(202601021500LL);
    record.closePrice = 12.5;
    CHECK_UNARY(forwardRealtimeUpdate("SH600000", KQuery::MIN5, record));
    CHECK_EQ(rec.last_market_code, "SH600000");
    CHECK_EQ(rec.last_rt_ktype, std::string(KQuery::MIN5));
    CHECK_EQ(rec.last_record.closePrice, 12.5);
    CHECK_EQ(rec.rt_count, 1);

    CHECK_UNARY(forwardGetLastUpdateTime("SZ000001", KQuery::DAY) == rec.lu_result);
    CHECK_EQ(rec.last_market_code, "SZ000001");
    CHECK_EQ(rec.last_lu_ktype, std::string(KQuery::DAY));
    CHECK_EQ(rec.lu_count, 1);

    CHECK_UNARY(forwardPullFromBufferServer("ipc://buf", {"SH600000", "SZ000001"}, KQuery::MIN));
    CHECK_EQ(rec.last_addr, "ipc://buf");
    CHECK_EQ(rec.last_codes.size(), 2);
    CHECK_EQ(rec.last_pull_ktype, std::string(KQuery::MIN));
    CHECK_EQ(rec.pull_count, 1);

    /** @arg When the implementer fails the thin layer passes it on faithfully, neither swallowing
     * nor rewriting it */
    rec.rt_result = false;
    rec.pull_result = false;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {}, KQuery::DAY));

    /** @arg After the unregistration (an empty callback group) it becomes a no-op again and the
     * unregistration is repeatable */
    registerShmClient(ShmClientForwarders());
    CHECK_UNARY(!shmClient());
    int rt_before = rec.rt_count;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_EQ(rec.rt_count, rt_before);
    registerShmClient(ShmClientForwarders());
    CHECK_UNARY(!shmClient());
}

/** @par Test points */
TEST_CASE("test_ShmClientHook_interrupt") {
    HKU_IF_RETURN(!pluginValid(), void());

    /** @arg false is returned when no checker is registered */
    setInterruptChecker(nullptr);
    CHECK_UNARY(!checkInterrupted());

    /** @arg After the registration the check result is reflected faithfully (for the plugin side
     * waiting loop callback) */
    setInterruptChecker([]() { return false; });
    CHECK_UNARY(!checkInterrupted());
    setInterruptChecker([]() { return true; });
    CHECK_UNARY(checkInterrupted());

    /** @arg Restore it to an empty checker, avoiding polluting the other cases in the same process
     */
    setInterruptChecker(nullptr);
    CHECK_UNARY(!checkInterrupted());
}

/** @} */
