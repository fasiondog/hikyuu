#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130220, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class KDataTest(unittest.TestCase):
    def test_null_kdata(self):
        k = KData()
        self.assertEqual(len(k), 0)
        self.assertEqual(k.empty(), True)
        self.assertEqual(k.start_pos, 0)
        self.assertEqual(k.end_pos, 0)
        self.assertEqual(k.last_pos, 0)
        stock = k.get_stock()
        self.assertEqual(stock.is_null(), True)

    def test_kdata(self):
        stock = sm["Sh000001"]
        q = Query(0, 10)
        k = stock.get_kdata(q)
        self.assertEqual(len(k), 10)
        self.assertEqual(k.empty(), False)
        self.assertEqual(k.start_pos, 0)
        self.assertEqual(k.end_pos, 10)
        self.assertEqual(k.last_pos, 9)
        self.assertEqual(k[0].datetime, Datetime(199012190000))
        self.assert_(abs(k[0].open - 96.05) < 0.0001)
        self.assert_(abs(k[0].high - 99.980) < 0.0001)
        self.assert_(abs(k[0].low - 95.79) < 0.0001)
        self.assert_(abs(k[0].close - 99.98) < 0.0001)
        self.assert_(abs(k[0].amount - 49.4) < 0.0001)
        self.assert_(abs(k[0].volume - 1260) < 0.0001)
        self.assert_(abs(k[1].open - 104.3) < 0.0001)
        self.assert_(abs(k[9].open - 127.61) < 0.0001)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        import pickle as pl
        filename = sm.tmpdir() + "/KData.plk"
        fh = open(filename, 'wb')
        kdata = sm['sh000001'].get_kdata(Query(10, 20))
        pl.dump(kdata, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(len(kdata), len(b))
        for i in range(len(kdata)):
            self.assertEqual(kdata[i], b[i])


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(KDataTest)