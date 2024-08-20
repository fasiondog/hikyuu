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
        balance = self.user.balance
        cash = 0.0
        for item in balance:
            cash += item['可用资金']

        positions = []
        for v in self.user.position:
            if v["交易市场"] == "沪A":
                market = "SH"
            elif v["交易市场"] == "深A":
                market = "SZ"
            else:
                hku_info(f"Ignored not supported market: {v['交易市场']}")
                continue

            code = v["证券代码"]
            market_code = f"{market}{code}"
            if market_code in self.buffer:
                stoploss, goal_price = self.buffer[market_code]
            else:
                stoploss, goal_price = 0.0, 0.0
            positions.append(dict(market=market, code=code, number=(
                v['当前持仓'] + v['买入冻结'] - v['卖出冻结']), stoploss=stoploss, goal_price=goal_price))

        return dict(datetime=str(Datetime.now()), cash=cash, positions=positions)
