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

import os
import subprocess
import sys
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

    def test_get_stock_list_filtered(self):
        blk = Block('test', 'test_block')
        blk.add([sm['sh000001'], sm['sz000001'], sm['sh600000']])
        result = blk.get_stock_list(filter=lambda s: s.market_code.startswith('SH'))
        self.assertEqual(len(result), 2)
        self.assertTrue(all(s.market_code.startswith('SH') for s in result))

    def test_filter_may_mutate(self):
        # the filter runs on a snapshot, so mutating the stock table (StockManager) or the
        # block itself inside the filter must neither deadlock nor crash
        root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
        config = "test_data/hikyuu_win.ini" if sys.platform == "win32" else "test_data/hikyuu_linux.ini"
        probe = (
            "import sys; sys.path.insert(0, r'{root}')\n"
            "import os; os.chdir(r'{root}')\n"
            "from hikyuu import *\n"
            "hikyuu_init(r'{config}')\n"
            "assert len(sm) > 0\n"
            "mutant = Stock('zz', '900001', 'mutant')\n"
            "def sm_filter(stk):\n"
            "    if mutant not in sm:\n"
            "        sm.add_stock(mutant)\n"
            "    return stk.market_code.startswith('SH')\n"
            "lst = sm.get_stock_list(filter=sm_filter)\n"
            "assert all(s.market_code.startswith('SH') for s in lst)\n"
            "assert mutant in sm\n"
            "blk = Block('test', 'mutant_block')\n"
            "blk.add(sm['sh000001'])\n"
            "def blk_filter(stk):\n"
            "    if len(blk) == 1:\n"
            "        blk.add('sz000001')\n"
            "    return stk.market_code == 'SH000001'\n"
            "lst = blk.get_stock_list(filter=blk_filter)\n"
            "assert len(lst) == 1 and lst[0].market_code == 'SH000001'\n"
            "assert len(blk) == 2\n"
        ).format(root=root, config=config)
        proc = subprocess.run(
          [sys.executable, "-c", probe], capture_output=True, text=True, timeout=120
        )
        self.assertEqual(
          proc.returncode, 0, "mutating filter probe failed:\n%s" % proc.stderr[-2000:]
        )


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(BlockTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
