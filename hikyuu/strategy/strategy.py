#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

from hikyuu import StrategyBase, Query, Datetime, TimeDelta, Seconds, Minutes
from hikyuu import StockManager


class TestStrategy(StrategyBase):
    def __init__(self):
        super(self.__class__, self).__init__()
        self.stock_list = ['sh600000', 'sz000001']
        self.ktype_list = [Query.MIN, Query.DAY]

    def init(self):
        print("strategy init")

    def on_received_spot(self, rev_time):
        print("rev_time:", rev_time)


def my_func():
    sm = StockManager.instance()
    print("{}".format(len(sm)))
    for s in sm:
        print(s)


if __name__ == '__main__':
    s = TestStrategy()
    # s.run_daily_at(my_func, Datetime.now() - Datetime.today() + Seconds(5))
    # s.on_received_spot(Datetime.now())
    s.run_daily(my_func, Minutes(1))
    # s.start()
