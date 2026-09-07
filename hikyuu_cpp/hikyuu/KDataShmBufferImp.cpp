/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-04
 *      Author: fasiondog
 */

#include "StockManager.h"
#include "data_driver/KDataDriver.h"
#include "data_driver/DriverConnectPool.h"
#include "KDataPrivatedBufferImp.h"
#include "KDataShmBufferImp.h"

namespace hku {

KDataShmBufferImp::KDataShmBufferImp(const Stock& stock, const KQuery& query,
                                     const KRecordView& view, size_t start)
: KDataImp(stock, query),
  m_start(start),
  m_end(start + view.count),
  m_size(view.count),
  m_data(view.data),
  m_pin(view.pin) {}

KDataShmBufferImp::~KDataShmBufferImp() {}

KDataImpPtr KDataShmBufferImp::create(const Stock& stock, const KQuery& query) {
    // 仅客户端模式尝试视图：主进程预加载走 KDataSharedBufferImp，无需再经驱动
    HKU_IF_RETURN(!StockManager::instance().isIpcClientMode(), nullptr);

    auto driver = stock.getKDataDirver();
    HKU_IF_RETURN(!driver, nullptr);

    // 合并为单次 getConnect：原实现分别经 Stock::getIndexRange 与 driver->getConnect()
    // 各取一次连接（两次池 mutex + 两次 refreshShmInfo 排他锁），高并发下锁竞争使 MF 等
    // 多因子全市场计算性能退化约一半；合并后一次连接复用两个操作，锁获取减半。
    auto conn = driver->getConnect();
    HKU_IF_RETURN(!conn, nullptr);

    const auto& market = stock.market();
    const auto& code = stock.code();
    size_t start = 0, end = 0;

    if (KQuery::isBaseKType(query.kType())) {
        if (query.queryType() == KQuery::INDEX) {
            // INDEX 查询：经同一连接获取总数后解析负索引/越界，与 Stock::_getIndexRangeByIndex 语义一致
            size_t total = conn->getCount(market, code, query.kType());
            HKU_IF_RETURN(total == 0, nullptr);

            int64_t startix = query.start();
            if (startix < 0) {
                startix += static_cast<int64_t>(total);
                if (startix < 0) {
                    startix = 0;
                }
            }

            int64_t endix = query.end();
            if (endix == Null<int64_t>()) {
                endix = static_cast<int64_t>(total);
            } else if (endix < 0) {
                endix += static_cast<int64_t>(total);
                if (endix < 0) {
                    endix = 0;
                }
            }

            start = static_cast<size_t>(startix);
            end = static_cast<size_t>(endix);
            if (end > total) {
                end = total;
            }
            HKU_IF_RETURN(start >= end, nullptr);

        } else if (query.queryType() == KQuery::DATE) {
            HKU_IF_RETURN(!conn->getIndexRangeByDate(market, code, query, start, end), nullptr);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }

    KRecordView view;
    // 复用同一连接：tryGetKRecordView 内部的 _tryRefreshShm 在 5s 限流窗口内为快速路径
    // （仅时间戳比较），不会触发额外 IPC；驱动不支持视图时返回 false 回退私有副本拷贝路径
    HKU_IF_RETURN(!conn->tryGetKRecordView(market, code, query.kType(), start, end, view),
                  nullptr);
    return KDataImpPtr(new KDataShmBufferImp(stock, query, view, start));
}

size_t KDataShmBufferImp::getPos(const Datetime& datetime) const noexcept {
    if (empty()) {
        return Null<size_t>();
    }

    size_t mid, low = 0, high = size() - 1;
    while (low <= high) {
        if (datetime > m_data[high].datetime) {
            mid = high + 1;
            break;
        }

        if (m_data[low].datetime >= datetime) {
            mid = low;
            break;
        }

        mid = (low + high) / 2;
        if (datetime > m_data[mid].datetime) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (mid >= size()) {
        return Null<size_t>();
    }

    return m_data[mid].datetime == datetime ? mid : Null<size_t>();
}

DatetimeList KDataShmBufferImp::getDatetimeList() const {
    DatetimeList result(m_size);
    for (size_t i = 0; i < m_size; ++i) {
        result[i] = m_data[i].datetime;
    }
    return result;
}

KDataImpPtr KDataShmBufferImp::getOtherFromSelf(const KQuery& query) const {
    // 派生查询若仍满足视图条件则复用零拷贝视图，否则回退私有副本拷贝路径
    auto imp = create(m_stock, query);
    return imp ? imp : std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
}

} /* namespace hku */
