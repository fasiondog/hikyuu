/*
 * Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "Parameter.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os, const Parameter& param) {
    //os << std::fixed;
    //os.precision(4);
    os << "params[";
    string strip(", ");
    string colon(": ");
    Parameter::param_map_t::const_iterator iter = param.m_params.begin();
    for (; iter != param.m_params.end(); ++iter) {
        os << iter->first;
        if (iter->second.type() == typeid(int)) {
            os << "(i): "
               << boost::any_cast<int>(iter->second) << strip;
        } else if (iter->second.type() == typeid(bool)) {
            os << "(b): "
               << boost::any_cast<bool>(iter->second) << strip;
        } else if (iter->second.type() == typeid(double)) {
            os << "(d): "
               << boost::any_cast<double>(iter->second) << strip;
        } else if (iter->second.type() == typeid(string)) {
            os << "(s): "
               << boost::any_cast<string>(iter->second) << strip;
        } else {
            os << "Unsupported" << strip;
        }
    }
    os << "]";
    return os;
}


Parameter::Parameter() {

}


Parameter::Parameter(const Parameter& p) {
    m_params = p.m_params;
}


Parameter::~Parameter() {

}


Parameter& Parameter::operator=(const Parameter& p) {
    if (this == &p) {
        return *this;
    }

    m_params = p.m_params;
    return *this;
}


bool Parameter::support(const boost::any& value) {
    if (value.type() == typeid(int)
            || value.type() == typeid(bool)
            || value.type() == typeid(double)
            || value.type() == typeid(string)) {
        return true;
    }

    return false;
}


StringList Parameter::getNameList() const {
    vector<string> result;
    param_map_t::const_iterator iter = m_params.begin();
    for (; iter != m_params.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}


string Parameter::getValueList() const {
    std::stringstream os;
    Parameter::param_map_t::const_iterator iter = m_params.begin();
    Parameter::param_map_t::const_iterator next_iter = iter;
    for (; iter != m_params.end(); ++iter) {
        if (iter->second.type() == typeid(int)) {
            os << boost::any_cast<int>(iter->second);
        } else if (iter->second.type() == typeid(bool)) {
            os << boost::any_cast<bool>(iter->second);
        } else if (iter->second.type() == typeid(double)) {
            os << boost::any_cast<double>(iter->second);
        } else if (iter->second.type() == typeid(string)) {
            os << boost::any_cast<string>(iter->second);
        } else {
            os << "Unsupported";
        }

        next_iter++;
        if (next_iter != m_params.end()) {
            os << ",";
        }
    }
    return os.str();
}


string Parameter::getNameValueList() const {
    std::stringstream os;
    Parameter::param_map_t::const_iterator iter = m_params.begin();
    Parameter::param_map_t::const_iterator next_iter = iter;
    string equal("=");
    for (; iter != m_params.end(); ++iter) {
        if (iter->second.type() == typeid(int)) {
            os << iter->first << equal
               << boost::any_cast<int>(iter->second);
        } else if (iter->second.type() == typeid(bool)) {
            os << iter->first << equal
               << boost::any_cast<bool>(iter->second);
        } else if (iter->second.type() == typeid(double)) {
            os << iter->first << equal
               << boost::any_cast<double>(iter->second);
        } else if (iter->second.type() == typeid(string)) {
            os << "\"" << iter->first << "\"" << equal
               << boost::any_cast<string>(iter->second);
        } else {
            os << "Unsupported";
        }

        next_iter++;
        if (next_iter != m_params.end()) {
            os << ",";
        }
    }
    return os.str();
}

HKU_API bool operator==(const Parameter& p1, const Parameter& p2) {
    //注意：参数大小写敏感
    return p1.getNameValueList() == p2.getNameValueList();
}

HKU_API bool operator!=(const Parameter& p1, const Parameter& p2) {
    //注意：参数大小写敏感
    return p1.getNameValueList() != p2.getNameValueList();
}

HKU_API bool operator<(const Parameter& p1, const Parameter& p2) {
    return p1.getNameValueList() < p2.getNameValueList();
}


} /* namespace hku */
