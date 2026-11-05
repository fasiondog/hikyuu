/*
 * ByTypeMargin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include <algorithm>
#include <cctype>
#include <cmath>
#include "ByTypeMargin.h"
#include "hikyuu/part/PartRegister.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/arithmetic.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ByTypeMargin)
#endif

namespace hku {

namespace {
/** Whether every non-blank character of the key is a digit, i.e. the key is a security type */
bool isDigitKey(const string& key) {
    bool has_digit = false;
    for (char c : key) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            has_digit = true;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(c)))
            continue;
        return false;
    }
    return has_digit;
}
}  // namespace

ByTypeMargin::ByTypeMargin() : MarginBase("MG_ByType") {
    setParam<string>("rates", "");
    setParam<double>("maintain_rate", 0.0);
    setParam<double>("default_rate", 0.6);
}

ByTypeMargin::~ByTypeMargin() {}

void ByTypeMargin::parseRates() const {
    const string& src = getParam<string>("rates");
    if (src == m_parsed_src)
        return;

    // Parse into locals and commit only when the whole expression is valid, so a rejected string
    // never leaves a partially filled table behind
    std::unordered_map<uint32_t, double> type_rates;
    std::unordered_map<string, double> market_rates;

    size_t pos = 0;
    while (pos < src.size()) {
        size_t end = src.find_first_of(",; \t\r\n", pos);
        end = end == string::npos ? src.size() : end;
        string item = src.substr(pos, end - pos);
        pos = end + 1;
        trim(item);
        if (item.empty())
            continue;

        size_t sep = item.find(':');
        HKU_CHECK(sep != string::npos && sep != 0 && sep + 1 < item.size(),
                  "MG_ByType: invalid rate item \"{}\", expected \"key:ratio\"!", item);

        string key = item.substr(0, sep);
        string value = item.substr(sep + 1);
        trim(key);
        trim(value);

        double rate = 0.0;
        size_t used = 0;
        try {
            rate = std::stod(value, &used);
        } catch (...) {
            HKU_THROW("MG_ByType: invalid ratio \"{}\" of key \"{}\"!", value, key);
        }
        HKU_CHECK(used == value.size(), "MG_ByType: invalid ratio \"{}\" of key \"{}\"!", value,
                  key);
        HKU_CHECK(std::isfinite(rate) && rate > 0.0 && rate <= 1.0,
                  "MG_ByType: ratio({}) of key \"{}\" must be in (0, 1]!", rate, key);

        if (isDigitKey(key)) {
            uint32_t type = 0;
            try {
                type = static_cast<uint32_t>(std::stoul(key));
            } catch (...) {
                HKU_THROW("MG_ByType: invalid type key \"{}\"!", key);
            }
            if (type_rates.count(type) > 0)
                HKU_WARN("MG_ByType: duplicated type key \"{}\", the last one wins!", key);
            type_rates[type] = rate;
        } else {
            to_upper(key);
            if (market_rates.count(key) > 0)
                HKU_WARN("MG_ByType: duplicated market key \"{}\", the last one wins!", key);
            market_rates[key] = rate;
        }
    }

    m_type_rates.swap(type_rates);
    m_market_rates.swap(market_rates);
    m_parsed_src = src;
}

void ByTypeMargin::_checkParam(const string& name) const {
    double maintain_rate = tryGetParam<double>("maintain_rate", 0.0);
    double default_rate = tryGetParam<double>("default_rate", 0.6);

    HKU_CHECK(std::isfinite(default_rate) && default_rate > 0.0 && default_rate <= 1.0,
              "MG_ByType: default_rate({}) must be in (0, 1]!", default_rate);

    // Parse before validating the maintain ratio, so a rates change re-validates it against the
    // new table (parseRates is a no-op when the expression did not change)
    if (name == "rates")
        parseRates();

    if (maintain_rate > 0.0) {
        HKU_CHECK(maintain_rate <= 1.0, "MG_ByType: maintain_rate({}) must be <= 1!",
                  maintain_rate);
        // The maintenance margin must stay below every opening ratio, otherwise the account would
        // be called the moment a position is opened
        double min_open_rate = default_rate;
        for (const auto& kv : m_type_rates)
            min_open_rate = std::min(min_open_rate, kv.second);
        for (const auto& kv : m_market_rates)
            min_open_rate = std::min(min_open_rate, kv.second);
        HKU_CHECK(maintain_rate <= min_open_rate,
                  "MG_ByType: maintain_rate({}) must be <= the minimum opening rate({})!",
                  maintain_rate, min_open_rate);
    }
}

double ByTypeMargin::rateOf(const Stock& stock) const {
    auto type_iter = m_type_rates.find(stock.type());
    if (type_iter != m_type_rates.end())
        return type_iter->second;

    string market(stock.market());
    to_upper(market);
    auto market_iter = m_market_rates.find(market);
    if (market_iter != m_market_rates.end())
        return market_iter->second;

    return getParam<double>("default_rate");
}

bool ByTypeMargin::isMarginTrading(const Stock& stock) const {
    // A position of a security matched by no key is a plain cash position only when the fallback
    // ratio is 1.0, otherwise it occupies the margin of that fallback ratio
    if (m_type_rates.count(stock.type()) > 0)
        return true;

    string market(stock.market());
    to_upper(market);
    if (m_market_rates.count(market) > 0)
        return true;

    return getParam<double>("default_rate") < 1.0;
}

price_t ByTypeMargin::getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                    double num, MarginSide side) const {
    return rateOf(stock) * nominal(stock, price, num);
}

double ByTypeMargin::getMarginRate(const Datetime& datetime, const Stock& stock,
                                   MarginSide side) const {
    return rateOf(stock);
}

price_t ByTypeMargin::getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                        double num, MarginSide side) const {
    price_t open = getOpenMargin(datetime, stock, price, num, side);
    double maintain_rate = getParam<double>("maintain_rate");
    if (maintain_rate <= 0.0)
        return open;
    return maintain_rate / rateOf(stock) * open;
}

MarginPtr ByTypeMargin::_clone() {
    return make_shared<ByTypeMargin>();
}

MarginPtr HKU_API MG_ByType(const string& rates, double maintain_rate, double default_rate) {
    auto margin = make_shared<ByTypeMargin>();
    // default_rate is set first because the maintain_rate check compares against it
    margin->setParam<double>("default_rate", default_rate);
    margin->setParam<double>("maintain_rate", maintain_rate);
    margin->setParam<string>("rates", rates);
    return margin;
}

void registerPart_MG_ByType() {
    auto& reg = PartRegistry::instance();
    HKU_IF_RETURN(reg.has("core.mg.by_type"), void());
    PartMeta meta;
    meta.type = PartType::MG;
    meta.name = "MG_ByType";
    meta.full_name = "core.mg.by_type";
    meta.params.push_back(ParamMeta{.name = "rates", .type = "string", .default_value = ""});
    meta.params.push_back(
      ParamMeta{.name = "maintain_rate", .type = "double", .default_value = "0.0"});
    meta.params.push_back(
      ParamMeta{.name = "default_rate", .type = "double", .default_value = "0.6"});
    reg.registerPart(meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
        // An empty rates string has no textual default to inject, so it is read defensively
        return MG_ByType(sc.tryGet<string>("rates", ""), sc.get<double>("maintain_rate"),
                         sc.get<double>("default_rate"));
    });
}

} /* namespace hku */
