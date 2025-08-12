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
        size_t total = sw.size();
        HKU_IF_RETURN(total == 0, py::array());

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

        // 使用 malloc 分配内存
        RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, len = sw.size(); i < len; i++) {
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

        // 使用 capsule 管理内存
        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("weights_to_df", [](const StockWeightList& sw) {
        size_t total = sw.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        // 创建数组
        py::array_t<int64_t> datetime_arr(total);
        py::array_t<double> countAsGift_arr(total);
        py::array_t<double> countForSell_arr(total);
        py::array_t<double> priceForSell_arr(total);
        py::array_t<double> bonus_arr(total);
        py::array_t<double> countOfIncreasement_arr(total);
        py::array_t<double> totalCount_arr(total);
        py::array_t<double> freeCount_arr(total);
        py::array_t<double> suogu_arr(total);

        // 获取缓冲区并填充数据
        auto datetime_buf = datetime_arr.request();
        auto countAsGift_buf = countAsGift_arr.request();
        auto countForSell_buf = countForSell_arr.request();
        auto priceForSell_buf = priceForSell_arr.request();
        auto bonus_buf = bonus_arr.request();
        auto countOfIncreasement_buf = countOfIncreasement_arr.request();
        auto totalCount_buf = totalCount_arr.request();
        auto freeCount_buf = freeCount_arr.request();
        auto suogu_buf = suogu_arr.request();

        int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
        double* countAsGift_ptr = static_cast<double*>(countAsGift_buf.ptr);
        double* countForSell_ptr = static_cast<double*>(countForSell_buf.ptr);
        double* priceForSell_ptr = static_cast<double*>(priceForSell_buf.ptr);
        double* bonus_ptr = static_cast<double*>(bonus_buf.ptr);
        double* countOfIncreasement_ptr = static_cast<double*>(countOfIncreasement_buf.ptr);
        double* totalCount_ptr = static_cast<double*>(totalCount_buf.ptr);
        double* freeCount_ptr = static_cast<double*>(freeCount_buf.ptr);
        double* suogu_ptr = static_cast<double*>(suogu_buf.ptr);

        for (size_t i = 0; i < total; i++) {
            const StockWeight& w = sw[i];
            datetime_ptr[i] = w.datetime().timestamp() * 1000LL;
            countAsGift_ptr[i] = w.countAsGift();
            countForSell_ptr[i] = w.countForSell();
            priceForSell_ptr[i] = w.priceForSell();
            bonus_ptr[i] = w.bonus();
            countOfIncreasement_ptr[i] = w.increasement();
            totalCount_ptr[i] = w.totalCount();
            freeCount_ptr[i] = w.freeCount();
            suogu_ptr[i] = w.suogu();
        }

        // 构建 DataFrame
        py::dict columns;
        columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
        columns["countAsGift"] = countAsGift_arr;
        columns["countForSell"] = countForSell_arr;
        columns["priceForSell"] = priceForSell_arr;
        columns["bonus"] = bonus_arr;
        columns["countOfIncreasement"] = countOfIncreasement_arr;
        columns["totalCount"] = totalCount_arr;
        columns["freeCount"] = freeCount_arr;
        columns["suogu"] = suogu_arr;

        return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
    });
}
