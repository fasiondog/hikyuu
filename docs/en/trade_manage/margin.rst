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

.. py:function:: MG_ByType([rates="", maintain_rate=0.0, default_rate=0.6, collateral_rates="",
                           default_collateral_rate=1.0])

    The margin model keyed by the security type or the market, for a mixed account (e.g. A-share
    cash positions plus futures margin)

    :param str rates: a list of ``key:ratio`` items separated by ``,`` ``;`` or blanks, e.g.
        ``"12:0.10, SHF:0.12"``; an all-digit key is a security type, any other key is a market
        code (compared upper-cased); a type hit wins over a market hit
    :param float maintain_rate: the maintain margin ratio; 0 means the same as the opening margin;
        when non-zero it must be <= the opening ratio (a higher one would call the account at open)
    :param float default_rate: the ratio used when no key matches, in (0, 1]; 1.0 also means the
        unmatched securities are plain cash positions and take no occupation
    :param str collateral_rates: the ``key:ratio`` list of the collateral conversion rates, with
        the same syntax as rates (the two tables are independent); an empty list counts every
        holding in full
    :param float default_collateral_rate: the conversion rate used when no collateral key matches,
        in (0, 1], 1.0 by default
    :return: an instance of a :py:class:`MarginBase` subclass

.. note::

    The long and the short share one ratio. Use the ``rate_short`` of :py:func:`MG_Fixed`, or
    subclass :py:class:`MarginBase` and override :py:meth:`MarginBase.get_open_margin` in Python,
    when they must differ. The collateral table is shared the same way (the built-in models accept
    the ``side`` and ignore it).

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

    .. py:method:: get_collateral_rate(self, date, stock, side=MarginSide.LONG)

        [Overload interface] The collateral conversion rate, in (0, 1], i.e. the part of the market
        value that may count towards the available funds; 1.0 (the full value) by default

        :param Datetime date: the queried moment
        :param Stock stock: the held security
        :param MarginSide side: the long or the short side (the built-in models do not separate them)
        :return: the conversion rate, 1.0 when no table is configured
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

        The funds available for a new opening, i.e. ``net assets - collateral haircut - occupied
        margin``

    .. py:method:: get_collateral_rate(self, date, stock, side=MarginSide.LONG)

        The collateral conversion rate of the security according to the account margin model; 1.0
        when the account has no margin model

    .. py:method:: get_collateral_haircut(self, date, ktype="DAY")

        The value the conversion table discounts away, i.e. ``sum of the value counted in the equity
        * (1 - conversion rate)`` over the long holdings (the locked margin plus the unrealized
        profit for a margin-accounted holding, the market value for a cash-accounted one); 0.0 when
        no rate is configured

.. note::

    The available funds use the net assets instead of the cash: a short opening brings the sale
    proceeds into the cash, which would otherwise allow an unlimited number of shorts.

    The buying power check runs only when ``support_borrow_cash=True`` (long side) or the margin
    model covers the security (short side); the buy / sell checks of a plain cash account (MG_None
    with no financing) stay exactly as before.

Margin accounting
-----------------

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
    ``get_available_margin() = net assets - collateral haircut - requirement``.

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

    The collateral conversion rate changes the asset side of ``get_available_margin()``: the
    holdings no longer count in full but as ``net assets - sum of the value counted in the equity *
    (1 - conversion rate) - margin requirement``. The rate applies to the **long holdings of the
    securities the margin model covers** - the same set the margin requirement is computed on, since
    a security of the plain cash books neither locks a margin nor counts as collateral. A holding
    booked by margin is discounted only on what it contributes to the equity (the locked margin plus
    the unrealized profit): the financed part is already a liability, so discounting the whole market
    value would remove it twice, and a loss deeper than the margin leaves nothing to discount. The
    cash and the liabilities always count in full, and a short contributes no asset to discount.
    The default rate of 1.0 makes the haircut 0, so the available funds stay bit for bit what they
    were before the rate existed; in the A-share practice a highly volatile or richly valued
    instrument is usually given a rate below 1.0 to limit how much may be financed on it. The rate
    touches neither the equity nor the maintenance requirement, it only tightens the opening quota.

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
    Lot-level (per-trade) margin and cost booking are not provided.

    The margin an opening locked is written into ``TradeRecord.margin`` (0 on the cash books and for
    every business other than an opening): ``add_trade_record`` and the historical replay prefer that
    snapshot and only recompute with the **current** parameters and margin model when it is 0. Changing
    ``margin_accounting``, swapping the model or adjusting its rate after the openings therefore still
    replays the books booked at that time; ``PositionRecord.margin > 0`` marks the holdings the live
    books already keep by margin.

    The daily settlement, the margin call and the forced liquidation follow in *Daily settlement and
    the risk loop* below; a margin holding **rejects** ``checkin_stock`` /
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

Daily settlement and the risk loop
----------------------------------

A margin account settles once per calendar day, at the first bar the trading system closes within it
(a minute-level system therefore still settles a single time a day): the unrealized profit is posted
into the cash, the lock is recomputed at the settlement price, and the equity is then checked against
the maintenance requirement; a shortfall warns and, with ``force_liquidate`` on, also queues a forced
liquidation that the trading system executes at the next open. The related parameters (account level
``set_param``):

- ``margin_accounting`` (False by default): the margin accounting switch, nothing is settled while it is off
- ``settle_with_close`` (True by default): the close of the period is the settlement price; turning it
  off makes ``settlement()`` return False (no other price source is provided yet)
- ``force_liquidate`` (False by default): a shortfall of the equity only warns; turning it on queues
  the forced liquidation as well
- ``annual_days`` (365 by default): the days of a year, i.e. the denominator that prorates the
  interest over the elapsed calendar days; it stays fixed across a year boundary by design

.. py:method:: TradeManager.settlement(date, ktype="DAY")

    Settles the margin holdings as of the given moment and returns whether this call **posted**
    anything (the mark-to-market or the interest billing): a repeated call of the same calendar day
    posts nothing twice and returns True (the settlement is idempotent per day, whatever the K type),
    an account that does not settle (``margin_accounting`` / ``settle_with_close`` off, no margin
    model) returns False, and an account with nothing to post and nothing to bill (empty, or every
    holding skipped and no debt) returns False as well, which is
    also what ``is_settled`` answers there

    :param Datetime date: the settlement moment, no earlier than ``last_datetime``
    :param str ktype: the K-line type the settlement price is taken from

.. py:method:: TradeManager.is_settled(date)

    Whether the calendar day of the moment has entered a settlement batch. Settling is idempotent per
    calendar day: a repeated call within the same day posts nothing twice. However, the posting and
    the billing halves are gated apart and this probe answers the **or** of them, so it is True even
    on a day that only billed interest, which does not mean the holdings were marked

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

Interest billing: after the mark-to-market posting and before the risk check, the interest
of the outstanding liabilities is billed in the same batch, so **a billed day is also a called day**:

- Financing: ``borrow_cash * get_borrow_cash_rate(the settled day) * billed days / annual_days``
- Securities lending: ``sum of borrowed number * unit * settlement price *
  get_borrow_stock_rate(the settled day, stock) * billed days / annual_days``, i.e. charged on the
  market value of the borrowed shares **at the settlement price**, not on the value they had when
  they were borrowed
- The billed days follow the **calendar**: the days gone by since the book was last billed (1 for the
  first bill, the day the debt was taken on), so a weekend, a suspension or a coarser K type bills
  the days it waited for instead of silently costing nothing
- The anchor is at the level of the **whole book**: one day, that of the last interest record, serves
  every liability, and no age is kept per debt. A debt taken on inside such a window is therefore
  billed over the whole of it (a little too much) and a debt that predates the very first bill is
  billed over a single day (a little too little). This is the accepted simplification of not keeping
  the days of every debt; billing each loan from its own date needs per loan accounting, which is not
  provided
- A lending of a security past its registered last trading day (an expiry, a delisting) is **not billed**
  any more, exactly like the holding settlement treats it: it belongs to the expiry liquidation, so its
  stale last close must not keep billing day after day. One with no settlement price at all is reported
  and not billed, never silently skipped
- The billed amount **joins the debt itself and is not paid from the cash** (a broker folds the
  interest into the principal, which then accrues on its own), and it is appended to the loan list as
  a loan of its own date, so ``return_cash`` can still repay the whole debt from it
- The rates belong to the **trade cost model** (see :doc:`trade_cost`); the built-in carrier is
  ``TC_MarginInterest``, which wraps any cost model, leaving the trading fees to the wrapped one and
  answering the two rates on top of it, and 0.0 bills nothing
- The billing is carried by ``settlement()``, hence it requires ``margin_accounting=True``: a plain
  cash account with a debt but no margin holding needs that switch too, and it must also configure a
  model that requires no margin at all (``MG_None``). The account is constructed with
  ``MG_Fixed(0.6)`` by default, and leaving that model in place turns **every** security into a
  60% margin-accounted one; with ``MG_None`` the books stay plain cash and only the explicit debt
  is billed. With the switch off the account neither settles nor bills

Settlement records: every settled holding writes one ``BUSINESS_SETTLEMENT`` record (not a fill) —
``real_price`` is the settlement price, ``number`` carries the direction (positive long, negative
short, so a security held on both sides is told apart), ``margin`` is the relocked margin, ``cash`` the
cash balance after the posting, ``cost`` is 0 and ``part`` is the invalid part. Interest records: every
liability item writes one ``BUSINESS_INTEREST`` per day (one for the cash debt, one per borrowed
security), ``real_price`` is the billed amount, ``number`` the borrowed quantity (0 for the cash
interest, the same caliber as ``BUSINESS_BORROW_CASH``) and ``stock`` the security (an empty stock for
the cash interest).

.. note::

    With the margin accounting off, or with ``settle_with_close=False``, nothing is settled and the
    books of a cash A-share account stay exactly as before.

    The ``System`` drives every bar as *trade -> expiry flattening -> settlement*: the parts
    trade the bar first, then a margin holding past the last registered bar of its instrument is
    flattened at the close (G12), and ``settlement()`` runs last, posting only on the first bar of a
    calendar day; the expiry filling before the
    settlement is what realizes the profit, and the settlement then works on the flattened books. A
    forced liquidation armed by the previous settlement is executed right after the next open, before
    the regular requests, at the open price of that bar and on both sides of the instrument; the
    filling joins the trade records and the position history like any other.

    The historical replay needs no extra state: the settlement records are part of the books, so
    ``get_funds(date)``, ``get_funds_list`` and ``Performance`` all reproduce the cash, the lock and
    the basis of the settled day; ``BUSINESS_INTEREST`` joins the
    debt balance the same way (the replay of ``get_debt_cash``, ``get_funds`` and ``get_funds_list``
    all accumulates it).

    ``MultiSystem`` calls ``settlement()`` on the parent account at the close of its aggregate phase as
    well (the parent is the real book of the modes A/B, the members being shadow books only); the mode C
    members share the parent account, so they have settled its day already and the
    repeated call is a no-op. The parent has no single instrument, so an armed forced liquidation is
    executed at the aggregate open by flattening every holding on both sides at the open price, and the
    delayed entry requests of the members are voided as well (the same caliber as the delayed buy
    requests ``System`` drops, the reducing orders kept).

    The action script written by ``save_action`` does **not** cover the settlement nor the interest
    billing: the script replays the fills only, so a regenerated run rebuilds an unsettled, unbilled
    book and has to call ``settlement()`` per period itself. The settlement and the interest records
    do **not** take part in the ``add_trade_record`` backfill: both are generated by
    ``settlement()``, a single record cannot reproduce the book state of that moment, and a manual
    backfill is rejected with False.
