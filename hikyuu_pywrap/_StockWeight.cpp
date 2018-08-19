/*
 * StockWeight.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

/*
 * _MarketInfo.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/StockWeight_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_StockWeight() {
    docstring_options doc_options(false, true, false);

    class_<StockWeight>("StockWeight", init<>())
            .def(init<const Datetime&>())
            .def(init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t, price_t>())
            .def(self_ns::str(self))
            .add_property("datetime", &StockWeight::datetime)
            .add_property("countAsGift", &StockWeight::countAsGift)
            .add_property("countForSell", &StockWeight::countForSell)
            .add_property("priceForSell", &StockWeight::priceForSell)
            .add_property("bonus", &StockWeight::bonus)
            .add_property("increasement", &StockWeight::increasement)
            .add_property("totalCount", &StockWeight::totalCount)
            .add_property("freeCount", &StockWeight::freeCount)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<StockWeight>())
#endif
            ;

    StockWeightList::const_reference (StockWeightList::*weightList_at)(StockWeightList::size_type) const = &StockWeightList::at;
    class_<StockWeightList>("StockWeightList")
            .def("__iter__", iterator<StockWeightList>())
            .def("size", &StockWeightList::size)
            .def("__len__", &StockWeightList::size)
            .def("__getitem__", weightList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<StockWeightList>())
#endif
            ;

}


