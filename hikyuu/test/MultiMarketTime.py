#!/usr/bin/python
# -*- coding: utf-8 -*-
# cp936

# Multi-market read-side unified expression assertions: intraday in Beijing time, day bars
# as trading-day labels, helper conversions mutually inverse (carrier nasdaq_5min.h5,
# synthetic, spans the 2016-11-06 US DST end).

import unittest

from test_init import *

# The same local wall-clock time inside DST (+12h) and in winter time (+13h)
DST_LOCAL = Datetime(201611040945)
DST_UNIFIED = Datetime(201611042145)
STD_LOCAL = Datetime(201611070945)
STD_UNIFIED = Datetime(201611072245)


class MultiMarketTimeTest(unittest.TestCase):
    def setUp(self):
        self.stk = sm.get_stock("NASDAQNDX")
        self.assertFalse(self.stk.is_null())

    def test_min15_python_exit_is_unified(self):
        """The Python-side MIN15 kdata is already in the unified clock; day labels are untouched"""
        ks = self.stk.get_krecord_list(
            Query(Datetime(201610010000), Datetime(201612310000), Query.MIN15))
        self.assertEqual(len(ks), 364)
        self.assertEqual(ks[0].datetime, Datetime(201610242145))
        # local close 11-10 16:00 (winter time) = Beijing 11-11 05:00
        self.assertEqual(ks[-1].datetime, Datetime(201611110500))

        # still strictly monotonic after normalization (premise of getPos / buffer bisection)
        for i in range(1, len(ks)):
            self.assertTrue(ks[i - 1].datetime < ks[i].datetime)

        # across the DST end: the same local wall clock maps to different Beijing times
        unified_values = [k.datetime for k in ks]
        self.assertIn(DST_UNIFIED, unified_values)
        self.assertIn(STD_UNIFIED, unified_values)

        # day bars are not converted: hour/minute are always 0
        days = self.stk.get_krecord_list(
            Query(Datetime(201610240000), Datetime(201611110000), Query.DAY))
        self.assertGreater(len(days), 12)
        for k in days:
            # Datetime.number is a property, not a method, on the Python side
            self.assertEqual(k.datetime.number % 10000, 0)

    def test_kdata_get_pos(self):
        """get_pos(bar.datetime) equals the index everywhere inside the KData"""
        kdata = self.stk.get_kdata(
            Query(Datetime(201610010000), Datetime(201612310000), Query.MIN15))
        self.assertEqual(len(kdata), 364)
        for i, r in enumerate(kdata):
            self.assertEqual(kdata.get_pos(r.datetime), i)

    def test_public_helpers(self):
        """The public helpers share the source with the internal conversion and are mutually inverse"""
        self.assertEqual(sm.market_local_to_unified(DST_LOCAL, "NASDAQ"), DST_UNIFIED)
        self.assertEqual(sm.market_local_to_unified(STD_LOCAL, "NASDAQ"), STD_UNIFIED)
        self.assertEqual(sm.unified_to_market_local(DST_UNIFIED, "NASDAQ"), DST_LOCAL)
        self.assertEqual(sm.unified_to_market_local(STD_UNIFIED, "NASDAQ"), STD_LOCAL)

        # identity market returns unchanged
        self.assertEqual(sm.market_local_to_unified(Datetime(201611040945), "SH"),
                         Datetime(201611040945))
        # market name is case-insensitive
        self.assertEqual(sm.market_local_to_unified(DST_LOCAL, "nasdaq"), DST_UNIFIED)
        # unregistered market raises explicitly instead of silently acting as identity
        with self.assertRaises(Exception):
            sm.market_local_to_unified(DST_LOCAL, "NOSUCH")

    def test_market_info_identity(self):
        """The conversion truth source: NASDAQ/NYSE are -300/US DST/lag 1, A-share is 480/none/0"""
        for market, (tz, dst, lag) in (("NASDAQ", (-300, 1, 1)), ("NYSE", (-300, 1, 1)),
                                       ("SH", (480, 0, 0)), ("SHF", (480, 0, 0))):
            info = sm.get_market_info(market)
            # unregistered markets return a Null MarketInfo (empty market string)
            self.assertEqual(info.market, market)
            self.assertEqual(info.tz_offset_min, tz)
            self.assertEqual(info.dst_rule, dst)
            self.assertEqual(info.data_lag, lag)


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MultiMarketTimeTest)


if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite())
