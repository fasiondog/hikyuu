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
 * Data driver factory class
 * @ingroup DataDriver
 */
class HKU_API DataDriverFactory {
public:
    /**
     * Initialize the supported default drivers
     */
    static void init();

    /**
     * Release the resources proactively, mainly used for memory leak detection, cleaning up
     * proactively on exit to avoid false positives
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
    static map<string, KDataDriverPtr> *m_kdataPrototypeDrivers;        // K-line driver prototype
    static map<string, KDataDriverConnectPoolPtr> *m_kdataDriverPools;  // K-line driver pool
};

} /* namespace hku */
#endif /* DATABASEDRIVERFACTORY_H_ */
