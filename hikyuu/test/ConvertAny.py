#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for boost::any C++->Python conversion (convert_any.h).
#          Verify that after switching to py::cast, Stock/Block/Datetime/KQuery/KData
#          round-trip correctly, without relying on the __main__ namespace or code injection.
# ===============================================================================

import unittest

from test_init import *


class ConvertAnyTest(unittest.TestCase):
    def _roundtrip(self, value):
        ind = CLOSE()
        ind.set_param("__inf404__", value)
        return ind.get_param("__inf404__")

    def test_stock(self):
        stock = sm['sh000001']
        self.assertFalse(stock.is_null())
        out = self._roundtrip(stock)
        self.assertIsInstance(out, Stock)
        self.assertEqual(out, stock)
        self.assertEqual(out.market_code, stock.market_code)

    def test_null_stock(self):
        out = self._roundtrip(Stock())
        self.assertIsInstance(out, Stock)
        self.assertTrue(out.is_null())

    def test_block(self):
        blk = Block('test', 'test_block')
        blk.add(sm['sh000001'])
        blk.add(sm['sz000001'])
        out = self._roundtrip(blk)
        self.assertIsInstance(out, Block)
        self.assertEqual(out.category, blk.category)
        self.assertEqual(out.name, blk.name)
        self.assertEqual(len(out.get_stock_list()), 2)
        self.assertEqual(set(out.get_stock_list()), set(blk.get_stock_list()))

    def test_null_block(self):
        out = self._roundtrip(Block())
        self.assertIsInstance(out, Block)
        self.assertTrue(out.is_null())

    def test_datetime(self):
        d = Datetime(2024, 1, 2, 3, 4, 5, 6, 7)
        out = self._roundtrip(d)
        self.assertIsInstance(out, Datetime)
        self.assertEqual(out, d)

    def test_null_datetime(self):
        out = self._roundtrip(Datetime())
        self.assertIsInstance(out, Datetime)
        self.assertTrue(out.is_null())

    def test_kquery_index(self):
        q = Query(10, 20, Query.DAY)
        out = self._roundtrip(q)
        self.assertIsInstance(out, Query)
        self.assertEqual(out.query_type, q.query_type)
        self.assertEqual(out.ktype, q.ktype)
        self.assertEqual(out.recover_type, q.recover_type)
        self.assertEqual(out.start, q.start)
        self.assertEqual(out.end, q.end)

    def test_kquery_datetime(self):
        q = Query(Datetime(2020, 1, 1), Datetime(2024, 1, 1), Query.DAY, Query.FORWARD)
        out = self._roundtrip(q)
        self.assertIsInstance(out, Query)
        self.assertEqual(out.query_type, q.query_type)
        self.assertEqual(out.ktype, q.ktype)
        self.assertEqual(out.recover_type, q.recover_type)
        self.assertEqual(out.start_datetime, q.start_datetime)
        self.assertEqual(out.end_datetime, q.end_datetime)

    def test_kdata(self):
        stock = sm['sh000001']
        k = stock.get_kdata(Query(-10))
        self.assertFalse(k.empty())
        out = self._roundtrip(k)
        self.assertIsInstance(out, KData)
        self.assertEqual(out.get_stock().market_code, stock.market_code)
        self.assertEqual(len(out), len(k))

    def test_kdata_null(self):
        out = self._roundtrip(KData())
        self.assertIsInstance(out, KData)
        self.assertTrue(out.empty())

    def test_no_code_injection(self):
        stock = sm['sh000001']
        out = self._roundtrip(stock)
        self.assertIsInstance(out, Stock)
        self.assertEqual(out.market_code, stock.market_code)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ConvertAnyTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
