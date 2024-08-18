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

BrokerPositionRecord::BrokerPositionRecord(BrokerPositionRecord&& rv)
: stock(std::move(rv.stock)), number(rv.number) {
    rv.number = 0.0;
}

BrokerPositionRecord& BrokerPositionRecord::operator=(BrokerPositionRecord&& rv) {
    if (this != &rv) {
        stock = std::move(rv.stock);
        number = rv.number;
        rv.number = 0.0;
    }
    return *this;
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

vector<Parameter> OrderBrokerBase::position() noexcept {
    vector<Parameter> ret;

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

            Parameter pos;
            auto market = brk_pos["market"].get<string>();
            auto code = brk_pos["code"].get<string>();
            auto stock = getStock(fmt::format("{}{}", market, code));
            if (stock.isNull()) {
                // 策略的上下文可能并不包含该股，此时忽略
                HKU_DEBUG("Not found the stock: {}{}", market, code);
                continue;
            }

            pos.set<Stock>("stock", stock);
            pos.set<double>("num", brk_pos["number"].get<double>());
            pos.set<double>("cost", brk_pos["cost"].get<double>());  // 总成本
            if (brk_pos.contains("buy_frozen_num")) {
                pos.set<double>("buy_frozen_num", brk_pos["buy_frozen_num"].get<double>());
            } else {
                pos.set<double>("buy_frozen_num", 0.0);
            }

            if (brk_pos.contains("sell_frozon_num")) {
                pos.set<double>("sell_frozon_num", brk_pos["sell_frozon_num"].get<double>());
            } else {
                pos.set<double>("sell_frozon_num", 0.0);
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
