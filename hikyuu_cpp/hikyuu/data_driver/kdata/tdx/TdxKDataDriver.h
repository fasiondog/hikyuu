/*
 * TdxKDataDriver.h
 *
 *  Created on: 2017年10月12日
 *      Author: fasiondog
 */

#pragma once
#ifndef DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_
#define DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_

#include "../../KDataDriver.h"

namespace hku {

class TdxKDataDriver : public KDataDriver {
public:
    TdxKDataDriver();
    virtual ~TdxKDataDriver();

    virtual KDataDriverPtr _clone() override {
        return std::make_shared<TdxKDataDriver>();
    }

    virtual bool _init() override;

    virtual bool isIndexFirst() override {
        return true;
    }

    virtual bool canParallelLoad() override {
        return true;
    }

    virtual size_t getCount(const string& market, const string& code,
                            const KQuery::KType& kType) override;
    virtual bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                     size_t& out_start, size_t& out_end) override;
    virtual KRecordList getKRecordList(const string& market, const string& code,
                                       const KQuery& query) override;

private:
    string _getFileName(const string& market, const string& code, const KQuery::KType&);
    size_t _getBaseCount(const string& market, const string& code, const KQuery::KType&);

    KRecordList _getDayKRecordList(const string& market, const string& code,
                                   const KQuery::KType& ktype, size_t start_ix, size_t end_ix);
    KRecordList _getMinKRecordList(const string& market, const string& code,
                                   const KQuery::KType& ktype, size_t start_ix, size_t end_ix);

    bool _getDayIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                 size_t& out_start, size_t& out_end);

    bool _getMinIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                 size_t& out_start, size_t& out_end);

private:
    string m_dirname;
};

} /* namespace hku */

#endif /* DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_ */
