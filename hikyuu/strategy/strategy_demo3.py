#!/usr/bin/python
# -*- coding: utf8 -*-

#
# 警告：Hikyuu 为量化研究工具，本身不包含程序化交易接口。此部分仅为策略调度运行时示例，
#      供自行实现程序化交易时参考，请自行负责程序化交易可能造成的损失。
#

if __name__ == '__main__':
    import easytrader
    from hikyuu import *

    # 创建 easytrade 订单代理（示例仅支持华泰）可自行参照 EasyTraderOrderBroker 修改
    user = easytrader.use('ht_client')
    user.connect(r'D:\htwt\xiadan.exe')
    easy_ob = EasyTraderOrderBroker(user)
    broker = crtOB(easy_ob)

    sys = get_part("default.sys.趋势双均线")

    # 直接使用 sys 创建 strategy 示例，如果为日线，则自动每日 14点55分 执行
    # 如果 query 为日线以下（分钟线、5分钟线）则自动按对应间隔（分钟、5分钟）循环执行
    stg = crt_sys_strategy(sys, "sz000001", Query(Datetime(20240101), ktype=Query.DAY), broker, TC_Zero(), "demo3")
    stg.start()
