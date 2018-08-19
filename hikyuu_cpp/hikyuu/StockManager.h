/*
 * StockManager.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#ifndef STOCKMANAGER_H_
#define STOCKMANAGER_H_

#include <hikyuu_utils/iniparser/IniParser.h>

#include "utilities/Parameter.h"
#include "data_driver/BlockInfoDriver.h"
#include "Block.h"
#include "MarketInfo.h"
#include "StockTypeInfo.h"

namespace hku {

typedef vector<string> MarketList;

Parameter default_preload_param();
Parameter default_other_param();

/**
 * 证券信息统一管理类
 * @ingroup StockManage
 */
class HKU_API StockManager {
public:
    /** 获取StockManager单例实例 */
    static StockManager& instance();
    virtual ~StockManager();

    /**
     * 初始化函数，必须在程序入口调用
     * @param baseInfoParam
     * @param blockParam
     * @param kdataParam
     * @param preloadParam
     * @param hikyuuParam
     */
    void init(const Parameter& baseInfoParam,
              const Parameter& blockParam,
              const Parameter& kdataParam,
              const Parameter& preloadParam = default_preload_param(),
              const Parameter& hikyuuParam = default_other_param());

    Parameter getBaseInfoDriverParameter() const;
    Parameter getBlockDriverParameter() const;
    Parameter getKDataDriverParameter() const;
    Parameter getPreloadParameter() const;
    Parameter getHikyuuParameter() const;

    void setKDataDriver(const KDataDriverPtr&);

    /**
     * 获取用于保存零时变量等的临时目录，如为配置则为当前目录
     * 由m_config中的“tmpdir”指定
     */
    string tmpdir() const;

    /** 获取证券数量 */
    size_t size() const;

    /**
     * 根据"市场简称证券代码"获取对应的证券实例
     * @param querystr 格式：“市场简称证券代码”，如"sh000001"
     * @return 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
     */
    Stock getStock(const string&) const;

    /** 同 getStock @see getStock */
    Stock operator[](const string&) const;

    /**
     * 获取相应的市场信息
     * @param market 指定的市场标识
     * @return 相应的市场信息，如果相应的市场信息不存在，则返回Null<MarketInfo>()
     */
    MarketInfo getMarketInfo(const string& market) const;

    /**
     * 获取相应的证券类型详细信息
     * @param type 证券类型
     * @return 对应的证券类型信息，如果不存在，则返回Null<StockTypeInf>()
     */
    StockTypeInfo getStockTypeInfo(hku_uint32 type) const;

    /** 获取市场简称列表 */
    MarketList getAllMarket() const;

    /**
     * 获取预定义的板块
     * @param category 板块分类
     * @param name 板块名称
     * @return 板块，如找不到返回空
     */
    Block getBlock(const string& category, const string& name);

    /**
     * 获取指定分类的板块列表
     * @param category 板块分类
     * @return 板块列表
     */
    BlockList getBlockList(const string&);

    /**
     * 获取所有板块
     * @return 板块列表
     */
    BlockList getBlockList();

    //目前支持"SH"
    DatetimeList getTradingCalendar(const KQuery& query,
            const string& market = "SH");

    /**
     * 初始化时，添加Stock，仅供BaseInfoDriver子类使用
     * @param stock
     * @return true 成功 | false 失败
     */
    bool addStock(const Stock& stock);

    /**
     * 初始化时，添加市场信息
     * @param marketInfo
     * @return
     */
    bool addMarketInfo(const MarketInfo& marketInfo);

    /**
     * 初始化时，添加证券类型信息
     * @param stkTypeInfo
     * @return
     */
    bool addStockTypeInfo(const StockTypeInfo& stkTypeInfo);

    /**
     * 从CSV文件（K线数据）增加临时的Stock，可用于只有CSV格式的K线数据时，进行临时测试
     * @details 增加的临时Stock，其market为“TMP”
     * @param code 自行编号的证券代码，不能和已有的Stock相同，否则将返回Null<Stock>
     * @param day_filename 日线CSV文件名
     * @param min_filename 分钟线CSV文件名
     * @param tick 最小跳动量，默认0.01
     * @param tickValue 最小跳动量价值，默认0.01
     * @param precision 价格精度，默认2
     * @param minTradeNumber 单笔最小交易量，默认1
     * @param maxTradeNumber 单笔最大交易量，默认1000000
     * @return
     */
    Stock addTempCsvStock(const string& code,
            const string& day_filename,
            const string& min_filename,
            price_t tick = 0.01,
            price_t tickValue = 0.01,
            int precision = 2,
            size_t minTradeNumber = 1,
            size_t maxTradeNumber = 1000000);

    /**
     * 移除增加的临时Stock
     * @param code
     */
    void removeTempCsvStock(const string& code);

public:
    typedef StockMapIterator const_iterator;
    const_iterator begin() const { return m_stockDict.begin(); }
    const_iterator end() const { return m_stockDict.end(); }

private:
    StockManager() { }

private:
    static shared_ptr<StockManager> m_sm;
    string m_tmpdir;
    BlockInfoDriverPtr m_blockDriver;

    StockMapIterator::stock_map_t m_stockDict;  // SH000001 -> stock

    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    MarketInfoMap m_marketInfoDict;

    typedef unordered_map<hku_uint32, StockTypeInfo> StockTypeInfoMap;
    StockTypeInfoMap m_stockTypeInfo;

    Parameter m_baseInfoDriverParam;
    Parameter m_blockDriverParam;
    Parameter m_kdataDriverParam;
    Parameter m_preloadParam;
    Parameter m_hikyuuParam;
};


inline size_t StockManager::size() const {
    return m_stockDict.size();
}


inline Stock StockManager::operator[](const string& query) const {
    return getStock(query);
}

inline Parameter StockManager::getBaseInfoDriverParameter() const {
    return m_baseInfoDriverParam;
}

inline Parameter StockManager::getBlockDriverParameter() const {
    return m_blockDriverParam;
}

inline Parameter StockManager::getKDataDriverParameter() const {
    return m_kdataDriverParam;
}

inline Parameter StockManager::getPreloadParameter() const {
    return m_preloadParam;
}

inline Parameter StockManager::getHikyuuParameter() const {
    return m_hikyuuParam;
}

} /* namespace */

#endif /* STOCKMANAGER_H_ */
