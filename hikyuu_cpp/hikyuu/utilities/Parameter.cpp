/*
 * Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/Block.h"
#include "Parameter.h"

namespace hku {

template <class T>
static bool isType(const boost::any& value) {
    return strcmp(value.type().name(), typeid(T).name()) == 0;
}

HKU_API std::ostream& operator<<(std::ostream& os, const Parameter& param) {
    // os << std::fixed;
    // os.precision(4);
    os << "params[";
    string strip(", ");
    Parameter::param_map_t::const_iterator iter = param.m_params.begin();
    for (; iter != param.m_params.end(); ++iter) {
        os << iter->first;
        if (iter->second.type() == typeid(int)) {
            os << "(int): " << boost::any_cast<int>(iter->second) << strip;
        } else if (iter->second.type() == typeid(int64_t)) {
            os << "(int64): " << boost::any_cast<int64_t>(iter->second) << strip;
        } else if (iter->second.type() == typeid(bool)) {
            os << "(bool): " << boost::any_cast<bool>(iter->second) << strip;
        } else if (iter->second.type() == typeid(double)) {
            os << "(double): " << boost::any_cast<double>(iter->second) << strip;
        } else if (isType<string>(iter->second)) {
            os << "(string): " << boost::any_cast<string>(iter->second) << strip;
        } else if (isType<Stock>(iter->second)) {
            os << "(Stock): " << boost::any_cast<Stock>(iter->second).market_code() << strip;
        } else if (isType<Block>(iter->second)) {
            os << "(Block): " << boost::any_cast<const Block&>(iter->second) << strip;
        } else if (isType<KQuery>(iter->second)) {
            os << "(Query): " << boost::any_cast<KQuery>(iter->second) << strip;
        } else if (isType<KData>(iter->second)) {
            os << "(KData): ";
            Stock stk = boost::any_cast<KData>(iter->second).getStock();
            if (stk.isNull()) {
                os << "Null" << strip;
            } else {
                os << stk.market_code() << strip;
            }
        } else if (isType<PriceList>(iter->second)) {
            os << "(PriceList): " << boost::any_cast<PriceList>(iter->second).size() << strip;
        } else if (isType<DatetimeList>(iter->second)) {  // (strcmp(iter->second.type().name(),
                                                          // typeid(DatetimeList).name()) == 0) {
            os << "(DatetimeList): " << boost::any_cast<DatetimeList>(iter->second).size() << strip;
        } else {
            os << "Unsupported" << strip;
        }
    }
    os << "]";
    return os;
}

Parameter::Parameter() {}

Parameter::Parameter(const Parameter& p) : m_params(p.m_params) {}

Parameter::~Parameter() {}

Parameter& Parameter::operator=(const Parameter& p) {
    if (this == &p) {
        return *this;
    }

    m_params = p.m_params;
    return *this;
}

bool Parameter::support(const boost::any& value) {
    return value.type() == typeid(int) || value.type() == typeid(int64_t) ||
           value.type() == typeid(bool) || value.type() == typeid(double) ||
           isType<string>(value) || isType<Stock>(value) || isType<Block>(value) ||
           isType<KQuery>(value) || isType<KData>(value) || isType<PriceList>(value) ||
           isType<DatetimeList>(value);
}

string Parameter::type(const string& name) const {
    auto iter = m_params.find(name);
    HKU_CHECK_THROW(iter != m_params.end(), std::out_of_range,
                    "out_of_range in Parameter::get : {}", name);
    HKU_IF_RETURN(iter->second.type() == typeid(int), "int");
    HKU_IF_RETURN(iter->second.type() == typeid(int64_t), "int64");
    HKU_IF_RETURN(iter->second.type() == typeid(bool), "bool");
    HKU_IF_RETURN(iter->second.type() == typeid(double), "double");
    HKU_IF_RETURN(isType<string>(iter->second), "string");
    HKU_IF_RETURN(isType<Stock>(iter->second), "Stock");
    HKU_IF_RETURN(isType<Block>(iter->second), "Block");
    HKU_IF_RETURN(isType<KQuery>(iter->second), "KQuery");
    HKU_IF_RETURN(isType<KData>(iter->second), "KData");
    HKU_IF_RETURN(isType<PriceList>(iter->second), "PriceList");
    HKU_IF_RETURN(isType<DatetimeList>(iter->second), "DatetimeList");
    return "Unknown";
}

StringList Parameter::getNameList() const {
    vector<string> result;
    param_map_t::const_iterator iter = m_params.begin();
    for (; iter != m_params.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

string Parameter::getNameValueList() const {
    std::stringstream os;
    Parameter::param_map_t::const_iterator iter = m_params.begin();
    Parameter::param_map_t::const_iterator next_iter = iter;
    string equal("=");
    for (; iter != m_params.end(); ++iter) {
        if (iter->second.type() == typeid(int)) {
            os << iter->first << equal << boost::any_cast<int>(iter->second);
        } else if (iter->second.type() == typeid(int64_t)) {
            os << iter->first << equal << boost::any_cast<int64_t>(iter->second);
        } else if (iter->second.type() == typeid(bool)) {
            os << iter->first << equal << boost::any_cast<bool>(iter->second);
        } else if (iter->second.type() == typeid(double)) {
            os << iter->first << equal << boost::any_cast<double>(iter->second);
        } else if (isType<string>(iter->second)) {
            os << "\"" << iter->first << "\"" << equal << boost::any_cast<string>(iter->second);
        } else if (isType<Stock>(iter->second)) {
            os << iter->first << equal << boost::any_cast<Stock>(iter->second);
        } else if (isType<Block>(iter->second)) {
            os << iter->first << equal << boost::any_cast<const Block&>(iter->second);
        } else if (isType<KQuery>(iter->second)) {
            os << iter->first << equal << boost::any_cast<KQuery>(iter->second);
        } else if (isType<KData>(iter->second)) {
            Stock stk = boost::any_cast<KData>(iter->second).getStock();
            if (stk.isNull()) {
                os << iter->first << equal << "KData(Null)";
            } else {
                os << iter->first << equal << "KData(" << stk.market_code() << "...)";
            }
        } else if (isType<PriceList>(iter->second)) {
            os << iter->first << equal << "PriceList(...)";
        } else if (isType<DatetimeList>(iter->second)) {
            os << iter->first << equal << "DatetimeList(...)";
        } else {
            os << "Unsupported";
        }

        ++next_iter;
        if (next_iter != m_params.end() && iter->second.type() != typeid(KData)) {
            os << ",";
        }
    }
    return os.str();
}

HKU_API bool operator==(const Parameter& p1, const Parameter& p2) {
    // 注意：参数大小写敏感
    HKU_IF_RETURN(p1.size() != p2.size(), false);

    auto iter1 = p1.begin();
    auto iter2 = p2.begin();
    for (; iter1 != p1.end() && iter2 != p2.end(); ++iter1, ++iter2) {
        try {
            HKU_IF_RETURN(
              iter1->first != iter2->first || iter1->second.type() != iter2->second.type(), false);
            HKU_IF_RETURN(
              iter1->second.type() == typeid(int) &&
                boost::any_cast<int>(iter1->second) != boost::any_cast<int>(iter2->second),
              false);
            HKU_IF_RETURN(
              iter1->second.type() == typeid(int64_t) &&
                boost::any_cast<int64_t>(iter1->second) != boost::any_cast<int64_t>(iter2->second),
              false);
            HKU_IF_RETURN(
              iter1->second.type() == typeid(bool) &&
                boost::any_cast<bool>(iter1->second) != boost::any_cast<bool>(iter2->second),
              false);
            HKU_IF_RETURN(
              iter1->second.type() == typeid(double) &&
                boost::any_cast<double>(iter1->second) != boost::any_cast<double>(iter2->second),
              false);

            if (iter1->second.type() == typeid(string)) {
                const string* x1 = boost::any_cast<string>(&iter1->second);
                const string* x2 = boost::any_cast<string>(&iter2->second);
                HKU_IF_RETURN(*x1 != *x2, false);
            }

            if (iter1->second.type() == typeid(Stock)) {
                const Stock* x1 = boost::any_cast<Stock>(&iter1->second);
                const Stock* x2 = boost::any_cast<Stock>(&iter2->second);
                HKU_IF_RETURN(*x1 != *x2, false);
            }

            if (iter1->second.type() == typeid(Block)) {
                const Block* x1 = boost::any_cast<Block>(&iter1->second);
                const Block* x2 = boost::any_cast<Block>(&iter2->second);
                HKU_IF_RETURN(*x1 != *x2, false);
            }

            if (iter1->second.type() == typeid(KQuery)) {
                const KQuery* x1 = boost::any_cast<KQuery>(&iter1->second);
                const KQuery* x2 = boost::any_cast<KQuery>(&iter2->second);
                HKU_IF_RETURN(*x1 != *x2, false);
            }

            if (iter1->second.type() == typeid(KData)) {
                const KData* x1 = boost::any_cast<KData>(&iter1->second);
                const KData* x2 = boost::any_cast<KData>(&iter2->second);
                HKU_IF_RETURN(*x1 != *x2, false);
            }

            if (iter1->second.type() == typeid(PriceList)) {
                const PriceList* x1 = boost::any_cast<PriceList>(&iter1->second);
                const PriceList* x2 = boost::any_cast<PriceList>(&iter2->second);
                HKU_IF_RETURN(x1->size() != x2->size(), false);
                for (size_t i = 0, len = x1->size(); i < len; i++) {
                    HKU_IF_RETURN((*x1)[i] != (*x2)[i], false);
                }
            }

            if (iter1->second.type() == typeid(DatetimeList)) {
                const DatetimeList* x1 = boost::any_cast<DatetimeList>(&iter1->second);
                const DatetimeList* x2 = boost::any_cast<DatetimeList>(&iter2->second);
                HKU_IF_RETURN(x1->size() != x2->size(), false);
                for (size_t i = 0, len = x1->size(); i < len; i++) {
                    HKU_IF_RETURN((*x1)[i] != (*x2)[i], false);
                }
            }
        } catch (...) {
            HKU_ERROR("failed conversion iter1 key: {}, iter2 key: {}", iter1->first, iter2->first);
        }
    }

    return true;
}

HKU_API bool operator!=(const Parameter& p1, const Parameter& p2) {
    // 注意：参数大小写敏感
    return !(p1 == p2);
}

HKU_API bool operator<(const Parameter& p1, const Parameter& p2) {
    return p1.getNameValueList() < p2.getNameValueList();
}

} /* namespace hku */
