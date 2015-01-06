#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130316, Added by fasiondog
#===============================================================================

import unittest

from test_init import *
from hikyuu.trade_sys.moneymanager import *

class MoneyManagerPython(MoneyManagerBase):
    def __init__(self):
        super(MoneyManagerPython, self).__init__("MoneyManagerPython")
        self.params.add("n", 10)
        self._m_flag = False
        
    def getBuyNumber(self, datetime, stock, price, risk):
        if self._m_flag:
            return 10
        else:
            return 20
    
    def _reset(self):
        if self._m_flag:
            self._m_flag = False
        else:
            self._m_flag = True
            
    def _clone(self):
        p = MoneyManagerPython()
        p._m_flag = self._m_flag
        return p

class MoneyManagerTest(unittest.TestCase):
    def test_ConditionBase(self):
        stock = sm['sh000001']
        p = MoneyManagerPython()
        self.assertEqual(p.name, "MoneyManagerPython")
        self.assertEqual(p.params.get("n"), 10)
        p.params.set("n",20)
        self.assertEqual(p.params.get("n"), 20)
        self.assertEqual(p.getBuyNumber(Datetime(200101010000), stock, 10.0, 0.0), 20)
        p.reset()
        self.assertEqual(p.getBuyNumber(Datetime(200101010000), stock, 10.0, 0.0), 10)
        
        p_clone = p.clone()
        self.assertEqual(p_clone.name, "MoneyManagerPython")
        self.assertEqual(p_clone.params.get("n"), 20)
        self.assertEqual(p_clone.getBuyNumber(Datetime(200101010000), stock, 10, 0.0), 10)

        p.params.set("n", 1)
        p_clone.params.set("n", 3)
        self.assertEqual(p.params.get("n"), 1)
        self.assertEqual(p_clone.params.get("n"), 3)
                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MoneyManagerTest)