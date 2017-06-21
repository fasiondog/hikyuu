#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130311, Added by fasiondog
#===============================================================================

import unittest

from test_init import *
from hikyuu.trade_sys.condition import *
from hikyuu.trade_sys.signal import SignalBase

class ConditionPython(ConditionBase):
    def __init__(self):
        super(ConditionPython, self).__init__("ConditionPython")
        self.setParam("n", 10)
        self._m_flag = False
        
    def isValid(self, datetime):
        return self._m_flag
    
    def _reset(self):
        if self._m_flag:
            self._m_flag = False
        else:
            self._m_flag = True
            
    def _clone(self):
        p = ConditionPython()
        p._m_flag = self._m_flag
        return p

class ConditionTest(unittest.TestCase):
    def test_ConditionBase(self):
        p = ConditionPython()
        self.assertEqual(p.name, "ConditionPython")
        self.assertEqual(p.getParam("n"), 10)
        p.setParam("n",20)
        self.assertEqual(p.getParam("n"), 20)
        self.assertEqual(p.isValid(Datetime(200101010000)), False)
        p.reset()
        self.assertEqual(p.isValid(Datetime(200101010000)), True)
        
        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ConditionPython")
        self.assertEqual(p_clone.getParam("n"), 20)
        self.assertEqual(p_clone.isValid(Datetime(200101010000)), True)

        p.setParam("n", 1)
        p_clone.setParam("n", 3)
        self.assertEqual(p.getParam("n"), 1)
        self.assertEqual(p_clone.getParam("n"), 3)
        
def testCondition(self): 
    self._addValid(Datetime(200101010000))
    self._addValid(Datetime(200101020000))
    
class TestCrtCN(unittest.TestCase):
    def test_crtCN(self):
        p = crtCN(testCondition, params={'n':10}, name='ConditionPython')
        self.assertEqual(p.getParam("n"), 10)
        p.setParam("n",20)
        self.assertEqual(p.getParam("n"), 20)

        k = sm['sh000001'].getKData(Query(-100))
        self.assertEqual(k.empty(), False)
        p.setSG(SignalBase())  #cn设置交易对象时，必须已经指定了信号指示器
        p.setTO(k)             #cn在设置交易对象时才会调用_caculate函数
        self.assertEqual(p.isValid(Datetime(200101010000)), True)
        self.assertEqual(p.isValid(Datetime(200101020000)), True)
        self.assertEqual(p.isValid(Datetime(200101030000)), False)
        
        p_clone = p.clone()
        self.assertEqual(p_clone.name, "ConditionPython")
        self.assertEqual(p_clone.getParam("n"), 20)
        self.assertEqual(p_clone.isValid(Datetime(200101010000)), True)

        p.setParam("n", 1)
        p_clone.setParam("n", 3)
        self.assertEqual(p.getParam("n"), 1)
        self.assertEqual(p_clone.getParam("n"), 3)        
                 
def suite():
    return unittest.TestLoader().loadTestsFromTestCase(ConditionTest)

def suiteTestCrtCN():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtCN)
