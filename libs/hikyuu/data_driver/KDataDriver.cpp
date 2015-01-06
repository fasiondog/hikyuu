/*
 * KDataDriver.cpp
 *
 *  Created on: 2014年9月2日
 *      Author: fasiondog
 */

#include "../StockManager.h"
#include "KDataDriver.h"
#include "h5_kdata/H5KDataDriverImp.h"
#include "mysql_kdata/MySQLKDataDriverImp.h"

namespace hku {

KDataDriver::KDataDriver(const shared_ptr<IniParser>& config) {
    string func_name(" [KDataDriver::KDataDriver]");
    if (!config) {
        HKU_ERROR("Null configure!" << func_name);
        return;
    }

    m_suffix[KQuery::DAY] = "_day";
    m_suffix[KQuery::WEEK] = "_week";
    m_suffix[KQuery::MONTH] = "_month";
    m_suffix[KQuery::QUARTER] = "_quarter";
    m_suffix[KQuery::HALFYEAR] = "_halfyear";
    m_suffix[KQuery::YEAR] = "_year";
    m_suffix[KQuery::MIN] = "_1min";
    m_suffix[KQuery::MIN5] = "_5min";
    m_suffix[KQuery::MIN15] = "_15min";
    m_suffix[KQuery::MIN30] = "_30min";
    m_suffix[KQuery::MIN60] = "_60min";

    for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
        map<KQuery::KType, string>::iterator iter;
        iter = m_suffix.find((KQuery::KType)i);
        if (iter == m_suffix.end()) {
            HKU_FATAL("Not match KQuery::KType " << i << func_name);
            exit(1);
        }
    }

    map<string, KDataDriverImpPtr> h5file_dict;
    StockManager& sm = StockManager::instance();
    MarketList market_list = sm.getAllMarket();
    MarketList::const_iterator market_iter = market_list.begin();
    for (; market_iter != market_list.end(); ++market_iter) {
        string market(*market_iter);
        for (int i = 0; i < KQuery::INVALID_KTYPE; ++i) {
            string driver_name(market + m_suffix[(KQuery::KType)i]);
            boost::to_lower(driver_name);
            if (!config->hasSection(driver_name)) {
                HKU_WARN("Not fount " << driver_name << " section" << func_name);
                continue;
            }

            if (!config->hasOption(driver_name, "type")) {
                HKU_WARN("Not fount " << driver_name
                        << " section's type option" << func_name);
                continue;
            }

            string dbtype = config->get(driver_name, "type");
            if (dbtype == "hdf5") {
                if (!config->hasOption(driver_name, "file")) {
                    HKU_WARN("Not found hdf file name in "
                            << driver_name << func_name);
                    continue;
                }

                string h5name(config->get(driver_name, "file"));
                map<string, KDataDriverImpPtr>::iterator h5_iter;
                h5_iter = h5file_dict.find(h5name);
                if (h5_iter != h5file_dict.end()) {
                    m_imp[driver_name] = h5_iter->second;
                } else {
                    KDataDriverImpPtr h5driver =
                            make_shared<H5KDataDriverImp>(H5KDataDriverImp(
                            config, h5name));
                    m_imp[driver_name] = h5driver;
                    h5file_dict[h5name] = h5driver;
                }

            } else if (dbtype == "mysql") {
                KDataDriverImpPtr mysql_driver =
                        make_shared<MySQLKDataDriverImp>(MySQLKDataDriverImp(
                        config, driver_name));
                m_imp[driver_name] = mysql_driver;

            }
        } /* for KQuery::KType */
    } /* for market_iter */
}


KDataDriverImpPtr KDataDriver::
_getImpPtr(const string& market, KQuery::KType ktype) {
    char *func_name = " [KDataDriver::_getImpPtr]";
    KDataDriverImpPtr result;
    if (ktype >= KQuery::INVALID_KTYPE) {
        HKU_ERROR("Invalid ktype!" << ktype << func_name);
        return result;
    }

    string key(market + m_suffix[ktype]);
    boost::to_lower(key);
    map<string, KDataDriverImpPtr>::iterator iter = m_imp.find(key);
    if (iter != m_imp.end())
        result = iter->second;

    return result;
}

void KDataDriver::
loadKData(const string& market, const string& code, KQuery::KType kType,
        size_t start_ix, size_t end_ix, KRecordList* out_buffer) {
    KDataDriverImpPtr imp = _getImpPtr(market, kType);
    if (imp)
        imp->loadKData(market, code, kType, start_ix, end_ix, out_buffer);
}

size_t KDataDriver::
getCount(const string& market, const string& code, KQuery::KType kType) {
    KDataDriverImpPtr imp = _getImpPtr(market, kType);
    if (imp)
        return imp->getCount(market, code, kType);
    return 0;
}

bool KDataDriver::
getIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    KDataDriverImpPtr imp = _getImpPtr(market, query.kType());
    if (imp)
        return imp->getIndexRangeByDate(market, code, query,
                out_start, out_end);
    return false;
}

KRecord KDataDriver::
getKRecord(const string& market, const string& code,
        size_t pos, KQuery::KType kType) {
    KDataDriverImpPtr imp = _getImpPtr(market, kType);
    if (imp)
        return imp->getKRecord(market, code, pos, kType);
    return Null<KRecord>();
}

} /* namespace hku */



