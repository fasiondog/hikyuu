/*
 * StockManager.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKMANAGER_H_
#define STOCKMANAGER_H_

#include <mutex>
#include <thread>
#include "hikyuu/utilities/Parameter.h"
#include "hikyuu/utilities/thread/thread.h"
#include "hikyuu/utilities/plugin/PluginManager.h"
#include "hikyuu/data_driver/DataDriverFactory.h"
#include "Block.h"
#include "MarketInfo.h"
#include "StockTypeInfo.h"
#include "StrategyContext.h"

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

    StockManager(const StockManager&) = delete;
    StockManager& operator=(const StockManager&) = delete;

    /**
     * 初始化函数，必须在程序入口调用
     * @param baseInfoParam 基础信息驱动参数
     * @param blockParam 板块驱动参数
     * @param kdataParam K线驱动参数
     * @param preloadParam 预加载参数
     * @param hikyuuParam 其他参数
     * @param context 策略上下文
     */
    void init(const Parameter& baseInfoParam, const Parameter& blockParam,
              const Parameter& kdataParam, const Parameter& preloadParam,
              const Parameter& hikyuuParam,
              const StrategyContext& context = StrategyContext({"all"}));

    /** 重新加载 */
    void reload();

    /** 主动退出并释放资源 */
    static void quit();

    /** 获取基础信息驱动参数 */
    const Parameter& getBaseInfoDriverParameter() const;

    /** 获取板块驱动参数 */
    const Parameter& getBlockDriverParameter() const;

    /** 获取 K 线数据驱动参数 */
    const Parameter& getKDataDriverParameter() const;

    /** 获取预加载参数 */
    const Parameter& getPreloadParameter() const;

    /** 获取其他参数 */
    const Parameter& getHikyuuParameter() const;

    /** 获取策略上下文 */
    const StrategyContext& getStrategyContext() const;

    /** 获取基础信息驱动 */
    BaseInfoDriverPtr getBaseInfoDriver() const;

    /**
     * 获取用于保存零时变量等的临时目录，如为配置则为当前目录
     * 由m_config中的“tmpdir”指定
     */
    string tmpdir() const;

    /** 获取数据目录 */
    string datadir() const;

    /** 获取证券数量 */
    size_t size() const;

    /** 是否所有数据准备完毕 */
    bool dataReady() const;

    /**
     * 根据"市场简称证券代码"获取对应的证券实例
     * @param querystr 格式：“市场简称证券代码”，如"sh000001"
     * @return 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
     */
    Stock getStock(const string& querystr) const;

    /** 同 getStock @see getStock */
    Stock operator[](const string&) const;

    StockList getStockList(
      std::function<bool(const Stock&)>&& filter = std::function<bool(const Stock&)>()) const;

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
    StockTypeInfo getStockTypeInfo(uint32_t type) const;

    /** 获取市场简称列表 */
    MarketList getAllMarket() const;

    /**
     * 获取预定义的板块
     * @param category 板块分类
     * @param name 板块名称
     * @return 板块，如找不到返回空
     */
    Block getBlock(const string& category, const string& name);

    void addBlock(const Block& blk) {
        saveBlock(blk);
    }

    void saveBlock(const Block& blk);
    void removeBlock(const string& category, const string& name);
    void removeBlock(const Block& blk) {
        removeBlock(blk.category(), blk.name());
    }

    /**
     * 获取指定分类的板块列表
     * @param category 板块分类
     * @return 板块列表
     */
    BlockList getBlockList(const string& category);

    /**
     * 获取所有板块
     * @return 板块列表
     */
    BlockList getBlockList();

    /**
     * 获取指定证券所属的板块列表
     * @param stk 指定证券
     * @param category 板块分类，如果为空字符串，返回所有板块分类下的所属板块
     * @return BlockList
     */
    BlockList getStockBelongs(const Stock& stk, const string& category);

    /**
     * 获取交易日历，目前支持"SH"
     * @param query
     * @param market
     * @return DatetimeList
     */
    DatetimeList getTradingCalendar(const KQuery& query, const string& market = "SH");

    /**
     * 获取10年期中国国债收益率
     */
    const ZhBond10List& getZhBond10() const;

    /**
     * 判断指定日期是否为节假日
     * @param d 指定日期
     */
    bool isHoliday(const Datetime& d) const;

    const string& getHistoryFinanceFieldName(size_t ix) const;
    size_t getHistoryFinanceFieldIndex(const string& name) const;
    vector<std::pair<size_t, string>> getHistoryFinanceAllFields() const;

    vector<HistoryFinanceInfo> getHistoryFinance(const Stock& stk, Datetime start, Datetime end);

    /**
     * 添加Stock，仅供临时增加的特殊Stock使用
     * @param stock
     * @return true 成功 | false 失败
     */
    bool addStock(const Stock& stock);

    /**
     * 从 StockManager 中移除相应的 Stock，一般用于将临时增加的 Stock 从 sm 中移除
     * @param market_code
     */
    void removeStock(const string& market_code);

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
    Stock addTempCsvStock(const string& code, const string& day_filename,
                          const string& min_filename, price_t tick = 0.01, price_t tickValue = 0.01,
                          int precision = 2, size_t minTradeNumber = 1,
                          size_t maxTradeNumber = 1000000);

    /**
     * 移除增加的临时Stock
     * @param code
     */
    void removeTempCsvStock(const string& code);

    /**
     * 获取当前执行线程id，主要用于判断 Strategy 是以独立进程还是线程方式运行
     */
    std::thread::id thread_id() const {
        return m_thread_id;
    }

    /** 仅由程序退出使使用！！！ */
    ThreadPool* getLoadTaskGroup() {
        return m_load_tg.get();
    }

    /** 设置插件路径（仅在初始化之前有效） */
    void setPluginPath(const std::string& path);

    /** 获取当前插件路径 */
    const string& getPluginPath() const {
        return m_plugin_manager.pluginPath();
    }

    template <typename PluginInterfaceT>
    PluginInterfaceT* getPlugin(const std::string& pluginname) noexcept;

public:
    typedef StockMapIterator const_iterator;
    const_iterator begin() const {
        return m_stockDict.begin();
    }
    const_iterator end() const {
        return m_stockDict.end();
    }

private:
    /* 加载全部数据 */
    void loadData();

    /* 加载 K线数据至缓存 */
    void loadAllKData();

    /* 加载节假日信息 */
    void loadAllHolidays();

    /* 初始化时，添加市场信息 */
    void loadAllMarketInfos();

    /* 初始化时，添加证券类型信息 */
    void loadAllStockTypeInfo();

    /* 加载所有证券 */
    void loadAllStocks();

    /* 加载所有权息数据 */
    void loadAllStockWeights();

    /** 加载10年期中国国债收益率数据 */
    void loadAllZhBond10();

    /** 加载历史财经字段索引 */
    void loadHistoryFinanceField();

private:
    StockManager();

private:
    static StockManager* m_sm;
    std::atomic_bool m_initializing;
    std::atomic_bool m_data_ready;  // 用于指示是否所有数据准备完毕
    std::thread::id m_thread_id;    // 记录线程id，用于判断Stratege是以独立进程方式还是线程方式运行
    string m_tmpdir;
    string m_datadir;
    BaseInfoDriverPtr m_baseInfoDriver;
    BlockInfoDriverPtr m_blockDriver;

    StockMapIterator::stock_map_t m_stockDict;  // SH000001 -> stock
    std::shared_mutex* m_stockDict_mutex;

    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    mutable MarketInfoMap m_marketInfoDict;
    std::shared_mutex* m_marketInfoDict_mutex;

    typedef unordered_map<uint32_t, StockTypeInfo> StockTypeInfoMap;
    mutable StockTypeInfoMap m_stockTypeInfo;
    std::shared_mutex* m_stockTypeInfo_mutex;

    std::unordered_set<Datetime> m_holidays;  // 节假日
    std::shared_mutex* m_holidays_mutex;

    ZhBond10List m_zh_bond10;  // 10年期中国国债收益率数据

    unordered_map<string, size_t> m_field_name_to_ix;  // 财经字段名称到字段索引映射
    unordered_map<size_t, string> m_field_ix_to_name;  // 财经字段索引到字段名称映射

    Parameter m_baseInfoDriverParam;
    Parameter m_blockDriverParam;
    Parameter m_kdataDriverParam;
    Parameter m_preloadParam;
    Parameter m_hikyuuParam;
    StrategyContext m_context;

    std::unique_ptr<ThreadPool> m_load_tg;  // 异步数据加载辅助线程组

    PluginManager m_plugin_manager;
};

inline size_t StockManager::size() const {
    return m_stockDict.size();
}

inline bool StockManager::dataReady() const {
    return m_data_ready;
}

inline Stock StockManager::operator[](const string& query) const {
    return getStock(query);
}

inline const Parameter& StockManager::getBaseInfoDriverParameter() const {
    return m_baseInfoDriverParam;
}

inline const Parameter& StockManager::getBlockDriverParameter() const {
    return m_blockDriverParam;
}

inline const Parameter& StockManager::getKDataDriverParameter() const {
    return m_kdataDriverParam;
}

inline const Parameter& StockManager::getPreloadParameter() const {
    return m_preloadParam;
}

inline const Parameter& StockManager::getHikyuuParameter() const {
    return m_hikyuuParam;
}

inline const StrategyContext& StockManager::getStrategyContext() const {
    return m_context;
}

inline BaseInfoDriverPtr StockManager::getBaseInfoDriver() const {
    return m_baseInfoDriver;
}

inline const string& StockManager::getHistoryFinanceFieldName(size_t ix) const {
    return m_field_ix_to_name.at(ix);
}

inline size_t StockManager::getHistoryFinanceFieldIndex(const string& name) const {
    return m_field_name_to_ix.at(name);
}

inline vector<HistoryFinanceInfo> StockManager::getHistoryFinance(const Stock& stk, Datetime start,
                                                                  Datetime end) {
    return m_baseInfoDriver->getHistoryFinance(stk.market(), stk.code(), start, end);
}

inline void StockManager::setPluginPath(const std::string& path) {
    m_plugin_manager.pluginPath(path);
}

template <typename PluginInterfaceT>
inline PluginInterfaceT* StockManager::getPlugin(const std::string& pluginname) noexcept {
    return m_plugin_manager.getPlugin<PluginInterfaceT>(pluginname);
}

}  // namespace hku

#endif /* STOCKMANAGER_H_ */
