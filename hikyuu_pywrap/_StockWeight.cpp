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
    class_<StockWeight>("StockWeight", "权息记录", init<>())
      .def(init<const Datetime&>())
      .def(init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t, price_t>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("datetime", &StockWeight::datetime, "权息日期")
      .add_property("count_as_gift", &StockWeight::countAsGift, "每10股送X股")
      .add_property("count_for_sell", &StockWeight::countForSell, "每10股配X股")
      .add_property("price_for_sell", &StockWeight::priceForSell, "配股价")
      .add_property("bonus", &StockWeight::bonus, "每10股红利")
      .add_property("increasement", &StockWeight::increasement, "每10股转增X股")
      .add_property("total_count", &StockWeight::totalCount, "总股本（万股）")
      .add_property("free_count", &StockWeight::freeCount, "流通股（万股）")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<StockWeight>())
#endif
      ;

    StockWeightList::const_reference (StockWeightList::*weightList_at)(StockWeightList::size_type)
      const = &StockWeightList::at;
    class_<StockWeightList>("StockWeightList", "std::vector<StockWeight> 包装")
      .def("__iter__", iterator<StockWeightList>())
      .def("__len__", &StockWeightList::size)
      .def("get", weightList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<StockWeightList>())
#endif
      ;
}
