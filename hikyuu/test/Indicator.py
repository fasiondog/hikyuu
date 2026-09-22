#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: 1)20130220, Added by fasiondog
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
        self.assertEqual(m.discard, 1)
        self.assertTrue(abs(m[1] - 0.5) < 0.0001)
        self.assertTrue(abs(m[2] - 1.5) < 0.0001)
        self.assertTrue(abs(m[3] - 2.5) < 0.0001)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        # TODO: fails on Python3, not resolved yet
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

    def test_BARSLASTS(self):
        data = [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0]
        ind = PRICELIST(data)

        # Test N=1, should be consistent with BARSLAST
        result1 = BARSLASTS(ind, 1)
        expected1 = BARSLAST(ind)

        # print(f"data: {data}")
        # print(f"BARSLASTS(ind, 1): {[result1[i] for i in range(len(result1))]}")
        # print(f"BARSLAST(ind):     {[expected1[i] for i in range(len(expected1))]}")

        # Verify that N=1 is consistent with BARSLAST
        for i in range(len(result1)):
            if np.isnan(expected1[i]):
                self.assertTrue(np.isnan(result1[i]))
            else:
                self.assertTrue(result1[i] == expected1[i])

        # print("✓ N=1 is consistent with BARSLAST")

        # Test N=2
        result2 = BARSLASTS(ind, 2)
        # print(f"BARSLASTS(ind, 2): {[result2[i] for i in range(len(result2))]}")
        # print(f"BARSLASTS(ind, 2) discard: {result2.discard}")

        # Verify the N=2 result
        # Positions 0-3: the condition holds only once, less than 2 times, should be NaN
        for i in range(4):
            self.assertTrue(np.isnan(result2[i]))

        # Position 4: the condition holds for the 2nd time, 4 bars from the 1st hold (position 0)
        self.assertTrue(result2[4] == 4)

        # Positions 5-7: the condition holds twice, 5, 6, 7 bars from the 1st hold (position 0)
        for i in range(5, 8):
            self.assertTrue(result2[i] == i)

        # Position 8: the condition holds for the 3rd time, 4 bars from the 2nd hold (position 4)
        self.assertTrue(result2[8] == 4)

        # print("✓ N=2 test passed")

        # Test N=3
        result3 = BARSLASTS(ind, 3)
        # print(f"BARSLASTS(ind, 3): {[result3[i] for i in range(len(result3))]}")

        # Verify the N=3 result
        # Positions 0-7: the condition holds only twice, less than 3 times, should be NaN
        for i in range(8):
            self.assertTrue(np.isnan(result3[i]))

        # Position 8: the condition holds for the 3rd time, 8 bars from the 1st hold (position 0)
        self.assertTrue(result3[8] == 8)

        # Positions 9-11: the condition holds 3 times, 9, 10, 11 bars from the 1st hold (position 0)
        for i in range(9, 12):
            self.assertTrue(result3[i] == i)

        """Test the edge cases"""
        # print("\nTesting the edge cases...")

        # Test N <= 0
        data = [1, 0, 1, 0, 1]
        ind = PRICELIST(data)

        result0 = BARSLASTS(ind, 0)
        # print(f"BARSLASTS(ind, 0): {[result0[i] for i in range(len(result0))]}")
        self.assertTrue(result0.discard == len(data))
        # print("✓ N=0 test passed")

        result_neg = BARSLASTS(ind, -1)
        # print(f"BARSLASTS(ind, -1): {[result_neg[i] for i in range(len(result_neg))]}")
        self.assertTrue(result_neg.discard == len(data))
        # print("✓ N=-1 test passed")

        # Test empty data
        empty_ind = PRICELIST([])
        result_empty = BARSLASTS(empty_ind, 1)
        # print(f"BARSLASTS(empty data, 1): length={len(result_empty)}")
        self.assertTrue(len(result_empty) == 0)
        # print("✓ empty data test passed")

        # Test all-zero data
        zeros = [0, 0, 0, 0, 0]
        ind_zeros = PRICELIST(zeros)
        result_zeros = BARSLASTS(ind_zeros, 1)
        # print(f"BARSLASTS(all-zero data, 1): discard={result_zeros.discard}")
        self.assertTrue(result_zeros.discard == len(zeros))
        # print("✓ all-zero data test passed")

        """Test the combination with K-line data"""
        # print("\nTesting the combination with K-line data...")

        # Get the stock data
        sm = StockManager.instance()
        stock = sm['sh000001']

        if stock.is_null():
            print("⚠ Unable to get stock data, skipping this test")
            return

        kdata = stock.get_kdata(Query(-100))

        if len(kdata) == 0:
            print("⚠ K-line data is empty, skipping this test")
            return

        # Test the rising close price
        close = CLOSE(kdata)
        up = close > REF(close, 1)

        # Calculate the days from the 1st rise to now
        result1 = BARSLASTS(up, 1)
        # print(f"BARSLASTS(rise, 1) first 5 values: {[result1[i] for i in range(min(5, len(result1)))]}")

        # Calculate the days from the 2nd rise to now
        result2 = BARSLASTS(up, 2)
        # print(f"BARSLASTS(rise, 2) first 5 values: {[result2[i] for i in range(min(5, len(result2)))]}")

        # Verify that N=1 is consistent with BARSLAST
        expected = BARSLAST(up)
        for i in range(min(10, len(result1))):
            if np.isnan(expected[i]):
                self.assertTrue(np.isnan(result1[i]))
            else:
                self.assertTrue(result1[i] == expected[i])

        # print("✓ K-line data test passed")

    def test_CODELIKE_NAMELIKE(self):
        """Test the CODELIKE and NAMELIKE indicators"""
        # Get the test stock
        stock = sm['sh000001']
        k = stock.get_kdata(Query(-10))
        
        # Test CODELIKE - exact match
        result = CODELIKE(k, "000001")
        self.assertEqual(len(result), len(k))
        # The code of sh000001 is 000001, it should match and return all 1
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        # Test CODELIKE - contains match (auto contains match when there is no wildcard)
        result = CODELIKE(k, "000")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        # Test CODELIKE - wildcard ? match
        result = CODELIKE(k, "??????")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        # Test CODELIKE - no match case
        result = CODELIKE(k, "01")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 0.0)
        
        # Test NAMELIKE - contains match (auto contains match when there is no wildcard)
        result = NAMELIKE(k, "上证")
        self.assertEqual(len(result), len(k))
        # The Chinese name of sh000001 contains the pattern above, it should match
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        # Test NAMELIKE - no match case
        result = NAMELIKE(k, "*non-existent name*")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 0.0)
        
        # Test a Shenzhen stock
        stock2 = sm['sz00001']
        if not stock2.is_null():
            k2 = stock2.get_kdata(Query(-10))
            
            # Test CODELIKE
            result = CODELIKE(k2, "000")
            self.assertEqual(len(result), len(k2))
            for i in range(len(result)):
                self.assertEqual(result[i], 1.0)

            result = CODELIKE(k2, "600")
            self.assertEqual(len(result), len(k2))
            for i in range(len(result)):
                self.assertEqual(result[i], 0.0)
        
        # Test NAMELIKE for a Shenzhen stock
        stock3 = sm['sz000955']
        if not stock3.is_null():
            k3 = stock3.get_kdata(Query(-10))
            
            result = NAMELIKE(k3, "欣龙")
            self.assertEqual(len(result), len(k3))
            for i in range(len(result)):
                self.assertEqual(result[i], 1.0)
    
    def test_CODELIKE_NAMELIKE_wildcard(self):
        """Test the wildcard function of CODELIKE and NAMELIKE"""
        stock = sm['sh000001']
        k = stock.get_kdata(Query(-10))
        
        # Test the wildcard * matching any sequence
        result = CODELIKE(k, "*")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        result = NAMELIKE(k, "*")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        # Test wildcard combinations
        result = CODELIKE(k, "0*1")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)
        
        result = CODELIKE(k, "0????1")
        self.assertEqual(len(result), len(k))
        for i in range(len(result)):
            self.assertEqual(result[i], 1.0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(IndicatorTest)
