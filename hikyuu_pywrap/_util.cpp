/*
 * _util.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/utilities/util.h>

using namespace boost::python;
using namespace hku;

BOOST_PYTHON_FUNCTION_OVERLOADS(roundEx_overload, roundEx, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(roundUp_overload, roundUp, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(roundDown_overload, roundDown, 1, 2);

void export_util() {
    def("roundEx", roundEx, roundEx_overload());
    def("roundUp", roundUp, roundUp_overload());
    def("roundDown", roundDown, roundDown_overload());
}
