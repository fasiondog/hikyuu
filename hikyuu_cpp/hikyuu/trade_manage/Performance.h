/*
 * Performance.h
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#pragma once

#include "TradeManagerBase.h"

namespace hku {

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#endif

/**
 * Simple performance statistics
 * @ingroup Performance
 */
class HKU_API Performance {
public:
    Performance();
    virtual ~Performance();

    Performance(const Performance& other) = default;
    Performance(Performance&& other) noexcept
    : m_result(std::move(other.m_result)), m_keys(std::move(other.m_keys)) {}

    Performance& operator=(const Performance& other) noexcept;
    Performance& operator=(Performance&& other) noexcept;

    /** Whether it is a valid statistics item
     *  @note The legacy Chinese keys (used before the i18n refactoring) are still accepted for
     *        backward compatibility, but they are deprecated */
    bool exist(const string& key);

    /** Reset, clearing the calculated results */
    void reset();

    /** Get the statistics value by the item name; it takes effect only after statistics or report
     *  has been run
     *  @note The legacy Chinese keys (used before the i18n refactoring) are still accepted for
     *        backward compatibility, but they are deprecated */
    double get(const string& name) const;

    /** The same as get */
    double operator[](const string& name) const {
        return get(name);
    }

    /**
     * A simple text statistics report, used for the direct printing output.
     * @note It takes effect only after statistics has been run, or when Performance itself is the
     *       result got from TM
     * @return
     */
    string report();

    /**
     * Count the system performance up to a certain moment according to the trade records; datetime
     * must be greater than or equal to lastDatetime so that it can be used to calculate the current
     * market value
     * @param tm the given trade management instance
     * @param datetime the statistics end moment
     */
    void statistics(const TradeManagerPtr& tm, const Datetime& datetime = Datetime::now());

    /** Get the names of all the statistics items, in the same order as values */
    const StringList& names() const {
        return m_keys;
    }

    /** Get the values of all the statistics items, in the same order as names */
    PriceList values() const;

    typedef std::map<string, double> map_type;
    typedef map_type::iterator iterator;
    typedef map_type::const_iterator const_iterator;

    const map_type& getAll() const {
        return m_result;
    }

    void addKey(const string& key);
    void setValue(const string& key, double value);

private:
    map_type m_result;
    StringList m_keys;  // Saves the order of the statistics items; neither map nor unordered_map
                        // can keep the insertion order when iterating
};

} /* namespace hku */
