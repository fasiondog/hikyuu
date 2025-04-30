.. currentmodule:: hikyuu
.. highlight:: python

实盘交易
=======================

程序化交易也就是自动化交易，也就是大家常见的各种量化框架，本质就是任务的定时调度 + 通知回调。

Hikyuu 主要聚焦于快速策略分析，本身不提供实盘交易，Strategy 运行时仅供大家学习参考如何和实盘进行对接，造成盈亏请自行负责。

具体可参见安装目录下的 strategy 子目录下的相关 demo。


公共参数：

    * **spot_worker_num=1** *(int)* : 接收行情数据时内部的线程数
    * **quotation_server=""** *(string)* : 指定行情服务地址，为空表示使用本机默认配置（hikyuu.ini）


.. py:class:: Strategy

    策略运行时

    .. py:attribute:: name 名称
    .. py:attribute:: context 策略上下文

    .. py:method:: start(self)

        启动策略执行，请在完成相关回调设置后执行。

        :param bool auto_recieve_spot: 是否自动接收行情数据    

    .. py:method:: on_change(self, func)

        设置证券数据更新回调通知

        :param func: 一个可调用的对象如普通函数，需接收 stock 和 ktype 参数

    .. py:method:: on_received_spot(self, func)

        设置证券数据更新通知回调

        :param func: 可调用对象如普通函数，没有参数

    .. py:method:: run_daily(self, func)
        
        设置日内循环执行回调。如果忽略市场开闭市，则自启动时刻开始按间隔时间循环，
        否则第一次执行时将开盘时间对齐时间间隔，且在非开市时间停止执行。

        :param func: 可调用对象如普通函数，没有参数
        :param TimeDelta time: 间隔时间，如间隔3秒：TimeDelta(0, 0, 0, 3) 或 Seconds(3)
        :param str market: 使用哪个市场的开闭市时间
        :param ignore_market: 忽略市场开闭市时间

    .. py:method:: run_daily_at(self, func)

        设置每日定点执行回调

        :param func: 可调用对象如普通函数，没有参数
        :param TimeDelta time: 执行时刻，如每日15点：TimeDelta(0, 15)
        :param ignore_holiday: 节假日不执行        