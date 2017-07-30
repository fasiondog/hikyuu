/*
 * StockManager.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#ifndef STOCKMANAGER_H_
#define STOCKMANAGER_H_

#include <hikyuu_utils/iniparser/IniParser.h>

#include "Block.h"
#include "MarketInfo.h"
#include "StockTypeInfo.h"

namespace hku {

typedef vector<string> MarketList;

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
     * @param filename 配置ini文件名
     */
    void init(const string& filename);

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

    Stock addTempCsvStock(const string& code,
            const string& day_filename,
            const string& min_filename,
            hku_uint32 stk_type = STOCKTYPE_INDEX);

    void removeTempCsvStock(const string& code);

public:
    typedef StockMapIterator const_iterator;
    const_iterator begin() const { return m_stockDict.begin(); }
    const_iterator end() const { return m_stockDict.end(); }

private:
    StockManager() { }

private:
    static shared_ptr<StockManager> m_sm;
    shared_ptr<IniParser> m_iniconfig;
    StockMapIterator::stock_map_t m_stockDict;  // SH000001 -> stock

    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    MarketInfoMap m_marketInfoDict;

    typedef unordered_map<hku_uint32, StockTypeInfo> StockTypeInfoMap;
    StockTypeInfoMap m_stockTypeInfo;
};


inline size_t StockManager::size() const {
    return m_stockDict.size();
}


inline Stock StockManager::operator[](const string& query) const {
    return getStock(query);
}

} /* namespace */

#endif /* STOCKMANAGER_H_ */
