/*
 * PositionRecord.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include "PositionRecord.h"
#include "../utilities/util.h"

namespace hku {

PositionRecord::PositionRecord(PositionRecord&& rv)
: stock(rv.stock),
  takeDatetime(rv.takeDatetime),
  cleanDatetime(rv.cleanDatetime),
  number(rv.number),
  stoploss(rv.stoploss),
  goalPrice(rv.goalPrice),
  totalNumber(rv.totalNumber),
  buyMoney(rv.buyMoney),
  totalCost(rv.totalCost),
  totalRisk(rv.totalRisk),
  sellMoney(rv.sellMoney),
  lastSettleDatetime(rv.lastSettleDatetime),
  lastSettleProfit(rv.lastSettleProfit),
  lastSettleClosePrice(rv.lastSettleClosePrice),
  contracts(std::move(rv.contracts)) {
    rv.stock = Null<Stock>();
}

PositionRecord& PositionRecord::operator=(PositionRecord&& rv) {
    HKU_IF_RETURN(this == &rv, *this);
    stock = rv.stock;
    takeDatetime = rv.takeDatetime;
    cleanDatetime = rv.cleanDatetime;
    number = rv.number;
    stoploss = rv.stoploss;
    goalPrice = rv.goalPrice;
    totalNumber = rv.totalNumber;
    buyMoney = rv.buyMoney;
    totalCost = rv.totalCost;
    totalRisk = rv.totalRisk;
    sellMoney = rv.sellMoney;
    lastSettleDatetime = rv.lastSettleDatetime;
    lastSettleProfit = rv.lastSettleProfit;
    lastSettleClosePrice = rv.lastSettleClosePrice;
    contracts = std::move(rv.contracts);
    rv.stock = Null<Stock>();
    return *this;
}

price_t PositionRecord::settleProfitOfPreDay(Datetime datetime, double marginRatio) {
    // 查找上一次结算日期后到当前时刻前的最后一个交易日K线记录
    price_t profit = 0.0;
    price_t addMarginCash = 0.0;  // 因保证金比例变化导致需要补缴的保证金
    KQuery query(lastSettleDatetime + Minutes(1), datetime, KQuery::DAY);
    size_t startix = 0, endix = 0;
    if (stock.getIndexRange(query, startix, endix)) {
        KRecord k = stock.getKRecord(endix - 1);
        for (auto& contract : contracts) {
            if (marginRatio != contract.marginRatio) {
                price_t currentNeed = contract.price * contract.number * stock.unit() * marginRatio;
                addMarginCash += currentNeed - contract.frozenCash;
                contract.frozenCash = currentNeed;
                contract.marginRatio = marginRatio;
            }
            profit += (k.closePrice - lastSettleClosePrice) * contract.number * stock.unit();
        }
        lastSettleDatetime = k.datetime;
        lastSettleClosePrice = k.closePrice;
        lastSettleProfit = profit;
    }
    return profit - addMarginCash;
}

price_t PositionRecord::calculateCloseProfit(price_t closePrice) const {
    price_t profit = 0.0;
    for (const auto& contract : contracts) {
        profit += (closePrice - contract.price) * contract.number * stock.unit();
    }
    return profit;
}

price_t PositionRecord::addTradeRecord(const TradeRecord& tr) {
    HKU_ASSERT(tr.business == BUSINESS_BUY || tr.business == BUSINESS_SELL);
    price_t return_cash = 0.0;
    if (stock.isNull()) {
        stock = tr.stock;
        takeDatetime = tr.datetime;
        lastSettleDatetime = takeDatetime;
        lastSettleClosePrice = tr.realPrice;
    }

    double tr_num = (tr.business == BUSINESS_BUY) ? tr.number : -tr.number;
    double new_number = number + tr_num;
    if (new_number < 0.0) {
        HKU_ERROR("The position number not match!");
        return 0.0;

    } else if (new_number == 0.0) {
        cleanDatetime = tr.datetime;
    }

    number = new_number;
    stoploss = tr.stoploss;
    goalPrice = tr.goalPrice;
    totalCost = roundEx(tr.cost.total + totalCost, stock.precision());
    totalRisk =
      roundEx(totalRisk + (tr.realPrice - tr.stoploss) * number * stock.unit(), stock.precision());

    if (tr.business == BUSINESS_BUY) {
        totalNumber += tr.number;
        buyMoney = roundEx(tr.realPrice * tr.number * stock.unit() * tr.marginRatio + buyMoney,
                           stock.precision());
        contracts.emplace_back(stock, tr.datetime, tr.realPrice, tr.number, tr.marginRatio);
    } else {
        price_t frozen_cash = 0.0;
        price_t remove_value = 0.0;
        double remove_num = 0.0;
        auto iter = contracts.begin();
        for (; iter != contracts.end(); ++iter) {
            remove_num += iter->number;
            if (remove_num == tr.number) {
                price_t value = iter->price * iter->number * stock.unit();
                remove_value += value;
                frozen_cash += iter->frozenCash;
                ++iter;
                break;
            } else if (remove_num > tr.number) {
                double sub_num = remove_num - tr.number;
                iter->number = sub_num;
                price_t value = sub_num * iter->price * stock.unit();
                remove_value += value;
                frozen_cash += value * iter->marginRatio;
                break;
            }
        }
        sellMoney = roundEx(sellMoney + frozen_cash, stock.precision());
        return_cash = frozen_cash + tr.number * tr.realPrice * stock.unit() - remove_value;
        contracts.erase(contracts.begin(), iter);
    }
    return return_cash;
}

string PositionRecord::toString() const {
    int precision = 2;
    std::string market(""), code(""), name("");
    if (!stock.isNull()) {
        market = stock.market();
        code = stock.code();
        name = stock.name();
    } else {
        precision = stock.precision();
    }

    string strip(", ");
    std::stringstream os;
    os << std::fixed;
    os.precision(precision);
    os << "Position(" << market << strip << code << strip << name << strip << takeDatetime << strip
       << cleanDatetime << strip << number << strip << stoploss << strip << goalPrice << strip
       << totalNumber << strip << buyMoney << strip << totalCost << strip << totalRisk << strip
       << sellMoney << ")";
    os.unsetf(std::ostream::floatfield);
    os.precision();
    return os.str();
}

bool HKU_API operator==(const PositionRecord& d1, const PositionRecord& d2) {
    return d1.stock == d2.stock && d1.takeDatetime == d2.takeDatetime &&
           d1.cleanDatetime == d2.cleanDatetime && fabs(d1.number - d2.number) < 0.00001 &&
           fabs(d1.stoploss - d2.stoploss) < 0.0001 && fabs(d1.goalPrice - d2.goalPrice) < 0.0001 &&
           fabs(d1.totalNumber - d2.totalNumber) < 0.00001 &&
           fabs(d1.buyMoney - d2.buyMoney) < 0.0001 && fabs(d1.totalCost - d2.totalCost) < 0.0001 &&
           fabs(d1.sellMoney - d2.sellMoney) < 0.0001;
}

} /* namespace hku */
