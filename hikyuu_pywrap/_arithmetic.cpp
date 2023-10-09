/*
 * _util.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/utilities/arithmetic.h>

using namespace boost::python;
using namespace hku;

namespace py = boost::python;

static py::list combinate_index(object seq) {
    size_t total = len(seq);
    std::vector<size_t> index_list(total);
    for (size_t i = 0; i < total; ++i) {
        index_list[i] = i;
    }

    py::list result;
    std::vector<std::vector<size_t>> comb = combinateIndex(index_list);
    for (size_t i = 0, count = comb.size(); i < count; i++) {
        py::list tmp = vector_to_py_list<std::vector<size_t>>(comb[i]);
        result.append(tmp);
    }
    return result;
}

void export_util() {
    def("roundEx", roundEx, (arg("number"), arg("ndigits") = 0),
        R"(roundEx(number[, ndigits=0])

    四舍五入，ROUND_HALF_EVEN 银行家舍入法

    :param float number  待四舍五入的数据
    :param int ndigits 保留小数位数
    :rype: float)");

    def("roundUp", roundUp, (arg("number"), arg("ndigits") = 0), R"(roundUp(number[, ndigits=0])

    向上截取，如10.1截取后为11

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");

    def("roundDown", roundDown, (arg("number"), arg("ndigits") = 0),
        R"(roundDown(number[, ndigits=0])

    向下截取，如10.1截取后为10

    :param float number  待处理数据
    :param int ndigits 保留小数位数
    :rtype: float)");

    def("combinate_index", combinate_index, R"(combinate_index(seq)

    获取序列组合的下标索引, 输入序列的长度最大不超过15，否则抛出异常

    :param seq: 可迭代的 python 对象
    :rtype: list

    )");
}
