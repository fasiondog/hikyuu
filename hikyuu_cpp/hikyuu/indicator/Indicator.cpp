/*
 * Indicator.cpp
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "crt/CVAL.h"
#include "imp/IContext.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Indicator& indicator) {
    os << indicator.str();
    return os;
}

string Indicator::str() const {
    return m_imp ? m_imp->str() : "Indicator{}";
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> Indicator::toArrow() const noexcept {
    arrow::TimestampBuilder date_builder(arrow::timestamp(arrow::TimeUnit::NANO),
                                         arrow::default_memory_pool());
    vector<HKU_ARROW_PRICE_BUILDER> value_builders;

    size_t result_num = getResultNumber();
    for (size_t i = 0; i < result_num; i++) {
        value_builders.emplace_back();
    }

    size_t total = size();
    auto dates = getDatetimeList();
    if (!dates.empty()) {
        for (size_t i = 0; i < total; i++) {
            HKU_ARROW_RETURN_NOT_OK(date_builder.Append(dates[i].timestamp() * 1000LL));
        }
    }

    for (size_t r = 0; r < result_num; r++) {
        HKU_ARROW_RETURN_NOT_OK(value_builders[r].AppendValues(data(r), total));
    }

    std::vector<std::shared_ptr<arrow::Field>> fields;
    vector<std::shared_ptr<arrow::Array>> arrs;

    if (!dates.empty()) {
        fields.emplace_back(arrow::field("datetime", arrow::timestamp(arrow::TimeUnit::NANO)));
        auto date_arr = date_builder.Finish();
        HKU_ARROW_RETURN_NOT_OK2(date_arr);
        arrs.push_back(*date_arr);
    }

    for (size_t r = 0; r < result_num; r++) {
        fields.emplace_back(arrow::field(fmt::format("value{}", r + 1), HKU_ARROW_PRICE_FIELD));
        auto value_arr = value_builders[r].Finish();
        HKU_ARROW_RETURN_NOT_OK2(value_arr);
        arrs.push_back(*value_arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

[[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> Indicator::toArrowOnlyValue()
  const noexcept {
    vector<HKU_ARROW_PRICE_BUILDER> value_builders;

    size_t result_num = getResultNumber();
    for (size_t i = 0; i < result_num; i++) {
        value_builders.emplace_back();
    }

    size_t total = size();
    for (size_t r = 0; r < result_num; r++) {
        HKU_ARROW_RETURN_NOT_OK(value_builders[r].AppendValues(data(r), total));
    }

    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (size_t r = 0; r < result_num; r++) {
        fields.emplace_back(arrow::field(fmt::format("value{}", r + 1), HKU_ARROW_PRICE_FIELD));
    }

    vector<std::shared_ptr<arrow::Array>> arrs;
    for (size_t r = 0; r < result_num; r++) {
        auto value_arr = value_builders[r].Finish();
        HKU_ARROW_RETURN_NOT_OK2(value_arr);
        arrs.push_back(*value_arr);
    }

    auto schema = arrow::schema(fields);
    return arrow::Table::Make(schema, arrs);
}

Indicator::Indicator(const IndicatorImpPtr& imp) : m_imp(imp) {}

Indicator::Indicator(const Indicator& indicator) : m_imp(indicator.m_imp) {}

Indicator::Indicator(Indicator&& ind) noexcept : m_imp(std::move(ind.m_imp)) {}

Indicator::~Indicator() {}

string Indicator::formula() const {
    return m_imp ? m_imp->formula() : "Indicator";
}

Indicator Indicator::operator()(const KData& k) {
    Indicator result = clone();
    result.setContext(k);
    return result;
}

void Indicator::setContext(const Stock& stock, const KQuery& query) {
    if (m_imp)
        m_imp->setContext(stock, query);
}

void Indicator::setContext(const KData& k) {
    if (m_imp)
        m_imp->setContext(k);
}

KData Indicator::getContext() const {
    return m_imp ? m_imp->getContext() : KData();
}

bool Indicator::alike(const Indicator& other) const {
    HKU_IF_RETURN(m_imp == other.m_imp, true);
    return m_imp->alike(*other.m_imp);
}

bool Indicator::equal(const Indicator& other) const {
    HKU_IF_RETURN(this == &other || m_imp == other.m_imp, true);
    HKU_IF_RETURN(size() != other.size() || discard() != other.discard() ||
                    getResultNumber() != other.getResultNumber(),
                  false);

    for (size_t r = 0, result_num = getResultNumber(); r < result_num; r++) {
        auto const* d1 = this->data(r);
        auto const* d2 = other.data(r);
        for (size_t i = 0, total = size(); i < total; i++) {
            HKU_IF_RETURN((std::isnan(d1[i]) && !std::isnan(d2[i])) ||
                            (!std::isnan(d1[i]) && std::isnan(d2[i])),
                          false);
            HKU_IF_RETURN(
              (!std::isnan(d1[i]) && !std::isnan(d2[i])) && (std::abs(d1[i] - d2[i]) >= 0.0001),
              false);
        }
    }
    return true;
}

Indicator& Indicator::operator=(const Indicator& indicator) {
    HKU_IF_RETURN(this == &indicator, *this);
    m_imp = indicator.m_imp;
    return *this;
}

Indicator& Indicator::operator=(Indicator&& indicator) {
    HKU_IF_RETURN(this == &indicator, *this);
    m_imp = std::move(indicator.m_imp);
    return *this;
}

PriceList Indicator::getResultAsPriceList(size_t num) const {
    HKU_WARN_IF_RETURN(!m_imp, PriceList(), "indicator imptr is null!");
    return m_imp->getResultAsPriceList(num);
}

Indicator Indicator::getResult(size_t num) const {
    HKU_WARN_IF_RETURN(!m_imp, Indicator(), "indicator imptr is null!");
    return m_imp->getResult(num);
}

Indicator Indicator::operator()(const Indicator& ind) {
    HKU_IF_RETURN(!m_imp, Indicator());
    HKU_IF_RETURN(!ind.getImp(), Indicator(m_imp));

    IndicatorImp const* context_ptr = dynamic_cast<IContext const*>(m_imp.get());
    if (context_ptr != nullptr) {
        auto p = make_shared<IContext>(ind);
        return p->calculate();
    }

    IndicatorImpPtr p = m_imp->clone();
    if (m_imp->alike(*ind.getImp())) {
        return Indicator(p);
    }
    p->add(IndicatorImp::OP, IndicatorImpPtr(), ind.getImp());
    return p->calculate();
}

HKU_API Indicator operator+(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::ADD, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator-(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::SUB, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator*(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::MUL, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator/(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::DIV, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator%(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::MOD, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator==(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::EQ, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator!=(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::NE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator>(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GT, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator<(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LT, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator>=(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator<=(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator&(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::AND, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator|(const Indicator& ind1, const Indicator& ind2) {
    HKU_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator());
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::OR, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator+(const Indicator& ind, Indicator::value_t val) {
    return ind + CVAL(ind, val);
}

HKU_API Indicator operator+(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) + ind;
}

HKU_API Indicator operator-(const Indicator& ind, Indicator::value_t val) {
    return ind - CVAL(ind, val);
}

HKU_API Indicator operator-(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) - ind;
}

HKU_API Indicator operator*(const Indicator& ind, Indicator::value_t val) {
    return ind * CVAL(ind, val);
}

HKU_API Indicator operator*(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) * ind;
}

HKU_API Indicator operator/(const Indicator& ind, Indicator::value_t val) {
    return ind / CVAL(ind, val);
}

HKU_API Indicator operator/(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) / ind;
}

HKU_API Indicator operator%(const Indicator& ind, Indicator::value_t val) {
    return ind % CVAL(ind, val);
}

HKU_API Indicator operator%(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) % ind;
}

HKU_API Indicator operator==(const Indicator& ind, Indicator::value_t val) {
    return ind == CVAL(ind, val);
}

HKU_API Indicator operator==(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) == ind;
}

HKU_API Indicator operator!=(const Indicator& ind, Indicator::value_t val) {
    return ind != CVAL(ind, val);
}

HKU_API Indicator operator!=(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) != ind;
}

HKU_API Indicator operator>(const Indicator& ind, Indicator::value_t val) {
    return ind > CVAL(ind, val);
}

HKU_API Indicator operator>(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) > ind;
}

HKU_API Indicator operator<(const Indicator& ind, Indicator::value_t val) {
    return ind < CVAL(ind, val);
}

HKU_API Indicator operator<(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) < ind;
}

HKU_API Indicator operator>=(const Indicator& ind, Indicator::value_t val) {
    return ind >= CVAL(ind, val);
}

HKU_API Indicator operator>=(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) >= ind;
}

HKU_API Indicator operator<=(const Indicator& ind, Indicator::value_t val) {
    return ind <= CVAL(ind, val);
}

HKU_API Indicator operator<=(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) <= ind;
}

HKU_API Indicator operator&(const Indicator& ind, Indicator::value_t val) {
    return ind & CVAL(ind, val);
}

HKU_API Indicator operator&(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) & ind;
}

HKU_API Indicator operator|(const Indicator& ind, Indicator::value_t val) {
    return ind | CVAL(ind, val);
}

HKU_API Indicator operator|(Indicator::value_t val, const Indicator& ind) {
    return CVAL(ind, val) | ind;
}

Indicator HKU_API WEAVE(const Indicator& ind1, const Indicator& ind2) {
    HKU_ERROR_IF_RETURN(!ind1.getImp() || !ind2.getImp(), Indicator(),
                        "ind1 or ind2 is Null Indicator!");
    IndicatorImpPtr p = make_shared<IndicatorImp>("WEAVE");
    p->add(IndicatorImp::WEAVE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

Indicator HKU_API IF(const Indicator& ind1, const Indicator& ind2, const Indicator& ind3) {
    HKU_ERROR_IF_RETURN(!ind1.getImp() || !ind2.getImp() || !ind3.getImp(), Indicator(),
                        "Exists null indicator!");
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add_if(ind1.getImp(), ind2.getImp(), ind3.getImp());
    return p->calculate();
}

Indicator HKU_API IF(const Indicator& x, Indicator::value_t a, const Indicator& b) {
    return IF(x, CVAL(b, a), b);
}

Indicator HKU_API IF(const Indicator& x, const Indicator& a, Indicator::value_t b) {
    return IF(x, a, CVAL(a, b));
}

Indicator HKU_API IF(const Indicator& x, Indicator::value_t a, Indicator::value_t b) {
    return IF(x, CVAL(x, a), CVAL(x, b));
}

} /* namespace hku */
