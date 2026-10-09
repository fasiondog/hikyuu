#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for interpreter-finalization-safe py::object lifetime.
#          A Py_AtExit flag set at shutdown start lets background-thread deleters
#          skip decref during finalization, so interpreter exit stays clean.
# ===============================================================================

import os
import subprocess
import sys
import unittest

# Registering a Strategy callback wraps it via make_gil_safe, which registers the
# Py_AtExit finalization flag. Tearing the strategy down during interpreter exit
# must not crash: the finalizing path skips decref instead of touching the GIL.
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
EXIT_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "from hikyuu import *\n"
    "stg = Strategy()\n"
    "stg.run_daily(lambda stg: None, Seconds(1))\n"
).format(root=REPO_ROOT)


class GilSafeTest(unittest.TestCase):
    def test_exit_clean_after_gil_safe_use(self):
        proc = subprocess.run(
          [sys.executable, "-c", EXIT_PROBE],
          capture_output=True,
          text=True,
          encoding="utf-8",
          errors="replace",
          timeout=180,
        )
        self.assertEqual(
          proc.returncode, 0, "interpreter exit crashed:\n%s" % proc.stderr[-2000:]
        )


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(GilSafeTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
