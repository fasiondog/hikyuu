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

   Starting with ``feature/next``, the **responsibilities of money management (MM) and fund allocation (AF) are separated**:

   * :class:`MoneyManagerBase` is restricted to the **single-system / single-security** scope (it operates on a single ``Stock``),
     handling single-system position sizing and no longer carrying the portfolio-level allocation;
   * the portfolio-level allocation (L1 system-level weights / L2 behavior-level conversion / L3 portfolio risk control) has been migrated in its entirety to
     :class:`AllocateFundsBase`.

   In other words, the fund allocation logic previously carried by the portfolio-level MMs and the standalone
   ``Portfolio`` is now handled by ``AF_*``.

Allocation modes (A / B / C)
----------------------------

AF selects the allocation mode through the :attr:`AllocateFundsBase.mode` attribute, which determines the overall
running form of the aggregate system. The mode is owned by the AF (the mode query of ``MultiSystem`` is delegated to it):

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
        (target total assets) and calibrates (recycle the unselected -> liquidate -> reduce the over quota ->
        inject the gap); the sub-systems trade autonomously within their quotas and the parent **mirrors
        continuously** their real instructions (the main account = the sum of the sub-system positions)
    * - ``"C"``
      - Shared Account Compatibility
      - No shadow accounts; the sub-systems trade directly on the parent real account
      - Compatible with the old ``Portfolio`` semantics (the driven set is decided by the SE semantics); the parent
        performs no L2 conversion and the AF acts only as the mode carrier

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
      - Portfolio risk-control clipping (e.g. the ``max-single-position`` concentration cap)
      - Off or total-only (the ``max-total-position`` quota-sum cap is carried at L1)
      - Off and cannot be enabled

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
      - ``_allocate(date, tm, contexts, query)``
      - Sub-system context -> weight ``{System: weight}``; in mode B it writes the actual quota back to ``contexts[i].quota`` in place
    * - L2 behavior-level conversion
      - ``_to_targets(date, tm, suggestions, sys_weight, query)``
      - Weights -> executable quantities for the parent account (in mode A, order quantities are converted proportionally; in mode B, sub-system instructions are passed through)
    * - L3 portfolio risk control
      - ``_check_risk(date, tm, suggestions, query)``
      - Portfolio-level risk-control clipping (e.g. the ``max-single-position`` concentration cap)

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
    * - ``max-single-position``
      - float, 1.0
      - A only
      - The L3 per-instrument concentration cap: the target market value of one instrument must not exceed
        the total assets x this value. <=0 disables the limit; a value above 1 takes effect as 1.0
    * - ``max-total-position``
      - float, 1.0
      - A / B
      - The total budget cap: in mode A it bounds the aggregated target market value of all instruments (L2)
        by the total assets x this value; in mode B it bounds the **sum of the sub-system quotas** (L1) by
        the total assets x this value. Scaled down proportionally with a warning when exceeded.
        <=0 disables the cap
    * - ``fixed-amount``
      - float, 0.0
      - A / B
      - The pricing switch: when >0 it **takes precedence over the weight pricing** with a warning.
        In mode A it is the target market value per instrument; in mode B the fixed quota per sub-system.
        The L1 weights then carry only the ranking meaning and take no part in the pricing
    * - ``adjust_running_sys``
      - bool, True
      - B only
      - Whether to force-adjust the **running sub-systems** to the new quota on the rebalancing day.
        A running sub-system = one that was funded in a previous period and still holds assets at the
        calibration time. ``True``: runs the full calibration (recycle the unselected -> liquidate ->
        reduce the over quota -> inject the gap); ``False``: leaves the running sub-systems untouched
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
        and the part exceeding the total budget relies on ``max-total-position`` to scale down
    * - Constant weights never trigger a reduction
      - The constant-weight AFs (e.g. ``AF_FixedWeightList``) recompute the quota from the **current** parent
        total assets on every period, so the sub value and the quota grow in the same proportion and the
        reduction is never triggered by the normal market movement; the force reduction only happens on a
        real cross-period shrink of the weights or the funds (and never happens when
        ``adjust_running_sys=False``)


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
    The weights must be finite, non-negative and not all zero (an all-zero list sets every sub-system target market value to 0 and drives the parent to actively clear the existing holdings on every rebalance, so the parameter check throws).

    :param list weights: the specified list of weight proportions
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedAmount([amount = 20000.0])

    Fixed-amount fund allocation (L1 equal weight + L2 fixed amount): allocates an equal amount of capital to each selected instrument.

    The current implementation requires ``fixed-amount >= 0`` and rebalances by the net amount.

    :param float amount: the maximum trade amount
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_MultiFactor([auto_adjust_weight = True])

    Creates a MultiFactor score-weighted fund allocation algorithm instance, i.e. one that uses the scores returned by the selector
    (SE; ``SubSystemContext.score``) as the weights (a non-finite score is treated as 0, an all-zero or non-finite sum falls back to the equal weight).

    :param bool auto_adjust_weight: the score normalization switch (default ``True``). When ``True`` the scores are
        normalized to sum 1 before allocating (a score sum less than 1 also allocates fully; see the notes above);
        when ``False`` the score itself is the final weight (no normalization) and
        the part exceeding the total budget relies on ``max-total-position`` to scale down
    :return: the fund allocation algorithm instance (``AFPtr``)


Custom Asset Allocation Algorithm
---------------------------------

``crtAF`` is the recommended way to build a custom AF quickly: only the L1 allocation callback is required; L2 / L3 are injected as needed.

.. py:function:: crtAF(allocate_func[, params, name, to_targets_func, check_risk_func])

    Quickly creates a fund allocation algorithm

    :param allocate_func: the L1 system-level allocation callback ``func(self, date, tm, contexts, query)``,
        returning ``{System: weight}``; in mode B it writes the quota back to ``contexts[i].quota`` in place and returns an empty table
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param to_targets_func: the L2 behavior-level conversion callback ``func(self, date, tm, suggestions, sys_weight, query)``, optional
    :param check_risk_func: the L3 portfolio risk-control callback ``func(self, date, tm, suggestions, query)``, optional
    :return: the custom fund allocation algorithm instance (``AFPtr``)

The following ``my_allocate`` demonstrates both mode A (returning the weight table) and mode B (writing back the quota):

.. code-block:: python

    from hikyuu import *

    def my_allocate(self, date, tm, contexts, query):
        # L1 [Required]: sub-system context -> weight
        #   Mode A: return {ctx.sys: weight}
        #   Mode B: write ctx.quota directly and return an empty table
        if self.mode == "B":
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
    .. py:attribute:: mode  The allocation mode: ``"A"`` Signal Aggregation (the default; the parent reads the sub-systems' current intent state, synthesizes the net target and orders uniformly), ``"B"`` Fund Allocation (on the rebalancing day the selected sub-systems are calibrated to their quota before being driven, and the parent mirrors their real instructions continuously) or ``"C"`` Shared Account Compatibility (the sub-systems trade on the parent real account, no L2 conversion)

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

    .. py:method:: allocate(self, date, tm, suggestions, contexts, query)

        The unified entry point for L1/L2/L3 (usually called internally by :class:`MultiSystem`): it first runs ``_allocate`` to obtain the weights,
        then converts them into executable quantities via ``_to_targets``, and finally applies risk-control clipping via ``_check_risk``.

        :param Datetime date: the allocation date
        :param TradeManager tm: the parent-account trade manager
        :param TradeSuggestionList suggestions: the sub-system trade suggestions (rewritten in place)
        :param list contexts: the sub-system contexts (``SubSystemContext``; in mode B their ``quota`` field is written back in place)
        :param KQuery query: the query conditions

    .. py:method:: _allocate(self, date, tm, contexts, query)

        [Override hook] L1 system-level allocation: determines the weight of each sub-system from the sub-system contexts.

        Mode A returns ``{System: weight}``; mode B writes the quota back to ``contexts[i].quota`` in place and returns an empty table.

        :return: the weight table (``{System: weight}``)

    .. py:method:: _to_targets(self, date, tm, suggestions, sys_weight, query)

        [Override hook] L2 behavior-level conversion: converts the weights into executable quantities for the parent account, rewriting ``suggestions`` in place.

        In mode A it converts the order quantities by ``assets_ratio`` (overallocation is turned into position reduction; SELL/CLEAR close the full position);
        in mode B it passes the sub-system instructions through and applies defensive clipping. The base class default implementation is used when this hook is not overridden.

    .. py:method:: _check_risk(self, date, tm, suggestions, query)

        [Override hook] L3 portfolio risk-control clipping: rewrites the quantities in ``suggestions`` in place.

        The default base-class implementation caps the per-instrument concentration by ``max-single-position`` (skipped in mode B).
        It is used when this hook is not overridden.

    .. py:method:: _reset(self)

        [Override hook] The subclass reset hook: resets the internal private state.
