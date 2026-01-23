/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_manage/TradeManagerBase.h"
#include "ScoreRecord.h"

namespace hku {

/**
 * 评分过滤器
 * @note 虽然直接使用 std::function 即可，但如果使用 python 函数会造成并行死锁
 * @ingroup Selector
 */
class HKU_API ScoresFilterBase {
    PARAMETER_SUPPORT_WITH_CHECK
    friend HKU_API std::ostream& operator<<(std::ostream&, const ScoresFilterBase&);

public:
    ScoresFilterBase() = default;
    ScoresFilterBase(const ScoresFilterBase&) = default;
    ScoresFilterBase(const string& name) : m_name(name) {}
    virtual ~ScoresFilterBase() = default;

    const string& name() const {
        return m_name;
    }

    void name(const string& name) {
        m_name = name;
    }

    ScoreRecordList filter(const ScoreRecordList& scores, const Datetime& date,
                           const KQuery& query);

    virtual ScoreRecordList _filter(const ScoreRecordList& scores, const Datetime& date,
                                    const KQuery& query) = 0;

    typedef std::shared_ptr<ScoresFilterBase> ScoresFilterPtr;
    ScoresFilterPtr clone();

    virtual ScoresFilterPtr _clone() = 0;

public:
    friend HKU_API ScoresFilterPtr operator|(const ScoresFilterPtr& a, const ScoresFilterPtr& b);

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    ScoresFilterPtr m_child;
    bool m_is_python_object{false};

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_child);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_child);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(ScoresFilterBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
#define SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                           \
    friend class boost::serialization::access;                     \
    template <class Archive>                                       \
    void serialize(Archive& ar, const unsigned int version) {      \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ScoresFilterBase); \
    }
#else
#define SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<ScoresFilterBase> ScoresFilterPtr;
typedef std::shared_ptr<ScoresFilterBase> SCFilterPtr;

#define SCORESFILTER_IMP(classname)                                                      \
public:                                                                                  \
    virtual ScoresFilterPtr _clone() override {                                          \
        return std::make_shared<classname>();                                            \
    }                                                                                    \
    virtual ScoreRecordList _filter(const ScoreRecordList& scores, const Datetime& date, \
                                    const KQuery& query) override;

HKU_API std::ostream& operator<<(std::ostream&, const ScoresFilterBase&);
HKU_API std::ostream& operator<<(std::ostream&, const ScoresFilterPtr&);

HKU_API ScoresFilterPtr operator|(const ScoresFilterPtr& a, const ScoresFilterPtr& b);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ScoresFilterBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::ScoresFilterPtr> : ostream_formatter {};
#endif