#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-01-30
#    Author: fasiondog

from hikyuu import Datetime, hku_info


class EasyTraderOrderBroker:
    '''
    使用华泰客户端实例
    注意：buy|sell 中已屏蔽实际通过easytrade下单，防止调试误操作，请自行根据需要打开
    '''

    def __init__(self, user):
        self.user = user
        self.buffer = {}

    def buy(self, market, code, price, num, stoploss, goal_price, part_from):
        # self.user.buy(code, price=price, amount=num)
        market_code = f"{market}{code}"
        print(f"计划买入：{market_code}  {price}  {num}")
        self.buffer[market_code] = (num, stoploss, goal_price)

    def sell(self, market, code, price, num, stoploss, goal_price, part_from):
        # self.user.sell(code, price=price, amount=num)
        market_code = f"{market}{code}"
        print(f"计划卖出：{market_code}  {price}  {num}")
        if market_code in self.buffer:
            old_num = self.buffer[market_code][0]
            if old_num == num:
                self.buffer.pop(market_code)
            else:
                self.buffer[market_code] = (old_num - num, stoploss, goal_price)

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

        ret = dict(datetime=str(Datetime.now()), cash=cash, positions=positions)
        print(ret)
        return ret
