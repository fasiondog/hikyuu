/*
 * DatabaseDriverFactory.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#pragma once
#ifndef DATADRIVERFACTORY_H_
#define DATADRIVERFACTORY_H_

#include "DriverConnectPool.h"
#include "BaseInfoDriver.h"
#include "KDataDriver.h"
#include "BlockInfoDriver.h"

namespace hku {

typedef DriverConnectPool<KDataDriverConnect> KDataDriverConnectPool;
typedef shared_ptr<KDataDriverConnectPool> KDataDriverConnectPoolPtr;

/**
 * 数据驱动工厂类
 * @ingroup DataDriver
 */
class HKU_API DataDriverFactory {
public:
    /**
     * 初始化支持的默认驱动
     */
    static void init();

    /**
     * 主动释放资源，主要用于内存泄漏检测，退出时主动清理，避免误报
     */
    static void release();

    static void regBaseInfoDriver(const BaseInfoDriverPtr &);
    static void removeBaseInfoDriver(const string &name);
    static BaseInfoDriverPtr getBaseInfoDriver(const Parameter &);

    static void regBlockDriver(const BlockInfoDriverPtr &);
    static void removeBlockDriver(const string &name);
    static BlockInfoDriverPtr getBlockDriver(const Parameter &);

    static void regKDataDriver(const KDataDriverPtr &);
    static void removeKDataDriver(const string &name);
    static KDataDriverConnectPoolPtr getKDataDriverPool(const Parameter &);

private:
    static map<string, BaseInfoDriverPtr> *m_baseInfoDrivers;
    static map<string, BlockInfoDriverPtr> *m_blockDrivers;
    static map<string, KDataDriverPtr> *m_kdataPrototypeDrivers;        // K线驱动原型
    static map<string, KDataDriverConnectPoolPtr> *m_kdataDriverPools;  // K线驱动池
};

} /* namespace hku */
#endif /* DATABASEDRIVERFACTORY_H_ */
