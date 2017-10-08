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

class HKU_API DataDriverFactory {
public:
    //DataDriverFactory();

    static KDataDriverPtr getKDataDriver(const shared_ptr<IniParser>&);
    //static BlockInfoDriverPtr getBlockDriver(const shared_ptr<IniParser>&);

    static void regBaseInfoDriver(const BaseInfoDriverPtr&);
    static void removeBaseInfoDriver(const string& name);
    static BaseInfoDriverPtr getBaseInfoDriver(const Parameter&);

    static void regBlockDriver(const BlockInfoDriverPtr&);
    static void removeBlockDriver(const string& name);
    static BlockInfoDriverPtr getBlockDriver(const Parameter&);

private:
    static map<string, BaseInfoDriverPtr> m_baseInfoDrivers;
    static map<string, BlockInfoDriverPtr> m_blockDrivers;
};

} /* namespace hku */
#endif /* DATABASEDRIVERFACTORY_H_ */
