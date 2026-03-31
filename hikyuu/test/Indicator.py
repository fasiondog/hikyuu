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
        self.assertEqual(m.discard, 1)
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

    def test_BARSLASTS(self):
        data = [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0]
        ind = PRICELIST(data)

        # 测试N=1，应该与BARSLAST一致
        result1 = BARSLASTS(ind, 1)
        expected1 = BARSLAST(ind)

        # print(f"数据: {data}")
        # print(f"BARSLASTS(ind, 1): {[result1[i] for i in range(len(result1))]}")
        # print(f"BARSLAST(ind):     {[expected1[i] for i in range(len(expected1))]}")

        # 验证N=1时与BARSLAST一致
        for i in range(len(result1)):
            if np.isnan(expected1[i]):
                self.assertTrue(np.isnan(result1[i]))
            else:
                self.assertTrue(result1[i] == expected1[i])

        # print("✓ N=1时与BARSLAST一致")

        # 测试N=2
        result2 = BARSLASTS(ind, 2)
        # print(f"BARSLASTS(ind, 2): {[result2[i] for i in range(len(result2))]}")
        # print(f"BARSLASTS(ind, 2) discard: {result2.discard}")

        # 验证N=2的结果
        # 位置0-3：条件只成立1次，不足2次，应为NaN
        for i in range(4):
            self.assertTrue(np.isnan(result2[i]))

        # 位置4：条件第2次成立，距离第1次成立(位置0)为4
        self.assertTrue(result2[4] == 4)

        # 位置5-7：条件已成立2次，距离第1次成立(位置0)分别为5,6,7
        for i in range(5, 8):
            self.assertTrue(result2[i] == i)

        # 位置8：条件第3次成立，距离第2次成立(位置4)为4
        self.assertTrue(result2[8] == 4)

        # print("✓ N=2测试通过")

        # 测试N=3
        result3 = BARSLASTS(ind, 3)
        # print(f"BARSLASTS(ind, 3): {[result3[i] for i in range(len(result3))]}")

        # 验证N=3的结果
        # 位置0-7：条件只成立2次，不足3次，应为NaN
        for i in range(8):
            self.assertTrue(np.isnan(result3[i]))

        # 位置8：条件第3次成立，距离第1次成立(位置0)为8
        self.assertTrue(result3[8] == 8)

        # 位置9-11：条件已成立3次，距离第1次成立(位置0)分别为9,10,11
        for i in range(9, 12):
            self.assertTrue(result3[i] == i)

        """测试边界情况"""
        # print("\n测试边界情况...")

        # 测试N <= 0
        data = [1, 0, 1, 0, 1]
        ind = PRICELIST(data)

        result0 = BARSLASTS(ind, 0)
        # print(f"BARSLASTS(ind, 0): {[result0[i] for i in range(len(result0))]}")
        self.assertTrue(result0.discard == len(data))
        # print("✓ N=0测试通过")

        result_neg = BARSLASTS(ind, -1)
        # print(f"BARSLASTS(ind, -1): {[result_neg[i] for i in range(len(result_neg))]}")
        self.assertTrue(result_neg.discard == len(data))
        # print("✓ N=-1测试通过")

        # 测试空数据
        empty_ind = PRICELIST([])
        result_empty = BARSLASTS(empty_ind, 1)
        # print(f"BARSLASTS(空数据, 1): 长度={len(result_empty)}")
        self.assertTrue(len(result_empty) == 0)
        # print("✓ 空数据测试通过")

        # 测试全0数据
        zeros = [0, 0, 0, 0, 0]
        ind_zeros = PRICELIST(zeros)
        result_zeros = BARSLASTS(ind_zeros, 1)
        # print(f"BARSLASTS(全0数据, 1): discard={result_zeros.discard}")
        self.assertTrue(result_zeros.discard == len(zeros))
        # print("✓ 全0数据测试通过")

        """测试与K线数据结合使用"""
        # print("\n测试与K线数据结合使用...")

        # 获取股票数据
        sm = StockManager.instance()
        stock = sm['sh000001']

        if stock.is_null():
            print("⚠ 无法获取股票数据，跳过此测试")
            return

        kdata = stock.get_kdata(Query(-100))

        if len(kdata) == 0:
            print("⚠ K线数据为空，跳过此测试")
            return

        # 测试收盘价上涨
        close = CLOSE(kdata)
        up = close > REF(close, 1)

        # 计算第1次上涨到现在的天数
        result1 = BARSLASTS(up, 1)
        # print(f"BARSLASTS(上涨, 1)前5个值: {[result1[i] for i in range(min(5, len(result1)))]}")

        # 计算第2次上涨到现在的天数
        result2 = BARSLASTS(up, 2)
        # print(f"BARSLASTS(上涨, 2)前5个值: {[result2[i] for i in range(min(5, len(result2)))]}")

        # 验证N=1时与BARSLAST一致
        expected = BARSLAST(up)
        for i in range(min(10, len(result1))):
            if np.isnan(expected[i]):
                self.assertTrue(np.isnan(result1[i]))
            else:
                self.assertTrue(result1[i] == expected[i])

        # print("✓ K线数据测试通过")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(IndicatorTest)
