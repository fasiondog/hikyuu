/*
 * OrderBrokerBase.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include <nlohmann/json.hpp>
#include "OrderBrokerBase.h"

namespace hku {

using json = nlohmann::json;

BrokerPositionRecord::BrokerPositionRecord(const Stock& stock_, price_t number_, price_t money_)
: stock(stock_), number(number_), money(money_) {}

BrokerPositionRecord::BrokerPositionRecord(BrokerPositionRecord&& rv)
: stock(std::move(rv.stock)), number(rv.number), money(rv.money) {
    rv.number = 0.0;
    rv.money = 0.0;
}

BrokerPositionRecord& BrokerPositionRecord::operator=(BrokerPositionRecord&& rv) {
    if (this != &rv) {
        stock = std::move(rv.stock);
        number = rv.number;
        money = rv.money;
        rv.number = 0.0;
        rv.money = 0.0;
    }
    return *this;
}

string BrokerPositionRecord::str() const {
    return fmt::format("BrokerPositionRecord({}, {:<.4f}, {:<.4f})", stock.market_code(), number,
                       money);
}

HKU_API std::ostream& operator<<(std::ostream& os, const BrokerPositionRecord& pos) {
    os << pos.str();
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerBase& broker) {
    os << "OrderBroker(" << broker.name() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const OrderBrokerPtr& broker) {
    os << "OrderBroker(" << broker->name() << ")";
    return os;
}

OrderBrokerBase::OrderBrokerBase() : m_name("NO_NAME") {}

OrderBrokerBase::OrderBrokerBase(const string& name) : m_name(name) {}

OrderBrokerBase::~OrderBrokerBase() {}

void OrderBrokerBase::buy(Datetime datetime, const string& market, const string& code,
                          price_t price, double num) noexcept {
    try {
        _buy(datetime, market, code, price, num);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
}

void OrderBrokerBase::sell(Datetime datetime, const string& market, const string& code,
                           price_t price, double num) noexcept {
    try {
        _sell(datetime, market, code, price, num);
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
}

price_t OrderBrokerBase::cash() noexcept {
    price_t ret = 0.0;
    try {
        ret = _cash();
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }
    return ret;
}

vector<BrokerPositionRecord> OrderBrokerBase::position() noexcept {
    vector<BrokerPositionRecord> ret;

    vector<string> brk_positions;
    try {
        brk_positions = _position();
    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }

    HKU_IF_RETURN(brk_positions.empty(), ret);

    for (size_t i = 0, len = brk_positions.size(); i < len; i++) {
        try {
            json brk_pos(brk_positions[i]);
            if (brk_pos.empty()) {
                continue;
            }

            BrokerPositionRecord pos;
            auto market = brk_pos["market"].get<string>();
            auto code = brk_pos["code"].get<string>();
            auto stock = getStock(fmt::format("{}{}", market, code));
            if (stock.isNull()) {
                // 策略的上下文可能并不包含该股，此时忽略
                HKU_WARN("Not found the stock: {}{}", market, code);
                continue;
            }

            pos.stock = stock;
            pos.number = brk_pos["number"].get<double>();
            pos.money = brk_pos["money"].get<double>();
            if (pos.number == 0.0 || pos.money == 0.0) {
                HKU_WARN(
                  "Fetched position number({:<.4f}) or money({:<.4f}) was zero! It's ignored!",
                  pos.number, pos.money);
                continue;
            }
            ret.emplace_back(pos);

        } catch (const std::exception& e) {
            HKU_ERROR("Failed parser the [{}] record ({})! {}", i, brk_positions[i], e.what());
        } catch (...) {
            HKU_ERROR("Failed parser the [{}] record ({})! Unknown error!", i, brk_positions[i]);
        }
    }

    return ret;
}

} /* namespace hku */
