/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240907 added by fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/system/System.h"

namespace hku {

class HKU_API OptimizeParam {
public:
    enum OPTIMIZE_PARAM_TYPE {
        OPT_PARAM_INT,
        OPT_PARAM_DOUBLE,
        OPT_PARAM_BOOL,
        OPT_PARAM_STRING,
        OPT_PARAM_INVLID,
    };

    OptimizeParam() = default;
    OptimizeParam(SystemPart part, const string& name_, OPTIMIZE_PARAM_TYPE type_)
    : m_name(name_), m_part(part), m_type(type_) {}
    virtual ~OptimizeParam() = default;

    SystemPart part() const noexcept {
        return m_part;
    }

    const string& name() const noexcept {
        return m_name;
    }

    OPTIMIZE_PARAM_TYPE type() const noexcept {
        return m_type;
    }

    virtual string str() const {
        return fmt::format("OptimizeParam({}, {})", getSystemPartName(m_part), m_name);
    }

protected:
    string m_name;
    SystemPart m_part;
    OPTIMIZE_PARAM_TYPE m_type{OPT_PARAM_INVLID};
};

typedef std::shared_ptr<OptimizeParam> OptimizeParamPtr;
typedef std::vector<OptimizeParamPtr> OptimizeParamList;

class HKU_API OptimizeParamInt : public OptimizeParam {
public:
    OptimizeParamInt() = default;
    OptimizeParamInt(SystemPart part, const string& name, int start_, int end_, int step_ = 1)
    : OptimizeParam(part, name, OPT_PARAM_INT),
      m_start(start_),
      m_end(end_),
      m_step(step_),
      m_current(start_) {}
    virtual ~OptimizeParamInt() = default;

    int getValue() {
        HKU_IF_RETURN(m_current >= m_end, Null<int>());
        int ret = m_current;
        m_current += m_step;
        return ret;
    }

    virtual string str() const override {
        return fmt::format("OptimizeParamInt({}, {}, {}, {}, {})", getSystemPartName(m_part),
                           m_name, m_start, m_end, m_step);
    }

private:
    int m_start{0};
    int m_end{0};
    int m_step{0};
    int m_current{0};
};

class HKU_API OptimizeParamDouble : public OptimizeParam {
public:
    OptimizeParamDouble() = default;
    OptimizeParamDouble(SystemPart part, const string& name, double start_, double end_,
                        double step_)
    : OptimizeParam(part, name, OPT_PARAM_DOUBLE),
      m_start(start_),
      m_end(end_),
      m_step(step_),
      m_current(start_) {}
    virtual ~OptimizeParamDouble() = default;

    double getValue() {
        HKU_IF_RETURN(m_current >= m_step, Null<double>());
        double ret = m_current;
        m_current += m_step;
        return ret;
    }

    virtual string str() const override {
        return fmt::format("OptimizeParamDouble({}, {}, {}, {}, {})", getSystemPartName(m_part),
                           m_name, m_start, m_end, m_step);
    }

private:
    double m_start{0.0};
    double m_end{0.0};
    double m_step{0.0};
    double m_current{0.0};
};

class HKU_API OptimizeParamBool : public OptimizeParam {
public:
    OptimizeParamBool() = default;
    OptimizeParamBool(SystemPart part, const string& name)
    : OptimizeParam(part, name, OPT_PARAM_BOOL) {}
    virtual ~OptimizeParamBool() = default;

    uint8_t getValue() {
        HKU_IF_RETURN(m_current >= 2, -1);
        uint8_t ret = m_current;
        m_current++;
        return ret;
    }

    virtual string str() const override {
        return fmt::format("OptimizeParamBool({}, {})", getSystemPartName(m_part), m_name);
    }

private:
    uint8_t m_current{0};
};

class HKU_API OptimizeParamString : public OptimizeParam {
public:
    OptimizeParamString() = default;
    OptimizeParamString(SystemPart part, const string& name, const StringList& values_)
    : OptimizeParam(part, name, OPT_PARAM_STRING), m_values(values_) {}
    virtual ~OptimizeParamString() = default;

    string getValue() {
        HKU_IF_RETURN(m_current >= m_values.size(), Null<string>());
        string ret = m_values[m_current];
        m_current++;
        return ret;
    }

    virtual string str() const override {
        return fmt::format("OptimizeParamString({}, {}, values=...)", getSystemPartName(m_part),
                           m_name);
    }

private:
    StringList m_values;
    size_t m_current;
};

SYSPtr HKU_API findOptimizeParam(const SYSPtr& sys, const Stock& stk, const KQuery& query,
                                 const OptimizeParamList& optParams,
                                 const string& sort_key = string());

}  // namespace hku