#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130321, Added by fasiondog
#===============================================================================

import unittest

from test_init import *
from hikyuu.trade_sys.profitgoal import *

class ProfitGoalPython(ProfitGoalBase):
    def __init__(self):
        super(ProfitGoalPython, self).__init__("ProfitGoalPython")
        self._x = 0
        
    def getGoal(self, datetime, price):
        if self._x < 10:
            return 0.0
        return 1.0
    
    def _reset(self):
        self._x = 0
            
    def _clone(self):
        p = ProfitGoalPython()
        p._x = self._x
        return p
    
    def _calculate(self):
        """ do nothin """

class ProfitGoalTest(unittest.TestCase):
    def test_ProfitGoalBase(self):
        p = ProfitGoalPython()
        self.assertEqual(p.name, "ProfitGoalPython")
        self.assertEqual(p.getGoal(Datetime(200101010000), 1.0), 0.0)
        
        self.assertEqual(p._x, 0)
        p._x = 10
        self.assertEqual(p._x, 10)
        self.assertEqual(p.getGoal(Datetime(200101010000), 1.0), 1.0)
        p.reset()
        self.assertEqual(p._x, 0)
        
        p._x = 10
        p_clone = p.clone()
        self.assertEqual(p_clone._x, 10)
        p.reset()
        self.assertEqual(p._x, 0)
        self.assertEqual(p_clone._x, 10)

                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ProfitGoalTest)