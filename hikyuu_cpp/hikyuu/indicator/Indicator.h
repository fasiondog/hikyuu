/*
 * Indicator.h
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_H_
#define INDICATOR_H_

#include "IndicatorImp.h"
#include "IndParam.h"

namespace hku {

#define IND_EQ_THRESHOLD 0.000001  ///< Threshold for judging float equality

/**
 * Indicator class, concretely implemented by IndicatorImp; when implementing a new indicator,
 * IndicatorImp should be inherited
 * @details When implementing a new indicator, the IndicatorImp class should be inherited and the
 * related indicator creation function should be defined, e.g.:
 * <pre>
 * class Ma: public IndicatorImp {
 * public:
 *     MA(size_t);
 *     MA(const Indicator&, size_t);
 *
 *     virtual string name() { return "MA"; }
 *     virtual string toString() { retun "Indicator(MA)"; }
 * };
 *
 * Indicator HKU_API MA(size_t n = 30) {
 *     return Indicator(IndicatorImpPtr(new MA(n)));
 * }
 *
 * Indicator HKU_API MA(const Indicator& indicator, size_t n = 30);
 * </pre>
 * @ingroup Indicator
 */
class HKU_API Indicator {
    HKU_API friend std::ostream& operator<<(std::ostream&, const Indicator&);

public:
    typedef IndicatorImp::value_t value_t;

public:
    Indicator() : m_imp(make_shared<IndicatorImp>()) {}
    explicit Indicator(const IndicatorImpPtr& imp) noexcept;
    Indicator(const Indicator& ind) noexcept;
    Indicator(Indicator&& ind) noexcept;
    virtual ~Indicator();

    Indicator& operator=(const Indicator&) noexcept;
    Indicator& operator=(Indicator&&) noexcept;

    /** Calculate a new value with the existing parameters, returning a brand new Indicator */
    Indicator operator()(const Indicator& ind);

    /** Create a new clone and use the parameter k as the context of the new instance */
    Indicator operator()(const KData& k) const;

    /** Create a new clone, the same as clone */
    Indicator operator()();

    /** Indicator name */
    string name() const noexcept;
    void name(const string& name) noexcept;

    IndicatorImp::OPType getOPType() const noexcept;

    /** It is returned in the form: Name(param1_val,param2_val,...) */
    string long_name() const;

    /** Clone operation */
    Indicator clone() const;

    /** Set the context */
    void setContext(const Stock&, const KQuery&);
    void setContext(const KData&);

    /** Get the context */
    KData getContext() const;

    /** When there is a context, the context is automatically extended to the current latest data
     *  and calculated */
    void extend();

    /** Display the indicator formula */
    string formula() const;

    /** Number of the values that need to be discarded in the result */
    size_t discard() const noexcept;

    /** Set the number to discard; it has no effect if it is less than the original discard */
    void setDiscard(size_t discard) noexcept;

    /** Update the discard number according to its own values; force=true forces the update,
     *  otherwise it updates starting from its current discard() */
    void updateDiscard(bool force = false) noexcept;

    /** Return how many result sets are output */
    size_t getResultNumber() const noexcept;

    /** Judge whether it is empty **/
    bool empty() const noexcept;

    /** Get the size **/
    size_t size() const noexcept;

    /** Get the output at the corresponding position of the first result set only, the same as
     *  get(pos, 0) */
    value_t operator[](size_t pos) const;

    /** Get the output at the corresponding position of the first result set only, the same as
     *  getByDate(date, 0) */
    value_t operator[](Datetime) const;

    /**
     * Get the data at the given position of the num-th result set
     * @param pos the position in the result set
     * @param num the num-th result set
     */
    value_t get(size_t pos, size_t num = 0) const;

    value_t front(size_t num = 0) const;
    value_t back(size_t num = 0) const;

    /**
     * Get the date of the given position
     * @param pos the position in the result set
     */
    Datetime getDatetime(size_t pos) const;

    /**
     * Get the data of the given date
     * @param date the given date
     * @param num the num-th result set
     */
    value_t getByDate(Datetime date, size_t num = 0) const;

    /** Get the index position corresponding to the given date */
    size_t getPos(Datetime) const;

    /**
     * Get the given result set as an indicator, its formula is not kept.
     * For an indicator that no longer needs to be calculated, the occupied memory can be released
     * through this method.
     * @param num the given result set
     */
    Indicator getResult(size_t num) const;

    /**
     * Get the given result set as a PriceList
     * @param num the given result set
     */
    PriceList getResultAsPriceList(size_t num) const;

    /** Whether the given result set contains nan values */
    bool existNan(size_t result_idx) const;

    /**
     * Get the DatetimeList
     */
    DatetimeList getDatetimeList() const;

    /** Judge whether it is equivalent to another indicator, i.e. the calculation effect is the
     *  same */
    bool alike(const Indicator& other) const;

    bool haveParam(const string& name) const {
        return m_imp ? m_imp->haveParam(name) : false;
    }

    template <typename ValueType>
    void setParam(const string& name, const ValueType& value) {
        if (m_imp) {
            m_imp->setParam<ValueType>(name, value);
        }
    }

    template <typename ValueType>
    ValueType getParam(const string& name) const {
        if (!m_imp) {
            throw std::out_of_range("out_of_range in Parameter::get : " + name);
        }
        return m_imp->getParam<ValueType>(name);
    }

    bool haveIndParam(const string& name) const;
    void setIndParam(const string& name, const Indicator& ind);
    void setIndParam(const string& name, const IndParam& ind);
    IndParam getIndParam(const string& name) const;
    const IndicatorImpPtr getIndParamImp(const string& name) const;

    IndicatorImpPtr getImp() const noexcept {
        return m_imp;
    }

    value_t* data(size_t result_idx = 0) noexcept {
        return m_imp ? m_imp->data(result_idx) : nullptr;
    }

    value_t const* data(size_t result_idx = 0) const noexcept {
        return m_imp ? m_imp->data(result_idx) : nullptr;
    }

    /**
     * Judge whether the values of two ind are equal
     * @note The operator== overload creates a new Indicator; this function is used to compare the
     *       values of two ind
     */
    bool equal(const Indicator& other) const noexcept;

    /** Judge whether it is the same instance */
    bool isSame(const Indicator& other) const noexcept {
        return !m_imp && m_imp == other.m_imp;
    }

    /** Judge whether the indicator formula contains the indicator with the given name (for special
     *  use) */
    bool contains(const string& name) const {
        return m_imp ? m_imp->contains(name) : false;
    }

    string str() const;

    bool isPythonObject() const noexcept;

public:
    class Iterator {
    private:
        const Indicator& container;
        size_t index;

    public:
        Iterator(const Indicator& cont, size_t idx) : container(cont), index(idx) {}

        Iterator& operator++() {
            index++;
            return *this;
        }

        value_t operator*() const {
            return container.get(index);
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }

        bool operator==(const Iterator& other) const {
            return index == other.index;
        }
    };

    Iterator begin() const {
        return Iterator(*this, 0);
    }

    Iterator end() const {
        return Iterator(*this, size());
    }

    typedef Iterator iterator;

public:
    static void enableIncrementCalculate(bool flag) {
        IndicatorImp::enableIncrementCalculate(flag);
    }

    static bool enableIncrementCalculate() {
        return IndicatorImp::enableIncrementCalculate();
    }

protected:
    IndicatorImpPtr m_imp;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_imp);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

/** @ingroup Indicator */
typedef vector<Indicator> IndicatorList;

inline string Indicator::name() const noexcept {
    return m_imp ? m_imp->name() : "IndicatorImp";
}

inline void Indicator::name(const string& name) noexcept {
    if (m_imp) {
        m_imp->name(name);
    }
}

inline IndicatorImp::OPType Indicator::getOPType() const noexcept {
    return m_imp ? m_imp->getOPType() : IndicatorImp::INVALID;
}

inline string Indicator::long_name() const {
    return m_imp ? m_imp->long_name() : "IndicatorImp()";
}

inline size_t Indicator::discard() const noexcept {
    return m_imp ? m_imp->discard() : 0;
}

inline void Indicator::setDiscard(size_t discard) noexcept {
    if (m_imp) {
        m_imp->setDiscard(discard);
    }
}

inline void Indicator::updateDiscard(bool force) noexcept {
    if (m_imp) {
        m_imp->updateDiscard(force);
    }
}

inline size_t Indicator::getResultNumber() const noexcept {
    return m_imp ? m_imp->getResultNumber() : 0;
}

inline bool Indicator::empty() const noexcept {
    return (!m_imp || m_imp->size() == 0) ? true : false;
}

inline size_t Indicator::size() const noexcept {
    return m_imp ? m_imp->size() : 0;
}

inline Indicator Indicator::operator()() {
    return clone();
}

inline Indicator Indicator::clone() const {
    return m_imp ? Indicator(m_imp->clone()) : Indicator();
}

inline DatetimeList Indicator::getDatetimeList() const {
    return m_imp ? m_imp->getDatetimeList() : DatetimeList();
}

inline bool Indicator::existNan(size_t result_idx) const {
    return m_imp ? m_imp->existNan(result_idx) : false;
}

inline Indicator::value_t Indicator::getByDate(Datetime date, size_t num) const {
    return m_imp ? m_imp->getByDate(date, num) : Null<Indicator::value_t>();
}

inline Indicator::value_t Indicator::operator[](size_t pos) const {
    return get(pos, 0);
}

inline Indicator::value_t Indicator::get(size_t pos, size_t num) const {
    return m_imp->get(pos, num);
}

inline Indicator::value_t Indicator::front(size_t num) const {
    return m_imp->front(num);
}

inline Indicator::value_t Indicator::back(size_t num) const {
    return m_imp->back(num);
}

inline Datetime Indicator::getDatetime(size_t pos) const {
    return m_imp ? m_imp->getDatetime(pos) : Null<Datetime>();
}

inline size_t Indicator::getPos(Datetime date) const {
    return m_imp ? m_imp->getPos(date) : Null<size_t>();
}

inline Indicator::value_t Indicator::operator[](Datetime date) const {
    return getByDate(date);
}

inline bool Indicator::haveIndParam(const string& name) const {
    return m_imp ? m_imp->haveIndParam(name) : false;
}

inline void Indicator::setIndParam(const string& name, const Indicator& ind) {
    if (m_imp) {
        m_imp->setIndParam(name, ind);
    }
}

inline void Indicator::setIndParam(const string& name, const IndParam& ind) {
    if (m_imp) {
        m_imp->setIndParam(name, ind);
    }
}

inline IndParam Indicator::getIndParam(const string& name) const {
    return m_imp ? m_imp->getIndParam(name) : IndParam();
}

inline const IndicatorImpPtr Indicator::getIndParamImp(const string& name) const {
    return m_imp ? m_imp->getIndParamImp(name) : IndicatorImpPtr();
}

inline bool Indicator::isPythonObject() const noexcept {
    return m_imp ? m_imp->isPythonObject() : false;
}

//--------------------------------------------------------------
// Indicator operations
//-------------------------------------------------------------
HKU_API Indicator operator+(const Indicator&, const Indicator&);
HKU_API Indicator operator-(const Indicator&, const Indicator&);
HKU_API Indicator operator*(const Indicator&, const Indicator&);
HKU_API Indicator operator/(const Indicator&, const Indicator&);
HKU_API Indicator operator%(const Indicator&, const Indicator&);
HKU_API Indicator operator==(const Indicator&, const Indicator&);
HKU_API Indicator operator!=(const Indicator&, const Indicator&);
HKU_API Indicator operator>(const Indicator&, const Indicator&);
HKU_API Indicator operator<(const Indicator&, const Indicator&);
HKU_API Indicator operator>=(const Indicator&, const Indicator&);
HKU_API Indicator operator<=(const Indicator&, const Indicator&);
HKU_API Indicator operator&(const Indicator&, const Indicator&);
HKU_API Indicator operator|(const Indicator&, const Indicator&);

HKU_API Indicator operator+(const Indicator&, Indicator::value_t);
HKU_API Indicator operator+(Indicator::value_t, const Indicator&);
inline Indicator operator+(const Indicator& ind) {
    return ind;
}

HKU_API Indicator operator-(const Indicator&, Indicator::value_t);
HKU_API Indicator operator-(Indicator::value_t, const Indicator&);
HKU_API Indicator operator-(const Indicator& ind);

HKU_API Indicator operator*(const Indicator&, Indicator::value_t);
HKU_API Indicator operator*(Indicator::value_t, const Indicator&);

HKU_API Indicator operator/(const Indicator&, Indicator::value_t);
HKU_API Indicator operator/(Indicator::value_t, const Indicator&);

HKU_API Indicator operator%(const Indicator&, Indicator::value_t);
HKU_API Indicator operator%(Indicator::value_t, const Indicator&);

HKU_API Indicator operator==(const Indicator&, Indicator::value_t);
HKU_API Indicator operator==(Indicator::value_t, const Indicator&);

HKU_API Indicator operator!=(const Indicator&, Indicator::value_t);
HKU_API Indicator operator!=(Indicator::value_t, const Indicator&);

HKU_API Indicator operator>(const Indicator&, Indicator::value_t);
HKU_API Indicator operator>(Indicator::value_t, const Indicator&);

HKU_API Indicator operator<(const Indicator&, Indicator::value_t);
HKU_API Indicator operator<(Indicator::value_t, const Indicator&);

HKU_API Indicator operator>=(const Indicator&, Indicator::value_t);
HKU_API Indicator operator>=(Indicator::value_t, const Indicator&);

HKU_API Indicator operator<=(const Indicator&, Indicator::value_t);
HKU_API Indicator operator<=(Indicator::value_t, const Indicator&);

HKU_API Indicator operator&(const Indicator&, Indicator::value_t);
HKU_API Indicator operator&(Indicator::value_t, const Indicator&);

HKU_API Indicator operator|(const Indicator&, Indicator::value_t);
HKU_API Indicator operator|(Indicator::value_t, const Indicator&);

/**
 * Combine the results of ind1 and ind2 into one Indicator. For example, for ind = WEAVE(ind1, ind2)
 * the ind contains multiple results at this time, stored in the order of ind1 and ind2
 * @param ind1 indicator 1
 * @param ind2 indicator 2
 * @ingroup Indicator
 */
Indicator HKU_API WEAVE(const Indicator& ind1, const Indicator& ind2);

template <typename... Args>
inline Indicator WEAVE(const Indicator& ind1, const Indicator& ind2, const Args&... others) {
    HKU_CHECK(sizeof...(others) <= 4, "WEAVE() only support 6 Indicator!");
    Indicator tmp = WEAVE(ind1, ind2);
    return WEAVE(std::move(tmp), others...);
}

/**
 * Conditional function, it returns different values according to the condition.
 * @details
 * <pre>
 * Usage: IF(X,A,B) returns A if X is not 0, otherwise it returns B
 * For example: IF(CLOSE>OPEN,HIGH,LOW) returns the high price if the period closes up, otherwise
 * it returns the low price
 * </pre>
 * @param x condition indicator
 * @param a candidate indicator a
 * @param b candidate indicator b
 * @ingroup Indicator
 */
Indicator HKU_API IF(const Indicator& x, const Indicator& a, const Indicator& b);
Indicator HKU_API IF(const Indicator& x, Indicator::value_t a, const Indicator& b);
Indicator HKU_API IF(const Indicator& x, const Indicator& a, Indicator::value_t b);
Indicator HKU_API IF(const Indicator& x, Indicator::value_t a, Indicator::value_t b);

/**
 * Combine and calculate multiple indicators
 * @details
 * Combine multiple indicators for a unified calculation, the dependency between the indicators and
 * the context settings are handled automatically. This function gets all the child nodes of all the
 * input indicators and removes the duplicates, then sets the given K-line data context for every
 * indicator, and finally performs the calculation and returns the result.
 *
 * <pre>
 * Usage example:
 * IndicatorList inds = {MA(CLOSE(), 5), MA(CLOSE(), 10), MACD(CLOSE())};
 * IndicatorList results = combineCalculateIndicators(inds, kdata);
 * // results contains all the calculated indicator results
 *
 * // Get the first result column only
 * IndicatorList first_results = combineCalculateIndicators(inds, kdata, true);
 * </pre>
 *
 * @param indicators indicator list, the indicator set to be calculated in combination
 * @param kdata K-line data context, used to set the environment of the indicator calculation
 * @param tovalue whether to return the first result column only, false by default (all the result
 *                columns are returned)
 * @return IndicatorList the calculated indicator result list
 * @ingroup Indicator
 */
IndicatorList HKU_API combineCalculateIndicators(const IndicatorList& indicators,
                                                 const KData& kdata, bool tovalue = false);

} /* namespace hku */

namespace std {
template <>
class hash<hku::Indicator> {
public:
    size_t operator()(hku::Indicator const& ind) const noexcept {
        auto imp = ind.getImp();
        return imp.get() ? std::hash<hku::IndicatorImpPtr>()(imp) : 0;
    }
};

inline string to_string(const hku::Indicator& ind) {
    return ind.str();
}
}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Indicator> : ostream_formatter {};
#endif

#endif /* INDICATOR_H_ */
