#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

# ===============================================================================
# 作者：fasiondog
# 历史：1）20120927, Added by fasiondog
# ===============================================================================

import unittest

from test_init import *


class DatetimeTest(unittest.TestCase):
    def test_Datetime(self):
        d = Datetime(201209272301)
        self.assertEqual(d.year, 2012)
        self.assertEqual(d.month, 9)
        self.assertEqual(d.day, 27)
        self.assertEqual(d.hour, 23)
        self.assertEqual(d.minute, 1)
        self.assertEqual(d.number, 201209272301)
        self.assertEqual(Datetime(140001010000), Datetime.min())
        self.assertEqual(Datetime(999912310000), Datetime.max())

        self.assertEqual(d, Datetime(201209272301))
        self.assertNotEqual(d, Datetime(200101010000))
        self.assertGreaterEqual(d, Datetime(201209272301))
        self.assertTrue(d >= Datetime(201209272259))
        self.assertTrue(d <= Datetime(201209272301))
        self.assertTrue(d <= Datetime(201209272302))
        self.assertTrue(not (d > Datetime(201209272301)))
        self.assertTrue(d > Datetime(201209272259))
        self.assertTrue(not (d < Datetime(201209272301)))
        self.assertTrue(d < Datetime(201209272302))

        d = Datetime(200101010159)
        self.assertEqual(str(d), "2001-01-01 01:59:00")
        self.assertEqual(d, Datetime("2001-Jan-01 01:59:00"))

        self.assertEqual(Datetime(), constant.null_datetime)

    def test_pickle(self):
        if not constant.pickle_support:
            return

        import pickle as pl
        a = Datetime(201001010000)
        filename = sm.tmpdir() + "/Datetime.plk"
        fh = open(filename, 'wb')
        pl.dump(a, fh)
        fh.close()
        fh = open(filename, 'rb')
        b = pl.load(fh)
        fh.close()
        self.assertEqual(a, b)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(DatetimeTest)
