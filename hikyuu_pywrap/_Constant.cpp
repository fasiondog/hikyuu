/*
 * _Constant.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include <hikyuu/DataType.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#ifdef STOCKTYPE_BLOCK
#undef STOCKTYPE_BLOCK
#endif

#ifdef STOCKTYPE_A
#undef STOCKTYPE_A
#endif

#ifdef STOCKTYPE_INDEX
#undef STOCKTYPE_INDEX
#endif

#ifdef STOCKTYPE_B
#undef STOCKTYPE_B
#endif

#ifdef STOCKTYPE_FUND
#undef STOCKTYPE_FUND
#endif

#ifdef STOCKTYPE_ETF
#undef STOCKTYPE_ETF
#endif

#ifdef STOCKTYPE_ND
#undef STOCKTYPE_ND
#endif

#ifdef STOCKTYPE_BOND
#undef STOCKTYPE_BOND
#endif

#ifdef STOCKTYPE_GEM
#undef STOCKTYPE_GEM
#endif

#ifdef STOCKTYPE_START
#undef STOCKTYPE_START
#endif

#ifdef STOCKTYPE_CRYPTO
#undef STOCKTYPE_CRYPTO
#endif

#ifdef STOCKTYPE_A_BJ
#undef STOCKTYPE_A_BJ
#endif

#ifdef STOCKTYPE_TMP
#undef STOCKTYPE_TMP
#endif

struct Constant {
    Constant()
    : null_datetime(Null<Datetime>()),
      inf(std::numeric_limits<double>::infinity()),
      infa(-std::numeric_limits<double>::infinity()),
      nan(std::numeric_limits<double>::quiet_NaN()),
      null_double(Null<double>()),
      max_double(std::numeric_limits<double>::max()),
      null_price(Null<price_t>()),
      null_int(Null<int>()),
      null_size(Null<size_t>()),
      null_int64(Null<int64_t>()),
      STOCKTYPE_BLOCK(0),
      STOCKTYPE_A(1),
      STOCKTYPE_INDEX(2),
      STOCKTYPE_B(3),
      STOCKTYPE_FUND(4),
      STOCKTYPE_ETF(5),
      STOCKTYPE_ND(6),
      STOCKTYPE_BOND(7),
      STOCKTYPE_GEM(8),
      STOCKTYPE_START(9),
      STOCKTYPE_CRYPTO(10),
      STOCKTYPE_A_BJ(11),
      STOCKTYPE_TMP(999) {
#if HKU_PYTHON_SUPPORT_PICKLE
        pickle_support = true;
#else
        pickle_support = false;
#endif /* HKU_PYTHON_SUPPORT_PICKLE */
    }

    Datetime null_datetime;
    double inf;
    double infa;  // The negative infinity
    double nan;
    double null_double;
    double max_double;
    double null_price;
    int null_int;
    size_t null_size;
    int64_t null_int64;
    bool pickle_support;  // Whether pickle is supported

    int STOCKTYPE_BLOCK;   /// Block
    int STOCKTYPE_A;       /// A-share
    int STOCKTYPE_INDEX;   /// Index
    int STOCKTYPE_B;       /// B-share
    int STOCKTYPE_FUND;    /// Fund
    int STOCKTYPE_ETF;     /// ETF
    int STOCKTYPE_ND;      /// Treasury bond
    int STOCKTYPE_BOND;    /// Bond
    int STOCKTYPE_GEM;     /// ChiNext
    int STOCKTYPE_START;   /// STAR Market
    int STOCKTYPE_CRYPTO;  /// Crypto
    int STOCKTYPE_A_BJ;    /// A-share of the Beijing Stock Exchange
    int STOCKTYPE_TMP;     /// Temporary Stock
};

void export_Constant(py::module& m) {
    py::class_<Constant>(m, "Constant")
      .def_readonly("null_datetime", &Constant::null_datetime, "An invalid Datetime")
      .def_readonly("inf", &Constant::inf, "The infinity")
      .def_readonly("infa", &Constant::infa, "The negative infinity")
      .def_readonly("nan", &Constant::nan, "Not a number")
      .def_readonly("null_double", &Constant::null_double, "The same as nan")
      .def_readonly("max_double", &Constant::max_double, "The maximum double value")
      .def_readonly("null_price", &Constant::null_price, "The same as nan")
      .def_readonly("null_int", &Constant::null_int, "An invalid int")
      .def_readonly("null_size", &Constant::null_size, "An invalid size")
      .def_readonly("null_int64", &Constant::null_int64, "An invalid int64_t")
      .def_readonly("pickle_support", &Constant::pickle_support, "Whether pickle is supported")

      .def_readonly("STOCKTYPE_BLOCK", &Constant::STOCKTYPE_BLOCK, "Block")
      .def_readonly("STOCKTYPE_A", &Constant::STOCKTYPE_A, "A-share")
      .def_readonly("STOCKTYPE_INDEX", &Constant::STOCKTYPE_INDEX, "Index")
      .def_readonly("STOCKTYPE_B", &Constant::STOCKTYPE_B, "B-share")
      .def_readonly("STOCKTYPE_FUND", &Constant::STOCKTYPE_FUND, "Fund")
      .def_readonly("STOCKTYPE_ETF", &Constant::STOCKTYPE_ETF, "ETF")
      .def_readonly("STOCKTYPE_ND", &Constant::STOCKTYPE_ND, "Treasury bond")
      .def_readonly("STOCKTYPE_BOND", &Constant::STOCKTYPE_BOND, "Bond")
      .def_readonly("STOCKTYPE_GEM", &Constant::STOCKTYPE_GEM, "ChiNext")
      .def_readonly("STOCKTYPE_START", &Constant::STOCKTYPE_START, "STAR Market")
      .def_readonly("STOCKTYPE_CRYPTO", &Constant::STOCKTYPE_START, "Crypto")
      .def_readonly("STOCKTYPE_A_BJ", &Constant::STOCKTYPE_A_BJ, "A-share of the Beijing Stock Exchange")
      .def_readonly("STOCKTYPE_TMP", &Constant::STOCKTYPE_TMP, "Temporary Stock");

    m.attr("constant") = Constant();
}
