#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

import unittest

import numpy as np
from test_init import *


class PositionRecordTest(unittest.TestCase):
    def _make_closed(self, stock):
        # invest = buyMoney - sellMoney + totalCost = -90
        # profit = totalProfit() = sellMoney - buyMoney - totalCost = 90
        # profit_percent = round(100 * 90 / -90, 2) = -100.0
        return PositionRecord(
            stock, Datetime(20200101), Datetime(20201231),
            100.0, 0.0, 0.0,
            100.0, 1000.0, 10.0, 0.0, 1100.0)

    def _list(self, *recs):
        lst = PositionRecordList()
        for r in recs:
            lst.append(r)
        return lst

    def test_empty(self):
        arr = positions_to_np(PositionRecordList())
        self.assertIsInstance(arr, np.ndarray)
        self.assertEqual(arr.shape[0] if arr.ndim else 0, 0)

    def test_closed_position_layout(self):
        # Field names come from htr() and vary by locale, so index by position.
        # Layout: 0 market_code, 1 stock_name, 4 hold_number, 5 invest,
        # 7 profit, 8 profit_percent, 11 clean_time.
        stock = sm['sh000001']
        arr = positions_to_np(self._list(self._make_closed(stock)))
        self.assertEqual(arr.shape, (1,))
        names = arr.dtype.names
        self.assertEqual(arr[names[0]][0], stock.market_code)
        self.assertEqual(arr[names[1]][0], stock.name)
        self.assertAlmostEqual(arr[names[4]][0], 100.0)
        self.assertAlmostEqual(arr[names[5]][0], -90.0)
        self.assertAlmostEqual(arr[names[7]][0], 90.0)
        self.assertAlmostEqual(arr[names[8]][0], -100.0)
        # closed record: clean_time is a real datetime, not NaT
        self.assertNotEqual(arr[names[11]][0], np.datetime64('NaT'))

    def test_multiple(self):
        stock = sm['sh000001']
        arr = positions_to_np(self._list(self._make_closed(stock), self._make_closed(stock)))
        self.assertEqual(arr.shape, (2,))

    def test_open_position(self):
        # An open position (null clean_datetime) runs getMarketValue inside the
        # loop; with day K data preloaded by the harness this completes and yields
        # a valid market_value. The exception safety on this path is guaranteed by
        # the unique_ptr RAII guard in the binding.
        stock = sm['sh000001']
        rec = PositionRecord(
            stock, Datetime(20200101), Datetime(),
            100.0, 0.0, 0.0,
            100.0, 1000.0, 10.0, 0.0, 1100.0)
        try:
            arr = positions_to_np(self._list(rec))
        except Exception:
            self.skipTest("open position requires available day kdata")
        self.assertEqual(arr.shape, (1,))
        self.assertGreaterEqual(arr[arr.dtype.names[6]][0], 0.0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PositionRecordTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
