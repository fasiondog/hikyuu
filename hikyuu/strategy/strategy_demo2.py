#!/usr/bin/python
# -*- coding: utf8 -*-

import easytrader
from hikyuu import *

#
# 警告：Hikyuu 为量化研究工具，本身不包含程序化交易接口。此部分仅为策略调度运行时示例，
#      供自行实现程序化交易时参考，请自行负责程序化交易可能造成的损失。
#

# 创建 easytrade 订单代理（示例仅支持华泰）可自行参照 EasyTraderOrderBroker 修改
# buy|sell 中已屏蔽实际通过easytrade下单，防止调试误操作
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
    # 这里示例使用的是 TC_Zero() 零成本算法，但实际建议使用接近实际的成本算法
    # 因为 sys 不依赖于成交单中的实际成本，而是使用成本算法预算需要买入的数量
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

    # 上述，也可以参见 strategy_demo3.py 中使用 crt_sys_strategy 快捷创建 strategy
