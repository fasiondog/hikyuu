#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20120927, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class StockTypeInfoTest(unittest.TestCase):
    def test_stockType(self):
        stockType = sm.get_stock_type_info(1)
        self.assertEqual(stockType.type, 1)
        self.assertEqual(stockType.description, u"A股")
        self.assertEqual(stockType.tick, 0.01)
        self.assertEqual(stockType.tick_value, 0.01)
        self.assertEqual(stockType.unit, 1.0)
        self.assertEqual(stockType.precision, 2)
        self.assertEqual(stockType.min_trade_num, 100)
        self.assertEqual(stockType.max_trade_num, 1000000)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        #TODO: Python3 出错，暂未解决
        """import pickle as pl
        filename = sm.tmpdir() + '/StockTypeInfo.plk'
        a = sm.getStockTypeInfo(1)
        fh = open(filename, 'wb')
        pl.dump(a, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(a.type, b.type)
        self.assertEqual(a.description, b.description)
        """


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(StockTypeInfoTest)