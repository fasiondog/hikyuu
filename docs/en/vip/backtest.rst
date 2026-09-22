Event-driven Backtest
=====================

Event-driven strategy backtest.

.. note:: 

    backtest is a supplement to the SYS and PF backtests, but the SYS and PF backtests differ in principle from the event backtest; do not test SYS and PF in the event backtest.


**Important notes**

.. note::

    In the on_bar(stg) function, to unify the backtest and live trading, **the following functions must not be used directly, otherwise the backtest will use future data**:
    
        - Datetime.now() and Datetime.today(); use stg.now() and stg.today() instead.
        - stock.get_kdata; use stg.get_kdata and stg.get_last_kdata instead.




.. py:function:: backtest([context], on_bar, tm, start_date, end_date, ktype, ref_market, mode, support_short, sp)

    Event-driven backtest; usually the body function in the Strategy is tested directly.

    If hikyuu has already loaded the data, the context parameter can be omitted. Otherwise, similar to Strategy, the context parameter needs to be passed in actively;
    the context contains the stock codes to load, the K-line types, the number of K-lines, the K-line start date and other information.
      
    :param StrategyContext context: the strategy context (in an environment where load_hikyuu has already been used, context can be omitted)
    :param func on_bar: the strategy body execution function, e.g.: on_bar(stg: Strategy)
    :param TradeManager tm: the strategy test account
    :param Datetime start_date: the start date
    :param Datetime end_date: the end date (exclusive)
    :param Query.KType ktype: the K-line type (the test is executed bar by bar with this type)
    :param str ref_market: the market it belongs to
    :param int mode: 0 - execute the buy/sell operations with the close price of the current bar; 1 - execute the buy/sell operations with the open price of the next bar
    :param support_short: whether short selling is supported
    :param Slippage sp: the slippage algorithm


Example 1 (in an environment where the data has been loaded with load_hikyuu, the context parameter can be omitted)

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


Example 2 (using StrategyContext)

.. note::

    In this way, a stock object such as sm['sz000001'] cannot be obtained outside stg, but only inside stg. To obtain it outside stg, see example 3.

::

    from hikyuu import *


    class Config:
        ktype = Query.DAY
        stock = 'sz000001'  # Note that sm['sz000001'] cannot be used here
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

        # Live trading
        # s.run_daily(my_func2, Minutes(1))  # , ignore_market=True)
        # s.start()

        # Backtest
        start_date = Datetime(2024, 1, 1)
        end_date = Datetime(2025, 1, 1)
        
        # In this way, the actual stk cannot be obtained here!!!
        # stk = sm['sz000001']
        # k = stk.get_kdata(Query(start_date, end_date, ktype=Config.ktype))

        tm = crtTM()
        backtest(s.context, on_bar, tm, start_date, end_date, Config.ktype)

        tm.performance(Query(start_date, end_date, Config.ktype))
        from matplotlib import pyplot as plt
        plt.show()


Example 3 (using the load_hikyuu loading function)

This way is generally used for backtesting or debugging.

::

    from hikyuu import *


    class Config:
        ktype = Query.DAY
        stock = 'sz000001'  # Note that sm['sz000001'] cannot be used here
        ma1 = MA(CLOSE(), 10)
        ma2 = MA(CLOSE(), 30)


    def on_bar(stg: Strategy):
        stk = sm[Config.stock]
        k = stg.get_last_kdata(stk, 100, Config.ktype)
        # hku_info("{}, 当前价: {:<.2f}", stg.today(), k[-1].close)
        if len(k) < 30 or k[-1].datetime != stg.today():
            return
        ind = CROSS(Config.ma1, Config.ma2)(k)
        if ind[-1] >= 1 and not stg.tm.have(stk):
            hku_info("{} triggers a buy", stg.today())
            stg.buy(stk, k[-1].close, 100)
            hku_info("{}", stg.tm.get_position(stg.today(), stk))
        elif ind[-1] < 1 and stg.tm.have(stk):
            stg.sell(stk, k[-1].close, 100)


    if __name__ == '__main__':
        import os
        import sys
        if sys.platform == 'win32':
            os.system('chcp 65001')

        options = {
            "stock_list": [Config.stock],
            "ktype_list": [Config.ktype],
            "load_history_finance": False,
            "load_weight": False,
            "start_spot": False
        }
        load_hikyuu(**options)

        s = Strategy()

        # Live trading
        # s.run_daily(my_func2, Minutes(1))  # , ignore_market=True)
        # s.start()

        # Backtest
        start_date = Datetime(2023, 1, 1)
        end_date = Datetime(2025, 1, 1)
        print(sm['sz000001'])

        tm = crtTM()
        backtest(on_bar, tm, start_date, end_date, Config.ktype)

        tm.performance(Query(start_date, end_date, Config.ktype))
        from matplotlib import pyplot as plt
        plt.show()
