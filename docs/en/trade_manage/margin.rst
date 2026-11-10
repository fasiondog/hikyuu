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
  ``closed money - remaining settlement basis * (closed number / current number)`` (negated on the
  short side), where ``closed money = price * number * unit``; the settlement basis is
  ``PositionRecord.settle_basis`` (rolled to the settlement price by the daily mark-to-market and
  equal to the opening notional ``open_basis`` until the first settlement), exact whatever the
  contract multiplier is; a close releases both bases proportionally and an opening only adds the
  new notional, so a partial close followed by a new opening never dilutes the cost of the lots
  still held
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
    Lot-level (per-trade) margin and cost booking are registered as follow-up tasks.

    The margin an opening locked is written into ``TradeRecord.margin`` (0 on the cash books and for
    every business other than an opening): ``add_trade_record`` and the historical replay prefer that
    snapshot and only recompute with the **current** parameters and margin model when it is 0. Changing
    ``margin_accounting``, swapping the model or adjusting its rate after the openings therefore still
    replays the books booked at that time; ``PositionRecord.margin > 0`` marks the holdings the live
    books already keep by margin.

    The daily settlement, the margin call and the forced liquidation follow in *Daily settlement and the risk
    loop (Phase B-2)* below; a margin holding **rejects** ``checkin_stock`` /
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

Daily settlement and the risk loop (Phase B-2)
----------------------------------------------

A margin account settles once per K-line period at its close: the unrealized profit is posted into the
cash, the lock is recomputed at the settlement price, and the equity is then checked against the
maintenance requirement; a shortfall warns and, with ``force_liquidate`` on, also queues a forced
liquidation that the trading system executes at the next open. The related parameters (account level
``set_param``):

- ``margin_accounting`` (False by default): the margin accounting switch, nothing is settled while it is off
- ``settle_with_close`` (True by default): the close of the period is the settlement price; turning it
  off makes ``settlement()`` return False (no other price source is provided yet)
- ``force_liquidate`` (False by default): a shortfall of the equity only warns; turning it on queues
  the forced liquidation as well

.. py:method:: TradeManager.settlement(date, ktype="DAY")

    Settles the margin holdings as of the given moment and returns whether this call **posted** the
    settlement: a moment settled already returns True (the settlement is idempotent), an account that
    does not settle (``margin_accounting`` / ``settle_with_close`` off, no margin model) returns False,
    and an account with nothing to post (empty, or every holding skipped) returns False as well, which
    is also what ``is_settled`` answers there

    :param Datetime date: the settlement moment, no earlier than ``last_datetime``
    :param str ktype: the K-line type the settlement price is taken from

.. py:method:: TradeManager.is_settled(date)

    Whether the moment is settled already. Settling is idempotent: a repeated call of the same moment
    posts nothing twice

.. py:method:: TradeManager.is_forced_liquidation_queued(date)

    Whether a forced liquidation is pending as of the moment: every settlement rebuilds the queue (a
    recovered account clears it), so it is True once armed until the holdings are flattened and always
    False on a flat account. The probe answers "armed with something left to close" and consumes
    nothing itself, so a flattened account stays False even though the armed date is still recorded

What the settlement does (per margin holding; the cash accounted holdings stay out):

- Settlement price: the close of the bar at that moment; without a bar (a suspension) the last valid
  close is used and the ``remark`` of the settlement record says so
- Mark-to-market: ``cash += +/- (settlement price * number * unit - settle_basis)`` (positive on the
  long side, negated on the short one), then ``settle_basis`` is reset to the notional at the
  settlement price, so the unrealized profit restarts from zero and a close realizes against the
  latest basis
- Relock: the lock is recomputed at the settlement price and the difference is posted on the spot
  (``cash -= difference``); an increase the cash cannot cover is financed like an opening
  (``support_borrow_cash=True``), otherwise the account goes overdrawn with a warning, because an
  account already in a margin call must not fail to settle
- Maintenance: ``sum of get_maintain_margin(...)`` over **every** margin holding, the ones the posting
  skipped included (a suspension, a bar past the last registered one; without a valid price the unit
  price is backed out of ``settle_basis``), otherwise those holdings would count on the asset side
  while missing from the requirement side; the equity is ``cash + sum of the holding equities -
  borrowed cash`` at that moment, the same caliber as ``get_funds``, and a shortfall raises the margin
  call (warned once when it turns on, reported as cleared when it recovers)
- A completed settlement with ``force_liquidate=True`` arms the forced liquidation,
  ``is_forced_liquidation_queued()`` turning True

Settlement records: every settled holding writes one ``BUSINESS_SETTLEMENT`` record (not a fill) —
``real_price`` is the settlement price, ``number`` carries the direction (positive long, negative
short, so a security held on both sides is told apart), ``margin`` is the relocked margin, ``cash`` the
cash balance after the posting, ``cost`` is 0 and ``part`` is the invalid part.

.. note::

    With the margin accounting off, or with ``settle_with_close=False``, nothing is settled and the
    books of a cash A-share account stay exactly as before.

    The ``System`` drives every bar as *trade -> expiry flattening -> daily settlement*: the parts
    trade the bar first, then a margin holding past the last registered bar of its instrument is
    flattened at the close (G12), and ``settlement()`` runs last; the expiry filling before the
    settlement is what realizes the profit, and the settlement then works on the flattened books. A
    forced liquidation armed by the previous settlement is executed right after the next open, before
    the regular requests, at the open price of that bar and on both sides of the instrument; the
    filling joins the trade records and the position history like any other.

    The historical replay needs no extra state: the settlement records are part of the books, so
    ``get_funds(date)``, ``get_funds_list``, ``Performance`` and the backfill of ``add_trade_record``
    all reproduce the cash, the lock and the basis of the settled day.

    ``MultiSystem`` calls ``settlement()`` on the parent account at the close of its aggregate phase as
    well (the parent is the real book of the modes A/B, the members being shadow books only); the mode C
    members share the parent account, so they have settled it at the same moment already and the
    repeated call is a no-op. The parent has no single instrument, so an armed forced liquidation is
    executed at the aggregate open by flattening every holding on both sides at the open price, and the
    delayed entry requests of the members are voided as well (the same caliber as the delayed buy
    requests ``System`` drops, the reducing orders kept).

    The action script written by ``save_action`` does **not** cover the settlement: the script replays
    the fills only, so a regenerated run rebuilds an unsettled book and has to call ``settlement()``
    per period itself (the settlement records still take part in the ``add_trade_record`` backfill,
    they are just not written to the script).
