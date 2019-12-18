/*
 * Datetime.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#pragma once
#ifndef DATETIME_H_
#define DATETIME_H_

#include <chrono>
#include <string>
#include <vector>
#include "TimeDelta.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#endif

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * 日期类型
 * @details 构造失败将抛出异常 std::out_of_range
 * @ingroup DataType
 */
class HKU_API Datetime {
public:
    /** 默认构造函数，Null<Datetime> */
    Datetime();

    Datetime(const Datetime&);

    /**
     * 构造函数
     * @param year 年
     * @param month 月
     * @param day 日
     * @param hh 时
     * @param mm 分
     * @param sec 秒
     * @param millisec 毫秒
     * @param microsec 微秒
     */
    Datetime(long year, long month, long day, long hh = 0, long mm = 0, long sec = 0,
             long millisec = 0, long microsec = 0);

    /** 从boost::gregorian::date构造日期类型 */
    explicit Datetime(const bd::date&);

    /** 从boost::posix_time::ptime构造 */
    explicit Datetime(const bt::ptime&);

    /** 通过数字方式构造日期类型，数字格式：YYYYMMDDhhmm，如 200101010000 */
    explicit Datetime(unsigned long long);

    /** 通过字符串方式构造日期类型，如："2001-01-01 18:00:00.12345" */
    explicit Datetime(const std::string&);

    Datetime& operator=(const Datetime&);

    /** 年份 */
    long year() const;

    /** 月份 [1, 12] */
    long month() const;

    /** 日 [1, 31] */
    long day() const;

    /** 时 [0, 23] */
    long hour() const;

    /** 分钟 [0, 59] */
    long minute() const;

    /** 秒 [0, 59] */
    long second() const;

    /** 毫秒 [0, 999] */
    long millisecond() const;

    /** 微秒 [0, 999] */
    long microsecond() const;

    /** 日期运算，加指定时长 */
    Datetime operator+(TimeDelta d) const;

    /** 日期运算，减指定时长 */
    Datetime operator-(TimeDelta d) const;

    /**
     * 返回如YYYYMMDDhhmmss格式的数字，方便比较操作，
     * Null<Datetime>()对应的 number 为 Null<unsigned long long>
     */
    unsigned long long number() const;

    std::string toString() const;

    /** 返回 boost::posix_time::ptime */
    bt::ptime ptime() const;

    /** 返回 boost::gregorian::date */
    bd::date date() const;

    /** 返回 std::time_t */
    std::time_t to_time_t() const;

    /** 返回一周中的第几天，周日为0，周一为1 */
    int dayOfWeek() const;

    /** 返回一年中的第几天，1月1日为1年中的第一天 */
    int dayOfYear() const;

    /** 当日起始日期，即0点 */
    Datetime startOfDay() const;

    /** 当日结束日期，即23:59:59 */
    Datetime endOfDay() const;

    /**
     * 返回指定的本周中第几天的日期，周日为0天，周六为第6天
     * @param day 指明本周的第几天，如不在0~6之间，将返回Null
     */
    Datetime dateOfWeek(int day) const;

    /** 返回周起始日期（周一） */
    Datetime startOfWeek() const;

    /** 返回周结束日期（周日） */
    Datetime endOfWeek() const;

    /** 返回月度起始日期 */
    Datetime startOfMonth() const;

    /** 返回月末日期，如12月31日 */
    Datetime endOfMonth() const;

    /** 返回季度起始日期 */
    Datetime startOfQuarter() const;

    /** 返回季度结束日期 */
    Datetime endOfQuarter() const;

    /** 返回半年起始日期 */
    Datetime startOfHalfyear() const;

    /** 返回半年结束日期 */
    Datetime endOfHalfyear() const;

    /** 返回年度起始日期 */
    Datetime startOfYear() const;

    /** 返回年度结束日期 */
    Datetime endOfYear() const;

    /** 下一自然日 */
    Datetime nextDay() const;

    /** 下周起始日期（周一） */
    Datetime nextWeek() const;

    /** 下月起始日期 */
    Datetime nextMonth() const;

    /** 下一季度起始日期 */
    Datetime nextQuarter() const;

    /** 下一半年度起始日期 */
    Datetime nextHalfyear() const;

    /** 下一年度起始日期 */
    Datetime nextYear() const;

    /** 上一自然日 */
    Datetime preDay() const;

    /** 上一周周一日期 */
    Datetime preWeek() const;

    /** 上一月起始日期 */
    Datetime preMonth() const;

    /** 上一季度起始日期 */
    Datetime preQuarter() const;

    /** 上一半年度起始日期 */
    Datetime preHalfyear() const;

    /** 上一年度起始日期 */
    Datetime preYear() const;

    /** 返回所能表示的最小日期：1400-Jan-01 00:00:00 */
    static Datetime min();

    /** 返回所能表示的最大日期：9999-Dec-31 00:00:00 */
    static Datetime max();

    /** 返回本地计算机当前时刻 */
    static Datetime now();

    /** 返回本地计算机今日日期 */
    static Datetime today();

private:
    bt::ptime m_data;
};

HKU_API std::ostream& operator<<(std::ostream&, const Datetime&);

/**
 * 日期列表
 * @ingroup DataType
 */
typedef std::vector<Datetime> DatetimeList;

/**
 * 获取指定范围的日历日期列表[start, end)，仅仅是日，不含时分秒
 * @param start 起始日期
 * @param end 结束日期
 * @return [start, end)范围内的日历日期
 */
DatetimeList HKU_API getDateRange(const Datetime& start, const Datetime& end);

///////////////////////////////////////////////////////////////////////////////
//
// 关系比较函数, 不直接在类中定义是为了支持 Null<>() == d，Null可以放在左边
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const Datetime&, const Datetime&);
bool operator!=(const Datetime&, const Datetime&);
bool operator>(const Datetime&, const Datetime&);
bool operator<(const Datetime&, const Datetime&);
bool operator>=(const Datetime&, const Datetime&);
bool operator<=(const Datetime&, const Datetime&);

inline bool operator==(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() == d2.ptime();
}

inline bool operator!=(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() != d2.ptime();
}

inline bool operator>(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() > d2.ptime();
}

inline bool operator<(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() < d2.ptime();
}

inline bool operator>=(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() >= d2.ptime();
}

inline bool operator<=(const Datetime& d1, const Datetime& d2) {
    return d1.ptime() <= d2.ptime();
}

///////////////////////////////////////////////////////////////////////////////
//
// inline 成员函数定义
//
///////////////////////////////////////////////////////////////////////////////

inline Datetime::Datetime() {
    bd::date d(bd::pos_infin);
    m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
}

inline Datetime::Datetime(const Datetime& d) : m_data(d.m_data) {}

inline Datetime::Datetime(const bd::date& d) : m_data(bt::ptime(d, bt::time_duration(0, 0, 0))) {}

inline Datetime::Datetime(const bt::ptime& d) : m_data(d) {}

inline Datetime::Datetime(const std::string& ts) {
    if (ts == "+infinity") {
        bd::date d(bd::pos_infin);
        m_data = bt::ptime(d, bt::time_duration(0, 0, 0));
    } else {
        m_data = bt::time_from_string(ts);
    }
}

inline long Datetime::year() const {
    return m_data.date().year();
}

inline long Datetime::month() const {
    return m_data.date().month();
}

inline long Datetime::day() const {
    return m_data.date().day();
}

inline long Datetime::hour() const {
    return long(m_data.time_of_day().hours());
}

inline long Datetime::minute() const {
    return long(m_data.time_of_day().minutes());
}

inline long Datetime::second() const {
    return long(m_data.time_of_day().seconds());
}

inline long Datetime::millisecond() const {
    return long(m_data.time_of_day().fractional_seconds()) / 1000;
}

inline long Datetime::microsecond() const {
    return long(m_data.time_of_day().fractional_seconds()) % 1000;
}

inline bt::ptime Datetime::ptime() const {
    return m_data;
}

inline bd::date Datetime::date() const {
    return m_data.date();
}

inline std::time_t Datetime::to_time_t() const {
    std::tm tt = bt::to_tm(m_data);
    return std::mktime(&tt);
}

inline int Datetime::dayOfWeek() const {
    return date().day_of_week();
}

inline int Datetime::dayOfYear() const {
    return date().day_of_year();
}

inline Datetime Datetime::startOfDay() const {
    return Datetime(date());
}

inline Datetime Datetime::operator+(TimeDelta d) const {
    return Datetime(m_data + d.time_duration());
}

inline Datetime Datetime::operator-(TimeDelta d) const {
    return Datetime(m_data - d.time_duration());
}

} /* namespace hku */
#endif /* DATETIME_H_ */
