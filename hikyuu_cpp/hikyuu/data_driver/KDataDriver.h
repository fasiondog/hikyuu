/*
 * KDataDriver.h
 *
 *  Created on: 2012-9-8
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATADRIVER_H_
#define KDATADRIVER_H_

#include "../utilities/Parameter.h"
#include "../KQuery.h"
#include "../TimeLineRecord.h"
#include "../TransRecord.h"

namespace hku {

/**
 * K线数据驱动基类
 * @ingroup DataDriver
 */
class HKU_API KDataDriver {
    PARAMETER_SUPPORT

public:
    KDataDriver();

    KDataDriver(const Parameter& params);

    /**
     * 构造函数
     * @param name 驱动名称
     */
    KDataDriver(const string& name);
    virtual ~KDataDriver() {}

    /** 获取驱动名称 */
    const string& name() const;

    /** 驱动初始化 */
    bool init(const Parameter&);

    typedef shared_ptr<KDataDriver> KDataDriverPtr;
    /**
     * 克隆实现
     */
    KDataDriverPtr clone();

    /**
     * 子类克隆函数实现
     */
    virtual KDataDriverPtr _clone() = 0;

    /**
     * 子类初始化私有变量接口
     * @return
     */
    virtual bool _init() {
        return true;
    }

    /**
     * 判断该引擎是否是位置索引方式查询速度更快，还是按日期方式查询更快
     */
    virtual bool isIndexFirst() = 0;

    /**
     * 是否支持并行数据加载
     */
    virtual bool canParallelLoad() = 0;

    /**
     * 获取指定类型的K线数据量
     * @param market 市场简称
     * @param code   证券代码
     * @param kType  K线类型
     * @return
     */
    virtual size_t getCount(const string& market, const string& code, const KQuery::KType& kType);

    /**
     * 获取指定日期范围对应的K线记录索引
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     * @param out_start [out] 对应K线记录位置
     * @param out_end [out] 对应的K线记录位置
     * @return
     */
    virtual bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                     size_t& out_start, size_t& out_end);

    /**
     * 获取 K 线数据
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     */
    virtual KRecordList getKRecordList(const string& market, const string& code,
                                       const KQuery& query);

    /**
     * 获取分时线
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     * @return
     */
    virtual TimeLineList getTimeLineList(const string& market, const string& code,
                                         const KQuery& query);

    /**
     * 获取历史分笔数据
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     * @return
     */
    virtual TransList getTransList(const string& market, const string& code, const KQuery& query);

private:
    bool checkType();

private:
    string m_name;
};

typedef shared_ptr<KDataDriver> KDataDriverPtr;

HKU_API std::ostream& operator<<(std::ostream&, const KDataDriver&);
HKU_API std::ostream& operator<<(std::ostream&, const KDataDriverPtr&);

inline const string& KDataDriver::name() const {
    return m_name;
}

class KDataDriverConnect {
public:
    typedef KDataDriver DriverType;
    typedef KDataDriverPtr DriverTypePtr;

    explicit KDataDriverConnect(const KDataDriverPtr& driver) : m_driver(driver) {}
    ~KDataDriverConnect() = default;

    KDataDriverConnect(const KDataDriverConnect&) = delete;
    KDataDriverConnect(KDataDriverConnect&&) = delete;
    KDataDriverConnect& operator=(const KDataDriverConnect&) = delete;
    KDataDriverConnect& operator=(KDataDriverConnect&&) = delete;

    explicit operator bool() const noexcept {
        return m_driver.get() != nullptr;
    }

    const string& name() const {
        return m_driver->name();
    }

    bool isIndexFirst() {
        return m_driver->isIndexFirst();
    }

    bool canParallelLoad() {
        return m_driver->canParallelLoad();
    }

    size_t getCount(const string& market, const string& code, KQuery::KType kType) {
        return m_driver->getCount(market, code, kType);
    }

    bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                             size_t& out_start, size_t& out_end) {
        return m_driver->getIndexRangeByDate(market, code, query, out_start, out_end);
    }

    KRecordList getKRecordList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getKRecordList(market, code, query);
    }

    TimeLineList getTimeLineList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getTimeLineList(market, code, query);
    }

    TransList getTransList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getTransList(market, code, query);
    }

private:
    KDataDriverPtr m_driver;
};

}  // namespace hku

#endif /* KDATADRIVER_H_ */
