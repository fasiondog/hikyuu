/*
 * BaseInfoDriver.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#ifndef BASEINFODRIVER_H_
#define BASEINFODRIVER_H_

#include "../utilities/Parameter.h"
#include "../MarketInfo.h"
#include "../StockTypeInfo.h"
#include "../Stock.h"

namespace hku {

/**
 * 基本信息数据获取驱动基类
 */
class HKU_API BaseInfoDriver {
    PARAMETER_SUPPORT

public:
    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    typedef unordered_map<hku_uint32, StockTypeInfo> StockTypeInfoMap;

    BaseInfoDriver(const string& name);
    virtual ~BaseInfoDriver() { }

    const string& name() const;

    /**
     * 驱动初始化
     * @param params
     * @return
     */
    bool init(const Parameter& params);

    /**
     * 加载基础信息
     * @param params
     * @return
     */
    bool loadBaseInfo();

    /**
     * 驱动初始化，具体实现时应注意将之前打开的相关资源关闭。
     * @return
     */
    virtual bool _init() = 0;

    /**
     * 加载市场信息
     * @return true 成功 | false 失败
     */
    virtual bool _loadMarketInfo() = 0;

    /**
     * 加载证券类型信息
     * @return true 成功 | false 失败
     */
    virtual bool _loadStockTypeInfo() = 0;

    /**
     * 加载股票信息
     * @return true 成功 | false 失败
     */
    virtual bool _loadStock() = 0;

private:
    bool checkType();

protected:
    string m_name;
};

typedef shared_ptr<BaseInfoDriver> BaseInfoDriverPtr;

HKU_API std::ostream & operator<<(std::ostream&, const BaseInfoDriver&);
HKU_API std::ostream & operator<<(std::ostream&, const BaseInfoDriverPtr&);


inline const string& BaseInfoDriver::name() const {
    return m_name;
}

} /* namespace hku */
#endif /* BASEINFODRIVER_H_ */
