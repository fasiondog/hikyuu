/*
 * IndicatorImp.h
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATORIMP_H_
#define INDICATORIMP_H_

#include <mimalloc.h>
#include "../config.h"
#include "../KData.h"
#include "../utilities/Parameter.h"
#include "../utilities/thread/thread.h"

namespace hku {

#define MAX_RESULT_NUM 6

class HKU_API Indicator;
class HKU_API IndParam;
class HKU_API IndicatorAssemble;

vector<Indicator> HKU_API combineCalculateIndicators(const vector<Indicator>& indicators,
                                                     const KData& kdata, bool tovalue);

/**
 * 指标实现类，定义新指标时，应从此类继承
 * @ingroup Indicator
 */
class HKU_API IndicatorImp : public enable_shared_from_this<IndicatorImp> {
    PARAMETER_SUPPORT_WITH_CHECK
    friend HKU_API std::ostream& operator<<(std::ostream& os, const IndicatorImp& imp);

    typedef vector<Indicator> IndicatorList;
    friend IndicatorList HKU_API combineCalculateIndicators(const IndicatorList& indicators,
                                                            const KData& kdata, bool tovalue);
    friend class HKU_API IndicatorAssemble;

public:
    enum OPType : uint8_t {
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
        INVALID
    };

#if HKU_USE_LOW_PRECISION
    typedef float value_t;
#else
    typedef double value_t;
#endif
    typedef vector<value_t, mi_stl_allocator<value_t>> buffer_t;

public:
    /** 默认构造函数   */
    IndicatorImp();
    explicit IndicatorImp(const string& name);
    IndicatorImp(const string& name, size_t result_num);

    virtual ~IndicatorImp();

    typedef shared_ptr<IndicatorImp> IndicatorImpPtr;
    IndicatorImpPtr operator()(const Indicator& ind);

    size_t getResultNumber() const noexcept;
    OPType getOPType() const noexcept;

    size_t discard() const noexcept;

    void setDiscard(size_t discard) noexcept;
    void updateDiscard(bool force = false) noexcept;

    size_t size() const noexcept;

    value_t get(size_t pos, size_t num = 0) const;

    value_t front(size_t num = 0) const;
    value_t back(size_t num = 0) const;

    value_t getByDate(Datetime, size_t num = 0);

    Datetime getDatetime(size_t pos) const;

    DatetimeList getDatetimeList() const;

    size_t getPos(Datetime) const;

    /** 以PriceList方式获取指定的输出集 */
    PriceList getResultAsPriceList(size_t result_num);

    /** 以Indicator的方式获取指定的输出集，该方式包含了discard的信息 */
    IndicatorImpPtr getResult(size_t result_num);

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

    /** 数据中是否包含 nan 值 */
    bool existNan(size_t result_idx = 0) const;

    const string& name() const noexcept;
    void name(const string& name) noexcept;

    /** 返回形如：Name(param1=val,param2=val,...) */
    string long_name() const;

    virtual string formula() const;
    virtual string str() const;

    bool isLeaf() const noexcept;

    Indicator calculate();

    void setContext(const Stock&, const KQuery&);

    void setContext(const KData&);

    const KData& getContext() const;

    void add(OPType, IndicatorImpPtr left, IndicatorImpPtr right);

    void add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right);

    IndicatorImpPtr clone();

    bool isPythonObject() const noexcept;

    /** 仅用于两个结果集数量相同、长度相同的指标交换数据，不交换其他参数。失败抛出异常 */
    void swap(IndicatorImp* other);
    void swap(IndicatorImp* other, size_t other_result_idx, size_t self_result_idx);

    bool haveIndParam(const string& name) const;
    void setIndParam(const string& name, const Indicator& ind);
    void setIndParam(const string& name, const IndParam& ind);
    IndParam getIndParam(const string& name) const;
    const IndicatorImpPtr& getIndParamImp(const string& name) const;

    typedef std::map<string, IndicatorImpPtr> ind_param_map_t;
    const ind_param_map_t& getIndParams() const;

    // ===================
    //  子类接口
    // ===================
    virtual void _calculate(const Indicator&);

    // ====== start 动态周期计算相关接口 ======
    /** 动态周期计算，子类可重载该函数，默认不支持动态周期计算 */
    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {}

    /** 是否必须串行计算 */
    bool isSerial() const noexcept {
        return m_is_serial;
    }
    // ====== end 动态周期计算相关接口 ======

    // ====== start 增量计算相关接口 =======
    /** 是否支持增量计算 */
    virtual bool supportIncrementCalculate() const;

    virtual size_t min_increment_start() const {
        return 0;
    }

    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) {}
    // ====== end 增量计算相关接口 =======

    virtual IndicatorImpPtr _clone() {
        return make_shared<IndicatorImp>();
    }

    bool isNeedContext() const noexcept {
        return m_need_context;
    }

    virtual void _dyn_calculate(const Indicator&);

public:
    static void enableIncrementCalculate(bool flag) {
        ms_enable_increment_calculate = flag;
    }

    static bool enableIncrementCalculate() {
        return ms_enable_increment_calculate;
    }

public:
    // ===================
    //  内部特殊用途公共接口
    // ===================

    /** 判断是否和另一个指标等效，即计算效果相同 */
    bool alike(const IndicatorImp& other) const;

    /** 判断指标公式中是否包含指定名称的指标（特殊用途） */
    bool contains(const string& name) const;

    value_t* data(size_t result_idx = 0) noexcept;
    value_t const* data(size_t result_idx = 0) const noexcept;

    void getAllSubNodes(vector<IndicatorImpPtr>& nodes) const;

    IndicatorImpPtr getRightNode() const noexcept;
    IndicatorImpPtr getLeftNode() const noexcept;
    IndicatorImpPtr getThreeNode() const noexcept;
    void printTree(bool show_long_name = false) const;
    void printAllSubTrees(bool show_long_name = false) const;
    void printLeaves(bool show_long_name = false) const;

    /* 特殊指标需自己实现 selfAlike 函数的, needSelfAlikeCompare 应返回 true */
    bool needSelfAlikeCompare() const noexcept {
        return m_need_self_alike_compare;
    }

    // 特殊指标需自己实现 selfAlike 函数，返回true表示两个指标等效
    virtual bool selfAlike(const IndicatorImp& other) const noexcept {
        return false;
    }

    // 使用输入上下文的特殊指标获取内部节点，以便合入指标树优化
    virtual void getSelfInnerNodesWithInputConext(vector<IndicatorImpPtr>& nodes) const {}

    // 指定独立 ktype 的叶子节点重载获取内部子节点
    virtual void getSeparateKTypeLeafSubNodes(vector<IndicatorImpPtr>& nodes) const {}

private:
    bool needCalculate();
    bool can_inner_calculate();
    bool can_increment_calculate();
    bool increment_execute_leaf_or_op(const Indicator& ind);
    size_t increment_execute();
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
    size_t increment_execute_if();

    static void inner_repeatALikeNodes(vector<IndicatorImpPtr>& sub_nodes);
    void repeatALikeNodes();
    void repeatSeparateKTypeLeafALikeNodes();

    void generateSpecialLeafSet();

    void _clearBuffer();

    void _printTree(int depth = 0, bool isLast = true, bool show_long_name = false) const;

    // 获取所有子树
    vector<IndicatorImp*> getAllSubTrees() const;

    // 获取子树中节点的数量
    static size_t treeSize(IndicatorImp* tree);

    static bool nodeInTree(IndicatorImp* node, IndicatorImp* tree);

protected:
    static size_t _get_step_start(size_t pos, size_t step, size_t discard);

    void onlySetContext(const KData&);

protected:
    string m_name;
    size_t m_discard{0};
    size_t m_result_num{0};
    KData m_context;
    KData m_old_context;

    buffer_t* m_pBuffer[MAX_RESULT_NUM];

    bool m_need_context{false};
    bool m_is_python_object{false};
    bool m_need_self_alike_compare{false};
    bool m_is_serial{false};
    bool m_need_calculate{true};
    bool m_param_changed{true};
    OPType m_optype{LEAF};
    IndicatorImpPtr m_left;
    IndicatorImpPtr m_right;
    IndicatorImpPtr m_three;
    ind_param_map_t m_ind_params;  // don't use unordered_map

    IndicatorImp* m_parent{nullptr};  // can't use shared_from_this in python, so not weak_ptr

public:
    static void initEngine();
    static void releaseEngine();

protected:
    static bool ms_enable_increment_calculate;

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
        ar& BOOST_SERIALIZATION_NVP(m_context);
        ar& BOOST_SERIALIZATION_NVP(m_old_context);
        ar& BOOST_SERIALIZATION_NVP(m_need_context);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_need_self_alike_compare);
        ar& BOOST_SERIALIZATION_NVP(m_is_serial);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar& BOOST_SERIALIZATION_NVP(m_param_changed);
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
            buffer_t& values = *m_pBuffer[i];
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
        ar& BOOST_SERIALIZATION_NVP(m_context);
        ar& BOOST_SERIALIZATION_NVP(m_old_context);
        ar& BOOST_SERIALIZATION_NVP(m_need_context);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_need_self_alike_compare);
        ar& BOOST_SERIALIZATION_NVP(m_is_serial);
        ar& BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar& BOOST_SERIALIZATION_NVP(m_param_changed);
        ar& BOOST_SERIALIZATION_NVP(m_optype);
        ar& BOOST_SERIALIZATION_NVP(m_left);
        ar& BOOST_SERIALIZATION_NVP(m_right);
        ar& BOOST_SERIALIZATION_NVP(m_three);
        ar& BOOST_SERIALIZATION_NVP(m_ind_params);

        size_t act_result_num = 0;
        ar& BOOST_SERIALIZATION_NVP(act_result_num);
        _readyBuffer(0, act_result_num);
        for (size_t i = 0; i < act_result_num; ++i) {
            size_t count = 0;
            ar& bs::make_nvp<size_t>(format("count_{}", i).c_str(), count);
            buffer_t& values = *m_pBuffer[i];
            values.resize(count);
            for (size_t j = 0; j < count; j++) {
                std::string vstr;
                ar >> boost::serialization::make_nvp<string>("item", vstr);
                if (vstr == "nan") {
                    values[j] = std::numeric_limits<double>::quiet_NaN();
                } else if (vstr == "+inf") {
                    values[j] = std::numeric_limits<double>::infinity();
                } else if (vstr == "-inf") {
                    values[j] = 0.0 - std::numeric_limits<double>::infinity();
                } else {
                    values[j] = std::atof(vstr.c_str());
                }
            }
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

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
    virtual void _calculate(const Indicator& data) override; \
    virtual IndicatorImpPtr _clone() override {              \
        return make_shared<classname>();                     \
    }

#define INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE \
public:                                     \
    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) override;

#define INDICATOR_IMP_SUPPORT_INCREMENT                                                 \
public:                                                                                 \
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override; \
    virtual bool supportIncrementCalculate() const override {                           \
        return true;                                                                    \
    }

/** 获取 OPType 名称字符串 */
string HKU_API getOPTypeName(IndicatorImp::OPType);

typedef shared_ptr<IndicatorImp> IndicatorImpPtr;

HKU_API std::ostream& operator<<(std::ostream&, const IndicatorImp&);
HKU_API std::ostream& operator<<(std::ostream&, const IndicatorImpPtr&);

inline IndicatorImp::OPType IndicatorImp::getOPType() const noexcept {
    return m_optype;
}

inline size_t IndicatorImp::getResultNumber() const noexcept {
    return m_result_num;
}

inline size_t IndicatorImp::discard() const noexcept {
    return m_discard;
}

inline size_t IndicatorImp::size() const noexcept {
    return m_pBuffer[0] ? m_pBuffer[0]->size() : 0;
}

inline const string& IndicatorImp::name() const noexcept {
    return m_name;
}

inline void IndicatorImp::name(const string& name) noexcept {
    m_name = name;
}

inline bool IndicatorImp::isLeaf() const noexcept {
    return m_optype == LEAF ? true : false;
}

inline const KData& IndicatorImp::getContext() const {
    return m_context;
}

inline void IndicatorImp::setContext(const Stock& stock, const KQuery& query) {
    setContext(stock.getKData(query));
}

inline void IndicatorImp::onlySetContext(const KData& k) {
    if (m_context != k) {
        m_old_context = m_context;
        m_context = k;
    }
}

inline const IndicatorImp::ind_param_map_t& IndicatorImp::getIndParams() const {
    return m_ind_params;
}

inline bool IndicatorImp::haveIndParam(const string& name) const {
    return m_ind_params.find(name) != m_ind_params.end();
}

inline IndicatorImp::value_t* IndicatorImp::data(size_t result_idx) noexcept {
    return m_pBuffer[result_idx] ? m_pBuffer[result_idx]->data() : nullptr;
}

inline IndicatorImp::value_t const* IndicatorImp::data(size_t result_idx) const noexcept {
    return m_pBuffer[result_idx] ? m_pBuffer[result_idx]->data() : nullptr;
}

inline size_t IndicatorImp::_get_step_start(size_t pos, size_t step, size_t discard) {
    return step == 0 || pos < discard + step ? discard : pos + 1 - step;
}

inline bool IndicatorImp::isPythonObject() const noexcept {
    return m_is_python_object;
}

inline IndicatorImpPtr IndicatorImp::getRightNode() const noexcept {
    return m_right;
}

inline IndicatorImpPtr IndicatorImp::getLeftNode() const noexcept {
    return m_left;
}

inline IndicatorImpPtr IndicatorImp::getThreeNode() const noexcept {
    return m_three;
}

inline std::ostream& operator<<(std::ostream& os, const IndicatorImp::buffer_t& p) {
    if (p.empty()) {
        os << "[]";
        return os;
    }

    size_t len = p.size();
    const size_t print = 3;
    os << "[";
    for (size_t i = 0; i < len; i++) {
        if ((i < print) || (i + print >= len)) {
            os << p[i];
            if (i + 1 != len)
                os << ", ";
        } else if (i == 3)
            os << "..., ";
    }
    os << "]";
    return os;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::IndicatorImp> : ostream_formatter {};
#endif

#endif /* INDICATORIMP_H_ */
