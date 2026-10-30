#!/usr/bin/python
# -*- coding: utf-8 -*-

# ===============================================================================
# Author: fasiondog
# ===============================================================================

from test_init import *
import json
import unittest

import hikyuu
from hikyuu.hub import PartNotFoundError


class PartTest(unittest.TestCase):
    def test_registry(self):
        reg = hikyuu.part.PartRegistry.instance()
        self.assertEqual(reg.size(), 201)
        self.assertTrue(reg.has("core.ind.ma"))
        self.assertTrue(reg.has("core.ind.MA"))
        self.assertFalse(reg.has("core.ind.nonexistent"))

        names = reg.get_name_list()
        self.assertEqual(len(names), 201)
        self.assertIn("core.ind.ma", names)

        ind_names = reg.get_name_list(hikyuu.part.PartType.IND)
        self.assertEqual(len(ind_names), 138)
        self.assertIn("core.ind.ma", ind_names)

    def test_meta(self):
        reg = hikyuu.part.PartRegistry.instance()
        m = reg.get_meta("core.ind.ma")
        self.assertEqual(m.name, "MA")
        self.assertEqual(m.full_name, "core.ind.ma")
        self.assertEqual(m.type, hikyuu.part.PartType.IND)
        self.assertEqual(m.type_name, "ind")

        self.assertEqual(len(m.params), 1)
        p = m.params[0]
        self.assertEqual(p.name, "n")
        self.assertEqual(p.type, "int")
        self.assertEqual(p.default_value, "22")
        self.assertFalse(p.required)
        self.assertFalse(p.is_part_param)
        self.assertFalse(p.is_list)

        self.assertIsNone(reg.try_get_meta("core.ind.nonexistent"))
        self.assertIsNotNone(reg.try_get_meta("core.ind.ma"))
        with self.assertRaises(IndexError):
            reg.get_meta("core.ind.nonexistent")

    def test_create(self):
        reg = hikyuu.part.PartRegistry.instance()

        ind = reg.create("core.ind.ma")
        self.assertIsInstance(ind, Indicator)
        self.assertTrue(ind.equal(MA()))

        sc = Parameter()
        sc["n"] = 10
        ind10 = reg.create("core.ind.ma", sc)
        self.assertTrue(ind10.equal(MA(10)))

        with self.assertRaises(ValueError):
            reg.create("core.sg.band")

    def test_create_from_string(self):
        reg = hikyuu.part.PartRegistry.instance()

        self.assertTrue(reg.create_from_string("MA(n=10)").equal(MA(10)))
        self.assertTrue(reg.create_from_string("core.ind.ma(n=5)").equal(MA(5)))

        band = reg.create_from_string("SG_Band(ind=MA(n=10),lower=10,upper=20)")
        self.assertEqual(band.name, "SG_Band")

        with self.assertRaises(ValueError):
            reg.create_from_string("MA(n=abc)")
        with self.assertRaises(ValueError):
            reg.create_from_string("MA(n=10,n=20)")
        with self.assertRaises(IndexError):
            reg.create_from_string("UNKNOWN(n=1)")
        with self.assertRaises(ValueError):
            reg.create_from_string(
                "SYS_WalkForward(candidate_sys_list=SYS_Simple)"
            )

    def test_search_and_json(self):
        reg = hikyuu.part.PartRegistry.instance()

        found = reg.search("ma", hikyuu.part.PartType.IND)
        self.assertTrue(len(found) > 0)
        self.assertTrue(all(m.type == hikyuu.part.PartType.IND for m in found))

        data = json.loads(reg.to_json(hikyuu.part.PartType.TC))
        self.assertIsInstance(data, list)
        self.assertTrue(len(data) > 0)
        self.assertTrue(all(item["type"] == "tc" for item in data))

    def test_hub_get_part(self):
        self.assertTrue(hikyuu.get_part("core.ind.ma").equal(MA()))
        self.assertTrue(hikyuu.get_part("core.ind.ma", n=10).equal(MA(10)))
        self.assertTrue(hikyuu.get_part("Core.IND.MA", n=10).equal(MA(10)))

        with self.assertRaises(PartNotFoundError):
            hikyuu.get_part("core.ind.nonexistent")
        with self.assertRaises(ValueError):
            hikyuu.get_part("core.ind.ma", n=10.5)

    def test_hub_get_part_with_part_param(self):
        band = hikyuu.get_part("core.sg.band", ind=MA(10), lower=10, upper=20)
        self.assertEqual(band.name, "SG_Band")

        self.assertEqual(hikyuu.get_part("core.st.indicator", ind=MA(5)).name, "ST_Indicator")
        # an int is accepted for a double param
        self.assertEqual(hikyuu.get_part("core.st.fixedpercent", p=1).get_param("p"), 1.0)

        with self.assertRaises(ValueError):
            hikyuu.get_part("core.sg.band", ind=MA(10))

    def test_hub_core_info_and_search(self):
        info = hikyuu.get_part_info("core.ind.ma")
        self.assertEqual(info["name"], "core.ind.ma")
        self.assertIn('part("core.ind.ma",n=22', info["doc"])

        names = hikyuu.get_part_name_list(hub="core", part_type="ind")
        self.assertEqual(len(names), 138)
        self.assertIn("core.ind.ma", names)
        self.assertIn("core.ind.ma", hikyuu.get_part_name_list())
        self.assertEqual(hikyuu.get_part_name_list(hub="CORE", part_type="ind"), names)

        self.assertEqual(hikyuu.search_part(name="sg.band", hub="core", part_type="sg"), ["core.sg.band"])
        self.assertEqual(len(hikyuu.search_part(hub="core", part_type="tc")), 6)
        self.assertEqual(hikyuu.search_part(hub="nonexistent-hub-xyz"), [])

    def test_python_part(self):
        def factory(scalars, parts):
            return MA(scalars["n"])

        meta = hikyuu.part.PartMeta()
        meta.type = hikyuu.part.PartType.IND
        meta.name = "PyMA"
        param = hikyuu.part.ParamMeta()
        param.name = "n"
        param.type = "int"
        param.default_value = "10"
        meta.params = [param]

        reg = hikyuu.part.PartRegistry.instance()
        hikyuu.part.register_python_part("core.ind.pyma", meta, factory)
        try:
            self.assertTrue(reg.has("core.ind.pyma"))
            m = reg.get_meta("core.ind.pyma")
            self.assertEqual(m.name, "PyMA")
            self.assertEqual(m.source, hikyuu.part.Source.PYTHON)
            self.assertFalse(m.support_batch_reuse)
            self.assertTrue(reg.create("core.ind.pyma").equal(MA(10)))

            sc = Parameter()
            sc["n"] = 5
            self.assertTrue(reg.create("core.ind.pyma", sc).equal(MA(5)))
        finally:
            hikyuu.part.release_python_parts()

        self.assertFalse(reg.has("core.ind.pyma"))
        self.assertEqual(reg.size(), 201)

    def test_python_part_conflict(self):
        def make_meta(name):
            meta = hikyuu.part.PartMeta()
            meta.type = hikyuu.part.PartType.IND
            meta.name = name
            param = hikyuu.part.ParamMeta()
            param.name = "n"
            param.type = "int"
            param.default_value = "10"
            meta.params = [param]
            return meta

        reg = hikyuu.part.PartRegistry.instance()
        try:
            hikyuu.part.register_python_part(
                "core.ind.pyma2", make_meta("PyMA2"), lambda s, p: MA(10)
            )

            with self.assertRaises(ValueError):
                hikyuu.part.register_python_part(
                    "core.ind.pyma2", make_meta("PyMA2"), lambda s, p: MA(20)
                )

            with self.assertRaises(ValueError):
                hikyuu.part.register_python_part(
                    "core.ind.ma", make_meta("MA"), lambda s, p: MA(30)
                )

            hikyuu.part.register_python_part(
                "core.ind.pyma2", make_meta("PyMA2"), lambda s, p: MA(99), overwrite=True
            )
            self.assertEqual(reg.create("core.ind.pyma2").get_param("n"), 99)
        finally:
            hikyuu.part.release_python_parts()

        self.assertFalse(reg.has("core.ind.pyma2"))
        self.assertTrue(reg.has("core.ind.ma"))
        self.assertEqual(reg.size(), 201)

    def test_python_part_bad_return(self):
        meta = hikyuu.part.PartMeta()
        meta.type = hikyuu.part.PartType.IND
        meta.name = "PyBad"

        reg = hikyuu.part.PartRegistry.instance()
        try:
            hikyuu.part.register_python_part("core.ind.pybad", meta, lambda s, p: 42)
            with self.assertRaises(hikyuu.HKUException):
                reg.create("core.ind.pybad")
        finally:
            hikyuu.part.release_python_parts()


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(PartTest)
