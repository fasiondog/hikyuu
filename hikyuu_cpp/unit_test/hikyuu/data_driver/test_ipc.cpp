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
#include <memory>
#include <algorithm>
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
#include "hikyuu/data_driver/ipc/IpcProxyDrivers.h"
#include "hikyuu/data_driver/ipc/KDataShmCache.h"
#include "hikyuu/plugin/dataserver.h"
#include "hikyuu/utilities/node/NodeServer.h"

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

TEST_CASE("test_IpcDatetimeFullPrecision") {
    // putDatetime/getDatetime 经 number() 仅精确到分钟；encodeDatetimeFull/decodeDatetimeFull
    // 须保留秒/毫秒/微秒（getLastUpdateTime 返回的墙钟时刻依赖此精度）
    SUBCASE("minute-precision codec drops sub-minute") {
        Datetime d(2026, 9, 4, 17, 55, 9, 781, 172);
        Encoder enc;
        enc.putDatetime(d);
        Reader rd(enc.data().data(), enc.data().size());
        Datetime got = rd.getDatetime();
        // 分钟及以上一致，秒/毫秒/微秒被截断为 0
        CHECK_EQ(got.ymdhm(), d.ymdhm());
        CHECK_EQ(got.second(), 0);
    }

    SUBCASE("full-precision codec round-trips exactly") {
        Datetime d(2026, 9, 4, 17, 55, 9, 781, 172);
        Encoder enc;
        encodeDatetimeFull(enc, d);
        Reader rd(enc.data().data(), enc.data().size());
        Datetime got = decodeDatetimeFull(rd);
        CHECK(rd.ok());
        CHECK_EQ(got, d);
        CHECK_EQ(got.second(), 9);
        CHECK_EQ(got.millisecond(), 781);
        CHECK_EQ(got.microsecond(), 172);
    }

    SUBCASE("now / min / null round-trip") {
        // now() 含微秒，往返须精确相等
        Datetime n = Datetime::now();
        Encoder e1;
        encodeDatetimeFull(e1, n);
        Reader r1(e1.data().data(), e1.data().size());
        CHECK_EQ(decodeDatetimeFull(r1), n);

        // min() 为 getLastUpdateTime 的降级哨兵值
        Encoder e2;
        encodeDatetimeFull(e2, Datetime::min());
        Reader r2(e2.data().data(), e2.data().size());
        CHECK_EQ(decodeDatetimeFull(r2), Datetime::min());

        // Null<Datetime> 往返仍为 null
        Encoder e3;
        encodeDatetimeFull(e3, Datetime());
        Reader r3(e3.data().data(), e3.data().size());
        CHECK(decodeDatetimeFull(r3).isNull());
    }
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

TEST_CASE("test_IpcConnectorWaitReady") {
    std::string addr = test_ipc_addr("hku_wait_test");
    // 以裸 IpcServer 复现服务端 STATUS_READY 应答（原依赖的 HikyuuDataServer 已迁至 shmserver
    // 插件）
    std::atomic<bool> ready{false};
    IpcServer server(addr);
    server.setHandler(
      [&ready](Cmd cmd, std::vector<uint8_t>&&, RetCode& ret) -> std::vector<uint8_t> {
          Encoder enc;
          if (cmd == Cmd::STATUS_READY) {
              bool r = ready.load();
              enc.putU8(r ? 1 : 0);
              enc.putU64(r ? 100 : 0);
              enc.putU64(100);
          } else {
              ret = RetCode::ERROR;
              enc.putString("unexpected cmd");
          }
          return enc.data();
      });
    server.start();

    IpcConnector conn;
    CHECK(conn.init(addr));

    // 服务端尚未就绪，客户端等待超时
    CHECK_FALSE(conn.waitReady(2));

    // 后台置为就绪后，等待成功
    std::thread t([&ready]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ready.store(true);
    });
    CHECK(conn.waitReady(10));
    t.join();

    server.stop();
}

// 从 StockManager 已预加载的 DAY 缓冲收集 Builder 输入（替代旧 Publisher.publish 的自动收集）。
// Builder 不做排序，而 Reader 依赖 market_code 升序二分，故此处显式按 market_code 升序排列。
static std::vector<KDataShmBuildKType> collectDayKtypesForBuild() {
    KDataShmBuildKType day;
    day.ktype = KQuery::DAY;
    auto& sm = StockManager::instance();
    for (const auto& stk : sm.getStockList(nullptr)) {
        auto records = stk.getKRecordListFromBuffer(KQuery::DAY);
        if (records.empty()) {
            continue;
        }
        KDataShmBuildEntry entry;
        entry.market_code = stk.market_code();
        entry.records = std::move(records);
        entry.reserved = 1;  // 与生产发布一致，预留 1 个交易日镜像区
        day.entries.push_back(std::move(entry));
    }
    std::sort(day.entries.begin(), day.entries.end(),
              [](const KDataShmBuildEntry& a, const KDataShmBuildEntry& b) {
                  return a.market_code < b.market_code;
              });
    return {day};
}

TEST_CASE("test_KDataShmCache") {
    // 测试配置预加载 day，以其缓冲为基准验证共享内存快照的发布与只读映射查询语义；
    // 段名前缀需较短（系统共享内存名长度限制）；
    // 预加载为后台异步，必须先等待完成，否则缓冲可能为空或不完整。
    StockManager::instance().waitDataReady();
    const std::string prefix = "hkushm";

    SUBCASE("publish and read") {
        KDataShmBuilder builder(prefix);
        auto day_ktypes = collectDayKtypesForBuild();
        std::string name = builder.build(20260901, day_ktypes);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));
        CHECK_EQ(reader.epoch(), 20260901);
        CHECK_GT(reader.coveredCount(), 0);

        // 未覆盖的证券/类型应返回 false（由调用方回退）
        size_t count = 0;
        CHECK_FALSE(reader.tryGetCount("SH999999", KQuery::DAY, count));
        CHECK_FALSE(reader.tryGetCount("SH600000", KQuery::MIN, count));

        // 逐一比对已缓冲证券：条数与全量记录一致（快照即预加载缓冲的镜像）
        auto& sm = StockManager::instance();
        size_t checked = 0;
        std::string sample_mc;
        for (const auto& stk : sm.getStockList(nullptr)) {
            auto buf_ks = stk.getKRecordListFromBuffer(KQuery::DAY);
            if (buf_ks.empty()) {
                continue;
            }
            const std::string mc = stk.market_code();
            size_t shm_count = 0;
            CHECK(reader.tryGetCount(mc, KQuery::DAY, shm_count));
            CHECK_EQ(shm_count, buf_ks.size());

            KRecordList shm_ks;
            // 注意：必须显式 int64_t，否则 Null<int64_t> 会隐式转换选中 KQuery 的日期重载
            CHECK(reader.tryGetKRecordList(
              mc, KQuery((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY), shm_ks));
            REQUIRE_EQ(shm_ks.size(), buf_ks.size());
            if (!shm_ks.empty()) {
                CHECK_EQ(shm_ks.front().datetime, buf_ks.front().datetime);
                CHECK_EQ(shm_ks.front().closePrice, buf_ks.front().closePrice);
                CHECK_EQ(shm_ks.back().datetime, buf_ks.back().datetime);
                CHECK_EQ(shm_ks.back().closePrice, buf_ks.back().closePrice);
            }
            if (sample_mc.empty() && buf_ks.size() > 3) {
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

    SUBCASE("krecord view (zero-copy)") {
        // 裸指针视图：内容与同区间 tryGetKRecordList 逐字段一致；越界/未覆盖/空区间返回
        // false；mirrorUpdate 追加后视图 count 随 readCount 增长、末根反映镜像（视图读的是实时段）
        KDataShmBuilder builder(prefix);
        auto day_ktypes = collectDayKtypesForBuild();
        std::string name = builder.build(20260908, day_ktypes);
        REQUIRE_FALSE(name.empty());

        KDataShmReader reader;
        REQUIRE(reader.open(name));

        auto& sm = StockManager::instance();
        Stock sample;
        KRecordList buf_ks;
        for (const auto& stk : sm.getStockList(nullptr)) {
            buf_ks = stk.getKRecordListFromBuffer(KQuery::DAY);
            if (buf_ks.size() > 3) {
                sample = stk;
                break;
            }
        }
        REQUIRE_FALSE(sample.isNull());
        const std::string mc = sample.market_code();
        size_t total = 0;
        REQUIRE(reader.tryGetCount(mc, KQuery::DAY, total));
        REQUIRE_GT(total, 3);

        // 全区间视图与同区间拷贝列表逐字段一致（datetime/OHLC/额/量）
        const KRecord* data = nullptr;
        size_t count = 0;
        REQUIRE(reader.tryGetKRecordView(mc, KQuery::DAY, 0, total, data, count));
        REQUIRE_NE(data, nullptr);
        CHECK_EQ(count, total);
        KRecordList list;
        REQUIRE(reader.tryGetKRecordList(
          mc, KQuery((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY), list));
        REQUIRE_EQ(list.size(), count);
        for (size_t i = 0; i < count; i++) {
            CHECK_EQ(data[i].datetime, list[i].datetime);
            CHECK_EQ(data[i].openPrice, list[i].openPrice);
            CHECK_EQ(data[i].highPrice, list[i].highPrice);
            CHECK_EQ(data[i].lowPrice, list[i].lowPrice);
            CHECK_EQ(data[i].closePrice, list[i].closePrice);
            CHECK_EQ(data[i].transAmount, list[i].transAmount);
            CHECK_EQ(data[i].transCount, list[i].transCount);
        }

        // 子区间 [1,3)：视图为同一映射内偏移（part == data+1，验证记录连续、步长 sizeof(KRecord)）
        const KRecord* part = nullptr;
        size_t part_count = 0;
        REQUIRE(reader.tryGetKRecordView(mc, KQuery::DAY, 1, 3, part, part_count));
        CHECK_EQ(part_count, 2);
        CHECK_EQ(part, data + 1);
        CHECK_EQ(part[0].datetime, list[1].datetime);
        CHECK_EQ(part[1].closePrice, list[2].closePrice);

        // end 越界被钳制到 total
        const KRecord* clamp = nullptr;
        size_t clamp_count = 0;
        REQUIRE(
          reader.tryGetKRecordView(mc, KQuery::DAY, total - 1, total + 1000, clamp, clamp_count));
        CHECK_EQ(clamp_count, 1);

        // 失败路径：未覆盖证券/类型、空区间(start==end / start>end)、起始越界
        const KRecord* bad = nullptr;
        size_t bad_count = 12345;
        CHECK_FALSE(reader.tryGetKRecordView("SH999999", KQuery::DAY, 0, 1, bad, bad_count));
        CHECK_FALSE(reader.tryGetKRecordView(mc, KQuery::MIN, 0, 1, bad, bad_count));
        CHECK_FALSE(reader.tryGetKRecordView(mc, KQuery::DAY, 2, 2, bad, bad_count));
        CHECK_FALSE(reader.tryGetKRecordView(mc, KQuery::DAY, 3, 1, bad, bad_count));
        CHECK_FALSE(reader.tryGetKRecordView(mc, KQuery::DAY, total, total + 1, bad, bad_count));
        CHECK_EQ(bad, nullptr);  // 失败时不写出 data
        CHECK_EQ(bad_count, 0);  // 失败时 count 归零
        // 注：本子用例保持只读零副作用；实时镜像追加/末根更新的“视图读实时段”验证
        // 依赖服务端镜像编排，已随 shmserver 插件迁移，核心库侧不再覆盖。
    }

    SUBCASE("invalid segment") {
        KDataShmReader reader;
        CHECK_FALSE(reader.open("hkushm_not_exist_0123456789"));
        CHECK_FALSE(reader.valid());
    }

    SUBCASE("oversized market_code") {
        // 索引项的 market_code 为定长字段，超长 key 必须在二分查找前被拒绝：
        // 截断后可能与其他证券重名而返回错误数据（发布端同理跳过超长证券）
        KDataShmBuilder builder(prefix);
        auto day_ktypes = collectDayKtypesForBuild();
        std::string name = builder.build(20260904, day_ktypes);
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
        KDataShmBuilder builder(prefix);
        auto day_ktypes = collectDayKtypesForBuild();
        std::string name = builder.build(20260903, day_ktypes);
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
}

TEST_CASE("test_IpcKDataDriverView") {
    // 驱动层零拷贝视图：IpcKDataDriver::tryGetKRecordView 经 _tryRefreshShm 映射服务端发布的
    // 快照后返回裸指针视图（pin 住映射），内容与源缓冲/拷贝路径一致；未覆盖证券/类型、
    // 空区间、起始越界返回 false（由调用方回退 getKRecordList 拷贝路径）。
    StockManager::instance().waitDataReady();
    const std::string prefix = "hkuview";
    const uint64_t epoch = 20260910;
    std::string addr = test_ipc_addr("hku_view_test");

    // 原用例经 HikyuuDataServer.publishShmCache 产段并应答 STATUS_SHM_INFO；服务端已迁至
    // shmserver 插件。此处以核心库 KDataShmBuilder 产段 + 裸 IpcServer 应答 STATUS_SHM_INFO
    // 复现握手，保留对核心客户端代理 IpcKDataDriver「shm 优先 + 零拷贝视图」路径的覆盖。
    KDataShmBuilder builder(prefix);
    auto day_ktypes = collectDayKtypesForBuild();
    std::string seg_name = builder.build(epoch, day_ktypes);
    REQUIRE_FALSE(seg_name.empty());

    IpcServer server(addr);
    server.setHandler([&](Cmd cmd, std::vector<uint8_t>&&, RetCode& ret) -> std::vector<uint8_t> {
        if (cmd != Cmd::STATUS_SHM_INFO) {
            ret = RetCode::ERROR;
            return {};
        }
        Encoder enc;
        enc.putU64(epoch);        // kdata_epoch
        enc.putString(seg_name);  // kdata_name
        enc.putU64(0);            // bi_epoch（本用例不涉及基础信息快照）
        enc.putString("");        // bi_name
        return enc.data();
    });
    server.start();

    auto conn = std::make_shared<IpcConnector>();
    REQUIRE(conn->init(addr));

    // 本地兜底池：视图命中时不使用；DoNothing 驱动仅用于满足构造（未覆盖类型不走本地）
    Parameter param;
    param.set<std::string>("type", "DoNothing");
    auto local_pool = DataDriverFactory::getKDataDriverPool(param);
    REQUIRE(local_pool);

    IpcKDataDriver driver(conn, local_pool);

    auto& sm = StockManager::instance();
    Stock sample;
    KRecordList buf_ks;
    for (const auto& stk : sm.getStockList(nullptr)) {
        buf_ks = stk.getKRecordListFromBuffer(KQuery::DAY);
        if (buf_ks.size() > 3) {
            sample = stk;
            break;
        }
    }
    REQUIRE_FALSE(sample.isNull());
    const size_t total = buf_ks.size();
    const std::string mkt = sample.market(), code = sample.code();

    SUBCASE("view hits shm, matches source buffer and copy path") {
        KRecordView view;
        REQUIRE(driver.tryGetKRecordView(mkt, code, KQuery::DAY, 0, total, view));
        REQUIRE_NE(view.data, nullptr);
        CHECK_EQ(view.count, total);
        CHECK(view.pin != nullptr);  // 映射被 pin 住（跨代存活由 reader shared_ptr 语义保证）

        // 与源缓冲逐字段一致（验证 publish → shm → view 全链路，datetime 二进制共享正确）
        for (size_t i = 0; i < total; i++) {
            CHECK_EQ(view.data[i].datetime, buf_ks[i].datetime);
            CHECK_EQ(view.data[i].openPrice, buf_ks[i].openPrice);
            CHECK_EQ(view.data[i].highPrice, buf_ks[i].highPrice);
            CHECK_EQ(view.data[i].lowPrice, buf_ks[i].lowPrice);
            CHECK_EQ(view.data[i].closePrice, buf_ks[i].closePrice);
            CHECK_EQ(view.data[i].transAmount, buf_ks[i].transAmount);
            CHECK_EQ(view.data[i].transCount, buf_ks[i].transCount);
        }

        // 与拷贝路径（getKRecordList，shm 优先）一致：视图与 memcpy 副本同源
        KRecordList cp =
          driver.getKRecordList(mkt, code, KQuery((int64_t)0, (int64_t)total, KQuery::DAY));
        REQUIRE_EQ(cp.size(), total);
        CHECK_EQ(cp.front().datetime, view.data[0].datetime);
        CHECK_EQ(cp.back().closePrice, view.data[total - 1].closePrice);
    }

    SUBCASE("end_ix clamped, sub-range shares mapping") {
        KRecordView full;
        REQUIRE(driver.tryGetKRecordView(mkt, code, KQuery::DAY, 0, total, full));
        KRecordView tail;
        REQUIRE(driver.tryGetKRecordView(mkt, code, KQuery::DAY, total - 2, total + 1000, tail));
        CHECK_EQ(tail.count, 2);                       // end 越界被钳制到 total
        CHECK_EQ(tail.data, full.data + (total - 2));  // 同一映射内偏移（记录连续）
        CHECK_EQ(tail.data[1].datetime, buf_ks.back().datetime);
    }

    SUBCASE("uncovered / invalid range returns false") {
        KRecordView view;
        CHECK_FALSE(driver.tryGetKRecordView("SH", "999999", KQuery::DAY, 0, 1, view));
        CHECK_FALSE(driver.tryGetKRecordView(mkt, code, KQuery::MIN, 0, 1, view));  // 未覆盖类型
        CHECK_FALSE(driver.tryGetKRecordView(mkt, code, KQuery::DAY, 2, 2, view));  // start==end
        CHECK_FALSE(
          driver.tryGetKRecordView(mkt, code, KQuery::DAY, total, total + 1, view));  // 起始越界
        CHECK_EQ(view.data, nullptr);
        CHECK_EQ(view.count, 0);
    }

    server.stop();
}

TEST_CASE("test_StockWeightClientModeDelegatesToDriver") {
    // 客户端模式下 loadAllStockWeights 跳过本地物化，Stock::getWeight 改为按需经驱动读取
    // （客户端即 shm 优先）。用一只“全新构造、m_weightList 为空”的 Stock 证明委托：主进程
    // 模式下它读自身空缓存返回空；客户端模式下同一对象经驱动取回该证券真实权息，且与
    // 主进程已物化证券的结果逐字段一致（证明客户端不再需要本地物化即可正确读取权息）。
    StockManager::instance().waitDataReady();
    auto& sm = StockManager::instance();

    // 选取一只本地已物化权息（m_weightList 非空）的真实证券作为基准
    Stock dict_stk;
    StockWeightList expect;
    for (const auto& stk : sm.getStockList(nullptr)) {
        auto ws = stk.getWeight();  // 主进程路径：读本地 m_weightList
        if (!ws.empty()) {
            dict_stk = stk;
            expect = std::move(ws);
            break;
        }
    }
    REQUIRE_FALSE(dict_stk.isNull());
    REQUIRE_FALSE(expect.empty());

    // 全新构造的同 market_code Stock：独立 m_data、m_weightList 为空（未被 loadAllStockWeights
    // 物化）
    Stock fresh(dict_stk.market(), dict_stk.code(), "fresh");
    /** @arg 主进程模式：读自身空 m_weightList，返回空（基线，证明 fresh 本地无权息缓存）*/
    CHECK(fresh.getWeight().empty());

    // RAII：无论用例中途 REQUIRE 失败与否，退出时复位客户端模式，避免污染其他用例
    struct ClientModeGuard {
        ~ClientModeGuard() {
            StockManager::instance()._testingSetIpcClientMode(false);
        }
    } guard;
    sm._testingSetIpcClientMode(true);

    /** @arg 客户端模式：fresh 本地缓存为空，getWeight 仍经驱动取回真实权息，与物化基准逐字段一致 */
    StockWeightList actual = fresh.getWeight();
    REQUIRE_EQ(actual.size(), expect.size());
    size_t mismatch = 0;
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
            mismatch++;
        }
    }
    CHECK_EQ(mismatch, 0);

    /** @arg 与直接调用委托入口结果一致（同一驱动路径、同一区间语义）*/
    StockWeightList direct = sm.getStockWeightList(fresh, Datetime::min(), Null<Datetime>());
    CHECK_EQ(direct.size(), actual.size());
}

// 以 StockManager 已加载的权息 / 历史财务为基准收集 Builder 输入（替代旧 Publisher.publish
// 的自动收集与建表门控）。Builder 不排序，Reader 按 market_code 升序二分，故显式排序；
// 历史财务表仅在存在数据时才建表（与旧发布门控一致，测试数据集 HistoryFinance 为空）。
static std::vector<BaseInfoShmBuildTable> collectBaseInfoTablesForBuild() {
    auto& sm = StockManager::instance();
    BaseInfoShmBuildTable weight;
    weight.name = SHM_BI_TABLE_WEIGHT;
    weight.value_count = 0;
    BaseInfoShmBuildTable finance;
    finance.name = SHM_BI_TABLE_FINANCE;
    for (const auto& stk : sm.getStockList(nullptr)) {
        auto w = stk.getWeight();
        if (!w.empty()) {
            BaseInfoShmBuildWeightEntry e;
            e.market_code = stk.market_code();
            e.weights = std::move(w);
            weight.weight_entries.push_back(std::move(e));
        }
        auto fin = stk.getHistoryFinance();
        if (!fin.empty()) {
            BaseInfoShmBuildFinanceEntry e;
            e.market_code = stk.market_code();
            e.finances = std::move(fin);
            finance.finance_entries.push_back(std::move(e));
        }
    }
    std::sort(weight.weight_entries.begin(), weight.weight_entries.end(),
              [](const BaseInfoShmBuildWeightEntry& a, const BaseInfoShmBuildWeightEntry& b) {
                  return a.market_code < b.market_code;
              });
    std::sort(finance.finance_entries.begin(), finance.finance_entries.end(),
              [](const BaseInfoShmBuildFinanceEntry& a, const BaseInfoShmBuildFinanceEntry& b) {
                  return a.market_code < b.market_code;
              });
    std::vector<BaseInfoShmBuildTable> tables;
    if (!weight.weight_entries.empty()) {
        tables.push_back(std::move(weight));
    }
    if (!finance.finance_entries.empty()) {
        finance.value_count =
          static_cast<uint32_t>(finance.finance_entries[0].finances[0].values.size());
        tables.push_back(std::move(finance));
    }
    return tables;
}

TEST_CASE("test_BaseInfoShmCache") {
    // 与 test_KDataShmCache 同构：以 StockManager 已加载的权息/历史财务为基准，
    // 验证基础信息（权息 + 历史财务）共享内存快照的发布与只读映射查询语义；
    // 段名前缀需较短（系统共享内存名长度限制）。预加载为后台异步，必须先等待完成。
    StockManager::instance().waitDataReady();
    const std::string prefix = "hkubshm";

    SUBCASE("publish and read weight") {
        BaseInfoShmBuilder builder(prefix);
        auto bi_tables = collectBaseInfoTablesForBuild();
        std::string name = builder.build(20260901, bi_tables);
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

        // 区间过滤语义须与主进程 Stock::getWeight(start, end) 一致（[start,
        // end)、按完整日期比较）， 否则快照命中会比服务端 IPC 应答多/少返回记录
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
        BaseInfoShmBuilder builder(prefix);
        auto bi_tables = collectBaseInfoTablesForBuild();
        std::string name = builder.build(20260902, bi_tables);
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
        CHECK_FALSE(reader.tryGetHistoryFinance("SH600000", Datetime(202001010000LL),
                                                Datetime(202101010000LL), dummy));

        reader.close();
    }

    SUBCASE("republish with new epoch") {
        // 历史财务就绪后重发布（以新代数重建整段），客户端经 epoch 变化感知；
        // 退化场景下旧映射在重发布（旧段被删）后仍可读取。
        BaseInfoShmBuilder builder(prefix);
        auto bi_tables = collectBaseInfoTablesForBuild();
        std::string n1 = builder.build(20260901, bi_tables);
        BaseInfoShmReader r1;
        REQUIRE(r1.open(n1));  // 必须在重发布前映射，否则 Windows 下旧段名已被删无法再打开
        std::string n2 = builder.build(20260902, bi_tables);
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
    // 以裸 IpcServer 恒返回未就绪复现"服务在但数据未就绪"场景（HikyuuDataServer 已迁插件）
    IpcServer server(addr);
    server.setHandler([](Cmd, std::vector<uint8_t>&&, RetCode&) -> std::vector<uint8_t> {
        Encoder enc;  // 永不就绪
        enc.putU8(0);
        enc.putU64(0);
        enc.putU64(100);
        return enc.data();
    });
    server.start();

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
