/*
 * StockManager.h
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#ifndef STOCKMANAGER_H_
#define STOCKMANAGER_H_

#include <mutex>
#include <thread>
#include "utilities/Parameter.h"
#include "data_driver/DataDriverFactory.h"
#include "Block.h"
#include "MarketInfo.h"
#include "StockTypeInfo.h"
#include "StrategyContext.h"

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
     * @param baseInfoParam 基础信息驱动参数
     * @param blockParam 板块驱动参数
     * @param kdataParam K线驱动参数
     * @param preloadParam 预加载参数
     * @param hikyuuParam 其他参数
     * @param context 策略上下文
     */
    void init(const Parameter& baseInfoParam, const Parameter& blockParam,
              const Parameter& kdataParam, const Parameter& preloadParam = default_preload_param(),
              const Parameter& hikyuuParam = default_other_param(),
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

    /**
     * 根据"市场简称证券代码"获取对应的证券实例
     * @param querystr 格式：“市场简称证券代码”，如"sh000001"
     * @return 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
     */
    Stock getStock(const string& querystr) const;

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

    // 目前支持"SH"
    DatetimeList getTradingCalendar(const KQuery& query, const string& market = "SH");

    /**
     * 判断指定日期是否为节假日
     * @param d 指定日期
     */
    bool isHoliday(const Datetime& d) const;

    /**
     * 添加Stock，仅供临时增加的特殊Stock使用
     * @param stock
     * @return true 成功 | false 失败
     */
    bool addStock(const Stock& stock);

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

public:
    typedef StockMapIterator const_iterator;
    const_iterator begin() const {
        return m_stockDict.begin();
    }
    const_iterator end() const {
        return m_stockDict.end();
    }

private:
    /* 设置K线驱动 */
    void setKDataDriver(const KDataDriverConnectPoolPtr&);

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

private:
    StockManager();

private:
    static StockManager* m_sm;
    bool m_initializing;
    std::thread::id m_thread_id;  // 记录线程id，用于判断Stratege是以独立进程方式还是线程方式运行
    string m_tmpdir;
    string m_datadir;
    BaseInfoDriverPtr m_baseInfoDriver;
    BlockInfoDriverPtr m_blockDriver;

    StockMapIterator::stock_map_t m_stockDict;  // SH000001 -> stock
    std::mutex* m_stockDict_mutex;

    typedef unordered_map<string, MarketInfo> MarketInfoMap;
    mutable MarketInfoMap m_marketInfoDict;
    std::mutex* m_marketInfoDict_mutex;

    typedef unordered_map<uint32_t, StockTypeInfo> StockTypeInfoMap;
    mutable StockTypeInfoMap m_stockTypeInfo;
    std::mutex* m_stockTypeInfo_mutex;

    std::unordered_set<Datetime> m_holidays;  // 节假日
    std::mutex* m_holidays_mutex;

    Parameter m_baseInfoDriverParam;
    Parameter m_blockDriverParam;
    Parameter m_kdataDriverParam;
    Parameter m_preloadParam;
    Parameter m_hikyuuParam;
    StrategyContext m_context;
};

inline size_t StockManager::size() const {
    return m_stockDict.size();
}

inline Stock StockManager::operator[](const string& query) const {
    return getStock(query);
}

inline bool StockManager::isHoliday(const Datetime& d) const {
    return m_holidays.count(d);
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

}  // namespace hku

#endif /* STOCKMANAGER_H_ */
