#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: FT01 regression test - Factor's structural fields (name / ktype /
#          formula) must be immutable after construction; block remains settable.
#          Assigning to these read-only attributes from Python must raise
#          AttributeError, so a factor held inside a FactorSet can no longer be
#          renamed/reformulated in place, keeping FactorSet.nameIndexMap in sync
#          with its factors.
# ===============================================================================

import pickle
import unittest

from test_init import *


def make_factor():
    # Build via (name, formula, ktype, ...) so it does not trigger the
    # DataDriver.getFactor recursion. Factor uppercases the name on construction.
    return Factor("test_factor", CVAL(1.0), "DAY", brief="brief", details="details")


class FactorImmutableTest(unittest.TestCase):
    def test_name_readonly(self):
        f = make_factor()
        self.assertEqual(f.name, "TEST_FACTOR")
        with self.assertRaises(AttributeError):
            f.name = "new_name"

    def test_ktype_readonly(self):
        f = make_factor()
        self.assertEqual(f.ktype, "DAY")
        with self.assertRaises(AttributeError):
            f.ktype = "5MIN"

    def test_formula_readonly(self):
        f = make_factor()
        # Data sets formula.name to (the uppercased) factor name on construction
        self.assertEqual(f.formula.name, "TEST_FACTOR")
        with self.assertRaises(AttributeError):
            f.formula = CVAL(2.0)

    def test_block_mutable(self):
        f = make_factor()
        self.assertEqual(len(f.block), 0)
        blk = Block("test", "test_block_mutable")
        f.block = blk
        self.assertEqual(f.block.name, "test_block_mutable")

    def test_readonly_inside_factor_set(self):
        # Core FT01 scenario: a factor inside a FactorSet cannot be renamed or
        # reformulated in place (block remains settable)
        fs = FactorSet("test_factor_set")
        fs.add(make_factor())
        self.assertEqual(len(fs), 1)
        self.assertEqual(fs[0].name, "TEST_FACTOR")

        with self.assertRaises(AttributeError):
            fs[0].name = "changed"
        with self.assertRaises(AttributeError):
            fs[0].ktype = "5MIN"
        with self.assertRaises(AttributeError):
            fs[0].formula = CVAL(2.0)

        # block remains settable even inside a FactorSet
        fs[0].block = Block("test", "test_block_mutable")
        self.assertEqual(fs[0].block.name, "test_block_mutable")

        # Metadata fields remain writable
        fs[0].brief = "new_brief"
        self.assertEqual(fs[0].brief, "new_brief")


class FactorSetIndexTest(unittest.TestCase):
    def _make_set(self):
        fs = FactorSet("ft02_index")
        fs.add("F1", CVAL(1.0))
        fs.add("F2", CVAL(2.0))
        return fs

    def test_index_access(self):
        fs = self._make_set()
        self.assertEqual(fs[0].name, "F1")
        self.assertEqual(fs[1].name, "F2")

    def test_negative_index(self):
        fs = self._make_set()
        self.assertEqual(fs[-1].name, "F2")
        self.assertEqual(fs[-2].name, "F1")

    def test_index_out_of_range(self):
        fs = self._make_set()
        for i in (2, 5):
            with self.assertRaises(IndexError):
                fs[i]

    def test_negative_index_out_of_range(self):
        fs = self._make_set()
        with self.assertRaises(IndexError):
            fs[-3]

    def test_remove_preserves_order(self):
        fs = FactorSet("ft05_order")
        fs.add("F1", CVAL(1.0))
        fs.add("F2", CVAL(2.0))
        fs.add("F3", CVAL(3.0))
        fs.add("F4", CVAL(4.0))
        self.assertEqual(len(fs), 4)
        self.assertEqual([fs[i].name for i in range(len(fs))], ["F1", "F2", "F3", "F4"])
        fs.remove("F2")
        self.assertEqual(len(fs), 3)
        self.assertEqual([fs[i].name for i in range(len(fs))], ["F1", "F3", "F4"])
        fs.remove("F4")
        self.assertEqual(len(fs), 2)
        self.assertEqual([fs[i].name for i in range(len(fs))], ["F1", "F3"])
        fs.remove("F1")
        fs.remove("F3")
        self.assertTrue(fs.empty())


class FactorNameGuardTest(unittest.TestCase):
    def test_name_injection_guard(self):
        sm = StockManager.instance()
        try:
            driver_type = sm.getKDataDriverParameter()["type"]
        except Exception:
            driver_type = ""

        f = Factor("FT04_VALID_NAME", CVAL(1.0), "DAY")
        self.assertFalse(f.is_null())
        try:
            f.save_to_db()
            f.remove_from_db()
        except Exception as e:
            self.fail("valid name should not raise in save/remove: %s" % e)

        if driver_type == "clickhouse":
            bad = Factor("a;DROP TABLE factors;--", CVAL(1.0), "DAY")
            with self.assertRaises(Exception):
                bad.save_to_db()
        else:
            # no factor DB -> name unrestricted, no false rejection
            bad = Factor("a;DROP TABLE factors;--", CVAL(1.0), "DAY")
            self.assertFalse(bad.is_null())


class FactorConstructionTest(unittest.TestCase):
    def test_default_ctor_is_null(self):
        f = Factor()
        self.assertTrue(f.is_null())

    def test_ctor_with_formula(self):
        f = Factor("ft09_factor", CVAL(1.0), "DAY")
        self.assertFalse(f.is_null())
        self.assertEqual(f.name, "FT09_FACTOR")
        self.assertEqual(f.ktype, "DAY")

    def test_load_by_name_graceful(self):
        # 缺失因子 / 无因子库时按名加载不抛异常（优雅降级契约）
        Factor("NO_SUCH_FACTOR_FT09", "DAY")

    def test_get_value_with_stock(self):
        sm = StockManager.instance()
        s = sm["sh000001"]
        q = Query(-10)
        f = Factor("FT09_MA", MA(CLOSE(), 3), "DAY")
        ind = f.get_value(s, q)
        self.assertEqual(len(ind), len(s.get_kdata(q)))

    def test_get_values_follow_stock_order(self):
        sm = StockManager.instance()
        stocks = [sm["sh000001"], sm["sz000001"]]
        q = Query(-10)
        f = Factor("FT09_ORDER", MA(CLOSE(), 3), "DAY")
        results = f.get_values(stocks, q)
        self.assertEqual(len(results), 2)
        for i, s in enumerate(stocks):
            k = s.get_kdata(q)
            single = f.get_value(s, q)
            self.assertEqual(len(results[i]), len(k))
            self.assertAlmostEqual(results[i][len(k) - 1], single[len(k) - 1], places=10)

    def test_pickle_factor(self):
        f = Factor("ft09_pickle", CVAL(1.0), "DAY")
        f2 = pickle.loads(pickle.dumps(f))
        self.assertFalse(f2.is_null())
        self.assertEqual(f2.name, "FT09_PICKLE")
        self.assertEqual(f2.ktype, "DAY")

    def test_pickle_factor_set(self):
        fs = FactorSet("ft09_pickle_set")
        fs.add("F1", CVAL(1.0))
        fs.add("F2", CVAL(2.0))
        fs2 = pickle.loads(pickle.dumps(fs))
        self.assertEqual(len(fs2), 2)
        self.assertEqual([fs2[i].name for i in range(len(fs2))], ["F1", "F2"])


def suite():
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromTestCase(FactorImmutableTest))
    suite.addTests(loader.loadTestsFromTestCase(FactorSetIndexTest))
    suite.addTests(loader.loadTestsFromTestCase(FactorNameGuardTest))
    suite.addTests(loader.loadTestsFromTestCase(FactorConstructionTest))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
