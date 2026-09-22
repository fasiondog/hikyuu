/*
 * IndicatorImp.h
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATORIMP_H_
#define INDICATORIMP_H_

#include "hikyuu/config.h"
#include "hikyuu/KData.h"
#include "hikyuu/utilities/Parameter.h"
#include "hikyuu/utilities/thread/algorithm.h"
#include "IndicatorImpBuffer.h"

namespace hku {

#define MAX_RESULT_NUM 6

class HKU_API Indicator;
class HKU_API IndParam;

namespace detail {
class CompiledFactorPlan;
}

vector<Indicator> HKU_API combineCalculateIndicators(const vector<Indicator>& indicators,
                                                     const KData& kdata, bool tovalue);

/**
 * Indicator implementation class; when defining a new indicator, this class should be inherited
 * @ingroup Indicator
 */
class HKU_API IndicatorImp : public enable_shared_from_this<IndicatorImp> {
    PARAMETER_SUPPORT_WITH_CHECK
    friend HKU_API std::ostream& operator<<(std::ostream& os, const IndicatorImp& imp);
    friend class detail::CompiledFactorPlan;

    typedef vector<Indicator> IndicatorList;
    friend IndicatorList HKU_API combineCalculateIndicators(const IndicatorList& indicators,
                                                            const KData& kdata, bool tovalue);

public:
    enum OPType : uint8_t {
        LEAF,   ///< Leaf node
        OP,     /// OP(OP1,OP2) OP1->calcalue(OP2->calculate(ind))
        ADD,    ///< Addition
        SUB,    ///< Subtraction
        MUL,    ///< Multiplication
        DIV,    ///< Division
        MOD,    ///< Modulo
        EQ,     ///< Equal
        GT,     ///< Greater than
        LT,     ///< Less than
        NE,     ///< Not equal
        GE,     ///< Greater than or equal
        LE,     ///< Less than or equal
        AND,    ///< And
        OR,     ///< Or
        WEAVE,  ///< Special, an indicator that requires two indicators as the parameters
        OP_IF,  /// if operation
        INVALID
    };

    typedef IndicatorImpBuffer::value_type value_t;
    typedef IndicatorImpBuffer buffer_t;

public:
    /** Default constructor   */
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

    /** Get the given output set in the form of a PriceList */
    PriceList getResultAsPriceList(size_t result_num);

    /** Get the given output set in the form of an Indicator, it contains the discard information */
    IndicatorImpPtr getResult(size_t result_num);

    /**
     * After using the IndicatorImp(const Indicator&...) constructor, this function is used for the
     * calculation result; no out-of-range protection is done
     */
    void _set(value_t val, size_t pos, size_t num = 0);

    /**
     * Prepare the memory
     * @param len the length; std::invalid_argument is thrown if the length is greater than
     *            MAX_RESULT_NUM
     * @param result_num number of the result sets
     * @return true success | false failure
     */
    void _readyBuffer(size_t len, size_t result_num);

    /** Whether the data contains nan values */
    bool existNan(size_t result_idx = 0) const;

    const string& name() const noexcept;
    void name(const string& name) noexcept;

    /** It is returned in the form: Name(param1=val,param2=val,...) */
    string long_name() const;

    /**
     * @brief Construction origin identifier (unique within the process, inherited by
     * clone/cloneNode, not serialized)
     *
     * It is used as the identity by the caching facilities (such as the cross-section panel cache).
     * One construction and its clone chain share the same id; independent constructions necessarily
     * have different ids. It does not participate in any existing equality judgment or display such
     * as alike()/operator==/formula().
     *
     * Note: it is defined in IndicatorImp.cpp (not inline). The inline member of a dllexport class
     * is not guaranteed to be exported, whether an external symbol reference is generated depends
     * on the inline decision of the caller, and a plugin dll will fail to link.
     *
     * Note (identity semantic limitation): origin_id only identifies the "origin", it is not aware
     * of the in-place mutation after the construction (setParam/setIndParam/add). Any indicator
     * object used as an identity by a caching facility must be regarded as immutable after the
     * construction; construct a new object if different parameters are needed (a new object always
     * holds a new id).
     */
    uint64_t originId() const noexcept;

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

    /**
     * Whether this implementation can be recalculated repeatedly on a reusable batch executor.
     *
     * It is allowed by default (a non-Python implementation without an explicit
     * `_support_batch_reuse=false` is regarded as reusable). Participating in the reuse means that
     * the calculation graph of the same node is repeatedly rebounded to the context across the
     * stocks and recalculated by `CompiledFactorPlan`, so a custom C++ indicator must satisfy:
     * apart from the result buffer of IndicatorImp itself and `m_params/m_ind_params`, it holds no
     * member state that would remain across the stocks (such as cached statistics, mutable buffers,
     * intermediate results that depend on the last input, and so on). Any implementation that keeps
     * the above state outside `_calculate`/`_dyn_calculate`, or cannot be reset cleanly through
     * `scrubTemplateNode`, should call `supportBatchReuse(false)` at construction to exit the fast
     * path actively and fall back to the old behavior.
     */
    bool supportBatchReuse() const;

    void supportBatchReuse(bool enable);

    /** Only used to swap the data of two indicators with the same number of result sets and the
     *  same length, no other parameters are swapped. An exception is thrown on failure */
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
    //  Subclass interface
    // ===================
    virtual void _calculate(const Indicator&);

    // ====== start dynamic parameter calculation related interface ======
    /**
     * Dynamic parameter calculation interface
     * @note If there is only one dynamic parameter and it represents the calculation window period,
     *       _dyn_run_one_step can be overloaded directly together with isSerial
     */
    virtual void _dyn_calculate(const Indicator&);

    /**
     * If there is only one dynamic parameter and it represents the calculation window period, this
     * macro can be used to define the dynamic cycle calculation interface; otherwise you need to
     * overload the _dyn_calculate function yourself to implement the dynamic parameter calculation
     */
    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {}

    /** Whether the calculation must be serial */
    bool isSerial() const noexcept {
        return m_is_serial;
    }
    // ====== end dynamic parameter calculation related interface ======

    // ====== start incremental calculation related interface =======
    /** Whether the incremental calculation is supported */
    virtual bool supportIncrementCalculate() const;

    virtual size_t min_increment_start() const {
        return 0;
    }

    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) {}
    // ====== end incremental calculation related interface ======

    virtual IndicatorImpPtr _clone() {
        return make_shared<IndicatorImp>();
    }

    bool isNeedContext() const noexcept {
        return m_need_context;
    }

public:
    static void enableIncrementCalculate(bool flag) {
        ms_enable_increment_calculate = flag;
    }

    static bool enableIncrementCalculate() {
        return ms_enable_increment_calculate;
    }

public:
    // ===================
    //  Internal public interface for special purposes
    // ===================

    void onlySetContext(const KData&);

    void setCalculateFlag(bool flag) noexcept;

    /** Judge whether it is equivalent to another indicator, i.e. the calculation effect is the
     *  same */
    bool alike(const IndicatorImp& other) const;

    /** Judge whether the indicator formula contains the indicator with the given name (for special
     *  use) */
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

    /* For a special indicator that needs to implement the selfAlike function itself,
     *  needSelfAlikeCompare should return true */
    bool needSelfAlikeCompare() const noexcept {
        return m_need_self_alike_compare;
    }

    // A special indicator needs to implement the selfAlike function itself; returning true means
    // the two indicators are equivalent
    virtual bool selfAlike(const IndicatorImp& other) const noexcept {
        return false;
    }

    // Get the inner nodes of the special indicator that uses the input context, so that it can be
    // merged into the indicator tree optimization
    virtual void getSelfInnerNodesWithInputConext(vector<IndicatorImpPtr>& nodes) const {}

    // Overload of a leaf node with a separate ktype to get its inner child nodes
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

    // Get all the subtrees
    vector<IndicatorImp*> getAllSubTrees() const;

    // Get the number of the nodes in the subtree
    static size_t treeSize(IndicatorImp* tree);

    static bool nodeInTree(IndicatorImp* node, IndicatorImp* tree);

protected:
    static size_t _get_step_start(size_t pos, size_t step, size_t discard);

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

    /** Construction id issuer: defined only in IndicatorImp.cpp, guaranteeing that there is only
     *  one counter across the DLLs */
    static uint64_t nextOriginId() noexcept;

    /** Construction origin identifier: issued at construction and inherited by clone/cloneNode,
     *  it does not enter the serialization NVP list */
    uint64_t m_origin_id{nextOriginId()};

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
        string value;
        for (size_t i = 0; i < act_result_num; ++i) {
            size_t count = size();
            ar& BOOST_SERIALIZATION_NVP(count);
            const buffer_t& values = *m_pBuffer[i];
            for (size_t j = 0; j < count; j++) {
                if (std::isnan(values[j])) {
                    ar& boost::serialization::make_nvp<string>("item", nan);
                } else if (std::isinf(values[j])) {
                    inf = values[j] > 0 ? "+inf" : "-inf";
                    ar& boost::serialization::make_nvp<string>("item", inf);
                } else {
                    value = std::to_string(values[j]);
                    ar& boost::serialization::make_nvp<string>("item", value);
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
            ar& BOOST_SERIALIZATION_NVP(count);
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
                    values[j] = std::stod(vstr);
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

// If there is only one dynamic parameter and it represents the calculation window period, this
// macro can be used to define the dynamic cycle calculation interface; otherwise you need to
// overload the _dyn_calculate function yourself to implement the dynamic parameter calculation
#define INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE \
public:                                     \
    virtual void _dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) override;

#define INDICATOR_IMP_SUPPORT_INCREMENT                                                 \
public:                                                                                 \
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override; \
    virtual bool supportIncrementCalculate() const override {                           \
        return true;                                                                    \
    }

/** Get the name string of OPType */
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

inline void IndicatorImp::setCalculateFlag(bool flag) noexcept {
    m_need_calculate = flag;
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

inline bool IndicatorImp::supportBatchReuse() const {
    static const string param_name("_support_batch_reuse");
    return !m_is_python_object && (!haveParam(param_name) || getParam<bool>(param_name));
}

inline void IndicatorImp::supportBatchReuse(bool enable) {
    m_params.set<bool>("_support_batch_reuse", enable);
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
