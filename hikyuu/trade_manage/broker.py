#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# ===============================================================================
# History:
# 1. 20170704, Added by fasiondog
# ===============================================================================

import json
from hikyuu import OrderBrokerBase
from hikyuu.util import hku_error


class OrderBrokerWrap(OrderBrokerBase):
    """Order broker wrapper class, users can refer to it to customize their own order broker and add extra processing
       Wraps the trading interface classes whose buy/sell operation parameters are only (code, price, num)
    """

    def __init__(self, broker, name):
        """
        Order broker wrapper class, users can refer to it to customize their own order broker and add extra processing
        """
        super(OrderBrokerWrap, self).__init__(name)
        self._broker = broker

    def _buy(self, datetime, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        """
        Implement the _buy interface of OrderBrokerBase
        :param str market: the stock market
        :param str code: the stock code
        :param float price: the buy price
        :param int num: the buy number
        """
        self._broker.buy(market, code, price, num, stoploss, goal_price, part_from, remark)

    def _sell(self, datetime, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        """Implement the _sell interface of OrderBrokerBase"""
        self._broker.sell(market, code, price, num, stoploss, goal_price, part_from, remark)

    def _get_asset_info(self):
        try:
            if hasattr(self._broker, "get_asset_info"):
                ret = self._broker.get_asset_info()
                return json.dumps(ret) if type(ret) == dict else str(ret)
            return str()
        except Exception as e:
            hku_error(str(e))
            return str()


class TestOrderBroker:
    """An order broker for testing, only prints the information when executing the buy/sell"""

    def __init__(self):
        pass

    def buy(self, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        print(f"Buy: {market}{code}, price: {price}, number: {num}, expected stop-loss price: {stoploss}, expected goal price: {goal_price}, signal source: {part_from}, remark: {remark}")

    def sell(self, market, code, price, num, stoploss, goal_price, part_from, remark=""):
        print(f"Sell: {market}{code}, price: {price}, number: {num}, signal source: {part_from}, remark: {remark}")


def crtOB(broker, name="NO_NAME"):
    """
    Quickly create an order broker wrapper object

    :param broker: the order broker instance, must have the buy and sell methods, and the parameters are code, price, num
    :param float slip: place the order if the absolute difference between the current sell-1 price and the indicated buy price does not exceed slip,
                        otherwise ignore; it has no effect on the sell operation, which sells immediately at the current price
    """
    return OrderBrokerWrap(broker, name)
