#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# 作者：fasiondog
# 历史：1）20120928, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class StockTest(unittest.TestCase):
    def test_stock(self):
        stock = sm["Sh000001"]
        self.assertEqual(stock.market, "SH")
        self.assertEqual(stock.code, "000001")
        self.assertEqual(stock.market_code, "SH000001")
        self.assertEqual(stock.name, u"上证指数")
        self.assertEqual(stock.type, 2)
        self.assertEqual(stock.start_datetime, Datetime(199012190000))
        self.assertEqual(stock.last_datetime, constant.null_datetime)
        self.assertEqual(stock.tick, 0.001)
        self.assertEqual(stock.tick_value, 0.001)
        self.assertEqual(stock.unit, 1.0)
        self.assertEqual(stock.precision, 3)
        self.assertEqual(stock.atom, 1)
        self.assertEqual(stock.min_trade_number, 1)
        self.assertEqual(stock.max_trade_number, 1000000)
        self.assertEqual(stock.get_count(), 5121)
        self.assertEqual(stock.get_count(Query.MIN), 682823)
        self.assertEqual(stock.get_krecord(0).datetime, Datetime(199012190000))
        self.assertEqual(stock.get_krecord(1, Query.MIN).datetime, Datetime(200001040932))

        s1 = sm['sh000001']
        s2 = sm['sh000001']
        self.assertTrue(s1 == s2)
        self.assertTrue(not (s1 != s2))

        s2 = sm['sz000001']
        self.assertTrue(not (s1 == s2))
        self.assertTrue(s1 != s2)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        import pickle as pl
        filename = sm.tmpdir() + '/Stock.plk'
        fh = open(filename, 'wb')
        stock = sm['sh000001']
        pl.dump(stock, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(stock.name, b.name)
        self.assertEqual(b.market_code, 'SH000001')

    def test_external_stock(self):
        stk = Stock("xy", "00z", "测试")
        self.assertEqual(stk.name, "测试")
        self.assertEqual(stk.market, "XY")
        self.assertEqual(stk.code, "00z")
        self.assertEqual(stk.market_code, "XY00z")

        stk.market = "aB"
        self.assertEqual(stk.market, "AB")
        self.assertEqual(stk.market_code, "AB00z")

        stk.code = "000001"
        self.assertEqual(stk.code, "000001")
        self.assertEqual(stk.market_code, "AB000001")

        stk.name = "test"
        self.assertEqual(stk.name, "test")

        self.assertEqual(stk.valid, False)
        stk.valid = True
        self.assertTrue(stk.valid)

        self.assertNotEqual(stk.type, constant.STOCKTYPE_A)
        stk.type = constant.STOCKTYPE_A
        self.assertEqual(stk.type, constant.STOCKTYPE_A)

        self.assertNotEqual(stk.tick, 1)
        stk.tick = 1
        self.assertEqual(stk.tick, 1)

        self.assertNotEqual(stk.tick_value, 2)
        stk.tick_value = 2
        self.assertEqual(stk.tick_value, 2)
        self.assertEqual(stk.unit, 2)

        self.assertNotEqual(stk.atom, 2)
        stk.atom = 2
        self.assertEqual(stk.atom, 2)

        self.assertNotEqual(stk.min_trade_number, 1)
        stk.min_trade_number = 1
        self.assertEqual(stk.min_trade_number, 1)

        self.assertNotEqual(stk.max_trade_number, 10000)
        stk.max_trade_number = 10000
        self.assertEqual(stk.max_trade_number, 10000)

        ks = [KRecord(Datetime(20010101), 5.0, 9.0, 4.0, 6.5, 1000.0, 100000.0)]
        stk.set_krecord_list(ks)
        self.assertEqual(stk.get_count(), 1)
        k = stk.get_kdata(Query(0))
        self.assertEqual(len(k), 1)
        self.assertEqual(k[0], KRecord(Datetime(20010101), 5.0, 9.0, 4.0, 6.5, 1000.0, 100000.0))

        self.assertTrue(stk not in sm)
        sm.add_stock(stk)
        self.assertTrue(stk in sm)
        stk2 = sm['ab000001']
        self.assertTrue(not stk2.is_null())
        self.assertTrue(stk2, stk)
        sm.remove_stock("ab000001")
        self.assertTrue(stk not in sm)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(StockTest)
