/*
 * trade_manage_main.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <boost/python.hpp>

using namespace boost::python;

void export_CostRecord();
void export_TradeCost();
void export_build_in();
void export_PositionRecord();
void export_TradeRecord();
void export_FundsRecord();
void export_BorrowRecord();
void export_LoanRecord();
void export_TradeManager();
void export_Performance();
void export_OrderBroker();

BOOST_PYTHON_MODULE(_trade_manage) {
    docstring_options doc_options(false);
    export_CostRecord();
    export_PositionRecord();
    export_TradeCost();
    export_TradeRecord();
    export_FundsRecord();
    export_BorrowRecord();
    export_LoanRecord();
    export_OrderBroker();
    export_TradeManager();
    export_Performance();
    export_build_in();
}

