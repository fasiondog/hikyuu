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
    """订单代理包装类，用户可以参考自定义自己的订单代理，加入额外的处理
       包装只有买卖操作参数只有(code, price, num)的交易接口类
    """

    def __init__(self, broker, name):
        """
        订单代理包装类，用户可以参考自定义自己的订单代理，加入额外的处理
        """
        super(OrderBrokerWrap, self).__init__(name)
        self._broker = broker

    def _buy(self, datetime, market, code, price, num, stoploss, goal_price, part_from):
        """
        实现 OrderBrokerBase 的 _buy 接口
        :param str market: 证券市场    
        :param str code: 证券代码
        :param float price: 买入价格
        :param int num: 买入数量        
        """
        self._broker.buy(market, code, price, num, stoploss, goal_price, part_from)

    def _sell(self, datetime, market, code, price, num, stoploss, goal_price, part_from):
        """实现 OrderBrokerBase 的 _sell 接口"""
        self._broker.sell(market, code, price, num, stoploss, goal_price, part_from)

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
    """用于测试的订单代理，仅在执行买入/卖出时打印信息"""

    def __init__(self):
        pass

    def buy(self, market, code, price, num, stoploss, goal_price, part_from):
        print(f"买入：{market}{code}, 价格: {price}, 数量: {num} 预期止损价: {stoploss}, 预期目标价: {goal_price}, 信号来源: {part_from}")

    def sell(self, market, code, price, num, stoploss, goal_price, part_from):
        print(f"卖出：{market}{code}, 价格: {price}, 数量: {num}, 信号来源: {part_from}")


def crtOB(broker, name="NO_NAME"):
    """
    快速生成订单代理包装对象

    :param broker: 订单代理示例，必须拥有buy和sell方法，并且参数为 code, price, num
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，
                        否则忽略; 对卖出操作无效，立即以当前价卖出
    """
    return OrderBrokerWrap(broker, name)
