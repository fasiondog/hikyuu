/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KData.h"
#include "ScoreRecord.h"

#define MF_USE_MULTI_THREAD 1

namespace hku {

/**
 * 合成多因子，当只有一个因子时相当于简易的评分板
 * @ingroup MultiFactor
 */
class HKU_API MultiFactorBase : public enable_shared_from_this<MultiFactorBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    typedef Indicator::value_t value_t;
    friend HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorBase&);

public:
    MultiFactorBase();
    explicit MultiFactorBase(const string& name);
    MultiFactorBase(const IndicatorList& inds, const StockList& stks, const KQuery& query,
                    const Stock& ref_stk, const string& name, int ic_n);
    MultiFactorBase(const MultiFactorBase&);
    virtual ~MultiFactorBase() = default;

    /** 获取名称 */
    const string& name() const {
        return m_name;
    }

    /** 设置名称 */
    void name(const string& name) {
        m_name = name;
    }

    /** 获取参考日期列表 */
    const DatetimeList& getDatetimeList() const {
        return m_ref_dates;
    }

    /** 获取查询范围 */
    const KQuery& getQuery() const {
        return m_query;
    }

    /** 设置查询范围 */
    void setQuery(const KQuery& query);

    /** 获取参考证券 */
    const Stock& getRefStock() const {
        return m_ref_stk;
    }

    /** 设置参考证券 */
    void setRefStock(const Stock& stk);

    /** 获取证券列表 */
    const StockList& getStockList() const {
        return m_stks;
    }

    /** 设置计算范围证券列表 */
    void setStockList(const StockList& stks);

    /** 获取证券列表当前证券数量 */
    size_t getStockListNumber() const {
        return m_stks.size();
    }

    /** 获取原始因子公式列表 */
    const IndicatorList& getRefIndicators() const {
        return m_inds;
    }

    /** 设置因子列表 */
    void setRefIndicators(const IndicatorList& inds);

    /** 获取指定证券合成因子 */
    const Indicator& getFactor(const Stock&);

    /**
     * 获取所有证券合成后的新因子，顺序与传入的证券组合相同
     */
    const IndicatorList& getAllFactors();

    /** 获取指定日期截面的所有因子值，已经降序排列 */
    ScoreRecordList getScores(const Datetime&);

    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end = Null<size_t>());

    /**
     * 获取指定日期截面 [start, end] 范围内的因子值（评分）, 并通过filer进行过滤
     * @param date 指定日期
     * @param start 排序起始点
     * @param end 排序起始点(不含该点)
     * @param filter 过滤函数
     */
    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                              std::function<bool(const ScoreRecord&)>&& filter);

    ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                              std::function<bool(const Datetime&, const ScoreRecord&)>&& filter);

    /** 获取所有截面数据，已按降序排列 */
    const vector<ScoreRecordList>& getAllScores();

    /**
     * 获取合成因子的IC, 长度与参考日期同
     * @note ndays 对于使用 IC/ICIR 加权的新因子，最好保持好 ic_n 一致，
     *       但对于等权计算的新因子，不一定非要使用 ic_n 计算。
     *       所以，ndays 增加了一个特殊值 0, 表示直接使用 ic_n 参数计算 IC
     * @param ndays 计算相对 ndays 日收益率的IC值
     */
    Indicator getIC(int ndays = 0);

    /**
     * 获取合成因子的 ICIR
     * @param ir_n 计算 IR 的 n 窗口
     * @param ic_n 计算 IC 的 n 窗口
     */
    Indicator getICIR(int ir_n, int ic_n = 0);

    /**
     * 获取所有处理过的原始因子值（归一化、标准化）
     * @note 考虑到内存占用，该数据没有缓存，一般用与测试或者想查看处理过的原始因子值
     * @return vector<IndicatorList>  stks x inds
     */
    vector<IndicatorList> getAllSrcFactors();

    void reset();

    typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
    MultiFactorPtr clone();

    virtual void _reset() {}
    virtual MultiFactorPtr _clone() = 0;
    virtual IndicatorList _calculate(const vector<IndicatorList>&) = 0;

private:
    /** 执行计算 */
    void calculate();

    void initParam();

protected:
    void _buildIndex();  // 计算完成后创建截面索引
    IndicatorList _getAllReturns(int ndays) const;
    void _checkData();

protected:
    string m_name;
    IndicatorList m_inds;  // 输入的原始因子列表
    StockList m_stks;      // 证券组合
    Stock m_ref_stk;       // 指定的参考证券
    KQuery m_query;        // 计算的日期范围条件

    // 以下变量为计算后生成
    DatetimeList m_ref_dates;  // 依据参考证券和query计算的参考日期，合成因子和该日期对齐
    unordered_map<Stock, size_t> m_stk_map;  // 证券->合成后因子位置索引
    IndicatorList m_all_factors;             // 保存所有证券合成后的新因子
    unordered_map<Datetime, size_t> m_date_index;
    vector<ScoreRecordList> m_stk_factor_by_date;
    Indicator m_ic;

private:
    std::mutex m_mutex;
    bool m_calculated{false};

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
        ar& BOOST_SERIALIZATION_NVP(m_inds);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_ref_dates);
        // 以下不需要保存，加载后重新计算
        // ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        // ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        // ar& BOOST_SERIALIZATION_NVP(m_date_index);
        // ar& BOOST_SERIALIZATION_NVP(m_ic);
        // ar& BOOST_SERIALIZATION_NVP(m_calculated);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_inds);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_ref_dates);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        // ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        // ar& BOOST_SERIALIZATION_NVP(m_date_index);
        // ar& BOOST_SERIALIZATION_NVP(m_ic);
        // ar& BOOST_SERIALIZATION_NVP(m_calculated);
        // ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MultiFactorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public MultiFactorBase {
 *     MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiFactorBase); \
    }
#else
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<MultiFactorBase> FactorPtr;
typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
typedef std::shared_ptr<MultiFactorBase> MFPtr;

#define MULTIFACTOR_IMP(classname)             \
public:                                        \
    virtual MultiFactorPtr _clone() override { \
        return std::make_shared<classname>();  \
    }                                          \
    virtual IndicatorList _calculate(const vector<IndicatorList>&) override;

HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorBase&);
HKU_API std::ostream& operator<<(std::ostream&, const MultiFactorPtr&);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::MultiFactorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::MultiFactorPtr> : ostream_formatter {};
#endif