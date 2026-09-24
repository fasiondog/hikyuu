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
    // The view is attempted in the client mode only: the preload of the main process uses
    // KDataSharedBufferImp and does not need the driver again
    HKU_IF_RETURN(!StockManager::instance().isIpcClientMode(), nullptr);

    auto driver = stock.getKDataDirver();
    HKU_IF_RETURN(!driver, nullptr);

    // Merged into a single getConnect: the original implementation took a connection through
    // Stock::getIndexRange and driver->getConnect() respectively (two pool mutexes + two exclusive
    // locks of refreshShmInfo); under a high concurrency the lock contention degraded the
    // performance of the whole market calculations such as MF by about half; after the merge a
    // single connection is reused by the two operations and the lock acquisitions are halved.
    auto conn = driver->getConnect();
    HKU_IF_RETURN(!conn, nullptr);

    const auto& market = stock.market();
    const auto& code = stock.code();
    size_t start = 0, end = 0;

    if (KQuery::isBaseKType(query.kType())) {
        if (query.queryType() == KQuery::INDEX) {
            // An INDEX query: the total count is obtained through the same connection and then the
            // negative index / out of range is resolved, the same semantics as
            // Stock::_getIndexRangeByIndex
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
    // Reuse the same connection: tryGetKRecordView only checks the coverage against the snapshot
    // fixed for the session lifetime (see mapSessionShm for the negotiation mapping) and returns a
    // zero-copy view without triggering an extra IPC on a hit; it returns false when the snapshot
    // does not cover it or the driver does not support the view, and the caller falls back to the
    // private copy path of KDataPrivatedBufferImp (fetching through getKRecordList over IPC /
    // locally)
    HKU_IF_RETURN(!conn->tryGetKRecordView(market, code, query.kType(), start, end, view), nullptr);
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
    // If a derived query still meets the view condition the zero-copy view is reused, otherwise it
    // falls back to the private copy path
    auto imp = create(m_stock, query);
    return imp ? imp : std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
}

} /* namespace hku */
