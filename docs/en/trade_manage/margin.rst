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

Margin accounting (Phase B)
---------------------------

``margin_accounting`` is an account-level switch (False by default). With it on, the openings of a
**margin security** (``is_margin_trading=True`` and ``support_borrow_cash=True`` or
``need_borrow_asset=False``, i.e. financing buys, futures and crypto) lock the margin only, while
the other holdings (e.g. a cash A-share buy) keep the full cash accounting.

Booking formulas:

- Open long / short: ``cash -= margin + fee``, ``PositionRecord.margin += margin``; ``buy_money`` /
  ``sell_money`` keep accumulating the nominal amount for reporting only
- Close long / short: ``cash += released margin + realized profit - fee``; the released margin is
  proportional to the closed quantity and the realized profit is computed on the **money basis**:
  ``closed money - remaining cost basis * (closed number / current number)`` (negated on the short
  side), where ``closed money = price * number * unit`` and the remaining cost basis is
  ``PositionRecord.open_basis``, exact whatever the contract multiplier is; a close releases the
  basis proportionally and an opening only adds the new notional, so a partial close followed by a
  new opening never dilutes the cost of the lots still held
- Equity: ``net assets = cash + locked margin + unrealized profit - borrowed funds``; the equity
  contribution of a margin holding joins ``FundsRecord.market_value`` on either side (the short
  unrealized profit already carries the direction), ``short_market_value`` keeps only the cash
  accounted short nominal value
- Reporting: ``FundsRecord.occupied_margin`` is the locked margin; the settlement price is
  approximated by the close of the day

.. note::

    ``get_occupied_margin()`` is the **margin requirement** recomputed at the given price over every
    holding the model covers, the ones booked by the plain cash accounting included (a broker counts
    the whole collateral), whereas ``FundsRecord.occupied_margin`` is the amount **already locked**
    on the openings; the gap covers both the mark-to-market drift and the accounting difference, and
    ``get_available_margin() = net assets - requirement``.

    The margin and the fee of an opening follow the same funding rule as the long side: an account
    that may borrow (``support_borrow_cash=True``) covers the shortfall with new debt recorded in
    ``borrow_cash``, the others are rejected when the cash does not suffice, so **an opening never
    takes the cash negative**; only a closing loss larger than the released margin may do so (a real
    overdraft), and the following openings are stopped by the quota gate.

    ``auto_checkin`` (a ``MoneyManager`` parameter that makes the ``System`` deposit right before
    submitting an order, the batch top-up of ``MultiSystem`` following the same caliber) deposits only
    what the order really pays: a margin-accounted long opening tops up its margin and its fee (the
    rest is financed, so the principal is not inflated), a margin-accounted short cover tops up
    nothing (the cover returns the margin and settles the profit, an overdraft being legal), while
    the plain books still deposit the full traded amount plus its fee.

    A margin requirement below one monetary unit (``10^-precision``, 0.01 by default) locks one unit
    instead of rounding down to zero: ``margin > 0`` marks a holding booked on the margin basis, so a
    zero lock would demote it to the cash books (valuing the full nominal amount while next to
    nothing was paid), which misstates its real exposure; the aggregate requirement returned by
    ``get_occupied_margin()`` carries no such floor.

    The records replayed by ``add_trade_record`` carry the trades only, not the debt entries
    (``BUSINESS_BORROW_CASH``) that funded them: when the cash falls short and
    ``support_borrow_cash=True``, the add path borrows again with the same iteration the live ``buy``
    / ``sell_short`` uses (the financing fee itself included), so a reimported margin opening is no
    longer rejected and the rebuilt cash, debt and locked margin match the live books;
    ``support_borrow_cash=False`` still rejects on insufficient cash. The securities lending fee is
    financed the same way.

    A short that both locks a margin and borrows securities validates read-only first, then finances
    once and finally borrows in a single commit: an insufficient lendable pool (unsold quantity too
    small, nothing to borrow, all sold) or a placeholder price rejects the order **before any booking
    happens**, leaving neither a ``borrow_cash`` debt nor a financing fee behind; the securities
    borrow fee counts into the required cash together with the margin, so an over-borrow whose fee
    has no funding can no longer take the cash negative.

    Switching the accounting for the same security midway (a cash-basis opening later joined by
    margin lots, or a ``realPrice == 0`` placeholder opening followed by a priced one) creates
    **mixed lots**: the position-level books keep a single locked margin and one money-basis prorated
    cost, so the valuation and the close cash flow fold the cash lots' cost into the margin formula,
    which deviates from the lot-by-lot market practice; the account logs a warning in **either**
    direction (margin lots joining a cash holding as well as cash lots joining a margin one).
    Lot-level (per-trade) margin and cost booking, and writing the locked margin into
    ``TradeRecord``, are registered as follow-up tasks.

    The historical replay (``getFunds(date)``, ``get_funds_list``, ``Performance``) and
    ``add_trade_record`` recompute the margin with the **current** parameters and margin model:
    changing ``margin_accounting``, swapping the model or adjusting its rate after the openings makes
    the replayed books differ from the ones booked at that time; ``PositionRecord.margin > 0`` only
    marks the holdings the live books already keep by margin.

    The maintain-margin call and the forced liquidation (Phase B-2) and the daily variation-margin
    posting are not implemented yet; a margin holding **rejects** ``checkin_stock`` /
    ``checkout_stock`` (a deposit or withdrawal would move the cost basis while the lock stays, which
    detaches the equity from the assets, so the order is refused and the backfill refuses it too). A
    ``realPrice == 0`` market-order placeholder opening still keeps the cash accounting, but a margin
    holding refuses to be closed at a placeholder price (the trade is rejected and the traded price
    is required, otherwise the locked margin could not be released); the historical replay keeps the
    margin locked and warns when such a record shows up in the trade list.

    When the margin model reports no requirement at all (or a non-finite one) for a security, that
    opening falls back to the cash books and pays the full notional (with a warning): a holding with
    ``margin == 0`` is valued as a cash holding, so keeping the margin branch would count the full
    market value as equity while no cash was paid.
