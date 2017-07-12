/*
 * IND_AND.cpp
 *
 *  Created on: 2017年7月13日
 *      Author: fasiondog
 */

#include "IND_LOGIC.h"

namespace hku {

HKU_API Indicator IND_AND(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind1.get(i, r) >= IND_EQ_THRESHOLD
                    && ind2.get(i, r) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator IND_AND(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    if (val >= IND_EQ_THRESHOLD) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (ind.get(i, r) >= IND_EQ_THRESHOLD) {
                    imp->_set(1, i, r);
                } else {
                    imp->_set(0, i, r);
                }
            }
        }

    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator IND_AND(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    if (val >= IND_EQ_THRESHOLD) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (ind.get(i, r) >= IND_EQ_THRESHOLD) {
                    imp->_set(1, i, r);
                } else {
                    imp->_set(0, i, r);
                }
            }
        }

    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator IND_OR(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind1.get(i, r) >= IND_EQ_THRESHOLD
                    || ind2.get(i, r) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator IND_OR(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    if (val >= IND_EQ_THRESHOLD) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                imp->_set(1, i, r);
            }
        }

    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (ind.get(i, r) >= IND_EQ_THRESHOLD) {
                    imp->_set(1, i, r);
                } else {
                    imp->_set(0, i, r);
                }
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator IND_OR(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    if (val >= IND_EQ_THRESHOLD) {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                imp->_set(1, i, r);
            }
        }

    } else {
        for (size_t i = discard; i < total; ++i) {
            for (size_t r = 0; r < result_number; ++r) {
                if (ind.get(i, r) >= IND_EQ_THRESHOLD) {
                    imp->_set(1, i, r);
                } else {
                    imp->_set(0, i, r);
                }
            }
        }
    }

    return Indicator(imp);
}

} /* namespace */


