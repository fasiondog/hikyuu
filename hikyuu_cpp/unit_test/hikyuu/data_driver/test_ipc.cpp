/*
 * test_ipc.cpp
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <thread>
#include <chrono>
#if defined(_WIN32)
#include <process.h>
#define HKU_TEST_GETPID _getpid
#else
#include <unistd.h>
#define HKU_TEST_GETPID getpid
#endif
#include "hikyuu/StockManager.h"
#include "hikyuu/data_driver/ipc/IpcProtocol.h"
#include "hikyuu/data_driver/ipc/IpcTransport.h"
#include "hikyuu/data_driver/ipc/HikyuuDataServer.h"
#include "hikyuu/data_driver/ipc/IpcProxyDrivers.h"
#include "hikyuu/data_driver/ipc/KDataShmCache.h"

using namespace hku;
using namespace hku::ipc;

static std::string test_ipc_addr(const std::string& name) {
    const char* tmp = getenv("TMPDIR");
    std::string dir = (tmp && *tmp) ? std::string(tmp) : "/tmp";
    return "ipc://" + dir + "/" + name + "_" + std::to_string(HKU_TEST_GETPID()) + ".ipc";
}

static std::string test_lock_path(const std::string& name) {
    const char* tmp = getenv("TMPDIR");
    std::string dir = (tmp && *tmp) ? std::string(tmp) : "/tmp";
    return dir + "/" + name + "_" + std::to_string(HKU_TEST_GETPID()) + ".lock";
}

TEST_CASE("test_IpcEncoderReader") {
    Encoder enc;
    enc.putU8(0xAB);
    enc.putU16(0x1234);
    enc.putU32(0xDEADBEEF);
    enc.putU64(0x1234567890ABCDEFULL);
    enc.putI32(-123456);
    enc.putI64(-9876543210LL);
    enc.putDouble(3.1415926);
    enc.putFloat(2.5f);
    enc.putString("中文混合test");
    enc.putDatetime(Datetime(202609011230LL));

    Reader rd(enc.data().data(), enc.data().size());
    CHECK_EQ(rd.getU8(), 0xAB);
    CHECK_EQ(rd.getU16(), 0x1234);
    CHECK_EQ(rd.getU32(), 0xDEADBEEF);
    CHECK_EQ(rd.getU64(), 0x1234567890ABCDEFULL);
    CHECK_EQ(rd.getI32(), -123456);
    CHECK_EQ(rd.getI64(), -9876543210LL);
    CHECK_EQ(rd.getDouble(), doctest::Approx(3.1415926));
    CHECK_EQ(rd.getFloat(), doctest::Approx(2.5f));
    CHECK_EQ(rd.getString(), "中文混合test");
    CHECK_EQ(rd.getDatetime(), Datetime(202609011230LL));
    CHECK(rd.ok());
    CHECK_EQ(rd.remain(), 0);

    // 越界读取应标记失败
    rd.getU8();
    CHECK_FALSE(rd.ok());
}

TEST_CASE("test_IpcFrameCodec") {
    std::vector<uint8_t> body = {1, 2, 3, 4, 5};
    auto req = encodeRequest(Cmd::KDATA_COUNT, body);
    CHECK_EQ(req.size(), REQUEST_HEADER_SIZE + body.size());

    Cmd cmd;
    std::vector<uint8_t> req_body;
    CHECK(decodeRequest(req, cmd, req_body));
    CHECK(cmd == Cmd::KDATA_COUNT);
    CHECK_EQ(req_body, body);

    auto res = encodeResponse(RetCode::LOADING, body);
    CHECK_EQ(res.size(), RESPONSE_HEADER_SIZE + body.size());

    RetCode ret;
    std::vector<uint8_t> res_body;
    CHECK(decodeResponse(res, ret, res_body));
    CHECK(ret == RetCode::LOADING);
    CHECK_EQ(res_body, body);

    // 非法帧
    std::vector<uint8_t> bad = {0, 1, 2};
    CHECK_FALSE(decodeRequest(bad, cmd, req_body));
    CHECK_FALSE(decodeResponse(bad, ret, res_body));
}

TEST_CASE("test_IpcRecordCodec") {
    SUBCASE("KRecordList") {
        KRecordList ks;
        ks.emplace_back(KRecord(Datetime(202608310930LL), 10.0, 10.5, 9.8, 10.2, 12345.0, 678.0));
        ks.emplace_back(KRecord(Datetime(202609010930LL), 10.2, 11.0, 10.1, 10.9, 23456.0, 789.0));
        Encoder enc;
        encodeKRecordList(enc, ks);
        Reader rd(enc.data().data(), enc.data().size());
        auto ks2 = decodeKRecordList(rd);
        CHECK(rd.ok());
        REQUIRE_EQ(ks2.size(), ks.size());
        for (size_t i = 0; i < ks.size(); i++) {
            CHECK_EQ(ks2[i].datetime, ks[i].datetime);
            CHECK_EQ(ks2[i].openPrice, ks[i].openPrice);
            CHECK_EQ(ks2[i].highPrice, ks[i].highPrice);
            CHECK_EQ(ks2[i].lowPrice, ks[i].lowPrice);
            CHECK_EQ(ks2[i].closePrice, ks[i].closePrice);
            CHECK_EQ(ks2[i].transAmount, ks[i].transAmount);
            CHECK_EQ(ks2[i].transCount, ks[i].transCount);
        }
    }

    SUBCASE("StockWeightList") {
        StockWeightList ws;
        ws.emplace_back(StockWeight(Datetime(202006010000LL), 0.1, 0.2, 0.3, 0.4, 0.5, 1000, 900,
                                    0.6));
        Encoder enc;
        encodeStockWeightList(enc, ws);
        Reader rd(enc.data().data(), enc.data().size());
        auto ws2 = decodeStockWeightList(rd);
        CHECK(rd.ok());
        REQUIRE_EQ(ws2.size(), 1);
        CHECK_EQ(ws2[0].datetime(), ws[0].datetime());
        CHECK_EQ(ws2[0].countAsGift(), ws[0].countAsGift());
        CHECK_EQ(ws2[0].suogu(), ws[0].suogu());
    }

    SUBCASE("KQuery") {
        Encoder enc;
        KQuery q1(10, -5, KQuery::MIN5);
        encodeKQuery(enc, q1);
        KQuery q2(Datetime(202001010000LL), Datetime(202601010000LL), KQuery::DAY);
        encodeKQuery(enc, q2);

        Reader rd(enc.data().data(), enc.data().size());
        auto d1 = decodeKQuery(rd);
        CHECK_EQ(d1.queryType(), KQuery::INDEX);
        CHECK_EQ(d1.start(), 10);
        CHECK_EQ(d1.end(), -5);
        CHECK_EQ(d1.kType(), KQuery::MIN5);

        auto d2 = decodeKQuery(rd);
        CHECK(rd.ok());
        CHECK_EQ(d2.queryType(), KQuery::DATE);
        CHECK_EQ(d2.startDatetime(), Datetime(202001010000LL));
        CHECK_EQ(d2.endDatetime(), Datetime(202601010000LL));
        CHECK_EQ(d2.kType(), KQuery::DAY);
    }

    SUBCASE("HolidaySet & FinanceField") {
        std::unordered_set<Datetime> holidays = {Datetime(202601010000LL),
                                                 Datetime(202605010000LL)};
        Encoder enc;
        encodeHolidaySet(enc, holidays);
        Reader rd(enc.data().data(), enc.data().size());
        auto h2 = decodeHolidaySet(rd);
        CHECK(rd.ok());
        CHECK_EQ(h2, holidays);

        std::vector<std::pair<size_t, std::string>> fields = {{0, "每股收益"}, {1, "净利润"}};
        Encoder enc2;
        encodeFinanceField(enc2, fields);
        Reader rd2(enc2.data().data(), enc2.data().size());
        auto f2 = decodeFinanceField(rd2);
        CHECK(rd2.ok());
        CHECK_EQ(f2, fields);
    }
}

TEST_CASE("test_IpcClientServer") {
    std::string addr = test_ipc_addr("hku_ipc_test");

    IpcServer server(addr);
    server.setHandler([](Cmd cmd, std::vector<uint8_t>&& body, RetCode& ret) {
        std::vector<uint8_t> res;
        if (cmd == Cmd::STATUS_READY) {
            Encoder enc;
            enc.putU8(1);
            enc.putU64(100);
            enc.putU64(100);
            res = enc.data();
        } else if (cmd == Cmd::KDATA_COUNT) {
            // 原样回显请求体
            res = std::move(body);
        } else {
            ret = RetCode::ERROR;
            Encoder enc;
            enc.putString("bad cmd");
            res = enc.data();
        }
        return res;
    });
    server.start();

    IpcClient client(addr);
    CHECK(client.dial());
    CHECK(client.connected());

    SUBCASE("STATUS_READY") {
        auto req = encodeRequest(Cmd::STATUS_READY, {});
        std::vector<uint8_t> res_frame;
        CHECK(client.request(req, res_frame));
        RetCode ret;
        std::vector<uint8_t> body;
        CHECK(decodeResponse(res_frame, ret, body));
        CHECK(ret == RetCode::SUCCESS);
        Reader rd(body.data(), body.size());
        CHECK_EQ(rd.getU8(), 1);
        CHECK_EQ(rd.getU64(), 100);
    }

    SUBCASE("echo & error") {
        std::vector<uint8_t> payload = {9, 8, 7, 6};
        auto req = encodeRequest(Cmd::KDATA_COUNT, payload);
        std::vector<uint8_t> res_frame;
        CHECK(client.request(req, res_frame));
        RetCode ret;
        std::vector<uint8_t> body;
        CHECK(decodeResponse(res_frame, ret, body));
        CHECK(ret == RetCode::SUCCESS);
        CHECK_EQ(body, payload);

        req = encodeRequest(Cmd::BLOCK_LOAD, {});
        CHECK(client.request(req, res_frame));
        CHECK(decodeResponse(res_frame, ret, body));
        CHECK(ret == RetCode::ERROR);
    }

    client.close();
    server.stop();
    CHECK_FALSE(server.running());
}

TEST_CASE("test_HikyuuDataServer") {
    std::string addr = test_ipc_addr("hku_ds_test");
    std::string lock_path = test_lock_path("hku_ds_test");

    HikyuuDataServer server1;
    CHECK(server1.start(addr, lock_path, "."));
    CHECK(server1.running());

    // 文件锁已被占用，第二个实例启动应失败
    HikyuuDataServer server2;
    CHECK_FALSE(server2.start(test_ipc_addr("hku_ds_test2"), lock_path, "."));

    SUBCASE("STATUS_READY handshake") {
        IpcConnector conn;
        CHECK(conn.init(addr));

        // 未就绪，request 返回 LOADING（非 SUCCESS）
        std::vector<uint8_t> res_body;
        CHECK_FALSE(conn.request(Cmd::STATUS_READY, {}, res_body));

        // 就绪后返回 SUCCESS，并携带进度信息
        server1.setAllReady();
        CHECK(conn.request(Cmd::STATUS_READY, {}, res_body));
        Reader rd(res_body.data(), res_body.size());
        CHECK_EQ(rd.getU8(), 1);
        CHECK(rd.ok());

        // 全量证券信息应正常返回（与 StockManager 中证券数一致）
        CHECK(conn.request(Cmd::BASE_ALL_STOCK_INFO, {}, res_body));
        Reader rd2(res_body.data(), res_body.size());
        uint64_t stock_count = rd2.getU64();
        CHECK_EQ(stock_count, StockManager::instance().size());
        CHECK(rd2.ok());

        // 日期回环回归：服务端按 8 位 YYYYMMDD 编码，客户端以 *10000 还原后必须与原始起止时间一致；
        // 防止直接写入 12 位 Datetime::number() 导致客户端日期溢出为 Null 的问题复发。
        if (stock_count > 0) {
            StockInfo info = decodeStockInfo(rd2);
            CHECK(rd2.ok());
            Stock expect = StockManager::instance().getStock(info.market + info.code);
            CHECK_FALSE(expect.isNull());
            Datetime start(info.startDate * 10000LL);
            CHECK_EQ(start, expect.startDatetime());
            if (expect.lastDatetime().isNull()) {
                CHECK_EQ(info.endDate, 99999999);
            } else {
                CHECK_EQ(Datetime(info.endDate * 10000LL), expect.lastDatetime());
            }

            // BASE_STOCK_INFO 单证券分支同样适用 8 位编码约定
            Encoder enc2;
            enc2.putString(expect.market_code());
            CHECK(conn.request(Cmd::BASE_STOCK_INFO, enc2.data(), res_body));
            Reader rd3(res_body.data(), res_body.size());
            StockInfo info2 = decodeStockInfo(rd3);
            CHECK(rd3.ok());
            CHECK_EQ(info2.startDate, info.startDate);
            CHECK_EQ(info2.endDate, info.endDate);
        }

        // 未知证券的 K 线查询应返回 ERROR（由服务端抛异常）
        Encoder enc;
        enc.putString("SH999999");
        encodeKQuery(enc, KQuery(0, 10, KQuery::DAY));
        CHECK_FALSE(conn.request(Cmd::KDATA_COUNT, enc.data(), res_body));
    }

    server1.stop();
    CHECK_FALSE(server1.running());

    // 锁释放后可再次启动
    HikyuuDataServer server3;
    CHECK(server3.start(addr, lock_path, "."));
    server3.stop();
}

TEST_CASE("test_IpcConnectorWaitReady") {
    std::string addr = test_ipc_addr("hku_wait_test");
    std::string lock_path = test_lock_path("hku_wait_test");

    HikyuuDataServer server;
    CHECK(server.start(addr, lock_path, "."));

    IpcConnector conn;
    CHECK(conn.init(addr));

    // 服务端尚未就绪，客户端等待超时
    CHECK_FALSE(conn.waitReady(2));

    // 后台置为就绪后，等待成功
    std::thread t([&server]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        server.setAllReady();
    });
    CHECK(conn.waitReady(10));
    t.join();

    server.stop();
}

TEST_CASE("test_KDataShmCache") {
    // 测试配置预加载 day，以其缓冲为基准验证共享内存快照的发布与只读映射查询语义；
    // 段名前缀需较短（系统共享内存名长度限制）；
    // 预加载为后台异步，必须先等待完成，否则缓冲可能为空或不完整。
    StockManager::instance().waitDataReady();
    const std::string prefix = "hkushm";

    SUBCASE("publish and read") {
        KDataShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260901);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));
        CHECK_EQ(reader.epoch(), 20260901);
        CHECK_GT(reader.coveredCount(), 0);

        // 未覆盖的证券/类型应返回 false（由调用方回退）
        size_t count = 0;
        CHECK_FALSE(reader.tryGetCount("SH999999", KQuery::DAY, count));
        CHECK_FALSE(reader.tryGetCount("SH600000", KQuery::MIN, count));

        // 逐一比对已缓冲证券：条数与全量记录一致（发布前提为未截断）
        auto& sm = StockManager::instance();
        size_t checked = 0;
        std::string sample_mc;
        for (const auto& stk : sm.getStockList(nullptr)) {
            size_t buf_size = stk.getKDataBufferSize(KQuery::DAY);
            if (buf_size == 0) {
                continue;
            }
            const std::string mc = stk.market_code();
            size_t shm_count = 0;
            CHECK(reader.tryGetCount(mc, KQuery::DAY, shm_count));
            CHECK_EQ(shm_count, buf_size);

            KRecordList shm_ks;
            // 注意：必须显式 int64_t，否则 Null<int64_t> 会隐式转换选中 KQuery 的日期重载
            CHECK(reader.tryGetKRecordList(mc, KQuery((int64_t)0, (int64_t)Null<int64_t>(),
                                                      KQuery::DAY),
                                           shm_ks));
            auto buf_ks = stk.getKRecordListFromBuffer(KQuery::DAY);
            REQUIRE_EQ(shm_ks.size(), buf_ks.size());
            if (!shm_ks.empty()) {
                CHECK_EQ(shm_ks.front().datetime, buf_ks.front().datetime);
                CHECK_EQ(shm_ks.front().closePrice, buf_ks.front().closePrice);
                CHECK_EQ(shm_ks.back().datetime, buf_ks.back().datetime);
                CHECK_EQ(shm_ks.back().closePrice, buf_ks.back().closePrice);
            }
            if (sample_mc.empty() && buf_size > 3) {
                sample_mc = mc;
            }
            checked++;
        }
        CHECK_EQ(checked, reader.coveredCount());
        REQUIRE_FALSE(sample_mc.empty());

        // 索引区间查询（含越界钳制）
        KRecordList part;
        CHECK(reader.tryGetKRecordList(sample_mc, KQuery(1, 3, KQuery::DAY), part));
        CHECK_EQ(part.size(), 2);
        CHECK(reader.tryGetKRecordList(sample_mc, KQuery(0, 100000000, KQuery::DAY), part));

        // 日期区间查询与主进程缓冲模式一致（_getIndexRangeByDateFromBuffer 语义）
        size_t start_ix = 0, end_ix = 0;
        auto full = StockManager::instance().getStock(sample_mc).getKRecordListFromBuffer(
          KQuery::DAY);
        Datetime mid_date = full[full.size() / 2].datetime;
        CHECK(reader.tryGetIndexRangeByDate(sample_mc,
                                            KQueryByDate(mid_date, Null<Datetime>(), KQuery::DAY),
                                            start_ix, end_ix));
        CHECK_EQ(start_ix, full.size() / 2);
        CHECK_EQ(end_ix, full.size());

        KRecordList by_date;
        CHECK(reader.tryGetKRecordList(sample_mc,
                                       KQueryByDate(mid_date, Null<Datetime>(), KQuery::DAY),
                                       by_date));
        CHECK_EQ(by_date.size(), full.size() - full.size() / 2);

        // 日期区间为空（覆盖但无数据）：返回 false 且不产生结果，由上层按覆盖处理；
        // 未覆盖（不存在的证券）同样返回 false，二者由 IpcKDataDriver 以 tryGetCount 区分
        Datetime before_all = full.front().datetime - TimeDelta(0, 1);
        CHECK_FALSE(reader.tryGetIndexRangeByDate(
          sample_mc, KQueryByDate(Datetime(190001010000LL), before_all, KQuery::DAY), start_ix,
          end_ix));

        reader.close();
        CHECK_FALSE(reader.valid());
    }

    SUBCASE("invalid segment") {
        KDataShmReader reader;
        CHECK_FALSE(reader.open("hkushm_not_exist_0123456789"));
        CHECK_FALSE(reader.valid());
    }

    SUBCASE("server shm info handshake") {
        std::string addr = test_ipc_addr("hku_shm_test");
        std::string lock_path = test_lock_path("hku_shm_test");
        HikyuuDataServer server;
        REQUIRE(server.start(addr, lock_path, "."));

        IpcConnector conn;
        REQUIRE(conn.init(addr));

        // 发布前：epoch 为 0、段名为空，客户端应保持纯 IPC 查询
        std::vector<uint8_t> res_body;
        REQUIRE(conn.request(Cmd::STATUS_SHM_INFO, {}, res_body));
        Reader rd(res_body.data(), res_body.size());
        CHECK_EQ(rd.getU64(), 0);
        CHECK(rd.getString().empty());
        CHECK(rd.ok());

        // 发布后：返回有效段名与代数，且客户端可映射读取
        CHECK(server.publishShmCache(prefix));
        REQUIRE(conn.request(Cmd::STATUS_SHM_INFO, {}, res_body));
        Reader rd2(res_body.data(), res_body.size());
        uint64_t epoch = rd2.getU64();
        std::string name = rd2.getString();
        CHECK(rd2.ok());
        CHECK_GT(epoch, 0);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        CHECK(reader.open(name));
        CHECK_EQ(reader.epoch(), epoch);
        CHECK_GT(reader.coveredCount(), 0);

        // 重发布应生成新代数，旧段被删除但已有映射不受影响；再次发布后读者可打开新段；
        // 新段数据与缓冲一致（条数不变）
        size_t old_covered = reader.coveredCount();
        CHECK(server.publishShmCache(prefix));
        REQUIRE(conn.request(Cmd::STATUS_SHM_INFO, {}, res_body));
        Reader rd3(res_body.data(), res_body.size());
        uint64_t epoch2 = rd3.getU64();
        std::string name2 = rd3.getString();
        CHECK(rd3.ok());
        CHECK_GT(epoch2, epoch);
        CHECK_NE(name2, name);

        KDataShmReader reader2;
        REQUIRE(reader2.open(name2));
        CHECK_EQ(reader2.coveredCount(), old_covered);
        // 旧映射在段删除后仍可读取（快照语义）
        CHECK(reader.valid());
        CHECK_EQ(reader.coveredCount(), old_covered);

        server.stop();
    }
}

TEST_CASE("test_IpcConnectorInterrupt") {
    std::string addr = test_ipc_addr("hku_intr_test");
    std::string lock_path = test_lock_path("hku_intr_test");

    HikyuuDataServer server;
    CHECK(server.start(addr, lock_path, "."));

    IpcConnector conn;
    CHECK(conn.init(addr));

    // 服务永不就绪；设置中断检查器后，等待应立即终止（用于响应 Ctrl+C 等外部中断）
    setInterruptChecker([]() { return true; });
    auto start_tp = std::chrono::steady_clock::now();
    CHECK_FALSE(conn.waitReady(60));
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::steady_clock::now() - start_tp)
                     .count();
    CHECK_LT(elapsed, 5);
    setInterruptChecker(nullptr);

    server.stop();
}

#endif  // HKU_ENABLE_NODE
