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
#include <atomic>
#include <chrono>
#include <limits>
#include <filesystem>
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

// 地址与文件锁构造统一走生产函数 ipc::makeIpcServerPaths（含 Windows 命名管道平台适配），
// 使单测覆盖真实的生产地址构造路径——回归防护：Windows 下 ipc:// 若含盘符/反斜杠，
// CreateNamedPipeA 必然失败（见评审 C2），而单测曾自行拼地址绕过该路径而漏测。
// 以 name + PID 作为派生哈希的输入，保证各用例、各测试进程间地址互不冲突。
static std::string test_ipc_addr(const std::string& name) {
    std::string addr, lock_path;
    makeIpcServerPaths(name + "_" + std::to_string(HKU_TEST_GETPID()), addr, lock_path);
    return addr;
}

static std::string test_lock_path(const std::string& name) {
    std::string addr, lock_path;
    makeIpcServerPaths(name + "_" + std::to_string(HKU_TEST_GETPID()), addr, lock_path);
    return lock_path;
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

TEST_CASE("test_IpcReaderCountBound") {
    // getCount / getCount32 须校验元素个数不超过剩余字节所能容纳的上限：
    // 坏帧或版本错配给出的天文数字若被直接用于 resize/reserve，会触发
    // length_error/bad_alloc；且超界必须置失败，否则调用方会把“坏帧”误读为“成功但集合为空”
    SUBCASE("count within bound") {
        Encoder enc;
        enc.putU64(2);
        // 每条记录最小 8 字节，补足 24 字节足以容纳 2 条
        for (int i = 0; i < 24; i++) {
            enc.putU8(0);
        }
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount(8), 2);
        CHECK(rd.ok());
    }

    SUBCASE("count exceeds bound") {
        Encoder enc;
        enc.putU64(1000);
        enc.putU8(0);  // 剩余 1 字节，容纳不下 1000 条
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount(8), 0);
        CHECK_FALSE(rd.ok());
    }

    SUBCASE("astronomic count") {
        Encoder enc;
        enc.putU64(std::numeric_limits<uint64_t>::max());
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount(8), 0);
        CHECK_FALSE(rd.ok());
    }

    SUBCASE("u32 variant") {
        Encoder enc;
        enc.putU32(std::numeric_limits<uint32_t>::max());
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount32(8), 0);
        CHECK_FALSE(rd.ok());

        Encoder enc2;
        enc2.putU32(3);
        for (int i = 0; i < 24; i++) {
            enc2.putU8(0);
        }
        Reader rd2(enc2.data().data(), enc2.data().size());
        CHECK_EQ(rd2.getCount32(8), 3);
        CHECK(rd2.ok());
    }

    SUBCASE("elem_min_size zero skips check") {
        // 变长元素（如字符串列表）给不出最小字节数，传 0 表示不做上界校验
        Encoder enc;
        enc.putU64(1000);
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount(0), 1000);
        CHECK(rd.ok());
    }

    SUBCASE("truncated count field") {
        // count 字段自身被截断：getU64 越界置失败，getCount 返回 0
        Encoder enc;
        enc.putU32(1);  // 仅 4 字节，不足 u64
        Reader rd(enc.data().data(), enc.data().size());
        CHECK_EQ(rd.getCount(8), 0);
        CHECK_FALSE(rd.ok());
    }
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
        ws.emplace_back(
          StockWeight(Datetime(202006010000LL), 0.1, 0.2, 0.3, 0.4, 0.5, 1000, 900, 0.6));
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
            CHECK(reader.tryGetKRecordList(
              mc, KQuery((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY), shm_ks));
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
        auto full =
          StockManager::instance().getStock(sample_mc).getKRecordListFromBuffer(KQuery::DAY);
        Datetime mid_date = full[full.size() / 2].datetime;
        CHECK(reader.tryGetIndexRangeByDate(
          sample_mc, KQueryByDate(mid_date, Null<Datetime>(), KQuery::DAY), start_ix, end_ix));
        CHECK_EQ(start_ix, full.size() / 2);
        CHECK_EQ(end_ix, full.size());

        KRecordList by_date;
        CHECK(reader.tryGetKRecordList(
          sample_mc, KQueryByDate(mid_date, Null<Datetime>(), KQuery::DAY), by_date));
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

    SUBCASE("realtime mirror") {
        // 验证实时镜像写入：末根更新/追加/过期忽略/预留区写满冻结，
        // 以及并发读写下 seqlock 的无撕裂一致性
        KDataShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260902);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));

        auto& sm = StockManager::instance();
        Stock sample;
        for (const auto& stk : sm.getStockList(nullptr)) {
            if (stk.getKDataBufferSize(KQuery::DAY) > 3) {
                sample = stk;
                break;
            }
        }
        REQUIRE_FALSE(sample.isNull());
        auto buf_ks = sample.getKRecordListFromBuffer(KQuery::DAY);
        REQUIRE_FALSE(buf_ks.empty());
        const std::string mc = sample.market_code();
        size_t base_count = buf_ks.size();
        Datetime last_dt = buf_ks.back().datetime;

        size_t count = 0;
        REQUIRE(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count);

        // 未覆盖证券/类型的镜像更新：静默跳过
        shmMirrorRealtimeUpdate("SH999999", KQuery::DAY, KRecord(last_dt, 1, 1, 1, 1, 1, 1));
        shmMirrorRealtimeUpdate(mc, KQuery::MIN, KRecord(last_dt, 1, 1, 1, 1, 1, 1));
        CHECK(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count);

        // 末根同日更新：收/量/额覆盖，高取大、低取小（与 realtimeUpdate 规则一致）
        shmMirrorRealtimeUpdate(
          mc, KQuery::DAY,
          KRecord(last_dt, buf_ks.back().openPrice, buf_ks.back().highPrice + 1.0,
                  buf_ks.back().lowPrice - 1.0, 12.34, 555.0, 666.0));
        KRecordList ks;
        REQUIRE(reader.tryGetKRecordList(
          mc, KQuery((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY), ks));
        REQUIRE_EQ(ks.size(), base_count);
        CHECK_EQ(ks.back().closePrice, 12.34);
        CHECK_EQ(ks.back().highPrice, buf_ks.back().highPrice + 1.0);
        CHECK_EQ(ks.back().lowPrice, buf_ks.back().lowPrice - 1.0);
        CHECK_EQ(ks.back().transAmount, 555.0);
        CHECK_EQ(ks.back().transCount, 666.0);

        // 追加：DAY 预留容量为 2，前两笔成功，第三笔被阻挡（冻结语义，
        // 现有部署每日定时重启重新发布即恢复）
        Datetime dt1 = last_dt + TimeDelta(1);
        Datetime dt2 = last_dt + TimeDelta(2);
        Datetime dt3 = last_dt + TimeDelta(3);
        shmMirrorRealtimeUpdate(mc, KQuery::DAY, KRecord(dt1, 10.0, 10.5, 9.5, 10.2, 100.0, 200.0));
        CHECK(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count + 1);
        shmMirrorRealtimeUpdate(mc, KQuery::DAY, KRecord(dt2, 10.1, 10.6, 9.6, 10.3, 101.0, 201.0));
        CHECK(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count + 2);
        shmMirrorRealtimeUpdate(mc, KQuery::DAY, KRecord(dt3, 10.2, 10.7, 9.7, 10.4, 103.0, 203.0));
        CHECK(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count + 2);  // 预留区满，追加被阻挡

        // 预留区满后末根更新仍生效
        shmMirrorRealtimeUpdate(mc, KQuery::DAY, KRecord(dt2, 10.1, 11.0, 9.0, 10.9, 102.0, 202.0));
        REQUIRE(reader.tryGetKRecordList(
          mc, KQuery((int64_t)(base_count + 1), (int64_t)(base_count + 2), KQuery::DAY), ks));
        REQUIRE_EQ(ks.size(), 1);
        CHECK_EQ(ks[0].closePrice, 10.9);
        CHECK_EQ(ks[0].highPrice, 11.0);

        // 过期记录被忽略
        shmMirrorRealtimeUpdate(mc, KQuery::DAY, KRecord(dt1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
        CHECK(reader.tryGetCount(mc, KQuery::DAY, count));
        CHECK_EQ(count, base_count + 2);

        // 日期查询覆盖镜像追加区
        size_t start_ix = 0, end_ix = 0;
        CHECK(reader.tryGetIndexRangeByDate(mc, KQueryByDate(dt1, Null<Datetime>(), KQuery::DAY),
                                            start_ix, end_ix));
        CHECK_EQ(start_ix, base_count);
        CHECK_EQ(end_ix, base_count + 2);

        // 并发读写：写者持续更新末根（close 与 transCount 同值写入），
        // 读者验证两字段恒相等，检验 seqlock 无撕裂读
        // 前置：先将末根 close 与 transCount 置为同值，否则读者会以初始不等状态误判撕裂
        shmMirrorRealtimeUpdate(mc, KQuery::DAY,
                                KRecord(dt2, 10.1, 11.0, 9.0, 102.0, 102.0, 102.0));
        std::atomic<bool> stop{false};
        std::atomic<bool> torn{false};
        std::atomic<size_t> reads{0};
        std::thread writer([&]() {
            for (int i = 0; i < 2000; i++) {
                double v = (double)(i % 97) + 1.0;
                shmMirrorRealtimeUpdate(mc, KQuery::DAY,
                                        KRecord(dt2, 10.1, 11.0, 9.0, v, 102.0, v));
            }
            stop.store(true);
        });
        while (!stop.load()) {
            KRecordList tail;
            if (reader.tryGetKRecordList(
                  mc, KQuery((int64_t)(base_count + 1), (int64_t)(base_count + 2), KQuery::DAY),
                  tail) &&
                tail.size() == 1) {
                reads++;
                if (tail[0].closePrice != tail[0].transCount) {
                    torn.store(true);
                    break;
                }
            }
        }
        writer.join();
        CHECK_FALSE(torn);
        CHECK_GT(reads.load(), 0);  // 确保读者确实取到快照，避免重试全失败导致假通过
    }

    SUBCASE("invalid segment") {
        KDataShmReader reader;
        CHECK_FALSE(reader.open("hkushm_not_exist_0123456789"));
        CHECK_FALSE(reader.valid());
    }

    SUBCASE("oversized market_code") {
        // 索引项的 market_code 为定长字段，超长 key 必须在二分查找前被拒绝：
        // 截断后可能与其他证券重名而返回错误数据（发布端同理跳过超长证券）
        KDataShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260904);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));

        size_t count = 12345;
        const std::string too_long(sizeof(ShmStockEntry::market_code) + 8, 'X');
        CHECK_FALSE(reader.tryGetCount(too_long, KQuery::DAY, count));
        CHECK_EQ(count, 12345);  // 失败时不得写出参

        size_t start_ix = 1, end_ix = 1;
        CHECK_FALSE(reader.tryGetIndexRangeByDate(
          too_long, KQueryByDate(Datetime(199001010000LL), Null<Datetime>(), KQuery::DAY), start_ix,
          end_ix));

        KRecordList ks;
        CHECK_FALSE(reader.tryGetKRecordList(
          too_long, KQuery((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY), ks));

        // 恰好等于字段长度的 key 亦不可容纳（需留 NUL 结束符），同样应被拒绝而非误匹配
        const std::string exact_len(sizeof(ShmStockEntry::market_code), 'Y');
        CHECK_FALSE(reader.tryGetCount(exact_len, KQuery::DAY, count));
    }

    SUBCASE("ktype coverage") {
        // ktype 表须反映主进程的预加载配置（测试配置仅 day=True），
        // 客户端据此判定某类型在主进程侧是否存在实时更新链路，从而决定是否直接走本地驱动
        KDataShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260903);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));
        CHECK(reader.coversKType(KQuery::DAY));
        // 大小写不敏感（表内统一存储大写）
        CHECK(reader.coversKType("day"));
        CHECK_FALSE(reader.coversKType(KQuery::MIN));
        CHECK_FALSE(reader.coversKType(KQuery::WEEK));
        // 标准配置不预加载分时/分笔，故不进表；客户端将直接走本地驱动
        CHECK_FALSE(reader.coversKType(KQuery::TIMELINE));
        CHECK_FALSE(reader.coversKType(KQuery::TRANS));

        // 段未映射时一律返回 false，调用方不得据此判定“主进程未预加载”
        reader.close();
        CHECK_FALSE(reader.coversKType(KQuery::DAY));
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

        // 扩展：基础信息快照字段（向后兼容，旧客户端解析到 kdata 部分即止）；
        // 发布基础信息快照后握手应返回非空 bi_epoch / bi_name，且可被映射读取。
        CHECK(server.publishBaseInfoShm(prefix));
        REQUIRE(conn.request(Cmd::STATUS_SHM_INFO, {}, res_body));
        Reader rd_bi(res_body.data(), res_body.size());
        rd_bi.getU64();  // kdata_epoch
        rd_bi.getString();  // kdata_name
        uint64_t bi_epoch = rd_bi.getU64();
        std::string bi_name = rd_bi.getString();
        CHECK(rd_bi.ok());
        CHECK_GT(bi_epoch, 0);
        REQUIRE_FALSE(bi_name.empty());

        BaseInfoShmReader bi_reader;
        CHECK(bi_reader.open(bi_name));
        CHECK_EQ(bi_reader.epoch(), bi_epoch);

        server.stop();
    }
}

TEST_CASE("test_BaseInfoShmCache") {
    // 与 test_KDataShmCache 同构：以 StockManager 已加载的权息/历史财务为基准，
    // 验证基础信息（权息 + 历史财务）共享内存快照的发布与只读映射查询语义；
    // 段名前缀需较短（系统共享内存名长度限制）。预加载为后台异步，必须先等待完成。
    StockManager::instance().waitDataReady();
    const std::string prefix = "hkubshm";

    SUBCASE("publish and read weight") {
        BaseInfoShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260901);
        REQUIRE_FALSE(name.empty());

        BaseInfoShmReader reader;
        REQUIRE(reader.open(name));
        CHECK_EQ(reader.epoch(), 20260901);

        auto& sm = StockManager::instance();

        // 测试数据集 stkWeight 含 3 万余行，权息表应被建表
        CHECK(reader.coversTable(SHM_BI_TABLE_WEIGHT));

        size_t checked = 0, weight_mismatch = 0, expect_weight_stocks = 0;
        for (const auto& stk : sm.getStockList(nullptr)) {
            const StockWeightList& expect = stk.getWeight();
            StockWeightList actual;
            if (expect.empty()) {
                // 无权息的证券不在快照中（发布端跳过空表项），查询应返回 false
                CHECK_FALSE(reader.tryGetWeightList(stk.market_code(), Datetime::min(),
                                                    Null<Datetime>(), actual));
                continue;
            }
            expect_weight_stocks++;
            CHECK(reader.tryGetWeightList(stk.market_code(), Datetime::min(), Null<Datetime>(),
                                          actual));
            if (expect.size() != actual.size()) {
                weight_mismatch++;
                continue;
            }
            for (size_t i = 0; i < expect.size(); i++) {
                if (expect[i].datetime() != actual[i].datetime() ||
                    expect[i].countAsGift() != actual[i].countAsGift() ||
                    expect[i].countForSell() != actual[i].countForSell() ||
                    expect[i].priceForSell() != actual[i].priceForSell() ||
                    expect[i].bonus() != actual[i].bonus() ||
                    expect[i].increasement() != actual[i].increasement() ||
                    expect[i].totalCount() != actual[i].totalCount() ||
                    expect[i].freeCount() != actual[i].freeCount() ||
                    expect[i].suogu() != actual[i].suogu()) {
                    weight_mismatch++;
                    break;
                }
            }
            checked++;
        }
        CHECK_EQ(weight_mismatch, 0);
        HKU_INFO("BaseInfoShmCache weight check: {} stocks with weight checked", checked);

        // 全量比对：快照内证券条目数应大于 0（且等于有权息的证券数）
        std::unordered_map<std::string, StockWeightList> all;
        CHECK(reader.tryGetAllWeightList(all));
        CHECK_GT(all.size(), 0);
        CHECK_EQ(all.size(), expect_weight_stocks);

        // 未覆盖的证券应返回 false（由上层回退 IPC/本地）
        StockWeightList dummy;
        CHECK_FALSE(reader.tryGetWeightList("SH999999", Datetime::min(), Null<Datetime>(), dummy));

        // 区间过滤语义须与主进程 Stock::getWeight(start, end) 一致（[start, end)、按完整日期比较），
        // 否则快照命中会比服务端 IPC 应答多/少返回记录
        for (const auto& stk : sm.getStockList(nullptr)) {
            auto full = stk.getWeight();
            if (full.size() < 3) {
                continue;
            }
            Datetime sub_start = full[1].datetime();
            Datetime sub_end = full[full.size() - 1].datetime();
            auto expect = stk.getWeight(sub_start, sub_end);
            StockWeightList actual;
            REQUIRE(reader.tryGetWeightList(stk.market_code(), sub_start, sub_end, actual));
            REQUIRE_GT(expect.size(), 0);
            REQUIRE_EQ(actual.size(), expect.size());
            CHECK_EQ(actual.front().datetime(), expect.front().datetime());
            CHECK_EQ(actual.back().datetime(), expect.back().datetime());

            // 空区间（start >= end）：与 Stock::getWeight 一致，命中但结果为空
            StockWeightList empty_range;
            CHECK(reader.tryGetWeightList(stk.market_code(), sub_end, sub_start, empty_range));
            CHECK(empty_range.empty());
            CHECK(stk.getWeight(sub_end, sub_start).empty());
            break;
        }

        reader.close();
        CHECK_FALSE(reader.valid());
    }

    SUBCASE("finance gated by data availability") {
        // 测试数据集 HistoryFinance 表为空，故历史财务表不应被建表（与 load_history_finance
        // 门控语义一致）；同时验证 coversTable 在表缺失时返回 false 且段映射仍有效。
        BaseInfoShmPublisher publisher(prefix);
        std::string name = publisher.publish(20260902);
        REQUIRE_FALSE(name.empty());

        BaseInfoShmReader reader;
        REQUIRE(reader.open(name));

        bool any_finance = false;
        for (const auto& stk : StockManager::instance().getStockList(nullptr)) {
            if (!stk.getHistoryFinance().empty()) {
                any_finance = true;
                break;
            }
        }
        CHECK_EQ(reader.coversTable(SHM_BI_TABLE_FINANCE), any_finance);
        std::vector<HistoryFinanceInfo> dummy;
        CHECK_FALSE(
          reader.tryGetHistoryFinance("SH600000", Datetime::min(), Null<Datetime>(), dummy));
        CHECK_FALSE(reader.tryGetHistoryFinance(
          "SH600000", Datetime(202001010000LL), Datetime(202101010000LL), dummy));

        reader.close();
    }

    SUBCASE("republish with new epoch") {
        // 历史财务就绪后重发布（以新代数重建整段），客户端经 epoch 变化感知；
        // 退化场景下旧映射在重发布（旧段被删）后仍可读取。
        BaseInfoShmPublisher publisher(prefix);
        std::string n1 = publisher.publish(20260901);
        BaseInfoShmReader r1;
        REQUIRE(r1.open(n1));  // 必须在重发布前映射，否则 Windows 下旧段名已被删无法再打开
        std::string n2 = publisher.publish(20260902);
        BaseInfoShmReader r2;
        REQUIRE(r2.open(n2));
        CHECK_NE(n1, n2);
        CHECK_EQ(r1.epoch(), 20260901);
        CHECK_EQ(r2.epoch(), 20260902);
        CHECK(r1.valid());  // 旧映射在段删除后仍可读（快照语义）
        CHECK(r2.valid());
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
    auto elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_tp)
        .count();
    CHECK_LT(elapsed, 5);
    setInterruptChecker(nullptr);

    server.stop();
}

#endif  // HKU_ENABLE_NODE
