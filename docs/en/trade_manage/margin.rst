.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Margin Model
============

A margin model answers "how much funds an opening occupies". It is separated from the trade cost
(fees and interest) and the account bookkeeping (cash and positions):

- **the margin model (this page)**: the funds an opening occupies, the maintain threshold, and
  whether a short needs borrowing the asset;
- **the trade cost** (see :doc:`trade_cost`): commission, stamp tax, transfer fee, the interest of
  the borrowed cash and the fee of the borrowed stock;
- **TradeManager**: the cash, positions, trades and equity records, with the order validation.

Built-in margin models
----------------------

.. py:function:: MG_None()

    The plain cash model: no leverage, an opening consumes the full nominal value and no position
    occupies margin

    :return: an instance of a :py:class:`MarginBase` subclass

.. py:function:: MG_Fixed([rate=0.6, rate_short=0.0, maintain_rate=0.0, margin_per_lot=0.0])

    The fixed margin model applied to every security of the account. Ratio mode:
    ``margin = rate * price * num * unit``; per-lot mode (``margin_per_lot > 0``):
    ``margin = margin_per_lot * ceil(num / minimum trade unit)``

    :param float rate: the opening margin ratio, in (0, 1]
    :param float rate_short: the short opening ratio; 0 means following rate
    :param float maintain_rate: the maintain margin ratio; 0 means the same as the opening margin;
        when non-zero it must be <= the opening ratio (a higher one would call the account at open)
    :param float margin_per_lot: the margin of one lot; 0 means using the ratio mode
    :return: an instance of a :py:class:`MarginBase` subclass

.. py:function:: MG_ByType([rates="", maintain_rate=0.0, default_rate=0.6])

    The margin model keyed by the security type or the market, for a mixed account (e.g. A-share
    cash positions plus futures margin)

    :param str rates: a list of ``key:ratio`` items separated by ``,`` ``;`` or blanks, e.g.
        ``"12:0.10, SHF:0.12"``; an all-digit key is a security type, any other key is a market
        code (compared upper-cased); a type hit wins over a market hit
    :param float maintain_rate: the maintain margin ratio; 0 means the same as the opening margin;
        when non-zero it must be <= the opening ratio (a higher one would call the account at open)
    :param float default_rate: the ratio used when no key matches, in (0, 1]; 1.0 also means the
        unmatched securities are plain cash positions and take no occupation
    :return: an instance of a :py:class:`MarginBase` subclass

.. note::

    The long and the short share one ratio. Use the ``rate_short`` of :py:func:`MG_Fixed`, or
    subclass :py:class:`MarginBase` and override :py:meth:`MarginBase.get_open_margin` in Python,
    when they must differ.

Custom margin model
-------------------

.. py:class:: MarginBase(name)

    The margin model base class

    .. py:attribute:: name

        (Read-only) the model name

    .. py:method:: get_param(self, name)

        Get the specified parameter

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter

    .. py:method:: set_param(self, name, value)

        Set a parameter (parameters are configuration-time values, read-only while running)

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type!

    .. py:method:: clone(self)

        The clone operation

    .. py:method:: is_margin_trading(self, stock)

        [Overload interface] Whether the security is covered by the margin accounting; when it is
        false its position takes no occupation

        :param Stock stock: the security to check
        :return: bool
        :rtype: bool

    .. py:method:: need_borrow_asset(self, stock)

        [Overload interface] Whether opening a short of the security needs borrowing the asset. By
        default the built-in categories return True (keeping the A-share securities lending
        accounting), while a cryptocurrency and any custom type (>= 12, e.g. futures, forex, US or
        HK stocks) return False

        :param Stock stock: the security to check
        :rtype: bool

    .. py:method:: get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

        [Overload interface, required] The margin occupied by an opening, in the account base
        currency

        :param Datetime date: the trade moment
        :param Stock stock: the security to open
        :param float price: the opening price
        :param float num: the opening quantity; pass 1.0 for the margin of a single unit
        :param MarginSide side: the long or the short side
        :return: the occupied funds, 0 for invalid inputs
        :rtype: float

    .. py:method:: get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

        [Overload interface] The maintain margin (the additional-margin threshold), equal to the
        opening margin by default

        :rtype: float

    .. py:method:: get_margin_rate(self, date, stock, side=MarginSide.LONG)

        [Overload interface] The ratio view of the margin, in (0, 1]; always 1.0 in per-lot mode

        :rtype: float

    .. py:method:: get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

        [Overload interface] The maximum openable quantity under the given funds, aligned down to
        the minimum trade unit and capped by the maximum order quantity

        :param float available: the available funds
        :return: the quantity, 0 when nothing can be opened
        :rtype: float

    .. py:method:: _clone(self)

        [Overload interface] The subclass clone interface

.. py:class:: MarginSide

    The side of an opened position

    .. py:attribute:: LONG

        The long side

    .. py:attribute:: SHORT

        The short side

Account side interfaces
-----------------------

.. py:class:: TradeManager

    .. py:attribute:: margin

        The margin model of the account (:py:class:`MarginBase`). Assigning None falls back to the
        plain cash model MG_None; a new account defaults to MG_Fixed(0.6), i.e. the legacy 60 %
        buying power

    .. py:method:: get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

        The margin occupied by an opening according to the account margin model

    .. py:method:: get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

        The maintain margin according to the account margin model

    .. py:method:: get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

        The openable quantity under the given funds according to the account margin model

    .. py:method:: get_occupied_margin(self, date, ktype="DAY")

        The margin occupied by the long and short positions at the given moment (the positions of a
        security whose ``is_margin_trading`` is false take no part)

    .. py:method:: get_available_margin(self, date, ktype="DAY")

        The funds available for a new opening, i.e. ``net assets - occupied margin``

.. note::

    The available funds use the net assets instead of the cash: a short opening brings the sale
    proceeds into the cash, which would otherwise allow an unlimited number of shorts.

    The buying power check runs only when ``support_borrow_cash=True`` (long side) or the margin
    model covers the security (short side); the buy / sell checks of a plain cash account (MG_None
    with no financing) stay exactly as before.
