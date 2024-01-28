/*
 * Performance.cpp
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/lexical_cast.hpp"
#include "Performance.h"

namespace hku {

Performance::Performance()
: m_result{{"帐户初始金额", 0.},
           {"累计投入本金", 0.},
           {"累计投入资产", 0.},
           {"累计借入现金", 0.},
           {"累计借入资产", 0.},
           {"累计红利", 0.},
           {"现金余额", 0.},
           {"未平仓头寸净值", 0.},
           {"当前总资产", 0.},
           {"已平仓交易总成本", 0.},
           {"已平仓净利润总额", 0.},
           {"单笔交易最大占用现金比例%", 0.},
           {"交易平均占用现金比例%", 0.},
           {"已平仓帐户收益率%", 0.},
           {"帐户年复合收益率%", 0.},
           {"帐户平均年收益率%", 0.},
           {"赢利交易赢利总额", 0.},
           {"亏损交易亏损总额", 0.},
           {"已平仓交易总数", 0.},
           {"赢利交易数", 0.},
           {"亏损交易数", 0.},
           {"赢利交易比例%", 0.},
           {"赢利期望值", 0.},
           {"赢利交易平均赢利", 0.},
           {"亏损交易平均亏损", 0.},
           {"平均赢利/平均亏损比例", 0.},
           {"净赢利/亏损比例", 0.},
           {"最大单笔赢利", 0.},
           {"最大单笔盈利百分比%", 0.},
           {"最大单笔亏损", 0.},
           {"最大单笔亏损百分比%", 0.},
           {"赢利交易平均持仓时间", 0.},
           {"赢利交易最大持仓时间", 0.},
           {"亏损交易平均持仓时间", 0.},
           {"亏损交易最大持仓时间", 0.},
           {"空仓总时间", 0.},
           {"空仓时间/总时间%", 0.},
           {"平均空仓时间", 0.},
           {"最长空仓时间", 0.},
           {"最大连续赢利笔数", 0.},
           {"最大连续亏损笔数", 0.},
           {"最大连续赢利金额", 0.},
           {"最大连续亏损金额", 0.},
           {"R乘数期望值", 0.},
           {"交易机会频率/年", 0.},
           {"年度期望R乘数", 0.},
           {"赢利交易平均R乘数", 0.},
           {"亏损交易平均R乘数", 0.},
           {"最大单笔赢利R乘数", 0.},
           {"最大单笔亏损R乘数", 0.},
           {"最大连续赢利R乘数", 0.},
           {"最大连续亏损R乘数", 0.}} {}

Performance::~Performance() {}

Performance& Performance::operator=(const Performance& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_result = other.m_result;
    return *this;
}

Performance& Performance::operator=(Performance&& other) {
    HKU_IF_RETURN(this == &other, *this);
    m_result = std::move(other.m_result);
    return *this;
}

void Performance::reset() {
    map_type::iterator iter = m_result.begin();
    for (; iter != m_result.end(); ++iter) {
        iter->second = 0.0;
    }
}

double Performance::get(const string& name) const {
    auto iter = m_result.find(name);
    if (iter != m_result.end()) {
        return iter->second;
    }
    return Null<double>();
}

StringList Performance::names() const {
    StringList result(m_result.size());
    size_t i = 0;
    for (auto iter = m_result.cbegin(); iter != m_result.cend(); ++iter, i++) {
        result[i] = iter->first;
    }
    return result;
}

PriceList Performance::values() const {
    PriceList result(m_result.size());
    size_t i = 0;
    for (auto iter = m_result.cbegin(); iter != m_result.cend(); ++iter, i++) {
        result[i] = iter->second;
    }
    return result;
}

string Performance::report(const TradeManagerPtr& tm, const Datetime& datetime) {
    std::stringstream buf;
    HKU_INFO_IF_RETURN(!tm, buf.str(), "TradeManagerPtr is Null!");

    statistics(tm, datetime);

    buf << std::fixed;
    buf.precision(2);

    buf.setf(std::ios_base::fixed);
    buf.precision(tm->precision());
    for (auto iter = m_result.begin(); iter != m_result.end(); ++iter) {
        buf << iter->first << ": " << iter->second << std::endl;
    }

    buf.unsetf(std::ostream::floatfield);
    buf.precision();
    return buf.str();
}

void Performance ::statistics(const TradeManagerPtr& tm, const Datetime& datetime) {
    // 清除上次统计结果
    reset();

    if (!tm) {
        HKU_INFO("TradeManagerPtr is Null!");
        return;
    }

    if (datetime < tm->lastDatetime()) {
        HKU_ERROR("datetime must >= tm->lastDatetime !");
        return;
    }

    int precision = tm->precision();
    m_result["帐户初始金额"] = tm->initCash();
    FundsRecord funds = tm->getFunds(datetime, KQuery::DAY);
    m_result["现金余额"] = funds.cash;
    m_result["累计投入本金"] = funds.base_cash;
    m_result["累计投入资产"] = funds.base_asset;
    m_result["累计借入现金"] = funds.borrow_cash;
    m_result["累计借入资产"] = funds.borrow_asset;
    m_result["未平仓头寸净值"] = funds.market_value;
    m_result["当前总资产"] =
      funds.cash + funds.market_value - funds.borrow_cash - funds.borrow_asset;
    price_t total_money = funds.base_cash + funds.base_asset;

    const TradeRecordList& trade_list = tm->getTradeList();
    TradeRecordList::const_iterator trade_iter = trade_list.begin();
    for (; trade_iter != trade_list.end(); ++trade_iter) {
        if (trade_iter->business == BUSINESS_BONUS) {
            m_result["累计红利"] += trade_iter->realPrice;
        }
    }

    struct CalData {
        CalData()
        : total_duration(0),
          continues(0),
          max_continues(0),
          continues_money(0.0),
          max_continues_money(0.0),
          total_r(0.0),
          max_continues_r(0.0) {}

        int total_duration;           // 总持仓时间
        int continues;                // 当前连续持仓时间
        int max_continues;            // 最大连续持仓数
        price_t continues_money;      // 当前连续持仓利润或损失
        price_t max_continues_money;  // 最大连续持仓利润或损失
        price_t total_r;              // 累计r乘数
        price_t max_continues_r;      // 最大连续盈利或损失的r乘数和
    };

    CalData earn, loss;

    bool pre_earn = true;
    const PositionRecordList& his_position = tm->getHistoryPositionList();
    price_t total_r = 0.0;
    m_result["已平仓交易总数"] = (double)his_position.size();
    PositionRecordList::const_iterator his_iter = his_position.begin();
    for (; his_iter != his_position.end(); ++his_iter) {
        const PositionRecord& pos = *his_iter;
        m_result["已平仓交易总成本"] += pos.totalCost;

        price_t profit = roundEx(pos.sellMoney - pos.totalCost - pos.buyMoney, precision);
        m_result["已平仓净利润总额"] = roundEx(m_result["已平仓净利润总额"] + profit, precision);

        price_t profit_percent = profit / (pos.buyMoney + pos.totalCost) * 100.;

        price_t r = roundEx(profit / pos.totalRisk, precision);
        total_r += r;

        if (profit > 0.0) {
            m_result["赢利交易数"]++;
            m_result["赢利交易赢利总额"] =
              roundEx(profit + m_result["赢利交易赢利总额"], precision);
            if (profit > m_result["最大单笔赢利"]) {
                m_result["最大单笔赢利"] = profit;
            }

            if (profit_percent > m_result["最大单笔盈利百分比%"]) {
                m_result["最大单笔盈利百分比%"] = profit_percent;
            }

            int duration = (pos.cleanDatetime.date() - pos.takeDatetime.date()).days();
            earn.total_duration += duration;
            if (duration > m_result["赢利交易最大持仓时间"]) {
                m_result["赢利交易最大持仓时间"] = duration;
            }

            earn.total_r += r;
            if (r > m_result["最大单笔赢利R乘数"]) {
                m_result["最大单笔赢利R乘数"] = r;
            }

            // 上一笔交易是盈利交易
            if (pre_earn) {
                earn.continues++;
                earn.continues_money = roundEx(profit + earn.continues_money, precision);
                if (earn.continues >= earn.max_continues) {
                    earn.max_continues = earn.continues;
                    if (earn.continues_money > earn.max_continues_money) {
                        earn.max_continues_money = earn.continues_money;
                        earn.max_continues_r += r;
                    }
                }
            } else {
                earn.continues = 1;
                earn.continues_money = profit;
                earn.max_continues = 1;
                if (profit > earn.max_continues_money) {
                    earn.max_continues_money = profit;
                    earn.max_continues_r = r;
                }
            }

            pre_earn = true;

        } else {
            // 没赚钱的，记为亏损交易
            m_result["亏损交易数"]++;
            m_result["亏损交易亏损总额"] =
              roundEx(profit + m_result["亏损交易亏损总额"], precision);
            if (profit < m_result["最大单笔亏损"]) {
                m_result["最大单笔亏损"] = profit;
            }

            if (profit_percent < m_result["最大单笔亏损百分比%"]) {
                m_result["最大单笔亏损百分比%"] = profit_percent;
            }

            int duration = (pos.cleanDatetime.date() - pos.takeDatetime.date()).days();
            loss.total_duration += duration;
            if (duration > m_result["亏损交易最大持仓时间"]) {
                m_result["亏损交易最大持仓时间"] = duration;
            }

            loss.total_r += r;
            if (r < m_result["最大单笔亏损R乘数"]) {
                m_result["最大单笔亏损R乘数"] = r;
            }

            // 上一次是亏损交易
            if (!pre_earn) {
                loss.continues++;
                loss.continues_money = roundEx(profit + loss.continues_money, precision);
                if (loss.continues >= loss.max_continues) {
                    loss.max_continues = loss.continues;
                    if (loss.continues_money < loss.max_continues_money) {
                        loss.max_continues_money = loss.continues_money;
                        loss.max_continues_r += r;
                    }
                }

            } else {
                loss.continues = 1;
                loss.continues_money = profit;
                loss.max_continues = 1;
                if (profit < loss.max_continues_money) {
                    loss.max_continues_money = profit;
                    loss.max_continues_r = r;
                }
            }

            pre_earn = false;
        }
    }

    m_result["最大连续赢利笔数"] = earn.max_continues;
    m_result["最大连续赢利金额"] = earn.max_continues_money;
    m_result["最大连续亏损笔数"] = loss.max_continues;
    m_result["最大连续亏损金额"] = loss.max_continues_money;

    if (m_result["最大连续赢利笔数"] != 0.0) {
        m_result["最大连续赢利R乘数"] =
          roundEx(earn.max_continues_r / m_result["最大连续赢利笔数"], precision);
    }

    if (m_result["最大连续亏损笔数"] != 0.0) {
        m_result["最大连续亏损R乘数"] =
          roundEx(loss.max_continues_r / m_result["最大连续亏损笔数"], precision);
    }

    if (m_result["累计投入本金"] != 0.0) {
        m_result["已平仓帐户收益率%"] =
          100 * m_result["已平仓净利润总额"] / m_result["累计投入本金"];
    }

    if (m_result["赢利交易数"] != 0.0) {
        m_result["赢利交易平均赢利"] =
          roundEx(m_result["赢利交易赢利总额"] / m_result["赢利交易数"], precision);
        m_result["赢利交易平均持仓时间"] = earn.total_duration / m_result["赢利交易数"];
        m_result["赢利交易平均R乘数"] = roundEx(earn.total_r / m_result["赢利交易数"], precision);
    }

    if (m_result["亏损交易数"] != 0.0) {
        m_result["亏损交易平均亏损"] =
          roundEx(m_result["亏损交易亏损总额"] / m_result["亏损交易数"], precision);
        m_result["亏损交易平均持仓时间"] = loss.total_duration / m_result["亏损交易数"];
        m_result["亏损交易平均R乘数"] = roundEx(loss.total_r / m_result["亏损交易数"], precision);
    }

    if (m_result["亏损交易平均亏损"] != 0.0) {
        m_result["平均赢利/平均亏损比例"] = roundEx(
          m_result["赢利交易平均赢利"] / std::fabs(m_result["亏损交易平均亏损"]), precision);
    }

    if (m_result["已平仓交易总数"] != 0.0) {
        m_result["赢利交易比例%"] = 100 * m_result["赢利交易数"] / m_result["已平仓交易总数"];
        m_result["R乘数期望值"] = roundEx(total_r / m_result["已平仓交易总数"], precision);
    }

    if (m_result["亏损交易亏损总额"] != 0.0) {
        m_result["净赢利/亏损比例"] =
          m_result["赢利交易赢利总额"] / std::fabs(m_result["亏损交易亏损总额"]);
    }

    m_result["赢利期望值"] = 0.01 * m_result["赢利交易比例%"] * m_result["赢利交易平均赢利"] +
                             (1 - 0.01 * m_result["赢利交易比例%"]) * m_result["亏损交易平均亏损"];

    int duration = 0;
    if (tm->firstDatetime() != Null<Datetime>()) {
        if (datetime == Null<Datetime>()) {
            duration = (Datetime::now().date() - tm->firstDatetime().date()).days();
        } else {
            duration = (datetime.date() - tm->firstDatetime().date()).days();
        }
    }

    double years = duration / 365.0;

    if (duration != 0) {
        m_result["交易机会频率/年"] = m_result["已平仓交易总数"] / years;
        m_result["年度期望R乘数"] =
          roundEx(m_result["R乘数期望值"] * m_result["交易机会频率/年"], precision);
    }

    if (total_money != 0.0 && years != 0.0) {
        m_result["帐户平均年收益率%"] =
          100 * (((m_result["当前总资产"] / total_money) - 1) / years);
        m_result["帐户年复合收益率%"] =
          100 * ((std::pow(10, (std::log10(m_result["当前总资产"] / total_money) / years)) - 1));
    }

    double max_percent = 0.0, sum_percent = 0.0;
    int trade_number = 0;
    trade_iter = trade_list.begin();
    for (; trade_iter != trade_list.end(); ++trade_iter) {
        if (trade_iter->business == BUSINESS_BUY) {
            trade_number++;
            const TradeRecord& record = *trade_iter;
            price_t hold_cash =
              roundEx(record.realPrice * record.number + record.cost.total, precision);
            price_t total_cash = roundEx(hold_cash + record.cash, precision);
            double percent = (total_cash != 0.0) ? hold_cash / total_cash : 0.0;
            sum_percent += percent;
            if (percent > max_percent) {
                max_percent = percent;
            }
        }
    }

    m_result["单笔交易最大占用现金比例%"] = 100 * max_percent;
    if (trade_number != 0) {
        m_result["交易平均占用现金比例%"] = 100 * sum_percent / trade_number;
    }

    PositionRecordList cur_position = tm->getPositionList();
    PositionRecordList::const_iterator cur_iter;
    int total_short_days = 0;

    if (tm->firstDatetime() != Null<Datetime>()) {
        int short_number = 0;
        int short_days = 0;
        int max_short_days = 0;
        bool pre_short = false;
        Datetime end_day;
        if (datetime == Null<Datetime>()) {
            end_day = Datetime(tm->lastDatetime().date() + bd::days(1));
        } else {
            end_day = Datetime(datetime.date() + bd::days(1));
        }

        DatetimeList day_range = getDateRange(tm->firstDatetime(), end_day);
        DatetimeList::const_iterator day_iter = day_range.begin();
        for (; day_iter != day_range.end(); ++day_iter) {
            bool hold = false;
            his_iter = his_position.begin();
            for (; his_iter != his_position.end(); ++his_iter) {
                if (his_iter->takeDatetime <= *day_iter && *day_iter < his_iter->cleanDatetime) {
                    hold = true;
                    break;
                }
            }

            if (hold) {
                if (pre_short) {
                    short_days = 0;
                    pre_short = false;
                }
                continue;
            }

            cur_iter = cur_position.begin();
            for (; cur_iter != cur_position.end(); ++cur_iter) {
                if (cur_iter->takeDatetime <= *day_iter) {
                    hold = false;
                    break;
                }
            }

            if (hold) {
                if (pre_short) {
                    short_days = 0;
                    pre_short = false;
                }
                continue;
            }

            // 当前是空仓
            total_short_days++;
            if (pre_short) {
                short_days++;
                if (short_days > max_short_days) {
                    max_short_days = short_days;
                }
            } else {
                short_number++;
                pre_short = true;
            }
        }

        m_result["空仓总时间"] = total_short_days;
        m_result["最长空仓时间"] = max_short_days;
        if (day_range.size() != 0) {
            m_result["空仓时间/总时间%"] = 100 * total_short_days / day_range.size();
        }
        if (short_number != 0) {
            m_result["平均空仓时间"] = total_short_days / short_number;
        }
    }
}

} /* namespace hku */
