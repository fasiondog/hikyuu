#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#
# Warning: Hikyuu is a quantitative research tool and does not include a programmatic trading interface itself. This part is only an example of the strategy scheduling runtime,
#      for reference when implementing programmatic trading by yourself; you are responsible for the losses that programmatic trading may cause.
#

from hikyuu import *


def on_change(stg: Strategy, stk: Stock, spot: SpotRecord):
    print("[on_change]:", stk.market_code, stk.name, spot.close, spot.bid[0], spot.ask[0])


def on_spot(stg: Strategy, rev_time: Datetime):
    print("[on_received_spot] rev_time:", rev_time)


def my_func1(stg: Strategy):
    print("[my_func1]", str(stg.now()))


def my_func2(stg: Strategy):
    print("[my_func2] calculate:", stg.now())
    for s in sm:
        print(s)


# Note:
#   1. Each Strategy can only be executed as an independent process, i.e. run in the way of python xxx.py!
#   2. Please enable the HikyuuTdx quote collection, otherwise no data can be received
# An example of running in the Strategy way
if __name__ == '__main__':
    # When creating the strategy runtime, the stock and ktype lists must be specified
    # The strategy only loads the data of the specified stock and ktype, and quote receiving only updates these data
    # If the trading calendar is needed, remember to specify sh000001 at the same time
    s = Strategy(['sh600000', 'sz000001'],  [Query.MIN, Query.DAY])

    # Currently it is executed automatically after a delay of 10/20 seconds, ignoring the holiday restriction
    s.run_daily_at(my_func1, Datetime.now() - Datetime.today() + Seconds(10), False)
    s.run_daily_at(my_func1, Datetime.now() - Datetime.today() + Seconds(20), False)

    # Received a quote update of the specified stock
    s.on_change(on_change)

    # Received a quote update
    s.on_received_spot(on_spot)

    # Loop once every 1 minute (ignore_market ignores the open/close time restriction, otherwise it runs only during the trading session)
    s.run_daily(my_func2, Minutes(1))  # , ignore_market=True)
    s.start()
