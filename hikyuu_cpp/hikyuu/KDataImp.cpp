/*
 * KDataImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include <functional>
#include "StockManager.h"
#include "KDataImp.h"

namespace hku {

KDataImp::KDataImp() : m_start(0), m_end(0), m_have_pos_in_stock(false) {}

KDataImp::KDataImp(const Stock& stock, const KQuery& query)
: m_query(query), m_stock(stock), m_start(0), m_end(0), m_have_pos_in_stock(false) {
    if (m_stock.isNull()) {
        return;
    }

    m_buffer = m_stock.getKRecordList(query);

    // 不支持复权时，直接返回
    if (query.recoverType() == KQuery::NO_RECOVER)
        return;

    // 日线以上复权处理
    if (query.kType() == KQuery::WEEK || query.kType() == KQuery::MONTH ||
        query.kType() == KQuery::QUARTER || query.kType() == KQuery::HALFYEAR ||
        query.kType() == KQuery::YEAR) {
        _recoverForUpDay();
        return;
    }

    switch (query.recoverType()) {
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

KDataImp::~KDataImp() {}

DatetimeList KDataImp::getDatetimeList() const {
    DatetimeList result;
    for (const auto& record : m_buffer) {
        result.emplace_back(record.datetime);
    }
    return result;
}

size_t KDataImp::startPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_start;
}

size_t KDataImp::endPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end;
}

size_t KDataImp::lastPos() {
    if (!m_have_pos_in_stock) {
        _getPosInStock();
    }
    return m_end == 0 ? 0 : m_end - 1;
}

void KDataImp::_getPosInStock() {
    bool sucess = m_stock.getIndexRange(m_query, m_start, m_end);
    if (!sucess) {
        m_start = 0;
        m_end = 0;
    }
    m_have_pos_in_stock = true;
}

size_t KDataImp::getPos(const Datetime& datetime) {
    KRecordList::const_iterator iter;
    KRecord comp_record;
    comp_record.datetime = datetime;
    iter = lower_bound(
      m_buffer.begin(), m_buffer.end(), comp_record,
      std::bind(std::less<Datetime>(), std::bind(&KRecord::datetime, std::placeholders::_1),
                std::bind(&KRecord::datetime, std::placeholders::_2)));
    if (iter == m_buffer.end() || iter->datetime != datetime) {
        return Null<size_t>();
    }

    return (iter - m_buffer.begin());
}

void KDataImp::_recoverForUpDay() {
    HKU_IF_RETURN(empty(), void());
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

    Datetime startDate = startOfPhase(m_buffer.front().datetime);
    Datetime endDate = m_buffer.back().datetime.nextDay();
    KQuery query = KQueryByDate(startDate, endDate, KQuery::DAY, m_query.recoverType());
    KData day_list = m_stock.getKData(query);
    if (day_list.empty())
        return;

    size_t day_pos = 0;
    size_t day_total = day_list.size();
    size_t length = size();
    for (size_t i = 0; i < length; i++) {
        Datetime phase_start_date = startOfPhase(m_buffer[i].datetime);
        Datetime phase_end_date = m_buffer[i].datetime;
        if (day_pos >= day_total)
            break;

        while (day_list[day_pos].datetime < phase_start_date) {
            day_pos++;
        }
        KRecord record = day_list[day_pos];
        int pre_day_pos = day_pos;
        while (day_pos < day_total && day_list[day_pos].datetime <= phase_end_date) {
            if (day_list[day_pos].lowPrice < record.lowPrice) {
                record.lowPrice = day_list[day_pos].lowPrice;
            } else if (day_list[day_pos].highPrice > record.highPrice) {
                record.highPrice = day_list[day_pos].highPrice;
            }
            record.closePrice = day_list[day_pos].closePrice;
            day_pos++;
        }
        if (pre_day_pos != day_pos) {
            m_buffer[i].openPrice = record.openPrice;
            m_buffer[i].highPrice = record.highPrice;
            m_buffer[i].lowPrice = record.lowPrice;
            m_buffer[i].closePrice = record.closePrice;
        }
    }

    return;
}

/******************************************************************************
 * 前复权公式:复权后价格＝[(复权前价格-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)
 * 向前复权指以除权后的股价为基准（即除权后的股价不变），将除权前的股价降下来。
 * 复权计算时首先从上市日开始，逐日向后判断，遇到除权日，则将上市日到除权日之间（不包括除权日）的
 * 全部股价通过复权计算降下来；然后再继续向后判断，遇到下一个除权日，则再次将上市日到该除权日之间
 * （不包括除权日）的全部股价通过复权计算降下来。
 *****************************************************************************/
void KDataImp::_recoverForward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.date());
    Datetime end_date(m_buffer.back().datetime.date() + bd::days(1));
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_iterator weightIter = weightList.begin();

    size_t pre_pos = 0;
    for (; weightIter != weightList.end(); ++weightIter) {
        // 计算流通股份变动比例,但不处理仅仅只有流通股本改变的情况
        if ((weightIter->countAsGift() == 0.0 && weightIter->countForSell() == 0.0 &&
             weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
             weightIter->increasement() == 0.0))
            continue;

        size_t i = pre_pos;
        while (i < total && m_buffer[i].datetime < weightIter->datetime()) {
            i++;
        }
        pre_pos = i;  // 除权日

        price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                weightIter->increasement());
        // change 小于 0 时为缩股
        price_t denominator =
          change < 0.0 ? std::abs(change * 0.1) : 1.0 + change;  // 分母 = (1+流通股份变动比例)
        price_t temp = weightIter->priceForSell() * change - 0.1 * weightIter->bonus();

        if (denominator == 1.0 && temp == 0.0)
            continue;

        for (i = 0; i < pre_pos; ++i) {
            m_buffer[i].openPrice =
              roundEx((m_buffer[i].openPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].highPrice =
              roundEx((m_buffer[i].highPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].lowPrice =
              roundEx((m_buffer[i].lowPrice + temp) / denominator, m_stock.precision());
            m_buffer[i].closePrice =
              roundEx((m_buffer[i].closePrice + temp) / denominator, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 后复权公式:复权后价格＝复权前价格×(1＋流通股份变动比例)-配(新)股价格×流通股份变动比例＋现金红利
 * 向后复权指以除权前的股价为基准（即除权前的股价不变），将除权后的股价升上去。复权计算时首先从最新日开始，
 * 逐日向前判断，遇到除权日，则将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去；然后再继续
 * 向前判断，遇到下一个除权日，则再次将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去。
 *****************************************************************************/
void KDataImp::_recoverBackward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.date());
    Datetime end_date(m_buffer.back().datetime.date() + bd::days(1));
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();

    size_t pre_pos = total - 1;
    for (; weightIter != weightList.rend(); ++weightIter) {
        // 计算流通股份变动比例,但不处理仅仅只有流通股本改变的情况
        if ((weightIter->countAsGift() == 0.0 && weightIter->countForSell() == 0.0 &&
             weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
             weightIter->increasement() == 0.0))
            continue;

        size_t i = pre_pos;
        while (i > 0 && m_buffer[i].datetime > weightIter->datetime()) {
            i--;
        }
        pre_pos = i;

        // 流通股份变动比例
        price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                weightIter->increasement());
        // change 小于 0 时为缩股
        price_t denominator =
          change < 0 ? std::abs(change * 0.1) : 1.0 + change;  //(1+流通股份变动比例)
        price_t temp = 0.1 * weightIter->bonus() - weightIter->priceForSell() * change;

        if (denominator == 1.0 && temp == 0.0)
            continue;

        for (i = pre_pos; i < total; ++i) {
            m_buffer[i].openPrice =
              roundEx(m_buffer[i].openPrice * denominator + temp, m_stock.precision());
            m_buffer[i].highPrice =
              roundEx(m_buffer[i].highPrice * denominator + temp, m_stock.precision());
            m_buffer[i].lowPrice =
              roundEx(m_buffer[i].lowPrice * denominator + temp, m_stock.precision());
            m_buffer[i].closePrice =
              roundEx(m_buffer[i].closePrice * denominator + temp, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 等比前复权公式:复权后价格＝复权前价格*复权率
 * 复权率＝｛[(股权登记日收盘价-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)｝÷股权登记日收盘价
 * 向前复权指以除权后的股价为基准（即除权后的股价不变），将除权前的股价降下来。
 * 复权计算时首先从上市日开始，逐日向后判断，遇到除权日，则将上市日到除权日之间（不包括除权日）的
 * 全部股价通过复权计算降下来；然后再继续向后判断，遇到下一个除权日，则再次将上市日到该除权日之间
 * （不包括除权日）的全部股价通过复权计算降下来。
 *****************************************************************************/
void KDataImp::_recoverEqualForward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.date());
    Datetime end_date(m_buffer.back().datetime.date() + bd::days(1));
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    if (weightList.empty()) {
        return;
    }

    KRecordList kdata = m_buffer;  // 防止同一天两条权息记录
    StockWeightList::const_iterator weightIter = weightList.begin();
    StockWeightList::const_iterator pre_weightIter;
    size_t pre_pos = 0;
    for (; weightIter != weightList.end(); ++weightIter) {
        // 计算流通股份变动比例,但不处理仅仅只有流通股本改变的情况
        if ((weightIter->countAsGift() == 0.0 && weightIter->countForSell() == 0.0 &&
             weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
             weightIter->increasement() == 0.0))
            continue;

        size_t i = pre_pos;
        while (i < total && m_buffer[i].datetime < weightIter->datetime()) {
            i++;
        }
        pre_pos = i;  // 除权日

        // 股权登记日（即除权日的前一天数据）收盘价
        if (pre_pos == 0) {
            continue;
        }
        price_t closePrice = kdata[pre_pos - 1].closePrice;
        if (closePrice == 0.0) {
            continue;  // 除零保护
        }

        // 流通股份变动比例
        price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                weightIter->increasement());
        // change 小于 0 时为缩股
        price_t denominator =
          change < 0.0 ? std::abs(change * 0.1) : 1.0 + change;  //(1+流通股份变动比例)
        price_t temp = weightIter->priceForSell() * change - 0.1 * weightIter->bonus();

        if (denominator == 0.0 || (denominator == 1.0 && temp == 0.0))
            continue;

        price_t k = (closePrice + temp) / (denominator * closePrice);

        for (i = 0; i < pre_pos; ++i) {
            m_buffer[i].openPrice = roundEx(k * m_buffer[i].openPrice, m_stock.precision());
            m_buffer[i].highPrice = roundEx(k * m_buffer[i].highPrice, m_stock.precision());
            m_buffer[i].lowPrice = roundEx(k * m_buffer[i].lowPrice, m_stock.precision());
            m_buffer[i].closePrice = roundEx(k * m_buffer[i].closePrice, m_stock.precision());
        }
    }
}

/******************************************************************************
 * 等比后复权公式:复权后价格＝复权前价格÷复权率
 * 复权率＝｛[(股权登记日收盘价-现金红利)＋配(新)股价格×流通股份变动比例]÷(1＋流通股份变动比例)｝÷股权登记日收盘价
 * 向后复权指以除权前的股价为基准（即除权前的股价不变），将除权后的股价升上去。复权计算时首先从最新日开始，
 * 逐日向前判断，遇到除权日，则将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去；然后再继续
 * 向前判断，遇到下一个除权日，则再次将除权日到最新日之间（包括除权日）的全部股价通过复权计算升上去。
 *****************************************************************************/
void KDataImp::_recoverEqualBackward() {
    size_t total = m_buffer.size();
    HKU_IF_RETURN(total == 0, void());

    Datetime start_date(m_buffer.front().datetime.date());
    Datetime end_date(m_buffer.back().datetime.date() + bd::days(1));
    StockWeightList weightList = m_stock.getWeight(start_date, end_date);
    StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();

    size_t pre_pos = total - 1;
    for (; weightIter != weightList.rend(); ++weightIter) {
        size_t i = pre_pos;
        while (i > 0 && m_buffer[i].datetime > weightIter->datetime()) {
            i--;
        }
        pre_pos = i;  // 除权日

        // 股权登记日（即除权日的前一天数据）收盘价
        if (pre_pos == 0) {
            continue;
        }
        price_t closePrice = m_buffer[pre_pos - 1].closePrice;

        // 流通股份变动比例
        price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                weightIter->increasement());
        // change 小于 0 时为缩股
        price_t denominator =
          change < 0.0 ? std::abs(change * 0.1) : 1.0 + change;  //(1+流通股份变动比例)
        price_t temp = closePrice + weightIter->priceForSell() * change - 0.1 * weightIter->bonus();
        if (temp == 0.0 || denominator == 0.0) {
            continue;
        }
        price_t k = (denominator * closePrice) / temp;

        for (i = pre_pos; i < total; ++i) {
            m_buffer[i].openPrice = roundEx(k * m_buffer[i].openPrice, m_stock.precision());
            m_buffer[i].highPrice = roundEx(k * m_buffer[i].highPrice, m_stock.precision());
            m_buffer[i].lowPrice = roundEx(k * m_buffer[i].lowPrice, m_stock.precision());
            m_buffer[i].closePrice = roundEx(k * m_buffer[i].closePrice, m_stock.precision());
        }
    }
}

} /* namespace hku */
