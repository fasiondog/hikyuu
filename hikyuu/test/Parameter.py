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
        self.assertEqual(p.have("n"), False)
        self.assertEqual(p.have("b"), False)
        self.assertEqual(p.have("d"), False)
        self.assertEqual(p.have("s"), False)
        p.set("n", 10)
        p.set("b", True)
        p.set("d", 10.01)
        p.set("s", "string")
        self.assertEqual(p.have("n"), True)
        self.assertEqual(p.have("b"), True)
        self.assertEqual(p.have("d"), True)
        self.assertEqual(p.have("s"), True)
        self.assertEqual(p.get("n"), 10)
        self.assertEqual(p.get("b"), True)
        self.assertEqual(p.get("d"), 10.01)
        self.assertEqual(p.get("s"), "string")
        
        p.set("n", 20)
        p.set("b", False)
        p.set("d", 10.001)
        p.set("s", "string2")
        self.assertEqual(p.get("n"), 20)
        self.assertEqual(p.get("b"), False)
        self.assertEqual(p.get("d"), 10.001)
        self.assertEqual(p.get("s"), "string2")
        
    def test_pickle(self):
        if not constant.pickle_support:
            return 
        tmpdir = sm.tmpdir()
        fh = open(tmpdir + "/Parameter.plk", "wb")
        a = Parameter()
        a.set("bool", True)
        a.set("string", "This is Parameter")
        import pickle as pl
        pl.dump(a, fh)
        fh.close()
        fh = open(tmpdir + "/Parameter.plk", "rb")
        b = pl.load(fh)
        self.assertEqual(b.get("bool"), True)
        self.assertEqual(b.get("string"), "This is Parameter")        
        
                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ParameterTest)