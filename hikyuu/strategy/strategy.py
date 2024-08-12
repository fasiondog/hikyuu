#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

from hikyuu import StrategyBase, Query, Datetime, TimeDelta
from hikyuu import StockManager


class TestStrategy(StrategyBase):
    def __init__(self):
        super(self.__class__, self).__init__()
        self.stock_list = ['sh600000', 'sz000001']
        self.ktype_list = [Query.MIN, Query.DAY]

    def init(self):
        print("strategy init")

    def on_bar(self, ktype):
        print("on bar {}".format(ktype))
        print("{}".format(len(StockManager.instance())))
        for s in self.sm:
            print(s)


def my_func():
    sm = StockManager.instance()
    print("{}".format(len(sm)))
    for s in sm:
        print(s)


if __name__ == '__main__':
    s = TestStrategy()
    s.run_daily_at(my_func, TimeDelta(0, 17, 6))
    s.start()
