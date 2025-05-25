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

    .. py:attribute:: tm 当前交易账户
    .. py:attribute:: sp 移滑价差算法（回测时使用）

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


    .. py:method:: today(self)

        获取当前交易日日期（使用该方法而不是 Datatime.today(), 以便回测和实盘一直）

        :return: 当前交易日日期

    .. py:method:: now(self)   

        获取当前时间（使用该方法而不是 Datatime.now(), 以便回测和实盘一直）

        :return: 当前时间

    .. py:method:: next_datetime(self)

        下一交易时间点（回测使用）

    .. py:method:: get_last_kdata(self, stk, start_date, ktype, recover_type)

        获取指定证券从指定日期开始到当前时间的对应K线数据(为保证实盘和回测一致，请使用本方法获取K线数据)

        或 指定当前能获取到的最后 last_num 条 K线数据(为保证实盘和回测一致，请使用本方法获取K线数据)

        :param Stock stk: 指定的证券
        :param Datetime start_date: 开始日期  (或为 int 类型，表示从当前日期往前推多少个交易日)
        :param KQuery.KType ktype: K线类型
        :param KQuery.RecoverType recover_type: 恢复方式
        :return: K线数据
        :rtype: KData

    .. py:method:: get_kdata(self, stk, start_date, end_date, ktype, recover_type)

        获取指定证券指定日期范围内的K线数据(为保证实盘和回测一致，请使用本方法获取K线数据)

        :param Stock stk: 指定的证券
        :param Datetime start_date: 开始日期
        :param Datetime end_date: 结束日期
        :param KQuery.KType ktype: K线类型
        :param KQuery.RecoverType recover_type: 恢复方式
        :return: K线数据
        :rtype: KData

    .. py:method:: order(self, stk, num, remark='')

        按数量下单（正数为买入，负数为卖出）

        :param Stock stk: 指定的证券
        :param int num: 下单数量
        :param str remark: 下单备注

    .. py:method:: order_value(self, stk, value, remark='')

        按预期的证劵市值下单，即希望买入多少钱的证券（正数为买入，负数为卖出）

        :param Stock stk: 指定的证券
        :param float value: 投入买入资金
        :param str remark: 下单备注

    
