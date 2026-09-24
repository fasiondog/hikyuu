/*
 * Parameter.h
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#pragma once
#ifndef PARAMETER_H_
#define PARAMETER_H_

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <boost/any.hpp>

#include "hikyuu/config.h"
#include "hikyuu/utilities/config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/lexical_cast.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
#include "../serialization/Block_serialization.h"
#include "../serialization/KData_serialization.h"
#include "../serialization/Datetime_serialization.h"
#else
#include "../KData.h"
#endif

#ifndef DATATYPE_H_
#ifndef HKU_API
#define HKU_API
#endif
#endif

namespace hku {

#ifndef DATATYPE_H_
using std::map;
using std::string;
using std::vector;
typedef vector<string> StringList;
#endif

#if HKU_SUPPORT_SERIALIZATION
struct ParamItemRecord {
    friend class boost::serialization::access;

    ParamItemRecord() {}
    ParamItemRecord(const string& name, const boost::any& arg) : name(name) {
        if (arg.type() == typeid(bool)) {
            type = "bool";
            bool x = boost::any_cast<bool>(arg);
            value = boost::lexical_cast<string>(x);
        } else if (arg.type() == typeid(int)) {
            type = "int64";
            int x = boost::any_cast<int>(arg);
            value = boost::lexical_cast<string>(x);
        } else if (arg.type() == typeid(int64_t)) {
            type = "int64";
            int64_t x = boost::any_cast<int64_t>(arg);
            value = boost::lexical_cast<string>(x);
        } else if (arg.type() == typeid(double)) {
            type = "double";
            double x = boost::any_cast<double>(arg);
            value = boost::lexical_cast<string>(x);
        } else if (strcmp(arg.type().name(), typeid(string).name()) == 0) {
            type = "string";
            value = boost::any_cast<string>(arg);
        } else if (strcmp(arg.type().name(), typeid(Datetime).name()) == 0) {
            type = "Datetime";
            datetime = boost::any_cast<Datetime>(arg);
        } else if (strcmp(arg.type().name(), typeid(Stock).name()) == 0) {
            type = "stock";
            value = "stock";
            stock = boost::any_cast<Stock>(arg);
        } else if (strcmp(arg.type().name(), typeid(Block).name()) == 0) {
            type = "block";
            value = "block";
            block = boost::any_cast<Block>(arg);
        } else if (strcmp(arg.type().name(), typeid(KQuery).name()) == 0) {
            type = "query";
            value = "query";
            query = boost::any_cast<KQuery>(arg);
        } else if (strcmp(arg.type().name(), typeid(KData).name()) == 0) {
            type = "kdata";
            value = "kdata";
            kdata = boost::any_cast<KData>(arg);
        } else if (strcmp(arg.type().name(), typeid(PriceList).name()) == 0) {
            type = "PriceList";
            value = "price_list";
            price_list = boost::any_cast<PriceList>(arg);
        } else if (strcmp(arg.type().name(), typeid(DatetimeList).name()) == 0) {
            type = "DatetimeList";
            value = "date_list";
            date_list = boost::any_cast<DatetimeList>(arg);
        } else {
            type = "Unknown";
            value = "Unknown";
        }
    }

    string name;
    string type;
    string value;
    Datetime datetime;
    Stock stock;
    Block block;
    KQuery query;
    KData kdata;
    PriceList price_list;
    DatetimeList date_list;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(name);
        ar& BOOST_SERIALIZATION_NVP(type);
        ar& BOOST_SERIALIZATION_NVP(value);
        ar& BOOST_SERIALIZATION_NVP(stock);
        ar& BOOST_SERIALIZATION_NVP(block);
        ar& BOOST_SERIALIZATION_NVP(query);
        ar& BOOST_SERIALIZATION_NVP(kdata);
        ar& BOOST_SERIALIZATION_NVP(price_list);
        ar& BOOST_SERIALIZATION_NVP(date_list);
        if (version >= 1) {
            ar& BOOST_SERIALIZATION_NVP(datetime);
        }
    }
};
#endif

/**
 * Used by the classes that need the named parameter settings
 * @details In the definition of a class that needs the named parameter settings, add the
 *          PARAMETER_SUPPORT macro, e.g.:
 * @code
 * #// C++ example:
 * class Test {
 *     PARAMETER_SUPPORT
 *
 * public:
 *     Test();
 *     virtual ~Test();
 *
 *     void change(int n);
 *     void calculate();
 * };
 *
 * Test::Test() {
 *     addParam<int>("n", 10);
 * }
 *
 * void Test::change(int n) {
 *     setParam<int>("n", n);
 * }
 *
 * void Test::calculate() {
 *     int n = getParam<int>("n");
 *     ....
 * }
 * @endcode
 *
 * @note
 * <pre>
 * Because of the limitation of Python, only the int, bool and double types are supported at
 * present; to add the support of a new type, the following places need to be modified; for an
 * unsupported type an exception is thrown directly at add, please do not catch this exception, so
 * that the program termination caused in this way can complete the types to be supported as soon as
 * possible: 1、Parameter::support 2、std::ostream& operator <<(std::ostream &os, const Parameter&
 * param)
 * 3. AnyToPython, Parameter::add<object> and Parameter::set<object> in _Parameter.cpp
 * 4、getNameValueList
 * 5. The serialization support of Parameter <br/>
 * In Python, to add and modify the parameters a Parameter object instance needs to be created or
 * got first, and through the Parameter instance it is done, afterwards the Parameter instance is
 * set and modified directly, e.g.:
 * 1. Add the parameters in init
 *    param = Parameter();
 *    param.add("n", 1")
 *    param.add("bool", false)
 *    self.setParameter(param)
 * 2. Modify the parameters
 *    param = x.getParameter()
 *    param.set("n", 10)
 *    param.set("bool", true)
 *    x.setParameter(param)
 * </pre>
 *
 * @ingroup Utilities
 */
class HKU_API Parameter {
    HKU_API friend std::ostream& operator<<(std::ostream& os, const Parameter& param);

public:
    Parameter();
    Parameter(const Parameter&);
    virtual ~Parameter();

    Parameter& operator=(const Parameter&);

    Parameter& operator=(Parameter&&);

    /** Judge whether the input object belongs to a supported type */
    static bool support(const boost::any&);

    /** Get the name list of all the parameters */
    StringList getNameList() const;

    /** Return a string in the form "name1=val1,name2=val2,..." */
    string getNameValueList() const;

    /** Whether a parameter with the given name exists */
    bool have(const string& name) const noexcept {
        return m_params.find(name) != m_params.end();
    }

    /** Get the number of the parameters */
    size_t size() const {
        return m_params.size();
    }

    /**
     * Get the actual type of the given parameter
     * @param name the given parameter name
     * @return "string" | "int" | "double" | "bool" | "Stock" | "Block"
     *         "KQuery" | "KData" | "PriceList" | "DatetimeList"
     */
    string type(const string& name) const;

    /**
     * Set the given parameter value
     * @note An existing parameter has its value modified, a non-existing one is added
     * @param name parameter name
     * @param value parameter value
     */
    template <typename ValueType>
    void set(const string& name, ValueType&& value);

    template <typename ValueType>
    void set(const string& name, const ValueType& value);

    /**
     * Get the given parameter value; an exception is thrown when the parameter does not exist or
     * the type does not match
     * @param name parameter name
     * @return parameter value
     */
    template <typename ValueType>
    ValueType get(const string& name) const;

    /**
     * Try to get the given parameter value; the default value is returned when the parameter does
     * not exist or the type does not match
     * @param name parameter name
     * @param val the default value
     * @return parameter value
     */
    template <typename ValueType>
    ValueType tryGet(const string& name, const ValueType& val) const;

    typedef map<string, boost::any> param_map_t;
    typedef param_map_t::const_iterator iterator;

    iterator begin() const {
        return m_params.begin();
    }

    iterator end() const {
        return m_params.end();
    }

private:
    param_map_t m_params;

//================================
// Serialization support
//================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;

    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        size_t total = m_params.size();
        ar& bs::make_nvp<size_t>("count", total);
        param_map_t::const_iterator iter = m_params.begin();
        for (; iter != m_params.end(); ++iter) {
            ParamItemRecord record(iter->first, iter->second);
            ar& bs::make_nvp<ParamItemRecord>("Item", record);
        }
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        size_t total = 0;
        ar& bs::make_nvp<size_t>("count", total);
        ParamItemRecord record;
        for (size_t i = 0; i < total; i++) {
            ar& bs::make_nvp<ParamItemRecord>("Item", record);

            if (record.type == "bool") {
                m_params[record.name] = boost::lexical_cast<bool>(record.value);
            } else if (record.type == "int") {
                m_params[record.name] = boost::lexical_cast<int64_t>(record.value);
            } else if (record.type == "int64") {
                m_params[record.name] = boost::lexical_cast<int64_t>(record.value);
            } else if (record.type == "double") {
                m_params[record.name] = boost::lexical_cast<double>(record.value);
            } else if (record.type == "string") {
                m_params[record.name] = record.value;
            } else if (record.type == "Datetime") {
                m_params[record.name] = record.datetime;
            } else if (record.type == "stock") {
                m_params[record.name] = record.stock;
            } else if (record.type == "block") {
                m_params[record.name] = record.block;
            } else if (record.type == "query") {
                m_params[record.name] = record.query;
            } else if (record.type == "kdata") {
                m_params[record.name] = record.kdata;
            } else if (record.type == "PriceList") {
                m_params[record.name] = record.price_list;
            } else if (record.type == "DatetimeList") {
                m_params[record.name] = record.date_list;
            } else {
                std::cout << "Unknown type! [Parameter::load]" << std::endl;
            }
        }
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#define PARAMETER_SUPPORT                                                   \
protected:                                                                  \
    Parameter m_params;                                                     \
                                                                            \
public:                                                                     \
    const Parameter& getParameter() const {                                 \
        return m_params;                                                    \
    }                                                                       \
                                                                            \
    void setParameter(const Parameter& param) {                             \
        m_params = param;                                                   \
    }                                                                       \
                                                                            \
    bool haveParam(const string& name) const noexcept {                     \
        return m_params.have(name);                                         \
    }                                                                       \
                                                                            \
    template <typename ValueType>                                           \
    void setParam(const string& name, const ValueType& value) {             \
        m_params.set<ValueType>(name, value);                               \
    }                                                                       \
    template <typename ValueType>                                           \
    void setParam(const string& name, ValueType& value) {                   \
        m_params.set<ValueType>(name, std::forward<ValueType>(value));      \
    }                                                                       \
                                                                            \
    template <typename ValueType>                                           \
    ValueType getParam(const string& name) const {                          \
        return m_params.get<ValueType>(name);                               \
    }                                                                       \
                                                                            \
    template <typename ValueType>                                           \
    ValueType tryGetParam(const string& name, const ValueType& val) const { \
        return m_params.tryGet<ValueType>(name, val);                       \
    }                                                                       \
                                                                            \
    template <typename ValueType>                                           \
    ValueType getParamFromOther(const Parameter& other, const string& name, \
                                const ValueType& default_value) {           \
        if (other.have(name)) {                                             \
            setParam<ValueType>(name, other.get<ValueType>(name));          \
        } else {                                                            \
            setParam<ValueType>(name, default_value);                       \
        }                                                                   \
        return getParam<ValueType>(name);                                   \
    }

/**
 * It supports the parameter checking and the change notification of a custom class
 * The subclass needs to implement (overload) the following virtual function interfaces:
 *    virtual void _checkParam(const string& name) const
 * The base class needs to implement the following interfaces:
 *    void baseCheckParam(const string& name) const
 *    void paramChanged()
 * Note: paramChanged/checkParam/_checkParam generally do not need to be exposed in python, the
 * python subclass can do the checking itself during the initialization
 */
#define PARAMETER_SUPPORT_WITH_CHECK                                         \
protected:                                                                   \
    Parameter m_params;                                                      \
    void paramChanged();                                                     \
    void checkParam(const string& name) const {                              \
        baseCheckParam(name);                                                \
        _checkParam(name);                                                   \
    }                                                                        \
    virtual void _checkParam(const string& name) const {}                    \
                                                                             \
private:                                                                     \
    void baseCheckParam(const string& name) const;                           \
                                                                             \
public:                                                                      \
    const Parameter& getParameter() const {                                  \
        return m_params;                                                     \
    }                                                                        \
                                                                             \
    void setParameter(const Parameter& param) {                              \
        m_params = param;                                                    \
        for (auto iter = m_params.begin(); iter != m_params.end(); ++iter) { \
            checkParam(iter->first);                                         \
        }                                                                    \
        paramChanged();                                                      \
    }                                                                        \
                                                                             \
    bool haveParam(const string& name) const noexcept {                      \
        return m_params.have(name);                                          \
    }                                                                        \
                                                                             \
    template <typename ValueType>                                            \
    void setParam(const string& name, const ValueType& value) {              \
        m_params.set<ValueType>(name, value);                                \
        checkParam(name);                                                    \
        paramChanged();                                                      \
    }                                                                        \
    template <typename ValueType>                                            \
    void setParam(const string& name, ValueType&& value) {                   \
        m_params.set<ValueType>(name, std::forward<ValueType>(value));       \
        checkParam(name);                                                    \
        paramChanged();                                                      \
    }                                                                        \
                                                                             \
    template <typename ValueType>                                            \
    ValueType getParam(const string& name) const {                           \
        return m_params.get<ValueType>(name);                                \
    }                                                                        \
                                                                             \
    template <typename ValueType>                                            \
    ValueType tryGetParam(const string& name, const ValueType& val) const {  \
        return m_params.tryGet<ValueType>(name, val);                        \
    }                                                                        \
                                                                             \
    template <typename ValueType>                                            \
    ValueType getParamFromOther(const Parameter& other, const string& name,  \
                                const ValueType& default_value) {            \
        if (other.have(name)) {                                              \
            setParam<ValueType>(name, other.get<ValueType>(name));           \
        } else {                                                             \
            setParam<ValueType>(name, default_value);                        \
        }                                                                    \
        return getParam<ValueType>(name);                                    \
    }

template <typename ValueType>
ValueType Parameter::get(const string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    try {
        return boost::any_cast<ValueType>(iter->second);
    } catch (...) {
        throw std::runtime_error("failed conversion param: " + name);
    }
}

template <typename ValueType>
ValueType Parameter::tryGet(const string& name, const ValueType& val) const {
    try {
        return get<ValueType>(name);
    } catch (...) {
        return val;
    }
}

template <typename ValueType>
void Parameter::set(const string& name, const ValueType& value) {
    if constexpr (std::same_as<std::decay_t<ValueType>, boost::any>) {
        if (!have(name)) {
            if (value.type() == typeid(int)) {
                m_params[name] = static_cast<int64_t>(boost::any_cast<int>(value));
            } else {
                m_params[name] = value;
            }
            return;
        }

        if (strcmp(m_params[name].type().name(), value.type().name()) != 0) {
            throw std::logic_error("Mismatching type! need type " +
                                   string(m_params[name].type().name()) + " but value type is " +
                                   string(value.type().name()));
        }

        m_params[name] = value;

    } else if constexpr (std::same_as<std::decay_t<ValueType>, int>) {
        if (!have(name)) {
            m_params[name] = static_cast<int64_t>(value);
            return;
        }

        if (m_params[name].type() != typeid(int64_t) && m_params[name].type() != typeid(int)) {
            throw std::logic_error(
              "Mismatching type! need type int or int64_t, but value type is " +
              string(typeid(ValueType).name()));
        }
        m_params[name] = static_cast<int64_t>(value);

    } else {
        if (!have(name)) {
            if (!support(value)) {
                throw std::logic_error("Unsuport Type! input valut type: " +
                                       string(typeid(ValueType).name()));
            }
            m_params[name] = value;
            return;
        }

        if (strcmp(m_params[name].type().name(), typeid(ValueType).name()) != 0) {
            if ((m_params[name].type() == typeid(int) ||
                 m_params[name].type() == typeid(int64_t)) &&
                (typeid(ValueType) == typeid(int) || typeid(ValueType) == typeid(int64_t))) {
                // Ignored, the setting is allowed
            } else {
                throw std::logic_error("Mismatching type! need type " +
                                       string(m_params[name].type().name()) +
                                       " but value type is " + string(typeid(ValueType).name()));
            }
        }

        m_params[name] = value;
    }
}

template <typename ValueType>
void Parameter::set(const string& name, ValueType&& value) {
    if constexpr (std::same_as<std::decay_t<ValueType>, boost::any>) {
        if (!have(name)) {
            m_params[name] = std::forward<ValueType>(value);
            return;
        }

        if (strcmp(m_params[name].type().name(), typeid(ValueType).name()) != 0) {
            if ((m_params[name].type() == typeid(int64_t) ||
                 m_params[name].type() == typeid(int)) &&
                (typeid(ValueType) == typeid(int64_t) || typeid(ValueType) == typeid(int))) {
                // Ignored, the setting is allowed
            } else {
                throw std::logic_error("Mismatching type! need type " +
                                       string(m_params[name].type().name()) +
                                       " but value type is " + string(typeid(ValueType).name()));
            }
        }

        m_params[name] = std::forward<ValueType>(value);

    } else if constexpr (std::same_as<std::decay_t<ValueType>, int>) {
        if (!have(name)) {
            m_params[name] = value;
            return;
        }

        if (m_params[name].type() != typeid(int64_t) && m_params[name].type() != typeid(int)) {
            throw std::logic_error(
              "Mismatching type! need type int or int64_t, but value type is " +
              string(typeid(ValueType).name()));
        }
        m_params[name] = static_cast<int64_t>(value);

    } else {
        if (!have(name)) {
            if (!support(value)) {
                throw std::logic_error("Unsuport Type! input valut type: " +
                                       string(typeid(ValueType).name()));
            }
            m_params[name] = std::forward<ValueType>(value);
            return;
        }

        if (strcmp(m_params[name].type().name(), typeid(ValueType).name()) != 0) {
            if ((m_params[name].type() == typeid(int) ||
                 m_params[name].type() == typeid(int64_t)) &&
                (typeid(ValueType) == typeid(int) || typeid(ValueType) == typeid(int64_t))) {
                // Ignored, the setting is allowed
            } else {
                throw std::logic_error("Mismatching type! need type " +
                                       string(m_params[name].type().name()) +
                                       " but value type is " + string(typeid(ValueType).name()));
            }
        }

        m_params[name] = std::forward<ValueType>(value);
    }
}

template <>
inline boost::any Parameter::get<boost::any>(const std::string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    return iter->second;
}

template <>
inline int Parameter::get(const string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    try {
        if (iter->second.type() == typeid(int)) {
            return boost::any_cast<int>(iter->second);
        }
        return static_cast<int>(boost::any_cast<int64_t>(iter->second));
    } catch (...) {
        throw std::runtime_error("failed conversion param: " + name);
    }
}

template <>
inline int64_t Parameter::get(const string& name) const {
    param_map_t::const_iterator iter;
    iter = m_params.find(name);
    if (iter == m_params.end()) {
        throw std::out_of_range("out_of_range in Parameter::get : " + name);
    }
    if (iter->second.type() == typeid(int64_t)) {
        return boost::any_cast<int64_t>(iter->second);
    } else {
        return static_cast<int64_t>(boost::any_cast<int>(iter->second));
    }
}

HKU_API bool operator==(const Parameter&, const Parameter&);
HKU_API bool operator!=(const Parameter&, const Parameter&);
HKU_API bool operator<(const Parameter&, const Parameter&);

} /* namespace hku */

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_VERSION(::hku::ParamItemRecord, 1)
#endif

#endif /* PARAMETER_H_ */
