#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20130311, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class EnvironmentPython(EnvironmentBase):
    def __init__(self):
        super(EnvironmentPython, self).__init__("EnvironmentPython")
        self.set_param("n", 10)
        self._m_flag = False

    def is_valid(self, market, datetime):
        return self._m_flag

    def _reset(self):
        if self._m_flag:
            self._m_flag = False
        else:
            self._m_flag = True

    def _clone(self):
        p = EnvironmentPython()
        p._m_flag = self._m_flag
        return p


class EnvironmentTest(unittest.TestCase):
    def test_EnvironmentBase(self):
        p = EnvironmentPython()
        self.assertEqual(p.name, "EnvironmentPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        self.assertEqual(p.is_valid("SH", Datetime(200101010000)), False)
        p.reset()
        self.assertEqual(p.is_valid("SH", Datetime(200101010000)), True)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "EnvironmentPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.is_valid("SH", Datetime(200101010000)), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def test_crtEV_func(self):
    self._add_valid(Datetime(200101010000))


class TestCrtEV(unittest.TestCase):
    def test_crtEV(self):
        p = crtEV(test_crtEV_func, params={'n': 10}, name='EnvironmentPython')
        self.assertEqual(p.name, "EnvironmentPython")
        self.assertEqual(p.get_param("n"), 10)
        p.set_param("n", 20)
        self.assertEqual(p.get_param("n"), 20)
        p.query = Query(-100)  #ev在设置查询对象时才会调用_caculate函数
        self.assertEqual(p.is_valid(Datetime(200101010000)), True)

        p_clone = p.clone()
        self.assertEqual(p_clone.name, "EnvironmentPython")
        self.assertEqual(p_clone.get_param("n"), 20)
        self.assertEqual(p_clone.is_valid(Datetime(200101010000)), True)

        p.set_param("n", 1)
        p_clone.set_param("n", 3)
        self.assertEqual(p.get_param("n"), 1)
        self.assertEqual(p_clone.get_param("n"), 3)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(EnvironmentTest)


def suiteTestCrtEV():
    return unittest.TestLoader().loadTestsFromTestCase(TestCrtEV)
