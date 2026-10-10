#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for register_extra_ktype python callback lifetime (TM-009).
#          The python phase-end callback is wrapped via make_gil_safe, so it is safe
#          to call from the core's worker threads (which run without the GIL) and safe
#          to destroy when the registry is released. The interpreter-finalization safety
#          of the shared make_gil_safe deleter is covered by GilSafe.py.
# ===============================================================================

import os
import subprocess
import sys
import unittest

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

# Register a python-defined phase-end conversion and compute the extended K-line, which
# invokes the callback from the core (the gil_scoped_acquire wrapper must not crash), then
# release it (the make_gil_safe deleter must not crash on destruction).
EXTRA_KTYPE_PROBE = (
    "import sys; sys.path.insert(0, r'{root}')\n"
    "from hikyuu import *\n"
    "set_log_level(LOG_LEVEL.OFF)\n"
    "hikyuu_init('test_data/hikyuu_win.ini')\n"
    "def get_min9_phase_end(d):\n"
    "    m = d.minute\n"
    "    end = ((m // 9) + 1) * 9\n"
    "    if end < 60:\n"
    "        return Datetime(d.year, d.month, d.day, d.hour, end)\n"
    "    nd = d + TimeDelta(0, 1)\n"
    "    return Datetime(nd.year, nd.month, nd.day, nd.hour, 0)\n"
    "register_extra_ktype('MIN9', 'MIN', 9, get_min9_phase_end)\n"
    "assert 'MIN9' in Query.get_extra_ktype_list(), 'extra ktype not registered'\n"
    "stk = StockManager.instance()['sh000001']\n"
    "k = stk.get_kdata(Query(-200, ktype='MIN9'))\n"
    "assert len(k) > 0, 'no MIN9 data: the python phase-end callback was not invoked'\n"
    "release_extra_ktype()  # destructor-safety path for the python callback\n"
    "print('OK')\n"
).format(root=REPO_ROOT)


class ExtraKTypeTest(unittest.TestCase):
    def test_register_extra_ktype_python_callback(self):
        # The python callback must be callable from the core (GIL wrapped) and destructible
        # on release without crashing.
        proc = subprocess.run(
            [sys.executable, "-c", EXTRA_KTYPE_PROBE],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=180,
        )
        self.assertEqual(proc.returncode, 0, "extra ktype probe crashed:\n%s" % proc.stderr[-2000:])
        self.assertIn("OK", proc.stdout)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ExtraKTypeTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
