backtest
================

事件驱动式策略回测

.. note:: 

    backtest 为 SYS 和 PF 回测的补充，但 SYS 和 PF 回测与事件回测原理不同，不要在事件回测中进行对 SYS 和 PF 的测试。


**重要注意事项**

.. note::

    on_bar(stg) 函数中，为了回测和实盘一体，**不能直接使用以下函数， 否则回测中使用的将是未来数据**：
    
        - Datetime.now() 和 Datetime.today()，而需要使用 stg.now() 和 today()。
        - stock.get_kdata, 使用 stg.get_kdata, stg.get_last_kdata




.. py:function:: backtest([context], on_bar, tm, start_date, end_date, ktype, ref_market, mode)

    事件驱动式回测, 通常直接测试 Strategy 中的主体函数

    如果 hikyuu 已经加载数据，可以忽略 context 参数。否则通 Strategy 类似，需要主动传入 context 参数，
    context 中包含需要加载的股票代码、K线类型、K线数量、K线起始日期等信息。
      
    :param StrategyContext context: 策略上下文 (在已经使用 load_hikyuu 的环境下，context可省略)
    :param func on_bar: 策略主体执行函数, 如: on_bar(stg: Strategy)
    :param TradeManager tm: 策略测试账户
    :param Datetime start_date: 起始日期
    :param Datetime end_date: 结束日期（不包含其本身）
    :param Query.KType ktype: K线类型(按该类型逐 Bar 执行测试)
    :param str ref_market: 所属市场
    :param mode 模式  0: 当前bar收盘价执行买卖操作; 1: 下一bar开盘价执行买卖操作


示例1 (在已使用 load_hikyuu 加载数据的环境下，可省略 context 参数)

::

    from matplotlib import pyplot as plt
    from hikyuu import *

    options = {
        "stock_list": ['sz000001'],
        "ktype_list": ['min'],
        "preload_num": {"min_max": 100000},
        "load_history_finance": False,
        "load_weight": False,
        "start_spot": False
    }
    load_hikyuu(**options)

    ma1 = MA(CLOSE(), 10)
    ma2 = MA(CLOSE(), 30)
    stk = sm['sz000001']


    def on_bar(stg: Strategy):
        k = stg.get_last_kdata(stk, 30, Query.MIN)
        if len(k) < 30 or k[-1].datetime != stg.today():
            # print("1")
            return
        ind = CROSS(ma1, ma2)(k)
        if ind[-1] >= 1 and not stg.tm.have(stk):
            stg.buy(stk, k[-1].close, 100)
        elif ind[-1] < 1 and stg.tm.have(stk):
            stg.sell(stk, k[-1].close, 100)


    start_date = Datetime(2024, 1, 1)
    end_date = Datetime(2025, 1, 1)
    stk = sm['sz000001']
    k = stk.get_kdata(Query(start_date, end_date, ktype=Query.MIN))

    tm = crtTM()
    backtest(on_bar, tm, start_date, end_date, Query.MIN)

    tm.performance(Query(start_date, end_date, Query.MIN))
    plt.show()


示例2 (使用 StrategyContext)

::

    from hikyuu import *


    class Config:
        ktype = Query.DAY
        stock = 'sz000001'  # 注意此时不能使用 sm['sz000001']
        ma1 = MA(CLOSE(), 10)
        ma2 = MA(CLOSE(), 30)


    def on_bar(stg: Strategy):
        stk = sm[Config.stock]
        k = stg.get_last_kdata(stk, 30, Config.ktype)
        if len(k) < 30 or k[-1].datetime != stg.today():
            return
        ind = CROSS(Config.ma1, Config.ma2)(k)
        if ind[-1] >= 1 and not stg.tm.have(stk):
            stg.buy(stk, k[-1].close, 100)
        elif ind[-1] < 1 and stg.tm.have(stk):
            stg.sell(stk, k[-1].close, 100)


    if __name__ == '__main__':

        s = Strategy(['sz000001'],  [Query.DAY])

        # 实盘
        # s.run_daily(my_func2, Minutes(1))  # , ignore_market=True)
        # s.start()

        # 回测
        start_date = Datetime(2024, 1, 1)
        end_date = Datetime(2025, 1, 1)
        stk = sm['sz000001']
        k = stk.get_kdata(Query(start_date, end_date, ktype=Config.ktype))

        tm = crtTM()
        backtest(s.context, on_bar, tm, start_date, end_date, Config.ktype)

        tm.performance(Query(start_date, end_date, Config.ktype))
        from matplotlib import pyplot as plt
        plt.show()