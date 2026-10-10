/*
 * KDataPrivatedBufferImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include <functional>
#include "StockManager.h"
#include "KDataPrivatedBufferImp.h"

namespace hku {

KDataPrivatedBufferImp::KDataPrivatedBufferImp() : KDataImp() {}

KDataPrivatedBufferImp::KDataPrivatedBufferImp(const Stock& stock, const KQuery& query)
: KDataImp(stock, query), m_buffer(m_stock.getKRecordList(query)) {
    _recover();
}

KDataPrivatedBufferImp::KDataPrivatedBufferImp(const Stock& stock, const KQuery& query,
                                               const KRecordList& krecords)
: KDataImp(stock, query), m_buffer(krecords) {}

KDataPrivatedBufferImp::~KDataPrivatedBufferImp() {}

DatetimeList KDataPrivatedBufferImp::getDatetimeList() const {
    DatetimeList result;
    result.reserve(m_buffer.size());
    for (const auto& record : m_buffer) {
        result.emplace_back(record.datetime);
    }
    return result;
}

size_t KDataPrivatedBufferImp::startPos() const {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_start;
}

size_t KDataPrivatedBufferImp::endPos() const {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end;
}

size_t KDataPrivatedBufferImp::lastPos() const {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end == 0 ? 0 : m_end - 1;
}

void KDataPrivatedBufferImp::_getPosInStock() const {
    bool sucess = m_stock.getIndexRange(m_query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
    }
    m_have_pos_in_stock = true;
}

size_t KDataPrivatedBufferImp::getPos(const Datetime& datetime) const noexcept {
    KRecordList::const_iterator iter;
    KRecord comp_record;
    comp_record.datetime = datetime;
    iter = lower_bound(
      m_buffer.cbegin(), m_buffer.cend(), comp_record,
      std::bind(std::less<Datetime>(), std::bind(&KRecord::datetime, std::placeholders::_1),
                std::bind(&KRecord::datetime, std::placeholders::_2)));
    if (iter == m_buffer.cend() || iter->datetime != datetime) {
        return Null<size_t>();
    }

    return (iter - m_buffer.cbegin());
}

void KDataPrivatedBufferImp::_recover() {
    // Return directly when the adjustment is not supported
    if (m_buffer.empty() || m_query.recoverType() == KQuery::NO_RECOVER)
        return;

    if (KQuery::isExtraKType(m_query.kType()))
        return;

    int64_t secs = KQuery::getKTypeInSeconds(m_query.kType());
    HKU_WARN_IF_RETURN(
      secs <= 0, void(),
      "Can't get the seconds of the ktype {}, the recovery is skipped and the raw data is kept",
      m_query.kType());

    if (secs > KQuery::getKTypeInSeconds(KQuery::DAY)) {
        _recoverForUpDay();
        return;
    }

    switch (m_query.recoverType()) {
        case KQuery::NO_RECOVER:
            // do nothing
            break;

        case KQuery::FORWARD:
            _recoverForward();
            break;

        case KQuery::BACKWARD:
            _recoverBackward();
            break;

        case KQuery::EQUAL_FORWARD:
            _recoverEqualForward();
            break;

        case KQuery::EQUAL_BACKWARD:
            _recoverEqualBackward();
            break;

        default:
            HKU_ERROR("Invalid RecvoerType!");
            return;
    }
}

void KDataPrivatedBufferImp::_recoverForUpDay() {
    HKU_IF_RETURN(m_buffer.empty(), void());
    std::function<Datetime(const Datetime&)> startOfPhase;
    if (m_query.kType() == KQuery::WEEK) {
        startOfPhase = &Datetime::startOfWeek;
    } else if (m_query.kType() == KQuery::MONTH) {
        startOfPhase = &Datetime::startOfMonth;
    } else if (m_query.kType() == KQuery::QUARTER) {
        startOfPhase = &Datetime::startOfQuarter;
    } else if (m_query.kType() == KQuery::HALFYEAR) {
        startOfPhase = &Datetime::startOfHalfyear;
    } else if (m_query.kType() == KQuery::YEAR) {
        startOfPhase = &Datetime::startOfYear;
    }

    // An extra ktype above the daily line (e.g. DAY3/DAY7) has no phase start rule; keep the raw
    // data instead of calling the empty function (std::bad_function_call)
    HKU_WARN_IF_RETURN(
      !startOfPhase, void(),
      "The ktype {} is above the daily line but has no phase start rule, the recovery is skipped "
      "and the raw data is kept",
      m_query.kType());

    Datetime startDate = startOfPhase(m_buffer.front().datetime);
    Datetime endDate = m_buffer.back().datetime.nextDay();
    KQuery query = KQueryByDate(startDate, endDate, KQuery::DAY, m_query.recoverType());
    KData day_list = m_stock.getKData(query);
    if (day_list.empty())
        return;

    size_t day_pos = 0;
    size_t day_total = day_list.size();
    size_t length = m_buffer.size();
    for (size_t i = 0; i < length; i++) {
        Datetime phase_start_date = startOfPhase(m_buffer[i].datetime);
        Datetime phase_end_date = m_buffer[i].datetime;
        if (day_pos >= day_total)
            break;

        while (day_pos < day_total && day_list[day_pos].datetime < phase_start_date) {
            day_pos++;
        }
        if (day_pos >= day_total)
            break;

        // The volume and the amount are summed inside the loop below, so the accumulator must not
        // keep the values of the first day (otherwise it is counted twice)
        KRecord record = day_list[day_pos];
        record.transCount = 0.0;
        record.transAmount = 0.0;
        int pre_day_pos = day_pos;
        while (day_pos < day_total && day_list[day_pos].datetime <= phase_end_date) {
            // The low and the high are updated independently: one day may set both a new low and a
            // new high of the phase
            if (day_list[day_pos].lowPrice < record.lowPrice) {
                record.lowPrice = day_list[day_pos].lowPrice;
            }
            if (day_list[day_pos].highPrice > record.highPrice) {
                record.highPrice = day_list[day_pos].highPrice;
            }
            record.closePrice = day_list[day_pos].closePrice;
            record.transCount += day_list[day_pos].transCount;
            record.transAmount += day_list[day_pos].transAmount;
            day_pos++;
        }
        if (pre_day_pos != day_pos) {
            m_buffer[i].openPrice = record.openPrice;
            m_buffer[i].highPrice = record.highPrice;
            m_buffer[i].lowPrice = record.lowPrice;
            m_buffer[i].closePrice = record.closePrice;
            m_buffer[i].transCount = record.transCount;
            m_buffer[i].transAmount = record.transAmount;
        }
    }

    return;
}

/******************************************************************************
 * The forward adjustment formula: the adjusted price = [(the pre-adjustment price - the cash
 * dividend) + the rights (new) share price x the change ratio of the outstanding shares] / (1 + the
 * change ratio of the outstanding shares) The forward adjustment takes the price after the
 * ex-rights as the base (i.e. the price after the ex-rights stays unchanged) and lowers the prices
 * before the ex-rights. In the adjustment calculation it starts from the listing date and goes
 * forward day by day; when an ex-rights date is met, all the prices between the listing date and
 * the ex-rights date (excluding the ex-rights date) are lowered by the adjustment calculation; then
 * it continues forward, and when the next ex-rights date is met, the prices between the listing
 * date and that ex-rights date (excluding the ex-rights date) are lowered by the adjustment
 * calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverForward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.startOfDay());
    // Anchor at the stock's last data day (ex-rights not yet reflected by the data are excluded);
    // weights after this buffer clamp to its end and are applied to it as a whole
    const Datetime& last = m_stock.lastDatetime();
    Datetime end_date = last.isNull() ? Null<Datetime>() : last + m_query.kTypeInSeconds();
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_iterator weightIter = weightList.begin();

    size_t pre_pos = 0;
    for (; weightIter != weightList.end(); ++weightIter) {
        // Calculate the change ratio of the outstanding shares; the case where only the outstanding
        // share capital changes is not handled
        if ((weightIter->countAsGift() == 0.0 && weightIter->countForSell() == 0.0 &&
             weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
             weightIter->increasement() == 0.0 && weightIter->suogu() == 0.0))
            continue;

        size_t i = pre_pos;
        while (i < total && m_buffer[i].datetime < weightIter->datetime()) {
            i++;
        }
        pre_pos = i;  // The ex-rights date

        price_t denominator = 0.0, temp = 0.0;
        if (weightIter->suogu() != 0.0) {
            denominator = weightIter->suogu();
        } else {
            price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                    weightIter->increasement());
            // A change less than 0 means a share contraction
            denominator =
              1.0 + change;  // The denominator = (1 + the change ratio of the outstanding shares)
            temp = weightIter->priceForSell() * change - 0.1 * weightIter->bonus();
        }

        if (denominator == 1.0 && temp == 0.0)
            continue;

        // Only the price is adjusted: the volume and the turnover amount are the quantities really
        // traded that day, so they are kept unchanged (the same convention as the mainstream data
        // sources, e.g. Wind / JoinQuant: the adjustment applies to the price only)
        for (i = 0; i < pre_pos; ++i) {
            m_buffer[i].openPrice = (m_buffer[i].openPrice + temp) / denominator;
            m_buffer[i].highPrice = (m_buffer[i].highPrice + temp) / denominator;
            m_buffer[i].lowPrice = (m_buffer[i].lowPrice + temp) / denominator;
            m_buffer[i].closePrice = (m_buffer[i].closePrice + temp) / denominator;
        }
    }
}

/******************************************************************************
 * The backward adjustment formula: the adjusted price = the pre-adjustment price x (1 + the change
 * ratio of the outstanding shares) - the rights (new) share price x the change ratio of the
 * outstanding shares + the cash dividend The backward adjustment takes the price before the
 * ex-rights as the base (i.e. the price before the ex-rights stays unchanged) and raises the prices
 * after the ex-rights. In the adjustment calculation it starts from the latest date and goes
 * backward day by day; when an ex-rights date is met, all the prices between the ex-rights date and
 * the latest date (including the ex-rights date) are raised by the adjustment calculation; then it
 * continues backward, and when the next ex-rights date is met, the prices between that ex-rights
 * date and the latest date (including the ex-rights date) are raised by the adjustment calculation
 * again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverBackward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    // Anchor at the stock's data start: fetch every ex-right from the beginning so the result does
    // not depend on this buffer's range; weights before the buffer clamp to its front and are
    // applied to it as a whole
    Datetime end_date(m_buffer.back().datetime + m_query.kTypeInSeconds());
    StockWeightList weightList = m_stock.getWeight(Datetime::min(), end_date);
    StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();

    size_t pre_pos = total - 1;
    for (; weightIter != weightList.rend(); ++weightIter) {
        // Calculate the change ratio of the outstanding shares; the case where only the outstanding
        // share capital changes is not handled
        if ((weightIter->countAsGift() == 0.0 && weightIter->countForSell() == 0.0 &&
             weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
             weightIter->increasement() == 0.0 && weightIter->suogu() == 0.0))
            continue;

        size_t i = pre_pos;
        while (i > 0 && m_buffer[i].datetime > weightIter->datetime()) {
            i--;
        }

        // For the minute data the adjustment starts at the first bar of the ex-rights day: only
        // skip the bar found here when it belongs to an earlier day (the buffer may already start
        // on the ex-rights day)
        if (i != pre_pos &&
            m_buffer[i].datetime.startOfDay() < weightIter->datetime().startOfDay()) {
            i++;
        }

        pre_pos = i;

        price_t denominator = 1.0, temp = 0.0;
        if (weightIter->suogu() != 0.0) {
            denominator = weightIter->suogu();
        } else {
            // The change ratio of the outstanding shares
            price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                    weightIter->increasement());
            // A change less than 0 means a share contraction
            denominator = 1.0 + change;  // (1 + the change ratio of the outstanding shares)
            temp = 0.1 * weightIter->bonus() - weightIter->priceForSell() * change;
        }

        if (denominator == 1.0 && temp == 0.0)
            continue;

        // Only the price is adjusted, see the note in _recoverForward
        for (i = pre_pos; i < total; ++i) {
            m_buffer[i].openPrice = m_buffer[i].openPrice * denominator + temp;
            m_buffer[i].highPrice = m_buffer[i].highPrice * denominator + temp;
            m_buffer[i].lowPrice = m_buffer[i].lowPrice * denominator + temp;
            m_buffer[i].closePrice = m_buffer[i].closePrice * denominator + temp;
        }
    }
}

/******************************************************************************
 * The proportional forward adjustment formula: the adjusted price = the pre-adjustment price * the
 * adjustment ratio the adjustment ratio = {[(the close price of the record date - the cash
 * dividend) + the rights (new) share price x the change ratio of the outstanding shares] / (1 + the
 * change ratio of the outstanding shares)} / the close price of the record date The forward
 * adjustment takes the price after the ex-rights as the base (i.e. the price after the ex-rights
 * stays unchanged) and lowers the prices before the ex-rights. In the adjustment calculation it
 * starts from the listing date and goes forward day by day; when an ex-rights date is met, all the
 * prices between the listing date and the ex-rights date (excluding the ex-rights date) are lowered
 * by the adjustment calculation; then it continues forward, and when the next ex-rights date is
 * met, the prices between the listing date and that ex-rights date (excluding the ex-rights date)
 * are lowered by the adjustment calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverEqualForward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime end_date(m_buffer.back().datetime + m_query.kTypeInSeconds());

    // The forward multipliers are the reciprocals of the backward ones: the ex-rights after this
    // buffer lower every bar, the ones inside it apply to the bars before the ex-rights day. Only the
    // price is adjusted, see the note in _recoverForward
    price_t seed_price = 1.0;
    size_t pos = 0;
    for (const auto& factor : m_stock.getEqualRecoverFactors()) {
        const price_t price_k = 1.0 / factor.price_k;
        if (factor.date >= end_date) {
            seed_price *= price_k;
            continue;
        }
        while (pos < total && m_buffer[pos].datetime < factor.date) {
            pos++;
        }
        for (size_t i = 0; i < pos; i++) {
            m_buffer[i].openPrice *= price_k;
            m_buffer[i].highPrice *= price_k;
            m_buffer[i].lowPrice *= price_k;
            m_buffer[i].closePrice *= price_k;
        }
    }

    if (seed_price != 1.0) {
        for (auto& record : m_buffer) {
            record.openPrice *= seed_price;
            record.highPrice *= seed_price;
            record.lowPrice *= seed_price;
            record.closePrice *= seed_price;
        }
    }
}

/******************************************************************************
 * The proportional backward adjustment formula: the adjusted price = the pre-adjustment price / the
 * adjustment ratio the adjustment ratio = {[(the close price of the record date - the cash
 * dividend) + the rights (new) share price x the change ratio of the outstanding shares] / (1 + the
 * change ratio of the outstanding shares)} / the close price of the record date The backward
 * adjustment takes the price before the ex-rights as the base (i.e. the price before the ex-rights
 * stays unchanged) and raises the prices after the ex-rights. In the adjustment calculation it
 * starts from the latest date and goes backward day by day; when an ex-rights date is met, all the
 * prices between the ex-rights date and the latest date (including the ex-rights date) are raised
 * by the adjustment calculation; then it continues backward, and when the next ex-rights date is
 * met, the prices between that ex-rights date and the latest date (including the ex-rights date)
 * are raised by the adjustment calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverEqualBackward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.startOfDay());
    Datetime end_date(m_buffer.back().datetime + m_query.kTypeInSeconds());

    // The equal-ratio multipliers are window independent: the ex-rights before the window scale the
    // whole buffer (the fixed baseline), the ones inside it apply from the ex-rights day on. Only the
    // price is adjusted, see the note in _recoverForward
    price_t seed_price = 1.0;
    size_t pos = 0;
    for (const auto& factor : m_stock.getEqualRecoverFactors()) {
        if (factor.date >= end_date) {
            break;
        }
        if (factor.date < start_date) {
            seed_price *= factor.price_k;
            continue;
        }
        while (pos < total && m_buffer[pos].datetime < factor.date) {
            pos++;
        }
        for (size_t i = pos; i < total; i++) {
            m_buffer[i].openPrice *= factor.price_k;
            m_buffer[i].highPrice *= factor.price_k;
            m_buffer[i].lowPrice *= factor.price_k;
            m_buffer[i].closePrice *= factor.price_k;
        }
    }

    if (seed_price != 1.0) {
        for (auto& record : m_buffer) {
            record.openPrice *= seed_price;
            record.highPrice *= seed_price;
            record.lowPrice *= seed_price;
            record.closePrice *= seed_price;
        }
    }
}

KDataImpPtr KDataPrivatedBufferImp::getOtherFromSelf(const KQuery& query) const {
    KDataImpPtr ret;
    // The other restrictions are guarded by the upper layer
    if (query.queryType() == KQuery::INDEX && m_query.queryType() == KQuery::INDEX) {
        ret = _getOtherFromSelfByIndex(query);
    } else if (query.queryType() == KQuery::DATE) {
        ret = _getOtherFromSelfByDate(query);
    } else {
        ret = std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
    }
    return ret;
}

KDataImpPtr KDataPrivatedBufferImp::_getOtherFromSelfByIndex(const KQuery& query) const {
    size_t new_start_pos = 0, new_end_pos = 0;
    bool success = m_stock.getIndexRange(query, new_start_pos, new_end_pos);
    if (!success || new_end_pos == 0) {
        auto* p = new KDataPrivatedBufferImp;
        p->m_stock = m_stock;
        p->m_query = query;
        if (query.recoverType() != KQuery::NO_RECOVER) {
            p->_recover();
        }
        return KDataImpPtr(p);
    }

    size_t new_last_pos = new_end_pos - 1;

    size_t old_start_pos = startPos();
    size_t old_last_pos = lastPos();
    if (new_start_pos < old_start_pos || new_start_pos > old_last_pos) {
        return std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
    }

    if (new_last_pos <= old_last_pos) {
        auto* p = new KDataPrivatedBufferImp;
        p->m_stock = m_stock;
        p->m_query = query;
        size_t new_len = new_last_pos + 1 - new_start_pos;
        p->m_buffer.resize(new_len);
        std::copy(m_buffer.begin() + new_start_pos - old_start_pos,
                  m_buffer.begin() + new_last_pos + 1 - old_start_pos, p->m_buffer.begin());
        // The copied slice is already recovered with the same anchor as self, never recover again
        return KDataImpPtr(p);
    }

    // The new range extends beyond the loaded buffer: rebuild from raw data; every type is anchored
    // at the fixed baseline, so the result stays consistent with the source KData
    auto* p = new KDataPrivatedBufferImp;
    p->m_stock = m_stock;
    p->m_query = query;
    p->m_buffer = m_stock.getKRecordList(query);
    p->_recover();
    return KDataImpPtr(p);
}

KDataImpPtr KDataPrivatedBufferImp::_getOtherFromSelfByDate(const KQuery& query) const {
    Datetime new_start_date = query.startDatetime();
    Datetime new_end_date = query.endDatetime();
    const auto& old_start_date = m_buffer.front().datetime;
    const auto& old_last_date = m_buffer.back().datetime;
    if (new_start_date >= new_end_date || new_start_date < old_start_date ||
        new_start_date > old_last_date ||
        (new_end_date != Null<Datetime>() && new_end_date <= old_start_date)) {
        return std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
    }

    auto iter =
      std::lower_bound(m_buffer.begin(), m_buffer.end(), KRecord{new_start_date},
                       [](const KRecord& a, const KRecord& b) { return a.datetime < b.datetime; });
    if (iter == m_buffer.end()) {
        return std::make_shared<KDataPrivatedBufferImp>(m_stock, query);
    }
    size_t new_start_pos_in_old = std::distance(m_buffer.begin(), iter);

    size_t new_end_pos_in_old = m_buffer.size();
    if (new_end_date != Null<Datetime>()) {
        iter = std::lower_bound(
          m_buffer.begin(), m_buffer.end(), KRecord{new_end_date},
          [](const KRecord& a, const KRecord& b) { return a.datetime < b.datetime; });
        if (iter != m_buffer.end()) {
            new_end_pos_in_old = std::distance(m_buffer.begin(), iter);
            auto* p = new KDataPrivatedBufferImp;
            p->m_stock = m_stock;
            p->m_query = query;
            size_t copy_len = new_end_pos_in_old - new_start_pos_in_old;
            p->m_buffer.resize(copy_len);
            std::copy(m_buffer.begin() + new_start_pos_in_old,
                      m_buffer.begin() + new_end_pos_in_old, p->m_buffer.begin());
            // The copied slice is already recovered with the same anchor as self, never recover again
            return KDataImpPtr(p);
        }
    }

    // The new range extends beyond the loaded buffer: rebuild from raw data; every type is anchored
    // at the fixed baseline, so the result stays consistent with the source KData
    auto* p = new KDataPrivatedBufferImp;
    p->m_stock = m_stock;
    p->m_query = query;
    p->m_buffer = m_stock.getKRecordList(query);
    p->_recover();
    return KDataImpPtr(p);
}

} /* namespace hku */
