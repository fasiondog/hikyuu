/*
 * test_shm_client_hook.cpp
 *
 *  Created on: 2026-09-07
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <string>
#include <vector>
#include "hikyuu/data_driver/ipc/ShmClientHook.h"

using namespace hku;
using namespace hku::ipc;

namespace {

/** 转发记录器：捕获注册回调的入参与返回，用于验证核心库只保存纯回调、不依赖任何 shm 实现类型 */
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

/** 构造注册到薄转发层的回调组（真实场景由 shmserver 插件 connect 成功后构造并注册） */
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

/** @par 检测点 */
TEST_CASE("test_ShmClientHook_forward") {
    ForwardRecorder rec;
    auto fwd = makeForwarders(rec);

    /** @arg 未注册客户端时（独立模式 / 未安装插件）：转发为安全空操作 */
    CHECK_UNARY(!shmClient());
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {"SH600000"}, KQuery::MIN));
    CHECK_UNARY(forwardGetLastUpdateTime("SH600000", KQuery::DAY) == Datetime::min());
    CHECK_EQ(rec.rt_count, 0);

    /** @arg 注册后三条转发链路按原样透传入参与返回值 */
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

    /** @arg 实现方返回失败时薄层如实传递，不吞掉也不改写 */
    rec.rt_result = false;
    rec.pull_result = false;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {}, KQuery::DAY));

    /** @arg 注销（注册空回调组）后回到空操作，且可重复注销 */
    registerShmClient(ShmClientForwarders());
    CHECK_UNARY(!shmClient());
    int rt_before = rec.rt_count;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_EQ(rec.rt_count, rt_before);
    registerShmClient(ShmClientForwarders());
    CHECK_UNARY(!shmClient());
}

/** @par 检测点 */
TEST_CASE("test_ShmClientHook_interrupt") {
    /** @arg 未注册检查器时返回 false */
    setInterruptChecker(nullptr);
    CHECK_UNARY(!checkInterrupted());

    /** @arg 注册后如实反映检查结果（供插件侧等待循环回调） */
    setInterruptChecker([]() { return false; });
    CHECK_UNARY(!checkInterrupted());
    setInterruptChecker([]() { return true; });
    CHECK_UNARY(checkInterrupted());

    /** @arg 恢复为空检查器，避免污染同进程内其它用例 */
    setInterruptChecker(nullptr);
    CHECK_UNARY(!checkInterrupted());
}

/** @} */

#endif  // HKU_ENABLE_NODE
