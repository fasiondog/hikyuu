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

   In other words, the portfolio-level MMs in master, such as `MM_FixedWeight` / `MM_FixedWeightList` / `MM_MultiFactor`,
   together with the fund allocation logic of the standalone ``Portfolio``, are now handled by ``AF_*``.

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

    * **weight-list** *(string|"")* : the L1 fixed-weight list (comma-separated); when it is non-empty and its length matches the number of sub-systems, the entries are applied in order,
      otherwise AF falls back to equal weights, and the weights are normalized automatically when their sum is not 1.
    * **fixed-amount** *(float|0.0)* : the L2 fixed amount: in mode A it is the target market value per instrument, in mode B the fixed quota per sub-system.
    * **max-single-position** *(float|1.0)* : the L3 per-instrument concentration cap: the target market value of a position must not exceed total assets multiplied by this value
      (<=0 or >=1 disables the limit).

.. note::

   The master parameters ``adjust_running_sys`` / ``auto_adjust_weight`` / ``ignore_zero_weight`` / ``reserve_percent``
   have **no dedicated switches** in v5: the semantics of ``auto_adjust_weight`` are carried directly by the "**no normalization**" behavior of ``AF_FixedWeight`` /
   ``AF_FixedWeightList``; the remaining parameters have no counterparts and must be handled by the user.


Built-in Asset Allocation Algorithms
------------------------------------

.. py:function:: AF_EqualWeight()

    Equal-weight fund allocation: allocates capital to the selected instruments in equal proportions (L1 equal weight 1/N).

    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeight([weight = 0.1])

    Fixed-weight fund allocation: each selected instrument accounts for a fixed proportion of total assets.

    L1 returns the fixed ``weight`` directly, **without normalization** (equivalent to master's ``auto_adjust_weight=False``).

    :param float weight: the specified weight as a proportion of total assets (0, 1]
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeightList(weights)

    Fixed-weight-list fund allocation: allocates capital to the selected sub-systems according to the specified weight list.

    L1 takes ``weights[i]`` in sub-system order, **without normalization**; when the length does not match, it falls back to equal weights.

    :param list weights: the specified list of weight proportions
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedAmount([amount = 20000.0])

    Fixed-amount fund allocation (L1 equal weight + L2 fixed amount): allocates an equal amount of capital to each selected instrument.

    .. note::

       The old master implementation asserts ``amount > 500`` and stops once the remaining cash drops below ``0.6 x amount``;
       the v5 implementation requires ``fixed-amount >= 0`` and rebalances by the net amount, so the **boundary behavior differs**.

    :param float amount: the maximum trade amount
    :return: the fund allocation algorithm instance (``AFPtr``)

.. py:function:: AF_MultiFactor()

    Creates a MultiFactor score-weighted fund allocation algorithm instance, i.e. one that uses the scores returned by the selector
    (SE; ``SubSystemContext.score``) directly as the weights.

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
        # L1 [Required]: sub-system context -> weight (corresponds to master's _allocateWeight)
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

Migration mapping:

.. list-table::
    :header-rows: 1

    * - master
      - v5
    * - ``class MyAF(AllocateFundsBase)``
      - ``class MyAF(AllocateFundsBase)`` (the class of the same name is retained, with unchanged semantics)
    * - ``_allocateWeight(date, se_list)``
      - ``_allocate(date, tm, contexts, query)`` (L1, system-level weights / quotas)
    * - (normalization is performed afterward by the framework)
      - ``_to_targets(...)`` (L2: in mode A it converts the quantities for the parent account; in mode B it passes instructions through)
    * - —
      - ``_check_risk(...)`` (L3, portfolio risk-control clipping, e.g. ``max-single-position``)
    * - ``crtAF(func)``
      - ``crtAF(func)`` (only the callback signature is updated to ``_allocate(date, tm, contexts, query)``; ``to_targets_func`` / ``check_risk_func`` can be added)


Asset Allocation Algorithm Base Class
-------------------------------------

.. py:class:: AllocateFundsBase

    The fund allocation algorithm (AF) base class, used exclusively by the aggregate system (MultiSystem).

    .. py:attribute:: name  Name
    .. py:attribute:: tm    Set or get the trade manager instance
    .. py:attribute:: query Set or get the query conditions
    .. py:attribute:: mode  The allocation mode: ``"A"`` (signal aggregation + unified parent-account ordering) or ``"B"`` (quota allocation + next-period quota write-back)

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
