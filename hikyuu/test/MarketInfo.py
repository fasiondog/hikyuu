#!/usr/bin/python
# -*- coding: utf8 -*-
# gb18030

#===============================================================================
# 作者：fasiondog
# 历史：1）20120927, Added by fasiondog
#===============================================================================

import unittest

from test_init import *


class MarketInfoTest(unittest.TestCase):
    def test_market(self):
        market = sm.get_market_info("Sh")
        self.assertEqual(market.market, "SH")
        self.assertEqual(market.name, u"上海证券交易所")
        self.assertEqual(market.description, u"上海市场")
        self.assertEqual(market.code, "000001")
        self.assertEqual(market.last_datetime, Datetime(201112060000))


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(MarketInfoTest)
