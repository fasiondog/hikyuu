#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# 作者：fasiondog
# 历史：1）20120927, Added by fasiondog
# ===============================================================================

import unittest

import Datetime
import Parameter
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
import AllocateFunds

if __name__ == "__main__":

    suite = unittest.TestSuite()
    suite.addTest(Datetime.suite())
    suite.addTest(Parameter.suite())

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

    suite.addTest(AllocateFunds.suite())

    unittest.TextTestRunner(verbosity=2).run(suite)
    # unittest.main()
