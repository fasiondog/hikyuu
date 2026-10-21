#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: FT01 regression test - Factor's structural fields (name / ktype /
#          formula / block) must be immutable after construction. Assigning to
#          these read-only attributes from Python must raise AttributeError, so a
#          factor held inside a FactorSet can no longer be renamed/reformulated
#          in place, keeping FactorSet.nameIndexMap in sync with its factors.
# ===============================================================================

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

    def test_block_readonly(self):
        f = make_factor()
        self.assertEqual(len(f.block), 0)
        with self.assertRaises(AttributeError):
            f.block = Block("test", "test_block_readonly")

    def test_readonly_inside_factor_set(self):
        # Core FT01 scenario: a factor inside a FactorSet cannot be renamed,
        # reformulated, or re-blocked in place
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
        with self.assertRaises(AttributeError):
            fs[0].block = Block("test", "test_block_readonly")

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


def suite():
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromTestCase(FactorImmutableTest))
    suite.addTests(loader.loadTestsFromTestCase(FactorSetIndexTest))
    suite.addTests(loader.loadTestsFromTestCase(FactorNameGuardTest))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
