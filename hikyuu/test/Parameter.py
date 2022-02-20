#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130220, Added by fasiondog
#===============================================================================

from test_init import *
import unittest
from hikyuu import Parameter


class ParameterTest(unittest.TestCase):
    def test_Parameter(self):
        p = Parameter()
        self.assertEqual("n" in p, False)
        self.assertEqual("b" in p, False)
        self.assertEqual("d" in p, False)
        self.assertEqual("s" in p, False)
        p["n"] = 10
        p["b"] = True
        p["d"] = 10.01
        p["s"] = "string"
        self.assertEqual("n" in p, True)
        self.assertEqual("b" in p, True)
        self.assertEqual("d" in p, True)
        self.assertEqual("s" in p, True)
        self.assertEqual(p["n"], 10)
        self.assertEqual(p["b"], True)
        self.assertEqual(p["d"], 10.01)
        self.assertEqual(p["s"], "string")

        p["n"] = 20
        p["b"] = False
        p["d"] = 10.001
        p["s"] = "string2"
        self.assertEqual(p["n"], 20)
        self.assertEqual(p["b"], False)
        self.assertEqual(p["d"], 10.001)
        self.assertEqual(p["s"], "string2")

    # def test_pickle(self):
    #     if not constant.pickle_support:
    #         return
    #     tmpdir = sm.tmpdir()
    #     fh = open(tmpdir + "/Parameter.plk", "wb")
    #     a = Parameter()
    #     a["bool"] = True
    #     a["string"] = "This is Parameter"
    #     import pickle as pl
    #     pl.dump(a, fh)
    #     fh.close()
    #     fh = open(tmpdir + "/Parameter.plk", "rb")
    #     b = pl.load(fh)
    #     self.assertEqual(b["bool"], True)
    #     self.assertEqual(b["string"], "This is Parameter")


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ParameterTest)