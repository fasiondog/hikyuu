/*
 * DatabaseDriverFactory.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#ifndef DATADRIVERFACTORY_H_
#define DATADRIVERFACTORY_H_

#include <hikyuu_utils/iniparser/IniParser.h>

#include "BaseInfoDriver.h"
#include "KDataDriver.h"

namespace hku {

class DataDriverFactory {
public:

    static BaseInfoDriverPtr getBaseInfoDriver(const shared_ptr<IniParser>&);
    static KDataDriverPtr getKDataDriver(const shared_ptr<IniParser>&);
};

} /* namespace hku */
#endif /* DATABASEDRIVERFACTORY_H_ */
