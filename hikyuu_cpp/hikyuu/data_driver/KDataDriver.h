/*
 * KDataDriver.h
 *
 *  Created on: 2012-9-8
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATADRIVER_H_
#define KDATADRIVER_H_

#include "hikyuu/utilities/Parameter.h"
#include "hikyuu/KQuery.h"
#include "hikyuu/TimeLineRecord.h"
#include "hikyuu/TransRecord.h"

namespace hku {

/**
 * Raw pointer view of the K-line records (zero copy)
 * @details It is filled by the driver that supports the shared memory view (the client IPC proxy
 * driver), data points to the count consecutive KRecord in the read-only shared memory; pin holds
 * the underlying mapping alive in a type-erased way (such as the shared_ptr of KDataShmReader), and
 * it must not be released within the valid period of the view. A driver that does not support the
 * view returns false, and the caller falls back to the copy path.
 */
struct KRecordView {
    const KRecord* data{nullptr};  ///< Pointer of the first record of the range (count consecutive
                                   ///< records)
    size_t count{0};               ///< Number of the records in the range
    shared_ptr<void> pin;          ///< Holds the mapping alive (type-erased)
};

/**
 * Base class of the K-line data driver
 * @ingroup DataDriver
 */
class HKU_API KDataDriver {
    PARAMETER_SUPPORT

public:
    KDataDriver();

    KDataDriver(const Parameter& params);

    /**
     * Constructor
     * @param name driver name
     */
    KDataDriver(const string& name);
    virtual ~KDataDriver() {}

    /** Get the driver name */
    const string& name() const;

    /** Driver initialization */
    bool init(const Parameter&);

    typedef shared_ptr<KDataDriver> KDataDriverPtr;
    /**
     * Clone implementation
     */
    KDataDriverPtr clone();

    /**
     * Subclass clone function implementation
     */
    virtual KDataDriverPtr _clone() = 0;

    /**
     * Interface for the subclass to initialize its private variables
     * @return
     */
    virtual bool _init() {
        return true;
    }

    /**
     * Judge whether the query by position index is faster for this engine, or the query by date is
     * faster
     */
    virtual bool isIndexFirst() = 0;

    /**
     * Whether parallel data loading is supported
     */
    virtual bool canParallelLoad() = 0;

    /**
     * Get the amount of the K-line data of the given type
     * @param market market abbreviation
     * @param code   security code
     * @param kType  K-line type
     * @return
     */
    virtual size_t getCount(const string& market, const string& code, const KQuery::KType& kType);

    /**
     * Get the K-line record index corresponding to the given date range
     * @param market market abbreviation
     * @param code   security code
     * @param query  query condition
     * @param out_start [out] the position of the corresponding K-line record
     * @param out_end [out] the position of the corresponding K-line record
     * @return
     */
    virtual bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                     size_t& out_start, size_t& out_end);

    /**
     * Get the K-line data
     * @param market market abbreviation
     * @param code   security code
     * @param query  query condition
     */
    virtual KRecordList getKRecordList(const string& market, const string& code,
                                       const KQuery& query);

    /**
     * Get the raw pointer view of the K-line records of the given range (zero copy, optional
     * implementation)
     * @details Only the client shared memory path supports it; [start_ix, end_ix) must be a
     *          resolved positive index. It returns false by default, and the caller falls back to
     *          the getKRecordList copy path.
     * @param market market abbreviation
     * @param code   security code
     * @param kType  K-line type
     * @param start_ix start index (inclusive)
     * @param end_ix   end index (exclusive)
     * @param out    [out] view handle
     * @return true is returned when it is supported and hit
     */
    virtual bool tryGetKRecordView(const string& market, const string& code,
                                   const KQuery::KType& kType, size_t start_ix, size_t end_ix,
                                   KRecordView& out);

    /**
     * Get the time-sharing (intraday) line
     * @param market market abbreviation
     * @param code   security code
     * @param query  query condition
     * @return
     */
    virtual TimeLineList getTimeLineList(const string& market, const string& code,
                                         const KQuery& query);

    /**
     * Get the historical tick data
     * @param market market abbreviation
     * @param code   security code
     * @param query  query condition
     * @return
     */
    virtual TransList getTransList(const string& market, const string& code, const KQuery& query);

    //---------------------------------------------------
    // The following is the column-oriented database interface
    //---------------------------------------------------

    /** Whether it is column-first (the K-line data is stored in a column database) */
    virtual bool isColumnFirst() const {
        return false;
    }

    virtual std::unordered_map<std::string, KRecordList> getAllKRecordList(
      const KQuery::KType& ktype, const Datetime& start_date, const std::atomic_bool& cancel_flag);

protected:
    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    bool checkType();

protected:
    string m_name;
    bool m_is_python_object{false};
};

typedef shared_ptr<KDataDriver> KDataDriverPtr;

HKU_API std::ostream& operator<<(std::ostream&, const KDataDriver&);
HKU_API std::ostream& operator<<(std::ostream&, const KDataDriverPtr&);

inline const string& KDataDriver::name() const {
    return m_name;
}

class KDataDriverConnect {
public:
    typedef KDataDriver DriverType;
    typedef KDataDriverPtr DriverTypePtr;

    explicit KDataDriverConnect(const KDataDriverPtr& driver) : m_driver(driver) {}
    ~KDataDriverConnect() = default;

    KDataDriverConnect(const KDataDriverConnect&) = delete;
    KDataDriverConnect(KDataDriverConnect&&) = delete;
    KDataDriverConnect& operator=(const KDataDriverConnect&) = delete;
    KDataDriverConnect& operator=(KDataDriverConnect&&) = delete;

    explicit operator bool() const noexcept {
        return m_driver.get() != nullptr;
    }

    const string& name() const {
        return m_driver->name();
    }

    bool isIndexFirst() {
        return m_driver->isIndexFirst();
    }

    bool canParallelLoad() {
        return m_driver->canParallelLoad();
    }

    size_t getCount(const string& market, const string& code, const KQuery::KType& kType) {
        return m_driver->getCount(market, code, kType);
    }

    bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                             size_t& out_start, size_t& out_end) {
        return m_driver->getIndexRangeByDate(market, code, query, out_start, out_end);
    }

    KRecordList getKRecordList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getKRecordList(market, code, query);
    }

    bool tryGetKRecordView(const string& market, const string& code, const KQuery::KType& kType,
                           size_t start_ix, size_t end_ix, KRecordView& out) {
        return m_driver->tryGetKRecordView(market, code, kType, start_ix, end_ix, out);
    }

    TimeLineList getTimeLineList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getTimeLineList(market, code, query);
    }

    TransList getTransList(const string& market, const string& code, const KQuery& query) {
        return m_driver->getTransList(market, code, query);
    }

    bool isColumnFirst() const {
        return m_driver->isColumnFirst();
    }

    std::unordered_map<std::string, KRecordList> getAllKRecordList(
      const KQuery::KType& ktype, const Datetime& start_date, const std::atomic_bool& cancel_flag) {
        return m_driver->getAllKRecordList(ktype, start_date, cancel_flag);
    }

private:
    KDataDriverPtr m_driver;
};

}  // namespace hku

#endif /* KDATADRIVER_H_ */
