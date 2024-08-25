#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import easytrader
from hikyuu import *


# 创建 easytrade 订单代理（示例仅支持华泰）可自行参照 EasyTraderOrderBroker 修改
# buy|sell 中已屏蔽实际通过easytrade下单，防止调试误操作，请自行根据需要打开
user = easytrader.use('ht_client')
user.connect(r'D:\htwt\xiadan.exe')
easy_ob = EasyTraderOrderBroker(user)
broker = crtOB(easy_ob)

# 获取交易系统策略
# 目前 run_in_strategy 只支持非延迟交易，即收盘时立刻买入/卖出
sys = get_part("default.sys.趋势双均线")
sys.set_param("buy_delay", False)
sys.set_param("sell_delay", False)


# 执行策略主体
def my_func():
    run_in_strategy(sys, sm['sz000001'], Query(Datetime(20240101)), broker, TC_Zero())


# 注意：
#   1.每一个Strategy 只能作为独立进程执行，即 python xxx.py 的方式执行！
#   2.请开启 HikyuuTdx 行情采集，否则接收不到数据
# Strategy 方式运行示例
if __name__ == '__main__':
    # 创建策略运行时，必须指定 stock 和 ktype 列表
    # strategy 只会加载指定的 stock, ktype 的数据，行情接收也只会更新这些数据
    # 如需使用交易日历，请记得同时指定 sh000001
    s = Strategy(['sh000001', 'sz000001'],  [Query.DAY])

    # 每交易日 14点55分 执行
    s.run_daily_at(my_func, TimeDelta(0, 14, 55))
    s.start()
