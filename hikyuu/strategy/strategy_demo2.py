#!/usr/bin/python
# -*- coding: utf8 -*-

import easytrader
from hikyuu import *

#
# Warning: Hikyuu is a quantitative research tool and does not include a programmatic trading interface itself. This part is only an example of the strategy scheduling runtime,
#      for reference when implementing programmatic trading by yourself; you are responsible for the losses that programmatic trading may cause.
#

# Create the easytrade order broker (the example only supports Huatai); you can modify it by referring to EasyTraderOrderBroker
# The actual ordering through easytrader is blocked in buy|sell to prevent misoperations during debugging
user = easytrader.use('ht_client')
user.connect(r'D:\htwt\xiadan.exe')
easy_ob = EasyTraderOrderBroker(user)
broker = crtOB(easy_ob)

# Get the trading system strategy
# Currently run_in_strategy only supports non-delayed trading, i.e. buying/selling immediately at the close
sys = get_part("default.sys.趋势双均线")
sys.set_param("buy_delay", False)
sys.set_param("sell_delay", False)


# Run the strategy body
def my_func(stg: Strategy):
    # This example uses the TC_Zero() zero-cost algorithm, but in practice it is recommended to use a cost algorithm close to reality
    # because sys does not depend on the actual cost in the trade records, but uses the cost algorithm to estimate the number to buy
    run_in_strategy(sys, sm['sz000001'], Query(Datetime(20240101)), broker, TC_Zero())


# Note:
#   1. Each Strategy can only be executed as an independent process, i.e. run in the way of python xxx.py!
#   2. Please enable the HikyuuTdx quote collection, otherwise no data can be received
# An example of running in the Strategy way
if __name__ == '__main__':
    # When creating the strategy runtime, the stock and ktype lists must be specified
    # The strategy only loads the data of the specified stock and ktype, and quote receiving only updates these data
    # If the trading calendar is needed, remember to specify sh000001 at the same time
    s = Strategy(['sh000001', 'sz000001'],  [Query.DAY])

    # Executed at 14:55 on each trading day
    s.run_daily_at(my_func, TimeDelta(0, 14, 55))
    s.start()

    # For the above, you can also see strategy_demo3.py, which uses crt_sys_strategy to quickly create a strategy
