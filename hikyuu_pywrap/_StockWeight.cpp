/*
 * StockWeight.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/StockWeight_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_StockWeight(py::module& m) {
    py::class_<StockWeight>(m, "StockWeight", "权息记录")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t, price_t,
                    price_t>())

      .def("__str__", to_py_str<StockWeight>)
      .def("__repr__", to_py_str<StockWeight>)

      .def_property_readonly("datetime", &StockWeight::datetime, "权息日期")
      .def_property_readonly("count_as_gift", &StockWeight::countAsGift, "每10股送X股")
      .def_property_readonly("count_for_sell", &StockWeight::countForSell, "每10股配X股")
      .def_property_readonly("price_for_sell", &StockWeight::priceForSell, "配股价")
      .def_property_readonly("bonus", &StockWeight::bonus, "每10股红利")
      .def_property_readonly("increasement", &StockWeight::increasement, "每10股转增X股")
      .def_property_readonly("total_count", &StockWeight::totalCount, "总股本（万股）")
      .def_property_readonly("free_count", &StockWeight::freeCount, "流通股（万股）")
      .def_property_readonly("suogu", &StockWeight::suogu, "扩缩股比例")

        DEF_PICKLE(StockWeight);

    m.def("weights_to_np", [](const StockWeightList& sw) {
        struct RawData {
            int64_t date;  // 日期(仅到天)
            double countAsGift;
            double countForSell;
            double priceForSell;
            double bonus;
            double countOfIncreasement;
            double totalCount;
            double freeCount;
            double suogu;
        };
        std::vector<RawData> data;
        data.resize(sw.size());

        for (size_t i = 0, total = sw.size(); i < total; i++) {
            const StockWeight& w = sw[i];
            data[i].date = (w.datetime() - Datetime(1970, 1, 1)).days();
            data[i].countAsGift = w.countAsGift();
            data[i].countForSell = w.countForSell();
            data[i].priceForSell = w.priceForSell();
            data[i].bonus = w.bonus();
            data[i].countOfIncreasement = w.increasement();
            data[i].totalCount = w.totalCount();
            data[i].freeCount = w.freeCount();
            data[i].suogu = w.suogu();
        }

        py::dtype dtype = py::dtype(
          vector_to_python_list<string>({"date", "countAsGift", "countForSell", "priceForSell",
                                         "bonus", "countOfIncreasement", "totalCount", "freeCount",
                                         "suogu"}),
          vector_to_python_list<string>({"datetime64[D]", "d", "d", "d", "d", "d", "d", "d", "d"}),
          vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48, 56, 64}), 72);
        return py::array(dtype, data.size(), data.data());
    });

    m.def("weights_to_df", [](const StockWeightList& sw) {
        size_t total = sw.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        std::vector<int64_t> datetime(total);
        std::vector<double> countAsGift(total), countForSell(total), priceForSell(total),
          bonus(total), countOfIncreasement(total), totalCount(total), freeCount(total),
          suogu(total);

        for (size_t i = 0; i < total; i++) {
            const StockWeight& w = sw[i];
            datetime[i] = w.datetime().timestamp() * 1000LL;
            countAsGift[i] = w.countAsGift();
            countForSell[i] = w.countForSell();
            priceForSell[i] = w.priceForSell();
            bonus[i] = w.bonus();
            countOfIncreasement[i] = w.increasement();
            totalCount[i] = w.totalCount();
            freeCount[i] = w.freeCount();
            suogu[i] = w.suogu();
        }

        py::dict columns;
        columns["datetime"] =
          py::array_t<int64_t>(total, datetime.data()).attr("astype")("datetime64[ns]");
        columns["countAsGift"] =
          py::array_t<double>(total, countAsGift.data(), py::dtype("float64"));
        columns["countForSell"] =
          py::array_t<double>(total, countForSell.data(), py::dtype("float64"));
        columns["priceForSell"] =
          py::array_t<double>(total, priceForSell.data(), py::dtype("float64"));
        columns["bonus"] = py::array_t<double>(total, bonus.data(), py::dtype("float64"));
        columns["countOfIncreasement"] =
          py::array_t<double>(total, countOfIncreasement.data(), py::dtype("float64"));
        columns["totalCount"] = py::array_t<double>(total, totalCount.data(), py::dtype("float64"));
        columns["freeCount"] = py::array_t<double>(total, freeCount.data(), py::dtype("float64"));
        columns["suogu"] = py::array_t<double>(total, suogu.data(), py::dtype("float64"));

        return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
    });
}
