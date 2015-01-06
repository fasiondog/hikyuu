/*
 * KDataDriverImp.h
 *
 *  Created on: 2014年9月2日
 *      Author: fasiondog
 */

#ifndef KDATADRIVERIMP_H_
#define KDATADRIVERIMP_H_

#include <hikyuu_utils/iniparser/IniParser.h>
#include "../KData.h"

namespace hku {

class KDataDriverImp {
public:
    KDataDriverImp(const shared_ptr<IniParser>& config): m_config(config) {}
    virtual ~KDataDriverImp(){}

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
            KRecordList* out_buffer) { }

    /**
     * 获取指定类型的K线数据量
     * @param market 市场简称
     * @param code   证券代码
     * @param kType  K线类型
     * @return
     */
    virtual size_t
    getCount(const string& market, const string& code,
             KQuery::KType kType) {
        return 0;
    }

    /**
     * 获取指定日期范围对应的K线记录索引
     * @param market 市场简称
     * @param code   证券代码
     * @param query  查询条件
     * @param out_start [out] 对应K线记录位置
     * @param out_end [out] 对应的K线记录位置
     * @return
     */
    virtual bool
    getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end) {
        out_start = 0;
        out_end = 0;
        return false;
    }

    /**
     * 获取指定的K线记录
     * @param market 市场简称
     * @param code   证券代码
     * @param pos    K线记录索引
     * @param kType  K线类型
     * @return
     */
    virtual KRecord
    getKRecord(const string& market, const string& code,
              size_t pos, KQuery::KType kType) {
        return Null<KRecord>();
    }

protected:
    shared_ptr<IniParser> m_config;
};

typedef shared_ptr<KDataDriverImp> KDataDriverImpPtr;

} /* namespace hku */

#endif /* KDATADRIVERIMP_H_ */
