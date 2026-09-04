/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-04
 *      Author: fasiondog
 */

#include "StockManager.h"
#include "data_driver/KDataDriver.h"
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
    // 复用 Stock::getIndexRange 将负索引/日期区间解析为正索引 [start, end)
    size_t start = 0, end = 0;
    HKU_IF_RETURN(!stock.getIndexRange(query, start, end), nullptr);
    auto driver = stock.getKDataDirver();
    HKU_IF_RETURN(!driver, nullptr);
    KRecordView view;
    // 驱动不支持视图（非 IPC 代理 / 快照未覆盖 / 需反缩放 / 区间无效）→ 返回 false 回退私有副本
    HKU_IF_RETURN(!driver->getConnect()->tryGetKRecordView(stock.market(), stock.code(),
                                                           query.kType(), start, end, view),
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
