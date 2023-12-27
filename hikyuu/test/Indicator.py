#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# 作者：fasiondog
# 历史：1）20130220, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class AddIndicator(IndicatorImp):
    def __init__(self, indicator):
        super(AddIndicator, self).__init__("AddIndicator")
        self._ready_buffer(len(indicator), 1)
        self.set_discard(0)
        for i in range(len(indicator)):
            self._set(indicator[i] + 1, i)

    def _clone(self):
        return AddIndicator(Indicator())

    def _calculate(self, ind):
        self.set_discard(0)
        for i in range(len(ind)):
            self._set(ind[i] + 1, i)


class IndicatorTest(unittest.TestCase):
    def test_PRICELIST(self):
        a = toPriceList([0, 1, 2, 3])
        x = PRICELIST(a)
        self.assertEqual(len(x), 4)
        self.assertEqual(x.empty(), False)
        self.assertEqual(x.discard, 0)
        self.assertEqual(x[0], 0)
        self.assertEqual(x[1], 1)
        self.assertEqual(x[2], 2)
        self.assertEqual(x[3], 3)

    def test_PythonIndicator(self):
        a = toPriceList([0, 1, 2, 3])
        x = PRICELIST(a)
        m = Indicator(AddIndicator(x))
        self.assertEqual(m.name, "AddIndicator")
        self.assertEqual(len(m), 4)
        self.assertEqual(m.empty(), False)
        self.assert_(abs(m[0] - 1) < 0.0001)
        self.assert_(abs(m[1] - 2) < 0.0001)
        self.assert_(abs(m[2] - 3) < 0.0001)
        self.assert_(abs(m[3] - 4) < 0.0001)

        b = toPriceList([1, 2, 3, 4])
        x = PRICELIST(b)
        m.get_imp()._calculate(m)
        m = m(x)
        self.assertEqual(len(m), 4)
        self.assertEqual(m.empty(), False)
        self.assert_(abs(m[0] - 2) < 0.0001)
        self.assert_(abs(m[1] - 3) < 0.0001)
        self.assert_(abs(m[2] - 4) < 0.0001)
        self.assert_(abs(m[3] - 5) < 0.0001)

    def test_operator(self):
        a = toPriceList([0, 1, 2, 3])
        x1 = PRICELIST(a)
        a = toPriceList([1, 2, 3, 4])
        x2 = PRICELIST(a)
        a = x1 + x2
        self.assertEqual(a[0], 1)
        self.assertEqual(a[1], 3)
        self.assertEqual(a[2], 5)
        self.assertEqual(a[3], 7)

        a = x2 - x1
        self.assertEqual(a[0], 1)
        self.assertEqual(a[1], 1)
        self.assertEqual(a[2], 1)
        self.assertEqual(a[3], 1)

        a = x1 * x2
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 6)
        self.assertEqual(a[3], 12)

        a = x2 / x1
        self.assert_(isnan(a[0]))
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 1.5)
        self.assertEqual(a[3], 4.0 / 3.0)

    def test_IKDATA(self):
        s = sm['sh000001']
        q = Query(0, 10)
        k = s.get_kdata(q)
        o = OPEN(k)
        h = HIGH(k)
        l = LOW(k)
        c = CLOSE(k)
        a = AMO(k)
        v = VOL(k)

        self.assertEqual(len(o), 10)
        self.assertEqual(len(h), 10)
        self.assertEqual(len(l), 10)
        self.assertEqual(len(c), 10)
        self.assertEqual(len(a), 10)
        self.assertEqual(len(v), 10)

        self.assertEqual(o.empty(), False)
        self.assertEqual(h.empty(), False)
        self.assertEqual(l.empty(), False)
        self.assertEqual(c.empty(), False)
        self.assertEqual(a.empty(), False)
        self.assertEqual(v.empty(), False)

        self.assert_(abs(o[0] - 96.05) < 0.0001)
        self.assert_(abs(h[0] - 99.98) < 0.0001)
        self.assert_(abs(l[0] - 95.79) < 0.0001)
        self.assert_(abs(c[0] - 99.98) < 0.0001)
        self.assert_(abs(a[0] - 49.4) < 0.0001)
        self.assert_(abs(v[0] - 1260) < 0.0001)

        self.assert_(abs(o[1] - 104.3) < 0.0001)
        self.assert_(abs(h[1] - 104.39) < 0.0001)
        self.assert_(abs(l[1] - 99.98) < 0.0001)
        self.assert_(abs(c[1] - 104.39) < 0.0001)
        self.assert_(abs(a[1] - 8.4) < 0.0001)
        self.assert_(abs(v[1] - 197) < 0.0001)

    def test_MA(self):
        a = toPriceList([0, 1, 2, 3])
        x = PRICELIST(a)
        m = MA(x, 2)
        self.assertEqual(len(m), 4)
        self.assertEqual(m.discard, 0)
        self.assert_(abs(m[0] - 0.0) < 0.0001)
        self.assert_(abs(m[1] - 0.5) < 0.0001)
        self.assert_(abs(m[2] - 1.5) < 0.0001)
        self.assert_(abs(m[3] - 2.5) < 0.0001)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        # TODO Python3出错，暂未解决
        """
        import pickle as pl
        filename = sm.tmpdir() + '/Indicator.plk'
        stock = sm['sh000001']
        kdata = stock.getKData(Query(0,10))
        a = CLOSE(kdata)
        fh = open(filename, 'wb')
        pl.dump(a, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(a.size(), b.size())
        self.assertNotEqual(a.size(), 0)
        for i in range(len(a)):
            self.assertEqual(a[i], b[i])
        """


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(IndicatorTest)
