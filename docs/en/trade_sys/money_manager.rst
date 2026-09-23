.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Money Management Strategies | MM
================================

The money management (MM) part performs position sizing: given a buy or
sell signal, it decides the share quantity (position size) to trade.

Common parameters:

    * **auto-checkin=False** *(bool)* : When the account does not hold enough cash to cover the share quantity requested by the money management strategy, automatically deposit (check in) sufficient cash into the account.
    * **max-stock=20000** *(int)* : Maximum number of distinct securities held simultaneously (i.e. how many instruments are held, not the share quantity of any single position)
    * **disable_ev_force_clean_position=False** *(bool)* : Disable forced liquidation when the market environment (EV) becomes invalid
    * **disable_cn_force_clean_position=False** *(bool)* : Disable forced liquidation when the system validity condition (CN) becomes invalid


Built-in Money Management Strategies
------------------------------------

No Money Management
^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_Nothing()

    A pass-through money management strategy that performs no position sizing: buy as many shares as the available cash allows.


Fixed Share-Quantity Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    Fixed share-quantity position sizing: buy the same fixed share quantity on every entry.

    :param float n: the share quantity bought on each entry (it should be an integer multiple of the instrument's board lot, i.e. its minimum trade size; the program does not validate this here)
    :return: the money management strategy instance

.. py:function:: MM_FixedCountTps([buy_counts, sell_counts])

    Position sizing that buys or sells a fixed share quantity on each consecutive order in a scale-in/scale-out sequence.

    :param list buy_counts: the list of share quantities for consecutive buys (scaling in)
    :param list sell_counts: the list of share quantities for consecutive sells (scaling out)
    :return: the money management strategy instance


Fixed-Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedRisk([risk = 1000.00])

    Fixed-risk position sizing limits the capital put at risk on each trade to a predetermined, fixed amount—for example, 1,000 yuan per trade. Formula: position size = fixed risk ÷ per-share trade risk.

    :param float risk: the fixed risk amount per trade
    :return: the money management strategy instance


Fixed-Capital Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCapital([capital = 10000.0])

    Fixed-capital position sizing, i.e. controlling the total capital committed to each buy. Position size = current cash ÷ capital.

    :param float capital: the fixed capital unit per buy
    :return: the money management strategy instance

.. py:function:: MM_FixedCapitalFunds([capital = 10000.0])

    Fixed-capital position sizing based on total assets. Position size = current total assets ÷ capital.

    :param float capital: the fixed capital unit per buy
    :return: the money management strategy instance


Fixed-Units Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    Fixed-units position sizing. Formula: position size = current cash ÷ n ÷ current per-share risk.

    :param int n: the number of equal capital units the cash is divided into
    :return: the money management strategy instance


Williams Fixed-Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    Williams' fixed-risk position sizing; position size = (account balance × risk fraction p) ÷ maximum per-share loss (max_loss)

    :param float p: the risk fraction of the account balance
    :param float max_loss: the maximum permissible per-share loss
    :return: the money management strategy instance


Fixed-Percentage Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    The fixed-percentage risk model. Formula: P (position size) = account balance × percentage ÷ R (the per-share trade risk). [BOOK3]_, [BOOK4]_ .

    :param float p: the fraction of the account balance risked per trade
    :return: the money management strategy instance


Fixed-Volatility Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



Custom Money Management Strategies
----------------------------------

The custom money management strategy interface:

* :py:meth:`MoneyManagerBase.buyNotify` - [Optional] Receive notification of an actual buy fill; reserved for strategies that handle repeated scaling in/out
* :py:meth:`MoneyManagerBase.sellNotify` - [Optional] Receive notification of an actual sell fill; reserved for strategies that handle repeated scaling in/out
* :py:meth:`MoneyManagerBase._getBuyNumber` - [Required] Return the share quantity that can be bought for the specified instrument
* :py:meth:`MoneyManagerBase._getSellNumber` - [Optional] Return the share quantity that can be sold for the specified instrument; if not overridden, it defaults to liquidating the entire held position
* :py:meth:`MoneyManagerBase._reset` - [Optional] Reset private attributes
* :py:meth:`MoneyManagerBase._clone` - [Required] The clone interface

Money Management Strategy Base Class
------------------------------------

.. py:class:: MoneyManagerBase

    Base class for money management (position sizing) strategies.

    .. py:attribute:: name  The strategy name
    .. py:attribute:: tm    Set or get the trade manager instance
    .. py:attribute:: query Set or get the query conditions

    .. py:method:: __init__(self[, name="MoneyManagerBase])

        Initialization constructor.

        :param str name: the strategy name

    .. py:method:: get_param(self, name)

        Get the specified parameter.

        .. note::

            Every money management strategy has the "auto-checkin" parameter (bool, defaults to False), which means "when the account does not hold enough cash to cover the share quantity requested by the money management strategy, automatically deposit (check in) sufficient cash into the account."

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: raised when no such parameter exists

    .. py:method:: set_param(self, name, value)

        Set a parameter.

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: unsupported parameter type

    .. py:method:: reset(self)

        Reset the strategy to its initial state.

    .. py:method:: clone(self)

        Return a clone (copy) of the strategy.

    .. py:method:: get_buy_num(self, datetime, stock, price, risk, part_from)

        Return the share quantity that can be bought for the specified instrument.

        :param Datetime datetime: the trade time
        :param Stock stock: the instrument (security)
        :param float price: the trade price
        :param float risk: the per-share risk assumed by the trade; a value of 0 denotes a total loss, i.e. the market value falling to zero
        :param System.Part part_from: the originating system part
        :return: the buyable share quantity
        :rtype: float

    .. py:method:: get_sell_num(self, datetime, stock, price, risk, part_from)

        Return the share quantity that can be sold for the specified instrument.

        :param Datetime datetime: the trade time
        :param Stock stock: the instrument (security)
        :param float price: the trade price
        :param float risk: the per-share risk assumed by the new trade; a value of 0 denotes a total loss, i.e. the market value falling to zero
        :param System.Part part_from: the originating system part
        :return: the sellable share quantity
        :rtype: float

    .. py:method:: current_buy_count(self, stock)

        The current consecutive buy count for the given instrument.

    .. py:method:: current_sell_count(self, stock)

        The current consecutive sell count for the given instrument.

    .. py:method:: _buy_notify(self, trade_record)

        [Override hook] Called when the trading system executes an actual buy, notifying the strategy of the resulting position change; it generally needs to be overridden only when the strategy scales in or out in multiple steps.

        :param TradeRecord trade_record: the trade record of the actual buy when the fill occurs

    .. py:method:: _sell_notify(self, trade_record)

        [Override hook] Called when the trading system executes an actual sell, notifying the strategy of the resulting position change; it generally needs to be overridden only when the strategy scales in or out in multiple steps.

        :param TradeRecord trade_record: the trade record of the actual sell when the fill occurs

    .. py:method:: _get_buy_num(self, datetime, stock, price, risk, part_from)

        [Override hook] Return the share quantity that can be bought for the specified instrument.

        :param Datetime datetime: the trade time
        :param Stock stock: the instrument (security)
        :param float price: the trade price
        :param float risk: the per-share risk assumed by the trade; a value of 0 denotes a total loss, i.e. the market value falling to zero
        :param System.Part part_from: the originating system part
        :return: the buyable share quantity
        :rtype: float

    .. py:method:: _get_sell_num(self, datetime, stock, price, risk, part_from)

        [Override hook] Return the share quantity that can be sold for the specified instrument. If not overridden, it defaults to liquidating the entire held position.

        :param Datetime datetime: the trade time
        :param Stock stock: the instrument (security)
        :param float price: the trade price
        :param float risk: the per-share risk assumed by the new trade; a value of 0 denotes a total loss, i.e. the market value falling to zero
        :param System.Part part_from: the originating system part
        :return: the sellable share quantity
        :rtype: float

    .. py:method:: _reset(self)

        [Override hook] The subclass reset hook, resetting internal private variables.

    .. py:method:: _clone(self)

        [Override hook] The subclass clone hook.
