.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _allocate_funds:

Fund Allocation Part (AF)
=========================

The fund allocation algorithm part (AllocateFunds, AF) performs the **portfolio-level** fund allocation
for the sub-systems selected by the aggregate system :class:`MultiSystem`. AF operates on the
"sub-system set" (a set of ``SYSPtr``), and every :class:`MultiSystem` owns a single AF instance
(see :doc:`portfolio`).

.. note::

   **The responsibilities of money management (MM) and fund allocation (AF) are separated**:

   * :class:`MoneyManagerBase` is restricted to the **single-system / single-security** scope (it operates on a single ``Stock``)
     and handles the single-system position sizing;
   * the portfolio-level allocation (L1 system-level weights / L2 behavior-level conversion / L3 portfolio risk control) is carried by
     :class:`AllocateFundsBase`.

   The portfolio-level fund allocation is handled by ``AF_*``.

Allocation modes (A / B / C)
----------------------------

The allocation mode determines the overall running form of the aggregate system. It is owned by
:class:`MultiSystem` itself (set via ``set_mode``, read via the read-only ``mode`` attribute) and is
passed explicitly into every AF hook (L1/L2/L3) as the trailing ``mode`` argument; the AF itself does
not hold it (see :doc:`../trade_sys/system`):

.. list-table::
    :header-rows: 1

    * - Mode
      - Name
      - Sub-account form
      - Rebalancing and execution semantics
    * - ``"A"``
      - Signal Aggregation (**the default**)
      - Independent shadow accounts per sub-system, always running, non-overlapping
      - The parent account is the **sole decision maker**: on the rebalancing day it reads the **current intent state**
        of the selected sub-systems (net exposure + unfulfilled signals), synthesizes a **signed net target** per
        instrument by the L1 weights (long and short net amounts cancel out), and places the delta orders on the
        parent account
    * - ``"B"``
      - Fund Allocation
      - Independent shadow accounts per sub-system, starting from zero and funded only by quotas
      - The parent account is the **fund allocator**: on the rebalancing day L1 computes each sub-system **quota**
        (target total assets) and calibrates (zero-quota liquidation -> unselected liquidation / selected recycle -> reduce the over quota ->
        inject the gap); the sub-systems trade autonomously within their quotas and the parent **mirrors
        continuously** their real instructions (the main account = the sum of the sub-system positions)
    * - ``"C"``
      - Shared Account Compatibility
      - No shadow accounts; the sub-systems trade directly on the parent real account
      - Compatible with the old ``Portfolio`` semantics (the driven set is decided by the SE semantics); the parent
        performs no L2 conversion and the AF provides only L1/L2/L3 no-ops (the mode is held by the aggregate system)

Mode x algorithm-part mapping:

.. list-table::
    :header-rows: 1

    * - Part
      - Mode A
      - Mode B
      - Mode C
    * - L1 system-level allocation
      - Produces the nominal weights ``{System: weight}`` (signal weights)
      - Produces the real quotas: writes ``contexts[i].quota`` (the target total assets) back in place, through nesting
      - Pass-through, no allocation
    * - L2 behavior-level conversion
      - Synthesizes a net target per instrument by ``w × ratio × parent net value`` and converts the order
        quantities (signed; the cash-releasing group first, the cash-consuming group second)
      - Passes the sub-system instructions through (both long and short)
      - Pass-through, no conversion
    * - L3 portfolio risk control
      - Portfolio risk-control clipping (e.g. the ``max_single_position`` concentration cap)
      - Off or total-only (the ``max_total_position`` quota-sum cap is carried at L1)
      - Off and cannot be enabled

.. note::

   Mode C is not a parameter special case of mode B: it cannot be reproduced by any parameter
   setting of mode B (any ``adjust_running_sys`` value, an AF that performs no allocation, turning
   off the parent proportional scaling, and so on). The two are **two different account topologies**
   -- in mode B the sub-systems run in **isolated shadow accounts** and the parent mirrors their
   trades continuously, while in mode C the sub-systems **share the parent real account** directly
   (a deliberate exception). With a shared account the sub-systems **compete for the same cash
   pool**, which mode B does not exhibit.

The three replaceable algorithm parts of AF
-------------------------------------------

AF splits portfolio-level allocation into three **independently replaceable** algorithm parts (each backed by one override hook of the base class),
and :meth:`AllocateFundsBase.allocate` chains them together into a pipeline:

.. list-table::
    :header-rows: 1

    * - Part
      - Override hook
      - Semantics
    * - L1 system-level allocation
      - ``_allocate(date, tm, contexts, query, mode)``
      - Sub-system context -> weight ``{System: weight}``; in mode B it writes the actual quota back to ``contexts[i].quota`` in place
    * - L2 behavior-level conversion
      - ``_to_targets(date, tm, suggestions, sys_weight, query, mode)``
      - Weights -> executable quantities for the parent account (in mode A, the per-instrument net target is synthesized by ``weight x signed net ratio x the parent net assets`` and traded by the delta; in mode B, sub-system instructions are passed through)
    * - L3 portfolio risk control
      - ``_check_risk(date, tm, suggestions, query, mode)``
      - Portfolio-level risk-control clipping (e.g. the ``max_single_position`` concentration cap)

Both ``_to_targets`` and ``_check_risk`` come with default implementations, so usually only ``_allocate`` needs to be overridden.

Common parameters (portfolio-level)
-----------------------------------

The base class :class:`AllocateFundsBase` carries the following common parameters:

.. list-table::
    :header-rows: 1

    * - Parameter
      - Type and default
      - Effective modes
      - Description
    * - ``max_single_position``
      - float, 1.0
      - A only
      - The L3 per-instrument concentration cap: the target market value of one instrument must not exceed
        the net assets x this value. <=0 disables the limit; a value above 1 takes effect as 1.0
    * - ``max_total_position``
      - float, 1.0
      - A / B
      - The total budget cap: in mode A it bounds the aggregated target market value of all instruments (L2)
        by the net assets x this value; in mode B it bounds the **sum of the sub-system quotas** (L1) by
        the net assets x this value. Both bounds **include** the ``fixed_amount`` pricing: the weight
        pricing is scaled down proportionally, while the fixed amounts are truncated to a whole number of
        quanta (see below). <=0 disables the cap; >1 allows leverage
    * - ``fixed_amount``
      - float, 0.0
      - A / B
      - The pricing switch: when >0 it **takes precedence over the weight pricing** with a warning.
        In mode A it is the target market value per instrument; in mode B the fixed quota per sub-system.
        The L1 weights then carry only the ranking meaning and take no part in the pricing.
        The aggregate is still bounded by ``max_total_position``
    * - ``adjust_running_sys``
      - bool, True
      - B only
      - Whether to force-adjust the **running sub-systems** to the new quota on the rebalancing day.
        A running sub-system = one that was funded in a previous period and still holds assets at the
        calibration time. ``True``: runs the full calibration (zero-quota liquidation -> unselected
        liquidation / selected recycle -> reduce the over quota -> inject the gap); ``False``: leaves the running sub-systems untouched
        (no recycle, no reduction, no injection) and only the new sub-systems get the remaining parent
        cash (limited by the parent balance, may be underfunded).
        Mode A re-targets every bar and always adjusts; mode C has no quota and never adjusts --
        neither is affected by this parameter

Notes:

.. list-table::
    :header-rows: 1

    * - Item
      - Description
    * - Full allocation after score normalization
      - ``AF_MultiFactor`` by default (``auto_adjust_weight=True``) **normalizes the scores to sum 1**
        before allocating, so **a score sum less than 1 also allocates fully** (the scores only express
        the relative ratios between the sub-systems). When ``False`` the score itself is the final weight
        and the part exceeding the total budget relies on ``max_total_position`` to scale down
    * - Constant weights never trigger a reduction
      - The constant-weight AFs (e.g. ``AF_FixedWeightList``) recompute the quota from the **current** parent
        net assets on every period, so the sub value and the quota grow in the same proportion and the
        reduction is never triggered by the normal market movement; the force reduction only happens on a
        real cross-period shrink of the weights or the funds (and never happens when
        ``adjust_running_sys=False``)
    * - ``fixed_amount`` is bounded by the total budget cap (truncated by whole quanta)
      - A fixed amount is an **absolute amount**, so an over-budget total is never scaled proportionally
        (the quota would drift with the net assets and the number of sub-systems); instead **only whole
        quanta are funded**: mode B hands out the full ``fixed_amount`` in sub-system order and gives a zero
        quota to the rest once the budget is exhausted, mode A does the same in first-appearance order and
        drives the unfunded instruments flat. If a single quantum already exceeds the cap (the amount >
        the net assets x ``max_total_position``) nothing is funded at all (the warning reports 0/N), so
        lower ``fixed_amount`` or raise the cap. In mode B a zero-quota sub-system follows the
        ``sell_at_not_selected`` / ``adjust_running_sys`` rules (liquidated and stopped when the former
        is true, a running sub-system stays untouched when the latter is false). Note: AF no longer
        scales the fixed amount down, but the executor still respects the parent funding: with automatic
        cash financing (``support_borrow_cash``, set on the ``MultiSystem`` and propagated to the parent
        TM by ``readyForRun``) the buying power is derived from the **margin rate per order** -- a single
        order may exceed the available cash, while a multi-leg batch is funded in order and the later legs
        may be shrunk (the same leg-by-leg model as the TradeManager); a short cover always needs full cash
        and is never financed. Without financing the orders are scaled to the available cash. ``MultiSystem``
        also has ``auto_checkin`` (default ``False``): it deposits **extra principal** for the gap computed
        from the money plus the commission (combined with financing it deposits the full amount, so every
        leg is cash-covered), which keeps the planned quantities tradable at the cost of an
        inflated backtest equity curve; because the targets may change every bar it can top up repeatedly. To run leverage or keep the original amounts, raise
        ``max_total_position`` (0 means no limit)
    * - ``fixed_amount`` is applied once per instrument in mode A
      - In mode A the fixed amount is priced **per instrument**, not per suggestion: when the same instrument
        is held by N sub-systems, only the aggregated signed direction is taken and one ``±fixed_amount``
        target is applied per instrument (it is not multiplied by N). Conflicting directions cancel out, so a
        flat aggregate drives the instrument to zero
    * - The first plan price of an instrument wins
      - Mode A converts the aggregated target market value into shares by the ``plan_price`` of the
        suggestions: when several suggestions of one instrument carry different prices the **first one
        wins** (the others are only logged at debug level). The plan prices should come from the same
        bar and the same price convention (same adjustment and slippage), otherwise the share count
        will not match the target market value
    * - Mode A re-targets only the instruments covered by this round's suggestions
      - Mode A takes this round's aggregated suggestions as its target snapshot: only the **instruments
        appearing in the suggestions** (those traded by a sub-system on the day, plus the liquidation legs
        MultiSystem injects for the unselected systems) are converted and traded by the delta; **an
        instrument that does not appear keeps the parent's current position untouched**. AF assumes the
        parent book mirrors the sub-system books and does not detect or close positions that no sub-system
        corresponds to (e.g. a manual trade on the parent account, or a sub-system removed from the list
        without liquidation)
    * - A suggestion without a weight is skipped
      - The mode A weight table is keyed by the **System instance**: the ``sys`` of a suggestion must be the
        very instance held by the L1 contexts. When the table is empty, the sub-system is missing from it,
        or an upstream returns a cloned instance (a different instance from the context one), the
        suggestion is **skipped with a warning** and never priced by a fabricated equal weight.
        Parent-synthesized suggestions (an empty ``sys``, e.g. the zero target of an unselected instrument)
        are not affected. With ``fixed_amount>0`` the weight lookup is skipped entirely (the absolute
        amount is the target), so this item does not apply
    * - L3 only shrinks, it never redistributes
      - ``max_total_position`` (total budget) then ``max_single_position`` (per-instrument concentration)
        apply in sequence and both only **shrink** the excess: the budget released by the L3 clipping is
        **not** redistributed to the other instruments, so the final gross exposure may stay well below
        ``max_total_position`` (e.g. 10 equal-weight instruments with a 0.05 per-instrument cap end up with
        a gross of only 0.5 x the net assets)


Built-in Asset Allocation Algorithms
------------------------------------

.. py:function:: AF_EqualWeight()

    Equal-weight fund allocation: allocates capital to the selected instruments in equal proportions (L1 equal weight 1/N).

    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeight([weight = 0.1])

    Fixed-weight fund allocation: each selected instrument accounts for a fixed proportion of total assets.

    L1 returns the fixed ``weight`` directly, **without normalization** (for the score-as-final-weight usage,
    see ``auto_adjust_weight=False`` of ``AF_MultiFactor``).

    :param float weight: the specified weight as a proportion of total assets (0, 1]
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeightList(weights)

    Fixed-weight-list fund allocation: allocates capital to the selected sub-systems according to the specified weight list.

    L1 takes ``weights[i]`` in sub-system order, **without normalization**; when the length does not match, it falls back to equal weights.
    An empty list cannot price anything: both the factory and the parameter check throw instead of falling back to the equal weight.
    The weights must be finite, non-negative and not all zero (an all-zero list sets every sub-system target market value to 0 and drives the parent to actively clear the existing holdings on every rebalance, so the parameter check throws).

    :param list weights: the specified list of weight proportions
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedAmount([amount = 20000.0])

    Fixed-amount fund allocation (L1 equal weight + L2 fixed amount): allocates an equal amount of capital to each selected instrument.

    The current implementation requires ``fixed_amount >= 0`` and rebalances by the net amount.

    :param float amount: the maximum trade amount
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_MultiFactor([auto_adjust_weight = True])

    Creates a MultiFactor score-weighted fund allocation algorithm instance, i.e. one that uses the scores returned by the selector
    (SE; ``SubSystemContext.score``) as the weights (a non-finite score is treated as 0, an all-zero or non-finite sum falls back to the equal weight).

    :param bool auto_adjust_weight: the score normalization switch (default ``True``). When ``True`` the scores are
        normalized to sum 1 before allocating (a score sum less than 1 also allocates fully; see the notes above);
        when ``False`` the score itself is the final weight (no normalization) and
        the part exceeding the total budget relies on ``max_total_position`` to scale down
    :return: the fund allocation algorithm instance (``AFPtr``)


Custom Asset Allocation Algorithm
---------------------------------

``crtAF`` is the recommended way to build a custom AF quickly: only the L1 allocation callback is required; L2 / L3 are injected as needed.

.. py:function:: crtAF(allocate_func[, params, name, to_targets_func, check_risk_func])

    Quickly creates a fund allocation algorithm

    :param allocate_func: the L1 system-level allocation callback ``func(self, date, tm, contexts, query, mode)``,
        returning ``{System: weight}``; in mode B it writes the quota back to ``contexts[i].quota`` in place and returns an empty table. ``mode`` is the running mode ("A"/"B"/"C") passed down by the aggregate system, which owns it
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param to_targets_func: the L2 behavior-level conversion callback ``func(self, date, tm, suggestions, sys_weight, query, mode)``, optional
    :param check_risk_func: the L3 portfolio risk-control callback ``func(self, date, tm, suggestions, query, mode)``, optional
    :return: the custom fund allocation algorithm instance (``AFPtr``)

The following ``my_allocate`` demonstrates both mode A (returning the weight table) and mode B (writing back the quota):

.. code-block:: python

    from hikyuu import *

    def my_allocate(self, date, tm, contexts, query, mode):
        # L1 [Required]: sub-system context -> weight
        #   Mode A: return {ctx.sys: weight}
        #   Mode B: write ctx.quota directly and return an empty table
        #   mode is the running mode ("A"/"B"/"C") passed down by the aggregate system
        if mode == "B":
            quota = tm.get_funds(date, query).total_assets / max(len(contexts), 1)
            for ctx in contexts:
                ctx.quota = quota
            return {}
        return {ctx.sys: 1.0 / len(contexts) for ctx in contexts}

    af = crtAF(my_allocate)

    # L2 [Optional] and L3 [Optional] are injected as needed (the base class default implementation is used when they are not injected):
    # af = crtAF(my_allocate, to_targets_func=my_to_targets, check_risk_func=my_check_risk)

The equivalent approach is to subclass :class:`AllocateFundsBase` directly, implement ``_clone``, and override
``_allocate`` / ``_to_targets`` / ``_check_risk`` as needed.


Asset Allocation Algorithm Base Class
-------------------------------------

.. py:class:: AllocateFundsBase

    The fund allocation algorithm (AF) base class, used exclusively by the aggregate system (MultiSystem).

    .. py:attribute:: name  Name
    .. py:attribute:: tm    Set or get the trade manager instance
    .. py:attribute:: query Set or get the query conditions

    .. py:method:: __init__(self[, name="AllocateFundsBase"])

        Constructor.

        :param str name: the name

    .. py:method:: get_param(self, name)

        Returns the value of the specified parameter.

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: if the parameter does not exist

    .. py:method:: set_param(self, name, value)

        Sets the value of a parameter.

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: have_param(self, name)

        Checks whether the specified parameter exists.

        :param str name: the parameter name
        :return: True if the parameter exists, False otherwise
        :rtype: bool

    .. py:method:: reset(self)

        Resets the instance.

    .. py:method:: clone(self)

        Returns a clone of the instance.

    .. py:method:: allocate(self, date, tm, suggestions, contexts, query, mode)

        The unified entry point for L1/L2/L3 (usually called internally by :class:`MultiSystem`): it first runs ``_allocate`` to obtain the weights,
        then converts them into executable quantities via ``_to_targets``, and finally applies risk-control clipping via ``_check_risk``.

        .. note::

            **Internal to MultiSystem only; it is not part of the user-facing API**. It assumes that
            ``contexts`` / ``suggestions`` / ``mode`` have been prepared by the aggregate system on a
            rebalancing day. Calling it directly with hand-built arguments only produces meaningless
            targets - it places no order and affects no backtest; drive a :class:`MultiSystem`
            instead. Custom algorithms should override ``_allocate`` / ``_to_targets`` /
            ``_check_risk`` rather than call this entry.

        :param Datetime date: the allocation date
        :param TradeManager tm: the parent-account trade manager
        :param TradeSuggestionList suggestions: the sub-system trade suggestions (rewritten in place)
        :param list contexts: the sub-system contexts (``SubSystemContext``; in mode B their ``quota`` field is written back in place)
        :param KQuery query: the query conditions
        :param str mode: the running mode ("A"/"B"/"C") owned by the aggregate system

    .. py:method:: _allocate(self, date, tm, contexts, query, mode)

        [Override hook] L1 system-level allocation: determines the weight of each sub-system from the sub-system contexts.

        Mode A returns ``{System: weight}``; mode B writes the quota back to ``contexts[i].quota`` in place and returns an empty table.

        :param str mode: the running mode ("A"/"B"/"C") owned by the aggregate system
        :return: the weight table (``{System: weight}``)

    .. py:method:: _to_targets(self, date, tm, suggestions, sys_weight, query, mode)

        [Override hook] L2 behavior-level conversion: converts the weights into executable quantities for the parent account, rewriting ``suggestions`` in place.

        In mode A it reads the sub-system current intent state (``target_position_ratio``, the signed net target), synthesizes the per-instrument net target by ``w x ratio x the parent net assets`` and trades the delta on the parent account;
        in mode B it passes the sub-system instructions through and applies defensive clipping. The base class default implementation is used when this hook is not overridden.

        :param str mode: the running mode ("A"/"B"/"C") owned by the aggregate system

    .. py:method:: _check_risk(self, date, tm, suggestions, query, mode)

        [Override hook] L3 portfolio risk-control clipping: rewrites the quantities in ``suggestions`` in place.

        The default base-class implementation caps the per-instrument concentration by ``max_single_position`` (skipped in mode B).
        It is used when this hook is not overridden.

        :param str mode: the running mode ("A"/"B"/"C") owned by the aggregate system

    .. py:method:: _reset(self)

        [Override hook] The subclass reset hook: resets the internal private state.
