#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression tests for Block.add with sequence inputs: the documented
#          string "market abbreviation + security code" sequences must work, a
#          mixed sequence of Stock and str is accepted, and an unresolvable code
#          reports a failure instead of raising a cast error.
# ===============================================================================

import unittest

from test_init import *


class BlockTest(unittest.TestCase):
    def test_add_stock_list(self):
        blk = Block('test', 'test_block')
        self.assertTrue(blk.add([sm['sh000001'], sm['sz000001']]))
        self.assertEqual(len(blk), 2)

    def test_add_str_list(self):
        blk = Block('test', 'test_block')
        self.assertTrue(blk.add(['sh000001', 'sz000001']))
        self.assertEqual(len(blk), 2)
        codes = [s.market_code for s in blk.get_stock_list()]
        self.assertIn('SH000001', codes)
        self.assertIn('SZ000001', codes)

    def test_add_mixed_list(self):
        blk = Block('test', 'test_block')
        self.assertTrue(blk.add([sm['sh000001'], 'sz000001']))
        self.assertEqual(len(blk), 2)

    def test_add_bad_code_returns_false(self):
        blk = Block('test', 'test_block')
        self.assertFalse(blk.add(['no_such_stock_code']))
        self.assertEqual(len(blk), 0)

    def test_add_empty(self):
        blk = Block('test', 'test_block')
        self.assertTrue(blk.add([]))
        self.assertEqual(len(blk), 0)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(BlockTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
