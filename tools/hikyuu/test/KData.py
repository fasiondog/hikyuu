#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130220, Added by fasiondog
#===============================================================================

import unittest

from test_init import *
from hikyuu.trade_sys import *

class KDataTest(unittest.TestCase):
    def test_null_kdata(self):
        k = KData()
        self.assertEqual(k.size(), 0)
        self.assertEqual(k.empty(), True)
        self.assertEqual(k.startPos, 0)
        self.assertEqual(k.endPos, 0)
        self.assertEqual(k.lastPos, 0)
        stock = k.getStock()
        self.assertEqual(stock.isNull(), True)
        
    def test_kdata(self):
        stock = sm["Sh000001"]
        q = KQuery(0, 10)
        k = stock.getKData(q)
        self.assertEqual(k.size(), 10)
        self.assertEqual(k.empty(), False)
        self.assertEqual(k.startPos, 0)
        self.assertEqual(k.endPos, 10)
        self.assertEqual(k.lastPos, 9)
        self.assertEqual(k[0].datetime, Datetime(199012190000))
        self.assert_(abs(k[0].openPrice - 96.05) < 0.0001)
        self.assert_(abs(k[0].highPrice - 99.980) < 0.0001)
        self.assert_(abs(k[0].lowPrice - 95.79) < 0.0001)
        self.assert_(abs(k[0].closePrice - 99.98) < 0.0001)
        self.assert_(abs(k[0].transAmount - 49.4) < 0.0001)
        self.assert_(abs(k[0].transCount - 1260) < 0.0001)
        self.assert_(abs(k[1].openPrice - 104.3) < 0.0001)
        self.assert_(abs(k[9].openPrice - 127.61) < 0.0001)
        
    def test_pickle(self):
        if not constant.pickle_support:
            return
        
        import pickle as pl
        filename = sm.tmpdir() + "/KData.plk"
        fh = open(filename, 'wb')
        kdata = sm['sh000001'].getKData(KQuery(10, 20))
        pl.dump(kdata, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(kdata.size(), b.size())
        for i in range(len(kdata)):
            self.assertEqual(kdata[i], b[i])
        
                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(KDataTest)