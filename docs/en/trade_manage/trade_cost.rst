.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Trade Cost Algorithms
=====================

Built-in Trade Cost Algorithms
------------------------------

Zero Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: TC_Zero()

    :return: an instance of a :py:class:`TradeCostBase` subclass


Shanghai-Shenzhen A-share Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* TC_FixedA is the rolling default cost model and always reflects the latest-year rates; the year-named models (TC_FixedAPre2015 / 2015 / 2017) stay frozen for historical backtests.
* Since 2022-04-29 the transfer fee is 0.01‰ of turnover (both directions, Shanghai, Shenzhen and Beijing); since 2023-08-28 the sell-side stamp duty is 0.5‰.

.. py:function:: TC_FixedA([commission=0.0018, lowest_commission=5.0, stamptax=0.0005, transferfee=0.00001])

    The current A-share trade cost algorithm (rolling default, always the latest rates): transfer fee 0.01‰ of turnover charged in both directions on all markets, sell-side stamp duty 0.5‰

    :param float commission: the commission rate
    :param float lowest_commission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :return: an instance of a :py:class:`TradeCostBase` subclass

.. py:function:: TC_FixedA2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    The A-share trade cost algorithm for August 1, 2015 and later; the Shanghai transfer fee was changed to 0.002% (0.02‰) of turnover

    :param float commission: the commission rate
    :param float lowest_commission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :return: an instance of a :py:class:`TradeCostBase` subclass

.. py:function:: TC_FixedA2017([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    The A-share trade cost algorithm for January 1, 2017 and later; the Shenzhen transfer fee is listed separately and both markets are charged 0.02‰ of turnover in both directions

    :param float commission: the commission rate
    :param float lowest_commission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :return: an instance of a :py:class:`TradeCostBase` subclass

.. py:function:: TC_FixedAPre2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.001, lowest_transferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015: the Shanghai transfer fee was 0.1% of the traded share quantity (1 yuan per 1000 shares, minimum 1 yuan), and the Shenzhen market charged no transfer fee

    :param float commission: the commission rate
    :param float lowest_commission: the minimum commission
    :param float stamptax: the stamp duty rate
    :param float transferfee: the transfer fee rate
    :param float lowest_transferfee: the minimum transfer fee
    :return: an instance of a :py:class:`TradeCostBase` subclass


Margin Financing Interest Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The trading fees and the interest are two separate concerns: the A-share, the ETF and the general
models answer the trading fees only, while **the annualized rates of the margin financing and the
securities lending are answered by a dedicated wrapper**, so any cost model can be wrapped without
changing it.

.. py:function:: TC_MarginInterest([trade_cost=TC_Zero(), borrow_cash_rate=0.0, borrow_stock_rates="", default_borrow_stock_rate=0.0])

    Wrap another trade cost algorithm with the annualized rates of the margin financing and the
    securities lending: the trading fees are delegated to the wrapped algorithm and this one answers
    the two rates only. The rates are billed by the account settlement, prorated over the calendar
    days gone by since the last bill with the account ``annual_days`` parameter as the denominator,
    added to the debt itself instead of paid from the cash and written as ``BUSINESS_INTEREST``
    records, see :doc:`margin`

    Both rates are 0.0 by default, i.e. nothing is billed, so wrapping an algorithm without
    configuring a rate leaves the books exactly as they were.

    :param TradeCost trade_cost: the algorithm answering the trading fees, TC_Zero by default
    :param float borrow_cash_rate: the annualized interest rate of the borrowed cash, in [0, 1); 0.0
        bills nothing
    :param str borrow_stock_rates: the securities lending rate table, a ``"key:rate"`` list, e.g.
        ``"12:0.10, SHF:0.12"``; an all-digit key is a security type, any other key is a market code
    :param float default_borrow_stock_rate: the rate of a security matched by no key, in [0, 1);
        0.0 bills nothing
    :return: an instance of a :py:class:`TradeCostBase` subclass

.. code-block:: python

    # the current A-share fees plus a 6% financing rate and a per-market lending fee table
    cost = TC_MarginInterest(TC_FixedA(), borrow_cash_rate=0.06, borrow_stock_rates="SH:0.10, SZ:0.12")
    tm = crtTM(cost_func=cost, margin=MG_ByType("1:0.5"))


ETF Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^

The calculation rules are as follows:: python

    Buy: commission (minimum 5 yuan)
    Sell: commission (minimum 5 yuan)
    
    No stamp duty or transfer fee.

.. py:function:: TC_FixedETF([commission=0.0001, lowest_commission=5.0])

    The ETF trade cost algorithm; commission is charged on both the buy and sell sides, with no stamp duty or transfer fee

    :param float commission: the commission rate, defaulting to one ten-thousandth (0.01%)
    :param float lowest_commission: the minimum commission, defaulting to 5 yuan per trade
    :return: an instance of a :py:class:`TradeCostBase` subclass


Generic Fixed-rate Trade Cost Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For the futures, FX and crypto markets, i.e. any market without a dedicated cost model. The
calculation rules are as follows:

* Buy: max(commission * turnover, lowest_commission) + per_lot * number + others
* Sell: the same, plus stamptax * turnover

The turnover is price * number * contract multiplier, where the multiplier comes from the unit of
the security (1 for stocks, the contract size for futures), so one formula covers both. The per-lot
fee and the extra charges both land in CostRecord.others.

.. py:function:: TC_Fixed([commission=0.0, lowest_commission=0.0, stamptax=0.0, per_lot=0.0, others=0.0])

    The generic fixed-rate trade cost algorithm for the futures, FX and crypto markets

    :param float commission: the commission rate of the turnover
    :param float lowest_commission: the minimum commission, applied as
        ``max(rate × turnover, minimum)`` — it is charged even when rate × turnover is zero;
        set it to 0 to waive the minimum
    :param float stamptax: the stamp duty rate, charged on sells only
    :param float per_lot: the fixed fee charged per traded unit (share / contract / coin),
        not per lot — it applies as ``per_lot × number``; divide the fee by the lot size to
        charge per lot
    :param float others: the fixed extra charge, e.g. the delivery and funding fees
    :return: an instance of a :py:class:`TradeCostBase` subclass


Custom Trade Cost Algorithm
---------------------------

The custom trade cost algorithm interface:

* :py:meth:`TradeCostBase.get_buy_cost` - [Required] Get the buy cost
* :py:meth:`TradeCostBase.get_sell_cost` - [Required] Get the sell cost
* :py:meth:`TradeCostBase.get_borrow_cash_cost` - [Optional] Get the cost of borrowing cash (margin financing)
* :py:meth:`TradeCostBase.get_return_cash_cost` - [Optional] Get the cost of returning the borrowed cash
* :py:meth:`TradeCostBase.get_borrow_stock_cost` - [Optional] Get the cost of borrowing stock (short selling)
* :py:meth:`TradeCostBase.get_return_stock_cost` - [Optional] Get the cost of returning the borrowed stock
* :py:meth:`TradeCostBase._clone` - [Required] The subclass clone interface

.. note::

    The names above are the ones a Python subclass has to implement: the C++ account code looks the override up by
    them and calls back into it. The binding used to look the overrides up by their C++ names (``getBuyCost`` and so
    on), which disagreed with this list, so an override written as listed here was silently ignored; the lookup is
    now on the snake_case names, and old code that defined the C++ names has to be renamed.



Trade Cost Algorithm Base Class
-------------------------------

.. py:class:: TradeCostBase(name)

    Base class for trade cost algorithms
    
    .. py:attribute:: name Name
        
    .. py:method:: get_param(self, name)

        Get the specified parameter
    
        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter
        
    .. py:method:: set_param(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: clone(self)
    
        Clone the instance

    .. py:method:: get_buy_cost(self, datetime, stock, price, num)
    
        [Override hook] Compute the buy cost
        
        :param Datetime datetime: the buy moment
        :param Stock stock: the security bought
        :param float price: the buy price
        :param int num: the number of shares bought
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_sell_cost(self, datetime, stock, price, num)
    
        [Override hook] Compute the sell cost
        
        :param Datetime datetime: the sell moment
        :param Stock stock: the security sold
        :param float price: the sell price
        :param int num: the number of shares sold
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_borrow_cash_cost(self, datetime, cash)
    
        [Override hook] Compute the cost of borrowing cash
        
        :param Datetime datetime: the borrow moment
        :param float cash: the borrowed cash
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_return_cash_cost(self, borrow_datetime, return_datetime, cash)
    
        [Override hook] Compute the cost of returning the borrowed cash
        
        :param Datetime borrow_datetime: the borrow moment
        :param Datetime return_datetime: the return moment
        :param float cash: the returned cash
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_borrow_stock_cost(self, datetime, stock, price, num)
    
        [Override hook] Compute the cost of borrowing stock
        
        :param Datetime datetime: the borrow moment
        :param Stock stock: the borrowed security
        :param float price: the borrow price
        :param int num: the borrowed quantity
        :return: the trade cost record
        :rtype: CostRecord
    
    .. py:method:: get_return_stock_cost(self, borrow_datetime, return_datetime, stock, price, num)
    
        [Override hook] Compute the cost of returning the borrowed stock
        
        :param Datetime borrow_datetime: the borrow moment
        :param Datetime return_datetime: the return moment
        :param Stock stock: the returned security
        :param float price: the return price
        :param int num: the returned quantity
        :return: the trade cost record
        :rtype: CostRecord

    .. py:method:: get_borrow_cash_rate(self, datetime)

        [Override hook] The annualized interest rate of the borrowed cash, 0.0 by default
        
        The rate is billed by the account settlement, prorated over the calendar days gone by since
        the last bill with its ``annual_days`` parameter as the denominator, added to the debt itself
        instead of paid from the cash, and written as a
        ``BUSINESS_INTEREST`` record; see :doc:`margin`. The built-in implementation is
        :py:func:`TC_MarginInterest`

        :param Datetime datetime: the settled moment
        :return: an annualized decimal rate, e.g. 0.06 for 6% per year
        :rtype: float

    .. py:method:: get_borrow_stock_rate(self, datetime, stock)

        [Override hook] The annualized fee rate of the borrowed stock, 0.0 by default

        It is billed on the market value of the borrowed shares at the settlement price and joins
        the cash debt the same way. The built-in implementation is :py:func:`TC_MarginInterest`

        :param Datetime datetime: the settled moment
        :param Stock stock: the borrowed security
        :return: an annualized decimal rate, e.g. 0.10 for 10% per year
        :rtype: float

    .. py:method:: _clone(self)
    
        [Override hook] Subclass clone interface
