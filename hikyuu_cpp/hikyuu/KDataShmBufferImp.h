/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-04
 *      Author: fasiondog
 */

#pragma once

#include "KDataImp.h"

namespace hku {

struct KRecordView;  // 定义见 data_driver/KDataDriver.h

/**
 * 客户端共享内存零拷贝 K 线视图
 * @details 客户端模式下 NO_RECOVER 查询直接以裸指针视图读取主进程发布的共享内存快照，
 * 不拷贝记录（所有客户端共享同一物理页），与主进程 KDataSharedBufferImp 的零拷贝语义对齐。
 * 视图指针由 m_pin（类型擦除持有 KDataShmReader）pin 住映射，epoch 换代后旧映射仍存活。
 * 仅当驱动支持视图（客户端 IPC 代理驱动且快照覆盖、无需反缩放）时经 create() 构造成功，
 * 否则返回空指针，由上层回退 KDataPrivatedBufferImp 拷贝路径。
 * @note 只读映射：data() 的可写重载返回 const_cast 指针仅为接口兼容，写入将崩溃，
 *       与主进程 KDataSharedBufferImp 的“谨慎使用（强制调整数据）”契约属同类风险。
 * @ingroup StockManage
 */
class HKU_API KDataShmBufferImp : public KDataImp {
public:
    /**
     * 工厂：客户端模式下尝试构造共享内存零拷贝视图
     * @param stock 目标证券
     * @param query 查询条件（应为 NO_RECOVER，由调用方保证）
     * @return 成功返回视图 imp；不适用（非客户端/未覆盖/需反缩放/区间无效）返回 nullptr
     */
    static KDataImpPtr create(const Stock& stock, const KQuery& query);

    virtual ~KDataShmBufferImp() override;

    virtual bool empty() const noexcept override {
        return m_size == 0;
    }

    virtual size_t size() const noexcept override {
        return m_size;
    }

    virtual size_t startPos() const override {
        return m_start;
    }

    virtual size_t endPos() const override {
        return m_end;
    }

    virtual size_t lastPos() const override {
        return m_end == 0 ? 0 : m_end - 1;
    }

    virtual size_t getPos(const Datetime& datetime) const noexcept override;

    virtual const KRecord& getKRecord(size_t pos) const noexcept override {
        return pos < m_size ? m_data[pos] : KRecord::NullKRecord;
    }

    virtual const KRecord& front() const override {
        return m_data[0];
    }

    virtual const KRecord& back() const override {
        return m_data[m_size - 1];
    }

    virtual const KRecord* data() const noexcept override {
        return m_data;
    }

    virtual KRecord* data() noexcept override {
        // 只读共享内存映射，写入将崩溃；仅为与 KDataImp 接口兼容（见类注释）
        return const_cast<KRecord*>(m_data);
    }

    virtual DatetimeList getDatetimeList() const override;

    virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

private:
    KDataShmBufferImp(const Stock& stock, const KQuery& query, const KRecordView& view,
                      size_t start);

    size_t m_start{0};
    size_t m_end{0};
    size_t m_size{0};
    const KRecord* m_data{nullptr};
    std::shared_ptr<void> m_pin;  ///< 持有 shm 映射存活（类型擦除的 KDataShmReaderPtr）
};

} /* namespace hku */
