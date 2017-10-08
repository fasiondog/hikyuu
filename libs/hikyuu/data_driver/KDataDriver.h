/*
 * KDataDriver.h
 *
 *  Created on: 2012-9-8
 *      Author: fasiondog
 */

#ifndef KDATADRIVER_H_
#define KDATADRIVER_H_

#include "KDataDriverImp.h"

namespace hku {

/**
 * K线数据驱动基类
 */
class HKU_API KDataDriver {
public:
    KDataDriver() {}
    KDataDriver(const shared_ptr<IniParser>& config);
    virtual ~KDataDriver() { }

    /**
     * 将指定类型的K线数据加载至缓存
     * @param market 市场简称
     * @param code   证券代码
     * @param kType  K线类型
     * @param start_ix 欲加载的起始位置
     * @param end_ix 欲加载的结束位置，不包含自身
     * @param out_buffer [out] 缓存指针
     */
    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer) ;

    /**
     * 获取指定类型的K线数据量
     * @param market 市场简称
     * @param code   证券代码
     * @param kType  K线类型
     * @return
     */
    virtual size_t getCount(const string& market, const string& code,
            KQuery::KType kType);

    /**
     * 获取指定日期范围对应的K线记录索引
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     * @param out_start [out] 对应K线记录位置
     * @param out_end [out] 对应的K线记录位置
     * @return
     */
    virtual bool getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    /**
     * 获取指定的K线记录
     * @param market 市场简称
     * @param code   证券代码
     * @param pos    K线记录索引
     * @param kType  K线类型
     * @return
     */
    virtual KRecord getKRecord(const string& market, const string& code,
              size_t pos, KQuery::KType kType);

private:
    KDataDriverImpPtr _getImpPtr(const string& market, KQuery::KType);

private:
    map<KQuery::KType, string> m_suffix;
    map<string, KDataDriverImpPtr> m_imp; /* key: market + _day */
};

typedef shared_ptr<KDataDriver> KDataDriverPtr;

} /* namespace */

#endif /* KDATADRIVER_H_ */
