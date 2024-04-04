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

#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/lexical_cast.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/nvp.hpp>
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

/**
 * 供需要命名参数设定的类使用
 * @details 在需要命名参数设定的类定义中，增加宏PARAMETER_SUPPORT，如：
 * @code
 * #//C++示例：
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
 * 由于Python的限制，目前只支持int、bool、double三种类型，增加新的类型支持，
 * 需要修改以下几处，对于不支持的类型将在add时直接抛出异常，请勿捕捉次异常，
 * 这样导致的程序终止，可以尽快补充需支持的类型：
 * 1、Parameter::support
 * 2、std::ostream& operator <<(std::ostream &os, const Parameter& param)
 * 3、_Parameter.cpp中的AnyToPython、arameter::add<object>、Parameter::set<object>
 * 4、getNameValueList
 * 5、Parameter的序列化支持 <br/>
 * 在Python中，增加和修改参数，需要先创建或获取Parameter对象实例，通过Parameter示例
 * 造成，之后将Parameter实例直接设定修改，如：
 * 1、在init中增加参数
 *    param = Parameter();
 *    param.add("n", 1")
 *    param.add("bool", false)
 *    self.setParameter(param)
 * 2、修改参数
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

    /** 判断输入对象是否属于支持的类型 */
    static bool support(const boost::any&);

    /** 获取所有参数名称列表 */
    StringList getNameList() const;

    /** 返回形如"name1=val1,name2=val2,..."的字符串 */
    string getNameValueList() const;

    /** 是否存在指定名称的参数 */
    bool have(const string& name) const noexcept {
        return m_params.find(name) != m_params.end();
    }

    /** 获取参数个数 */
    size_t size() const {
        return m_params.size();
    }

    /**
     * 获取指定参数的实际类型
     * @param name 指定参数名称
     * @return "string" | "int" | "double" | "bool" | "Stock" |
     *         "KQuery" | "KData" | "PriceList" | "DatetimeList"
     */
    string type(const string& name) const;

    /**
     * 设定指定的参数值
     * @note 已经存在的参数修改其值，不存在的参数进行增加
     * @param name 参数名称
     * @param value 参数值
     */
    template <typename ValueType>
    void set(const string& name, const ValueType& value);

    /**
     * 获取指定的参数值，参数不存在或类型不匹配时，抛出异常
     * @param name 参数名
     * @return 参数值
     */
    template <typename ValueType>
    ValueType get(const string& name) const;

    /**
     * 尝试获取指定的参数值，参数不存在或类型不匹配时，返回缺省值
     * @param name 参数名
     * @param val 缺省值
     * @return 参数值
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
// 序列化支持
//================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;

    struct ItemRecord {
        friend class boost::serialization::access;

        ItemRecord() {}
        ItemRecord(const string& name, const boost::any& arg) : name(name) {
            if (arg.type() == typeid(bool)) {
                type = "bool";
                bool x = boost::any_cast<bool>(arg);
                value = boost::lexical_cast<string>(x);
            } else if (arg.type() == typeid(int)) {
                type = "int";
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
            } else if (arg.type() == typeid(string)) {
                type = "string";
                value = boost::any_cast<string>(arg);
            } else if (arg.type() == typeid(Stock)) {
                type = "stock";
                value = "stock";
                stock = boost::any_cast<Stock>(arg);
            } else if (arg.type() == typeid(KQuery)) {
                type = "query";
                value = "query";
                query = boost::any_cast<KQuery>(arg);
            } else if (arg.type() == typeid(KData)) {
                type = "kdata";
                value = "kdata";
                kdata = boost::any_cast<KData>(arg);
            } else if (arg.type() == typeid(PriceList)) {
                type = "PriceList";
                value = "price_list";
                price_list = boost::any_cast<PriceList>(arg);
            } else if (arg.type() == typeid(DatetimeList)) {
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
        Stock stock;
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
            ar& BOOST_SERIALIZATION_NVP(query);
            ar& BOOST_SERIALIZATION_NVP(kdata);
            ar& BOOST_SERIALIZATION_NVP(price_list);
            ar& BOOST_SERIALIZATION_NVP(date_list);
        }
    };

    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        size_t total = m_params.size();
        ar& bs::make_nvp<size_t>("count", total);
        param_map_t::const_iterator iter = m_params.begin();
        for (; iter != m_params.end(); ++iter) {
            ItemRecord record(iter->first, iter->second);
            ar& bs::make_nvp<ItemRecord>("Item", record);
        }
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        size_t total = 0;
        ar& bs::make_nvp<size_t>("count", total);
        ItemRecord record;
        for (size_t i = 0; i < total; i++) {
            ar& bs::make_nvp<ItemRecord>("Item", record);

            if (record.type == "bool") {
                m_params[record.name] = boost::lexical_cast<bool>(record.value);
            } else if (record.type == "int") {
                m_params[record.name] = boost::lexical_cast<int>(record.value);
            } else if (record.type == "int64") {
                m_params[record.name] = boost::lexical_cast<int64_t>(record.value);
            } else if (record.type == "double") {
                m_params[record.name] = boost::lexical_cast<double>(record.value);
            } else if (record.type == "string") {
                m_params[record.name] = record.value;
            } else if (record.type == "stock") {
                m_params[record.name] = record.stock;
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
 * 支持自定义类参数检查及变化通知
 * 子类需要实现重载以下虚函数接口:
 *    virtual void _checkParam(const string& name) const
 * 基类需要实现以下接口:
 *    void baseCheckParam(const string& name)
 *    void paramChanged()
 * 另：python 中一般不需要引出 paramChanged/checkParam/_checkParam，python
 * 类继承时可以自己在初始化时进行检查
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
    return boost::any_cast<ValueType>(iter->second);
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
    if (!have(name)) {
        if (!support(value)) {
            throw std::logic_error("Unsuport Type! input valut type: " +
                                   string(typeid(ValueType).name()));
        }
        m_params[name] = value;
        return;
    }

    if (m_params[name].type() != typeid(ValueType)) {
        if ((m_params[name].type() == typeid(int) || m_params[name].type() == typeid(int64_t)) &&
            (typeid(ValueType) == typeid(int) || typeid(ValueType) == typeid(int64_t))) {
            // 忽略，允许设定
        } else {
            throw std::logic_error("Mismatching type! need type " +
                                   string(m_params[name].type().name()) + " but value type is " +
                                   string(typeid(ValueType).name()));
        }
    }

    m_params[name] = value;
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
inline void Parameter::set(const string& name, const boost::any& value) {
    if (!have(name)) {
        m_params[name] = value;
        return;
    }

    if (m_params[name].type() != value.type()) {
        throw std::logic_error("Mismatching type! need type " +
                               string(m_params[name].type().name()) + " but value type is " +
                               string(value.type().name()));
    }

    m_params[name] = value;
}

HKU_API bool operator==(const Parameter&, const Parameter&);
HKU_API bool operator!=(const Parameter&, const Parameter&);
HKU_API bool operator<(const Parameter&, const Parameter&);

} /* namespace hku */
#endif /* PARAMETER_H_ */
