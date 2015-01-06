/*
 * BaseInfoDriver.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#ifndef BASEINFODRIVER_H_
#define BASEINFODRIVER_H_

#include <hikyuu_utils/iniparser/IniParser.h>
#include "../MarketInfo.h"
#include "../StockTypeInfo.h"
#include "../Stock.h"

namespace hku {

/**
 * 基本信息数据获取驱动基类
 */
class BaseInfoDriver {
public:
    BaseInfoDriver(const shared_ptr<IniParser>& config) : m_config(config) {}

    virtual ~BaseInfoDriver() { }

    /**
     * 加载市场信息
     * @param out map<市场标识, 市场信息>
     * @return true 成功 | false 失败
     */
    virtual bool loadMarketInfo(map<string, MarketInfo>& out) = 0;

    /**
     * 加载证券类型信息
     * @param out map<证券类型, 类型信息>
     * @return true 成功 | false 失败
     */
    virtual bool loadStockTypeInfo(map<hku_uint32, StockTypeInfo>& out) = 0;

    /**
     * 加载股票信息
     * @param out map<市场标识+证券代码, 证券对象>
     * @return true 成功 | false 失败
     */
    virtual bool loadStock() = 0;

protected:
    shared_ptr<IniParser> m_config;
};

typedef shared_ptr<BaseInfoDriver> BaseInfoDriverPtr;

} /* namespace hku */
#endif /* BASEINFODRIVER_H_ */
