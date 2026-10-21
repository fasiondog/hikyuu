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


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(FactorImmutableTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
