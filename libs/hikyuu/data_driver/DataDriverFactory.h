/*
 * DatabaseDriverFactory.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#ifndef DATADRIVERFACTORY_H_
#define DATADRIVERFACTORY_H_

#include "BaseInfoDriver.h"
#include "KDataDriver.h"
#include "BlockInfoDriver.h"

namespace hku {

/**
 * 数据驱动工厂类
 * @ingroup DataDriver
 */
class HKU_API DataDriverFactory {
public:

    static void regBaseInfoDriver(const BaseInfoDriverPtr&);
    static void removeBaseInfoDriver(const string& name);
    static BaseInfoDriverPtr getBaseInfoDriver(const Parameter&);

    static void regBlockDriver(const BlockInfoDriverPtr&);
    static void removeBlockDriver(const string& name);
    static BlockInfoDriverPtr getBlockDriver(const Parameter&);

    static void regKDataDriver(const KDataDriverPtr&);
    static void removeKDataDriver(const string& name);
    static KDataDriverPtr getKDataDriver(const Parameter&);


private:
    static map<string, BaseInfoDriverPtr> m_baseInfoDrivers;
    static map<string, BlockInfoDriverPtr> m_blockDrivers;
    static map<string, KDataDriverPtr> m_kdataDrivers;
    static map<Parameter, KDataDriverPtr> m_param_kdataDrivers;
};

} /* namespace hku */
#endif /* DATABASEDRIVERFACTORY_H_ */
