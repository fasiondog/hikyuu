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
void export_TradeManager();
void export_Performance();
void export_OrderBroker();

void export_trade_manage_main() {
    export_CostRecord();
    export_PositionRecord();
    export_TradeCost();
    export_TradeRecord();
    export_FundsRecord();
    export_OrderBroker();
    export_TradeManager();
    export_Performance();
    export_build_in();
}
