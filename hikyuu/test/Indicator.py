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
        self.assertTrue(abs(m[0] - 1) < 0.0001)
        self.assertTrue(abs(m[1] - 2) < 0.0001)
        self.assertTrue(abs(m[2] - 3) < 0.0001)
        self.assertTrue(abs(m[3] - 4) < 0.0001)

        b = toPriceList([1, 2, 3, 4])
        x = PRICELIST(b)
        m.get_imp()._calculate(m)
        m = m(x)
        self.assertEqual(len(m), 4)
        self.assertEqual(m.empty(), False)
        self.assertTrue(abs(m[0] - 2) < 0.0001)
        self.assertTrue(abs(m[1] - 3) < 0.0001)
        self.assertTrue(abs(m[2] - 4) < 0.0001)
        self.assertTrue(abs(m[3] - 5) < 0.0001)

    def test_operator(self):
        a = toPriceList([0, 1, 2, 3, 5])
        x1 = PRICELIST(a)
        a = toPriceList([1, 2, 3, 4, 5])
        x2 = PRICELIST(a)
        a = x1 + x2
        self.assertEqual(a[0], 1)
        self.assertEqual(a[1], 3)
        self.assertEqual(a[2], 5)
        self.assertEqual(a[3], 7)

        a = x1 + 1.1
        self.assertTrue(abs(a[0] - 1.1) < 0.0001)
        self.assertTrue(abs(a[1] - 2.1) < 0.0001)
        self.assertTrue(abs(a[2] - 3.1) < 0.0001)
        self.assertTrue(abs(a[3] - 4.1) < 0.0001)

        a = 2.1 + x1
        self.assertTrue(abs(a[0] - 2.1) < 0.0001)
        self.assertTrue(abs(a[1] - 3.1) < 0.0001)
        self.assertTrue(abs(a[2] - 4.1) < 0.0001)
        self.assertTrue(abs(a[3] - 5.1) < 0.0001)

        a = x2 - x1
        self.assertEqual(a[0], 1)
        self.assertEqual(a[1], 1)
        self.assertEqual(a[2], 1)
        self.assertEqual(a[3], 1)

        a = x1 - 1.5
        self.assertEqual(a[0], -1.5)
        self.assertEqual(a[1], -0.5)
        self.assertEqual(a[2], 0.5)
        self.assertEqual(a[3], 1.5)

        a = 1.5 - x1
        self.assertEqual(a[0], 1.5)
        self.assertEqual(a[1], 0.5)
        self.assertEqual(a[2], -0.5)
        self.assertEqual(a[3], -1.5)

        a = x1 * x2
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 6)
        self.assertEqual(a[3], 12)

        a = x1 * 2.0
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 4)
        self.assertEqual(a[3], 6)

        a = 2.0 * x1
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 4)
        self.assertEqual(a[3], 6)

        a = x2 / x1
        self.assertTrue(isinf(a[0]))
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 1.5)
        self.assertTrue(abs(a[3] - 4.0 / 3.0) < 0.0001)

        a = x1 / 0.5
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 2)
        self.assertEqual(a[2], 4)
        self.assertEqual(a[3], 6)

        a = 2. / x1
        self.assertTrue(isinf(a[0]))
        self.assertEqual(a[1], 2.0)
        self.assertEqual(a[2], 1.)
        self.assertTrue(abs(a[3] - 2.0/3.0) < 0.0001)

        a = x1 > x2
        self.assertEqual(a[0], 0 > 1)
        self.assertEqual(a[1], 1 > 2)
        self.assertEqual(a[2], 2 > 3)
        self.assertEqual(a[3], 3 > 4)
        self.assertEqual(a[4], 5 > 5)

        a = x2 > x1
        self.assertEqual(a[0], 0 < 1)
        self.assertEqual(a[1], 1 < 2)
        self.assertEqual(a[2], 2 < 3)
        self.assertEqual(a[3], 3 < 4)
        self.assertEqual(a[4], 5 < 5)

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

        self.assertTrue(abs(o[0] - 96.05) < 0.0001)
        self.assertTrue(abs(h[0] - 99.98) < 0.0001)
        self.assertTrue(abs(l[0] - 95.79) < 0.0001)
        self.assertTrue(abs(c[0] - 99.98) < 0.0001)
        self.assertTrue(abs(a[0] - 49.4) < 0.0001)
        self.assertTrue(abs(v[0] - 1260) < 0.0001)

        self.assertTrue(abs(o[1] - 104.3) < 0.0001)
        self.assertTrue(abs(h[1] - 104.39) < 0.0001)
        self.assertTrue(abs(l[1] - 99.98) < 0.0001)
        self.assertTrue(abs(c[1] - 104.39) < 0.0001)
        self.assertTrue(abs(a[1] - 8.4) < 0.0001)
        self.assertTrue(abs(v[1] - 197) < 0.0001)

    def test_MA(self):
        a = toPriceList([0, 1, 2, 3])
        x = PRICELIST(a)
        m = MA(x, 2)
        self.assertEqual(len(m), 4)
        self.assertEqual(m.discard, 0)
        self.assertTrue(abs(m[0] - 0.0) < 0.0001)
        self.assertTrue(abs(m[1] - 0.5) < 0.0001)
        self.assertTrue(abs(m[2] - 1.5) < 0.0001)
        self.assertTrue(abs(m[3] - 2.5) < 0.0001)

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
