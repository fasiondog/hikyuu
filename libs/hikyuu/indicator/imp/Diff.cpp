/*
 * Diff.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "Diff.h"

namespace hku {

Diff::Diff(): IndicatorImp(1, 1) {

}

Diff::Diff(const Indicator& data): IndicatorImp(data, 1, 1) {
    size_t total = data.size();
    if (total <= discard()) {
        return;
    }

    for (size_t i = discard(); i < total; ++i) {
        _set(data[i] - data[i-1], i);
    }

}

Diff::~Diff() {

}

string Diff::name() const {
    return "DIFF";
}

IndicatorImpPtr Diff::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new Diff(ind));
}

Indicator HKU_API DIFF() {
    return Indicator(IndicatorImpPtr(new Diff()));
}

Indicator HKU_API DIFF(const Indicator& data) {
    return Indicator(IndicatorImpPtr(new Diff(data)));
}

} /* namespace hku */
