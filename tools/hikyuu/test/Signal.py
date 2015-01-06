#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130321, Added by fasiondog
#===============================================================================

import unittest

from test_init import *
from hikyuu.trade_sys.signal import *

class SignalPython(SignalBase):
    def __init__(self):
        super(SignalPython, self).__init__("SignalPython")
        self._x = 0
    
    def _reset(self):
        self._x = 0
            
    def _clone(self):
        p = SignalPython()
        p._x = self._x
        return p

class SignalTest(unittest.TestCase):
    def test_SignalBase(self):
        p = SignalPython()
        self.assertEqual(p.name, "SignalPython")
        
        self.assertEqual(p.shouldBuy(Datetime(200101010000)), False)
        p._addBuySignal(Datetime(200101010000))
        self.assertEqual(p.shouldBuy(Datetime(200101010000)), True)
        
        self.assertEqual(p.shouldSell(Datetime(200101010000)), False)
        p._addSellSignal(Datetime(200101010000))
        self.assertEqual(p.shouldSell(Datetime(200101010000)), True)
        
        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        
        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)

                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(SignalTest)