#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# Author: fasiondog
# History: 1)20120927, Added by fasiondog
# ===============================================================================

import unittest

import Datetime
import Parameter
import DataType
import MarketInfo
import StockTypeInfo
import Stock
import KData
import Indicator
import TradeCost

import Environment
import Condition
import MoneyManager
import Signal
import Stoploss
import ProfitGoal
import Slippage
import SystemWeight
import AllocateFunds
import MultiSystem
import Strategy
import ConvertAny
import Pickle
import GilSafe
import Misc
import Block
import PositionRecord
import Selector
import MultiFactor
import ExtInd
import ExtraKType
import test_common_sql

if __name__ == "__main__":

    suite = unittest.TestSuite()
    suite.addTest(Datetime.suite())
    suite.addTest(Parameter.suite())
    suite.addTest(DataType.suite())

    suite.addTest(MarketInfo.suite())
    suite.addTest(StockTypeInfo.suite())
    suite.addTest(Stock.suite())
    suite.addTest(KData.suite())
    suite.addTest(Indicator.suite())
    suite.addTest(TradeCost.suite())

    suite.addTest(Environment.suite())
    suite.addTest(Environment.suiteTestCrtEV())
    suite.addTest(Condition.suite())
    suite.addTest(Condition.suiteTestCrtCN())
    suite.addTest(MoneyManager.suite())
    suite.addTest(MoneyManager.suiteTestCrtMM())
    suite.addTest(Signal.suite())
    suite.addTest(Signal.suiteTestCrtSG())
    suite.addTest(Signal.suiteTestCrtSGWithClone())

    suite.addTest(Stoploss.suite())
    suite.addTest(Stoploss.suiteTestCrtST())
    suite.addTest(ProfitGoal.suite())
    suite.addTest(ProfitGoal.suiteTestCrtPG())
    suite.addTest(Slippage.suite())
    suite.addTest(Slippage.suiteTestCrtSL())

    suite.addTest(SystemWeight.suite())
    suite.addTest(AllocateFunds.suite())
    suite.addTest(MultiSystem.suite())
    suite.addTest(Strategy.suite())
    suite.addTest(ConvertAny.suite())
    suite.addTest(Pickle.suite())
    suite.addTest(GilSafe.suite())
    suite.addTest(Selector.suite())
    suite.addTest(MultiFactor.suite())
    suite.addTest(ExtInd.suite())
    suite.addTest(ExtraKType.suite())
    suite.addTest(Misc.suite())
    suite.addTest(Block.suite())
    suite.addTest(PositionRecord.suite())
    suite.addTest(test_common_sql.suite())

    unittest.TextTestRunner(verbosity=2).run(suite)
    # unittest.main()
