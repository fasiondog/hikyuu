#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-01-30
#    Author: fasiondog

from hikyuu import Datetime, hku_info


class EasyTraderOrderBroker:
    def __init__(self, user):
        self.user = user
        self.buffer = {}

    def buy(self, code, price, num, stoploss, goal_price, part_from):
        self.user.buy(code[2:], price=price, amount=num)
        print("买入：%s  %.3f  %i" % (code, price, num))
        self.buffer[code] = (num, stoploss, goal_price)

    def sell(self, code, price, num, stoploss, goal_price, part_from):
        self.user.sell(code[2:], price=price, amount=num)
        print("卖出：%s  %.3f  %i" % (code, price, num))
        if code in self.buffer:
            old_num = self.buffer[code][0]
            if old_num == num:
                self.buffer.pop(code)
            else:
                self.buffer[code] = (old_num - num, stoploss, goal_price)

    def get_asset_info(self):
        '''以下只适用于华泰'''
        balance = self.user.balance
        cash = balance['可用金额']

        positions = []
        for v in self.user.position:
            if v["市场类别"] == "上海Ａ":
                market = "SH"
            elif v["市场类别"] == "深圳Ａ":
                market = "SZ"
            else:
                hku_info(f"Ignored not supported market: {v['市场类别']}")
                continue

            code = v["证券代码"]
            market_code = f"{market}{code}"
            if market_code in self.buffer:
                stoploss, goal_price = self.buffer[market_code]
            else:
                stoploss, goal_price = 0.0, 0.0
            positions.append(dict(market=market, code=code,
                             number=v['可用余额'], stoploss=stoploss, goal_price=goal_price, cost_price=v['成本价']))

        return dict(datetime=str(Datetime.now()), cash=cash, positions=positions)
