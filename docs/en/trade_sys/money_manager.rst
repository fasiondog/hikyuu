.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Money Management Strategies | MM
================================

The money management (MM) part performs position sizing: given a buy or
sell signal, it decides the share quantity (position size) to trade.

Common parameters:

    * **auto_checkin=False** *(bool)* : When the account does not hold enough cash to cover the share quantity requested by the money management strategy, automatically deposit (check in) sufficient cash into the account; the deposit happens once at the actual trade made by the System, the quantity query itself never changes the account.
    * **max_stock=20000** *(int)* : Maximum number of distinct securities held simultaneously (i.e. how many instruments are held, not the share quantity of any single position)
    * **disable_ev_force_clean_position=False** *(bool)* : Disable forced liquidation when the market environment (EV) becomes invalid
    * **disable_cn_force_clean_position=False** *(bool)* : Disable forced liquidation when the system validity condition (CN) becomes invalid

    .. note::

        All the parameter names are unified to the lowercase-underscore style (e.g. ``auto_checkin``,
        ``max_stock``, ``max_loss``). The early hyphenated names (``auto-checkin``, ``max-stock``)
        have been renamed; using an old name raises an exception.

    .. note::
        A money management instance keeps the consecutive buy/sell counters of every security
        (``current_buy_count`` / ``current_sell_count``, which the tier-based strategies such as
        :func:`MM_FixedCountTps` use to advance the tranche) and these counters are not locked. With
        ``shared_mm=True`` of the ``System``, multiple systems write the same instance concurrently,
        so a parallel backtest has a data race and corrupts the tranches; keep an instance per system
        (``shared_mm=False``, the default).


Built-in Money Management Strategies
------------------------------------

No Money Management
^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_Nothing()

    A pass-through money management strategy that performs no position sizing: buy as many shares as the available cash allows, and open a short as many shares as the net assets allows.

    Parameters (set via ``set_param``):

    * ``if_have_a_position_will_not_buy``: do not buy again when a long position is already held; default ``False``
    * ``if_have_short_will_not_sell_short``: do not open another short when a short position is already held; default ``False``


Fixed Share-Quantity Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedCount([n = 100])

    Fixed share-quantity position sizing: buy the same fixed share quantity on every entry.

    :param float n: the share quantity bought on each entry (it should be an integer multiple of the instrument's board lot, i.e. its minimum trade size; the program does not validate this here)
    :return: the money management strategy instance

.. py:function:: MM_FixedCountTps([buy_counts, sell_counts])

    Position sizing that buys or sells a fixed share quantity on each consecutive order in a scale-in/scale-out sequence.

    When the stop-loss is breached (the per-share risk is <= 0), the sell tiers are exhausted, or the sell quantity exceeds the current position (e.g. after an ex-rights adjustment), the whole remaining position is sold.

    Each tier quantity must be a finite, strictly positive number (NaN, Inf, 0 and negative values are all rejected); to trade fewer tiers, use a shorter list. buy_counts must not be empty (at least one tier is required).

    sell_counts may be empty: an empty list is treated as "sell tiers already exhausted", so the first sell signal liquidates the whole position.

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

    Fixed-capital position sizing, i.e. controlling the total capital committed to each buy. Position size = the available cash ÷ capital.

    :param float capital: the fixed capital unit per buy
    :return: the money management strategy instance

.. py:function:: MM_FixedCapitalFunds([capital = 10000.0])

    Fixed-capital position sizing based on the net assets. Position size = the current net assets ÷ capital. The base is the account net assets (cash + market value of the held positions - short market value - borrowed cash) instead of the available cash, suiting the proportional-to-equity position adding.

    :param float capital: the fixed capital unit per buy
    :return: the money management strategy instance


Fixed-Units Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedUnits([n = 33])

    Fixed-units position sizing. Formula: position size = max(the current cash, the initial capital) ÷ n ÷ the current per-share risk, i.e. the initial capital is used when the cash falls below it.

    :param int n: the number of equal capital units the cash is divided into
    :return: the money management strategy instance


Williams Fixed-Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    Williams' fixed-risk position sizing; position size = (the available cash × risk fraction p) ÷ maximum per-share loss (max_loss)

    :param float p: the risk fraction of the available cash
    :param float max_loss: the maximum permissible per-share loss
    :return: the money management strategy instance


Fixed-Percentage Risk Position Sizing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: MM_FixedPercent([p = 0.03])

    The fixed-percentage risk model. Formula: P (position size) = the available cash × percentage ÷ R (the per-share trade risk). [BOOK3]_, [BOOK4]_ .

    :param float p: the fraction of the available cash risked per trade
    :return: the money management strategy instance


Long/Short Trading Capability
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The long side (buy to open, sell to close) is supported by every strategy. The short side
defaults to "can cover but cannot open":

.. list-table::
   :header-rows: 1

   * - Strategy
     - Short entry (``_get_sell_short_num``)
     - Short cover (``_get_buy_short_num``)
   * - MM_FixedCount
     - sells in tranches of n
     - covers the entire position
   * - MM_FixedRisk / MM_FixedPercent / MM_FixedUnits
     - the same risk-budget semantics as the long buy (the risk is the short per-share risk, i.e.
       the stop-loss price minus the short entry price; MM_FixedPercent / MM_FixedUnits base it
       on the net assets)
     - covers the entire position
   * - MM_FixedCapital / MM_FixedCapitalFunds / MM_WilliamsFixedRisk
     - the same risk-budget semantics as the long buy (the risk is not used, based on the net
       assets)
     - covers the entire position
   * - MM_Nothing
     - sized by the net assets (the net assets ÷ (price × unit), the risk is not used)
     - covers the entire position
   * - MM_FixedCountTps
     - 0 (no short entry)
     - covers the entire position

.. note::

    The MM_FixedCountTps tranches are long-only, so it does not open shorts. To open shorts use
    the strategies above, or override
    ``_get_sell_short_num`` / ``_get_buy_short_num`` in a Python custom money management strategy.

    Opening a short requires the trade manager account with ``support_borrow_stock=True`` (the
    margin trading mode), otherwise the short order fails directly. The short quantity is aligned
    to the lot size and clamped to the maximum trade quantity like the buy; it is not constrained
    by the collateral — when the buying power is insufficient the trade manager rejects the whole
    order without scaling it down.

    The short side bases its size on the account net assets (cash + market value of the held
    positions - short market value - borrowed cash, the Van Tharp equity basis), so the borrowed
    proceeds never inflate the risk budget; the long side keeps the available cash base (equal to
    the equity with no position and more conservative otherwise).


Custom Money Management Strategies
----------------------------------

The custom money management strategy interface:

* :py:meth:`MoneyManagerBase._buy_notify` - [Optional] Receive notification of an actual buy fill; reserved for strategies that handle repeated scaling in/out
* :py:meth:`MoneyManagerBase._sell_notify` - [Optional] Receive notification of an actual sell fill; reserved for strategies that handle repeated scaling in/out
* :py:meth:`MoneyManagerBase._get_buy_num` - [Required] Return the share quantity that can be bought for the specified instrument
* :py:meth:`MoneyManagerBase._get_sell_num` - [Optional] Return the share quantity that can be sold for the specified instrument; if not overridden, it defaults to liquidating the entire held position
* :py:meth:`MoneyManagerBase._get_sell_short_num` - [Optional] Return the quantity that can be short sold for the specified instrument; if not overridden, it defaults to 0 (no short entry)
* :py:meth:`MoneyManagerBase._get_buy_short_num` - [Optional] Return the quantity that covers the short position of the specified instrument; if not overridden, it defaults to covering the entire short position
* :py:meth:`MoneyManagerBase._reset` - [Optional] Reset private attributes
* :py:meth:`MoneyManagerBase._clone` - [Required] The clone interface

Money Management Strategy Base Class
------------------------------------

.. py:class:: MoneyManagerBase

    Base class for money management (position sizing) strategies.

    .. py:attribute:: name  The strategy name
    .. py:attribute:: tm    Set or get the trade manager instance
    .. py:attribute:: query Set or get the query conditions

    .. py:attribute:: support_mult_buy_sell  Whether the strategy supports repeated scaling in/out (defaults to False)

        Only a Python custom money management strategy may set this property (assigning it on a built-in strategy raises an exception); the flag survives cloning. When False (the default), a non-positive risk (NaN included, i.e. the price has reached the stop loss) liquidates the whole position on the sell and the short cover directly; when True, the situation is left to the subclass hooks ``_get_sell_num`` / ``_get_buy_short_num``.

    .. py:method:: __init__(self[, name="MoneyManagerBase])

        Initialization constructor.

        :param str name: the strategy name

    .. py:method:: get_param(self, name)

        Get the specified parameter.

        .. note::

            Every money management strategy has the "auto_checkin" parameter (bool, defaults to False), which means "when the account does not hold enough cash to cover the share quantity requested by the money management strategy, automatically deposit (check in) sufficient cash into the account. The deposit happens once at the actual trade made by the System, the quantity query itself never changes the account."

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
        :param float risk: the per-share risk taken by the trade; it must be positive, a non-positive value (NaN included) is rejected and 0 is returned
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
        :param float risk: the per-share risk taken by the trade; it must be positive, a non-positive value (NaN included) is rejected and 0 is returned
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
