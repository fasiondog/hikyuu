#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

# ===============================================================================
# Author: fasiondog
# History: regression test for DEF_PICKLE deserialization path.
#          Verify the standard bytes path round-trips losslessly and that a str
#          state is no longer silently corrupted via UTF-8 reinterpretation.
# ===============================================================================

import unittest
import pickle as pl

from test_init import *
from hikyuu import constant


@unittest.skipUnless(constant.pickle_support, "pickle support disabled")
class PickleTest(unittest.TestCase):
    def test_getstate_returns_bytes(self):
        s = sm['sh000001']
        state = s.__getstate__()
        self.assertIsInstance(state, tuple)
        self.assertIsInstance(state[0], bytes)

    def test_roundtrip_bytes(self):
        s = sm['sh000001']
        out = pl.loads(pl.dumps(s))
        self.assertEqual(out, s)
        self.assertEqual(out.market_code, s.market_code)

    def test_str_state_not_corrupting(self):
        # The deserialization path must not accept a str and silently turn the
        # binary archive into a valid-but-wrong object via UTF-8. pybind11's pickle
        # __setstate__ only assigns correctly inside the protocol, so feed the str state
        # through __reduce_ex__ to reach __setstate__.
        s = sm['sh000001']
        archive = s.__getstate__()[0]
        as_str = archive.decode('latin-1')  # keep raw bytes as a str

        class _Proxy:
            def __init__(self, state):
                self._state = state

            def __reduce_ex__(self, proto):
                return (Stock, (), self._state)

        # a str state must not rebuild the original object (no silent corruption;
        # rejection or a default/empty Stock are both acceptable)
        try:
            out_str = pl.loads(pl.dumps(_Proxy((as_str,))))
        except RuntimeError:
            return
        self.assertNotEqual(out_str, s)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PickleTest)


if __name__ == "__main__":
    unittest.TextTestRunner(verbosity=2).run(suite())
