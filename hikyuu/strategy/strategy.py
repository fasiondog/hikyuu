#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

from hikyuu import Strategy, Query, Datetime, TimeDelta, Seconds, Minutes
from hikyuu import StockManager


def on_change(stk, spot):
    print(stk, spot.bid1, spot.ask1)


def on_spot(rev_time):
    print("rev_time:", rev_time)


def my_func():
    sm = StockManager.instance()
    print("{}".format(len(sm)))
    for s in sm:
        print(s)


if __name__ == '__main__':
    s = Strategy(['sh600000', 'sz000001', 'xxx'],  [Query.MIN, Query.DAY])
    # s.run_daily_at(my_func, Datetime.now() - Datetime.today() + Seconds(5))
    # s.on_change(on_change)
    s.on_received_spot(on_spot)
    # s.run_daily(my_func, Minutes(1))
    s.start()
