/*
 * test_IndicatorImp_hasPythonPart.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-22
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_has_python_part test_indicator_has_python_part
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

namespace {

class PyLikeImp : public IndicatorImp {
public:
    PyLikeImp() : IndicatorImp("PyLikeImp") {
        m_is_python_object = true;
    }

    virtual IndicatorImpPtr _clone() override {
        return std::make_shared<PyLikeImp>();
    }
};

Indicator pyIndicator() {
    return Indicator(std::make_shared<PyLikeImp>());
}

}  // namespace

/** @par Test points */
TEST_CASE("test_IndicatorImp_hasPythonPart_leaf") {
    /** @arg a python implementation reports itself */
    Indicator p = pyIndicator();
    CHECK_UNARY(p.hasPythonPart());
    CHECK_UNARY(p.isPythonObject());

    /** @arg a plain C++ implementation reports nothing */
    Indicator c = CLOSE();
    CHECK_UNARY(!c.hasPythonPart());

    /** @arg an indicator without any implementation */
    Indicator empty;
    CHECK_UNARY(empty.empty());
    CHECK_UNARY(!empty.hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_IndicatorImp_hasPythonPart_operand") {
    Indicator p = pyIndicator();

    /** @arg the python implementation sits at the left of an arithmetic expression */
    Indicator left_mixed = p + CLOSE();
    CHECK_UNARY(left_mixed.hasPythonPart());

    /** @arg the python implementation sits at the right of an arithmetic expression */
    Indicator right_mixed = CLOSE() + p;
    CHECK_UNARY(right_mixed.hasPythonPart());

    /** @arg a deeply nested expression keeps the mark */
    Indicator deep = ((CLOSE() + p) * MA(CLOSE(), 5)) - REF(OPEN(), 1);
    CHECK_UNARY(deep.hasPythonPart());

    /** @arg the same shape without any python part */
    Indicator clean = ((CLOSE() + OPEN()) * MA(CLOSE(), 5)) - REF(OPEN(), 1);
    CHECK_UNARY(!clean.hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_IndicatorImp_hasPythonPart_dynamic_param") {
    Indicator p = pyIndicator();

    /** @arg an input indicator held as a dynamic parameter */
    Indicator wrapped = MA(p, 5);
    CHECK_UNARY(wrapped.hasPythonPart());
    Indicator delayed = REF(p, 1);
    CHECK_UNARY(delayed.hasPythonPart());

    /** @arg a host implementation with the python indicator registered by name */
    IndicatorImpPtr host = std::make_shared<IndicatorImp>("HostImp");
    host->setIndParam("param1", p);
    Indicator h(host);
    CHECK_UNARY(h.hasPythonPart());

    /** @arg the very same slot with a C++ indicator */
    IndicatorImpPtr clean_host = std::make_shared<IndicatorImp>("HostImp");
    clean_host->setIndParam("param1", CLOSE());
    Indicator ch(clean_host);
    CHECK_UNARY(!ch.hasPythonPart());

    /** @arg the python mark survives being nested twice */
    IndicatorImpPtr outer = std::make_shared<IndicatorImp>("HostImp");
    outer->setIndParam("param1", MA(p, 3));
    Indicator oh(outer);
    CHECK_UNARY(oh.hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_IndicatorImp_hasPythonPart_three_node") {
    Indicator p = pyIndicator();

    /** @arg the python implementation hangs on the IF condition node */
    IndicatorImpPtr node = std::make_shared<IndicatorImp>("HostImp");
    node->add_if(p.getImp(), CLOSE().getImp(), OPEN().getImp());
    Indicator iif(node);
    CHECK_UNARY(iif.hasPythonPart());

    /** @arg the same shape without any python part */
    IndicatorImpPtr clean_node = std::make_shared<IndicatorImp>("HostImp");
    clean_node->add_if(CLOSE().getImp(), OPEN().getImp(), HIGH().getImp());
    Indicator clean_iif(clean_node);
    CHECK_UNARY(!clean_iif.hasPythonPart());
}

/** @} */
