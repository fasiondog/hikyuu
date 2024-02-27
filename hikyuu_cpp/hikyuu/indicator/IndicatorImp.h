/*
 * IndicatorImp.h
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATORIMP_H_
#define INDICATORIMP_H_

#include "../config.h"
#include "../KData.h"
#include "../utilities/Parameter.h"
#include "../utilities/thread/StealThreadPool.h"

#if HKU_SUPPORT_SERIALIZATION
#if HKU_SUPPORT_XML_ARCHIVE
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif /* HKU_SUPPORT_XML_ARCHIVE */

#if HKU_SUPPORT_TEXT_ARCHIVE
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif /* HKU_SUPPORT_TEXT_ARCHIVE */

#if HKU_SUPPORT_BINARY_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif /* HKU_SUPPORT_BINARY_ARCHIVE */

#include <boost/serialization/map.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>

// linux 下，PriceList_serialization 始终无法特化（及时拷贝到本文件内也一样），取消引用
// #if HKU_SUPPORT_XML_ARCHIVE || HKU_SUPPORT_TEXT_ARCHIVE
// #include "../serialization/PriceList_serialization.h"
// #endif
#endif /* HKU_SUPPORT_SERIALIZATION */

namespace hku {

#define MAX_RESULT_NUM 6

class HKU_API Indicator;
class HKU_API IndParam;

/**
 * 指标实现类，定义新指标时，应从此类继承
 * @ingroup Indicator
 */
class HKU_API IndicatorImp : public enable_shared_from_this<IndicatorImp> {
    PARAMETER_SUPPORT
    friend HKU_API std::ostream& operator<<(std::ostream& os, const IndicatorImp& imp);

public:
    enum OPType {
        LEAF,   ///< 叶子节点
        OP,     /// OP(OP1,OP2) OP1->calcalue(OP2->calculate(ind))
        ADD,    ///< 加
        SUB,    ///< 减
        MUL,    ///< 乘
        DIV,    ///< 除
        MOD,    ///< 取模
        EQ,     ///< 等于
        GT,     ///< 大于
        LT,     ///< 小于
        NE,     ///< 不等于
        GE,     ///< 大于等于
        LE,     ///< 小于等于
        AND,    ///< 与
        OR,     ///< 或
        WEAVE,  ///< 特殊的，需要两个指标作为参数的指标
        OP_IF,  /// if操作
        CORR,   ///< 相关系数，需要两个指标作为参数
        INVALID
    };

#if HKU_USE_LOW_PRECISION
    typedef float value_t;
#else
    typedef double value_t;
#endif

public:
    /** 默认构造函数   */
    IndicatorImp();
    explicit IndicatorImp(const string& name);
    IndicatorImp(const string& name, size_t result_num);

    virtual ~IndicatorImp();

    typedef shared_ptr<IndicatorImp> IndicatorImpPtr;
    IndicatorImpPtr operator()(const Indicator& ind);

    size_t getResultNumber() const;

    size_t discard() const;

    void setDiscard(size_t discard);

    size_t size() const;

    value_t get(size_t pos, size_t num = 0) const;

    value_t getByDate(Datetime, size_t num = 0);

    Datetime getDatetime(size_t pos) const;

    DatetimeList getDatetimeList() const;

    size_t getPos(Datetime) const;

    /** 以PriceList方式获取指定的输出集 */
    PriceList getResultAsPriceList(size_t result_num);

    /** 以Indicator的方式获取指定的输出集，该方式包含了discard的信息 */
    IndicatorImpPtr getResult(size_t result_num);

    /** 判断是否和另一个指标等效，即计算效果相同 */
    bool alike(const IndicatorImp& other) const;

    /**
     * 使用IndicatorImp(const Indicator&...)构造函数后，计算结果使用该函数,
     * 未做越界保护
     */
    void _set(value_t val, size_t pos, size_t num = 0);

    /**
     * 准备内存
     * @param len 长度，如果长度大于MAX_RESULT_NUM将抛出异常std::invalid_argument
     * @param result_num 结果集数量
     * @return true 成功 | false 失败
     */
    void _readyBuffer(size_t len, size_t result_num);

    const string& name() const;
    void name(const string& name);

    /** 返回形如：Name(param1=val,param2=val,...) */
    string long_name() const;

    string formula() const;

    bool isLeaf() const;

    Indicator calculate();

    void setContext(const Stock&, const KQuery&);

    void setContext(const KData&);

    KData getContext() const;

    void add(OPType, IndicatorImpPtr left, IndicatorImpPtr right);

    void add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right);

    IndicatorImpPtr clone();

    bool haveIndParam(const string& name) const;
    void setIndParam(const string& name, const Indicator& ind);
    void setIndParam(const string& name, const IndParam& ind);
    IndParam getIndParam(const string& name) const;
    const IndicatorImpPtr& getIndParamImp(const string& name) const;

    typedef std::map<string, IndicatorImpPtr> ind_param_map_t;
    const ind_param_map_t& getIndParams() const;

    value_t* data(size_t result_num = 0);
    value_t const* data(size_t result_num = 0) const;

    // ===================
    //  子类接口
    // ===================
    virtual bool check() {
        return true;
    }

    virtual void _calculate(const Indicator&) {}

    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {}

    /** 是否支持指标动态参数 */
    virtual bool supportIndParam() const {
        return false;
    }

    /** 是否必须串行计算 */
    virtual bool isSerial() const {
        return false;
    }

    virtual IndicatorImpPtr _clone() {
        return make_shared<IndicatorImp>();
    }

    virtual bool isNeedContext() const {
        return false;
    }

    virtual void _dyn_calculate(const Indicator&);

private:
    void initContext();
    bool needCalculate();
    void execute_add();
    void execute_sub();
    void execute_mul();
    void execute_div();
    void execute_mod();
    void execute_eq();
    void execute_ne();
    void execute_gt();
    void execute_lt();
    void execute_ge();
    void execute_le();
    void execute_and();
    void execute_or();
    void execute_weave();
    void execute_if();
    void execute_corr();

    std::vector<IndicatorImpPtr> getAllSubNodes();
    void repeatALikeNodes();

protected:
    static size_t _get_step_start(size_t pos, size_t step, size_t discard);

    // 用于动态参数时，更新 discard
    void _update_discard();

protected:
    string m_name;
    size_t m_discard;
    size_t m_result_num;
    vector<value_t>* m_pBuffer[MAX_RESULT_NUM];

    bool m_need_calculate;
    OPType m_optype;
    IndicatorImpPtr m_left;
    IndicatorImpPtr m_right;
    IndicatorImpPtr m_three;
    ind_param_map_t m_ind_params;  // don't use unordered_map

    IndicatorImp* m_parent{nullptr};  // can't use shared_from_this in python, so not weak_ptr

public:
    static void initDynEngine();
    static void releaseDynEngine();

protected:
    static StealThreadPool* ms_tg;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_discard);
        ar& BOOST_SERIALIZATION_NVP(m_result_num);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar& BOOST_SERIALIZATION_NVP(m_optype);
        ar& BOOST_SERIALIZATION_NVP(m_left);
        ar& BOOST_SERIALIZATION_NVP(m_right);
        ar& BOOST_SERIALIZATION_NVP(m_three);
        ar& BOOST_SERIALIZATION_NVP(m_ind_params);

        size_t act_result_num = 0;
        for (size_t i = 0; i < m_result_num; i++) {
            if (m_pBuffer[i]) {
                act_result_num++;
            }
        }
        ar& BOOST_SERIALIZATION_NVP(act_result_num);
        string nan("nan");
        string inf;
        for (size_t i = 0; i < act_result_num; ++i) {
            size_t count = size();
            ar& bs::make_nvp<size_t>(format("count_{}", i).c_str(), count);
            vector<value_t>& values = *m_pBuffer[i];
            for (size_t j = 0; j < count; j++) {
                if (std::isnan(values[j])) {
                    ar& boost::serialization::make_nvp<string>("item", nan);
                } else if (std::isinf(values[j])) {
                    inf = values[j] > 0 ? "+inf" : "-inf";
                    ar& boost::serialization::make_nvp<string>("item", inf);
                } else {
                    ar& boost::serialization::make_nvp<value_t>("item", values[j]);
                }
            }
        }
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_discard);
        ar& BOOST_SERIALIZATION_NVP(m_result_num);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar& BOOST_SERIALIZATION_NVP(m_optype);
        ar& BOOST_SERIALIZATION_NVP(m_left);
        ar& BOOST_SERIALIZATION_NVP(m_right);
        ar& BOOST_SERIALIZATION_NVP(m_three);
        ar& BOOST_SERIALIZATION_NVP(m_ind_params);

        size_t act_result_num = 0;
        ar& BOOST_SERIALIZATION_NVP(act_result_num);
        for (size_t i = 0; i < act_result_num; ++i) {
            m_pBuffer[i] = new vector<value_t>();
            size_t count = 0;
            ar& bs::make_nvp<size_t>(format("count_{}", i).c_str(), count);
            vector<value_t>& values = *m_pBuffer[i];
            values.resize(count);
            for (size_t i = 0; i < count; i++) {
                std::string vstr;
                ar >> boost::serialization::make_nvp<string>("item", vstr);
                if (vstr == "nan") {
                    values[i] = std::numeric_limits<double>::quiet_NaN();
                } else if (vstr == "+inf") {
                    values[i] = std::numeric_limits<double>::infinity();
                } else if (vstr == "-inf") {
                    values[i] = 0.0 - std::numeric_limits<double>::infinity();
                } else {
                    values[i] = std::atof(vstr.c_str());
                }
            }
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(IndicatorImp)
#endif

#if HKU_SUPPORT_SERIALIZATION
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION          \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp); \
    }
#else
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define INDICATOR_IMP(classname)                             \
public:                                                      \
    virtual bool check() override;                           \
    virtual void _calculate(const Indicator& data) override; \
    virtual IndicatorImpPtr _clone() override {              \
        return make_shared<classname>();                     \
    }

#define INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(classname)                                          \
public:                                                                                        \
    virtual bool check() override;                                                             \
    virtual void _calculate(const Indicator& ind) override;                                    \
    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) override; \
    virtual bool supportIndParam() const override {                                            \
        return true;                                                                           \
    }                                                                                          \
    virtual IndicatorImpPtr _clone() override {                                                \
        return make_shared<classname>();                                                       \
    }

#define INDICATOR_NEED_CONTEXT                    \
public:                                           \
    virtual bool isNeedContext() const override { \
        return true;                              \
    }

/** 获取 OPType 名称字符串 */
string HKU_API getOPTypeName(IndicatorImp::OPType);

typedef shared_ptr<IndicatorImp> IndicatorImpPtr;

HKU_API std::ostream& operator<<(std::ostream&, const IndicatorImp&);
HKU_API std::ostream& operator<<(std::ostream&, const IndicatorImpPtr&);

inline size_t IndicatorImp::getResultNumber() const {
    return m_result_num;
}

inline size_t IndicatorImp::discard() const {
    return m_discard;
}

inline size_t IndicatorImp::size() const {
    return m_pBuffer[0] ? m_pBuffer[0]->size() : 0;
}

inline const string& IndicatorImp::name() const {
    return m_name;
}

inline void IndicatorImp::name(const string& name) {
    m_name = name;
}

inline bool IndicatorImp::isLeaf() const {
    return m_optype == LEAF ? true : false;
}

inline KData IndicatorImp::getContext() const {
    return getParam<KData>("kdata");
}

inline const IndicatorImp::ind_param_map_t& IndicatorImp::getIndParams() const {
    return m_ind_params;
}

inline bool IndicatorImp::haveIndParam(const string& name) const {
    return m_ind_params.find(name) != m_ind_params.end();
}

inline IndicatorImp::value_t* IndicatorImp::data(size_t result_num) {
    return m_pBuffer[result_num] ? m_pBuffer[result_num]->data() : nullptr;
}

inline IndicatorImp::value_t const* IndicatorImp::data(size_t result_num) const {
    return m_pBuffer[result_num] ? m_pBuffer[result_num]->data() : nullptr;
}

inline size_t IndicatorImp::_get_step_start(size_t pos, size_t step, size_t discard) {
    return step == 0 || pos < discard + step ? discard : pos + 1 - step;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::IndicatorImp> : ostream_formatter {};
#endif

#endif /* INDICATORIMP_H_ */
