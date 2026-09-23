#!/usr/bin/python
# -*- coding: utf8 -*-

#
# Warning: Hikyuu is a quantitative research tool and does not include a programmatic trading interface itself. This part is only an example of the strategy scheduling runtime,
#      for reference when implementing programmatic trading by yourself; you are responsible for the losses that programmatic trading may cause.
#

if __name__ == '__main__':
    import easytrader
    from hikyuu import *

    # Create the easytrade order broker (the example only supports Huatai); you can modify it by referring to EasyTraderOrderBroker
    user = easytrader.use('ht_client')
    user.connect(r'D:\htwt\xiadan.exe')
    easy_ob = EasyTraderOrderBroker(user)
    broker = crtOB(easy_ob)

    sys = get_part("default.sys.趋势双均线")

    # An example of creating the strategy directly with sys; if it is daily, it is executed automatically at 14:55 every day
    # If the query is below daily (minute, 5-minute), it runs cyclically at the corresponding interval (minute, 5-minute)
    stg = crt_sys_strategy(sys, "sz000001", Query(Datetime(20240101), ktype=Query.DAY), broker, TC_Zero(), "demo3")
    stg.start()
