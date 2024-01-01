/*
 * Datetime.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#pragma once
#ifndef DATETIME_H_
#define DATETIME_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <chrono>
#include <string>
#include <vector>
#include <fmt/ostream.h>
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
    /** 返回所能表示的最小日期：1400-Jan-01 00:00:00 */
    static Datetime min();

    /** 返回所能表示的最大日期：9999-Dec-31 00:00:00 */
    static Datetime max();

    /** 返回本地计算机当前时刻 */
    static Datetime now();

    /** 返回本地计算机今日日期 */
    static Datetime today();

    /**
     * 兼容oracle
     * datetime格式，除最高端的字节外，每个字节依次表示世纪、世纪年、月、日、时、分、秒
     */
    static Datetime fromHex(uint64_t time);

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

    /**
     * 通过数字方式构造日期类型
     * @details 支持以下两种数字格式
     * <pre>
     *     1、YYYYMMDDhhmm，如 200101010000
     *     2、YYYYMMDD, 如 20010101
     * </pre>
     */
    explicit Datetime(unsigned long long);

    /**
     * 通过字符串方式构造日期类型
     * @details 支持以下格式的字符串构造：
     * <pre>
     *     1、"2001-01-01" 或 "2001/1/1"
     *     2、"20010101"
     *     3、"2001-01-01 18:00:00.12345"
     *     4、"20010101T181159"
     * </pre>
     */
    explicit Datetime(const std::string&);

    Datetime& operator=(const Datetime&);

    /** 年份，如果是 Null 将抛出异常 */
    long year() const;

    /** 月份 [1, 12]，如果是 Null 将抛出异常 */
    long month() const;

    /** 日 [1, 31]，如果是 Null 将抛出异常 */
    long day() const;

    /** 时 [0, 23]，如果是 Null 将抛出异常 */
    long hour() const;

    /** 分钟 [0, 59]，如果是 Null 将抛出异常 */
    long minute() const;

    /** 秒 [0, 59]，如果是 Null 将抛出异常 */
    long second() const;

    /** 毫秒 [0, 999]，如果是 Null 将抛出异常 */
    long millisecond() const;

    /** 微秒 [0, 999]，如果是 Null 将抛出异常 */
    long microsecond() const;

    /** 是否为 Null<Datetime> */
    bool isNull() const;

    /** 日期运算，加指定时长 */
    Datetime operator+(TimeDelta d) const;

    /** 日期运算，减指定时长 */
    Datetime operator-(TimeDelta d) const;

    /**
     * 返回如YYYYMMDDhhmm格式的数字，方便比较操作
     * Null<Datetime>()对应的 number 为 Null<unsigned long long>
     * @note 精度到分钟
     */
    uint64_t number() const noexcept;

    /** 返回如YYYYMMDD格式的数字*/
    uint64_t ym() const noexcept;

    /** 返回如YYYYMMDD格式的数字*/
    uint64_t ymd() const noexcept;

    /** 返回如YYYYMMDDHH格式的数字*/
    uint64_t ymdh() const noexcept;

    /** 返回如YYYYMMDDhhmm格式的数字*/
    uint64_t ymdhm() const noexcept;

    /** 返回如YYYYMMDDhhmmss格式的数字*/
    uint64_t ymdhms() const noexcept;

    /**
     * 转化为 oracle datetime 方式的数字，后 7 个字节分别表示世纪、世纪中的年、月、日、时、分、秒
     * @note 精度到秒
     */
    uint64_t hex() const noexcept;

    /** 距离最小日期过去的微秒数 */
    uint64_t ticks() const noexcept;

    /**
     * 转化为字符串，供打印阅读，格式：
     * <pre>
     * 毫秒数、微秒数为零时： 2019-01-02 01:01:00
     * 毫秒数、微秒数不为零时：2019-01-02 01:01:00:000001
     * </pre>
     */
    std::string str() const;

    /**
     * 转化为字符串,
     * 格式为：Datetime(year, month, day, hour, minute, second, millisecond, microsecond)
     */
    std::string repr() const;

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
// 加、减法运算补充
//
///////////////////////////////////////////////////////////////////////////////
inline Datetime operator+(const TimeDelta& delta, const Datetime& date) {
    return date + delta;
}

inline TimeDelta operator-(const Datetime& d1, const Datetime& d2) {
    return TimeDelta(d1.ptime() - d2.ptime());
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

namespace std {
template <>
class hash<hku::Datetime> {
public:
    size_t operator()(hku::Datetime const& d) const noexcept {
        return d.ticks();  // or use boost::hash_combine
    }
};
}  // namespace std

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::Datetime> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const hku::Datetime& d, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}", d.str());
    }
};
#endif

#endif /* DATETIME_H_ */
