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

/**
 * 桩客户端实现：记录薄转发的入参与返回，用于验证核心库不持有任何 shm 实现，
 * 只在注册后把调用透传给实现方（真正的实现在 shmserver 插件内）
 */
class StubShmClient : public ShmClientInterface {
public:
    bool connect(const std::string&, uint64_t) noexcept override {
        return true;
    }

    std::string serverAddr() const noexcept override {
        return "stub://shm-server";
    }

    KDataDriverPtr createKDataDriver(const KDataDriverConnectPoolPtr&) noexcept override {
        return nullptr;
    }

    BaseInfoDriverPtr createBaseInfoDriver(const BaseInfoDriverPtr& local) noexcept override {
        return local;
    }

    BlockInfoDriverPtr createBlockDriver(const BlockInfoDriverPtr& local) noexcept override {
        return local;
    }

    bool forwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                               const KRecord& record) noexcept override {
        last_market_code = market_code;
        last_rt_ktype = ktype;
        last_record = record;
        ++rt_count;
        return rt_result;
    }

    Datetime forwardGetLastUpdateTime(const std::string& market_code,
                                      const KQuery::KType& ktype) noexcept override {
        last_market_code = market_code;
        last_lu_ktype = ktype;
        ++lu_count;
        return lu_result;
    }

    bool forwardPullFromBufferServer(const std::string& addr, const std::vector<std::string>& codes,
                                     const KQuery::KType& ktype) noexcept override {
        last_addr = addr;
        last_codes = codes;
        last_pull_ktype = ktype;
        ++pull_count;
        return pull_result;
    }

    void disconnect() noexcept override {
        ++disconnect_count;
    }

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
    int disconnect_count{0};
    bool rt_result{true};
    bool pull_result{true};
    Datetime lu_result{202601011200LL};
};

}  // namespace

/**
 * @defgroup test_shm_client_hook_suite test_shm_client_hook_suite
 * @ingroup DataDriver
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ShmClientHook_forward") {
    StubShmClient stub;

    /** @arg 未注册客户端时（独立模式 / 未安装插件）：转发为安全空操作 */
    CHECK_UNARY(!shmClient());
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {"SH600000"}, KQuery::MIN));
    CHECK_UNARY(forwardGetLastUpdateTime("SH600000", KQuery::DAY) == Datetime::min());
    CHECK_EQ(stub.rt_count, 0);

    /** @arg 注册后三条转发链路按原样透传入参与返回值 */
    registerShmClient(&stub);
    CHECK_EQ(shmClient(), &stub);
    CHECK_EQ(stub.serverAddr(), "stub://shm-server");

    KRecord record;
    record.datetime = Datetime(202601021500LL);
    record.closePrice = 12.5;
    CHECK_UNARY(forwardRealtimeUpdate("SH600000", KQuery::MIN5, record));
    CHECK_EQ(stub.last_market_code, "SH600000");
    CHECK_EQ(stub.last_rt_ktype, std::string(KQuery::MIN5));
    CHECK_EQ(stub.last_record.closePrice, 12.5);
    CHECK_EQ(stub.rt_count, 1);

    CHECK_UNARY(forwardGetLastUpdateTime("SZ000001", KQuery::DAY) == stub.lu_result);
    CHECK_EQ(stub.last_market_code, "SZ000001");
    CHECK_EQ(stub.last_lu_ktype, std::string(KQuery::DAY));
    CHECK_EQ(stub.lu_count, 1);

    CHECK_UNARY(forwardPullFromBufferServer("ipc://buf", {"SH600000", "SZ000001"}, KQuery::MIN));
    CHECK_EQ(stub.last_addr, "ipc://buf");
    CHECK_EQ(stub.last_codes.size(), 2);
    CHECK_EQ(stub.last_pull_ktype, std::string(KQuery::MIN));
    CHECK_EQ(stub.pull_count, 1);

    /** @arg 实现方返回失败时薄层如实传递，不吞掉也不改写 */
    stub.rt_result = false;
    stub.pull_result = false;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_UNARY(!forwardPullFromBufferServer("ipc://buf", {}, KQuery::DAY));

    /** @arg 注销（传 nullptr）后回到空操作，且可重复注销 */
    registerShmClient(nullptr);
    CHECK_UNARY(!shmClient());
    int rt_before = stub.rt_count;
    CHECK_UNARY(!forwardRealtimeUpdate("SH600000", KQuery::DAY, KRecord()));
    CHECK_EQ(stub.rt_count, rt_before);
    registerShmClient(nullptr);
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
