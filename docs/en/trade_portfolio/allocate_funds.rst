.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _allocate_funds:

Asset Allocation Algorithm Component|AF
=======================================

The asset allocation algorithm component (AllocateFunds, AF) is used to perform the **portfolio-level** fund allocation for the sub-systems selected in the aggregate system :class:`MultiSystem`. The operation domain of AF is the "sub-system set" (a set of ``SYSPtr``), every :class:`MultiSystem` holds one AF instance (see :doc:`portfolio`).

.. note::

   Since ``feature/next``, the **responsibilities of the money management (MM) and the asset allocation (AF) are separated**:

   * :class:`MoneyManagerBase` is restricted to the **single-system / single-security** form (the operation domain is a single ``Stock``),
     responsible for the single-system sizing, no longer carrying the portfolio-level allocation;
   * The portfolio-level allocation (L1 system-level weight / L2 behavior-level conversion / L3 portfolio risk control) is entirely migrated to
     :class:`AllocateFundsBase`.

   That is, the portfolio-level MMs such as `MM_FixedWeight` / `MM_FixedWeightList` / `MM_MultiFactor` in master,
   and the fund allocation logic of the independent ``Portfolio``, are now taken over by ``AF_*``.

The three replaceable algorithm parts of AF
-------------------------------------------

AF splits the portfolio-level allocation into three **independently replaceable** algorithm parts (each corresponding to one overload interface of the base class),
and :meth:`AllocateFundsBase.allocate` chains them into a pipeline:

.. list-table::
    :header-rows: 1

    * - Part
      - Overload interface
      - Semantics
    * - L1 system-level allocation
      - ``_allocate(date, tm, contexts, query)``
      - Sub-system context -> weight ``{System: weight}``; in mode B it writes back ``contexts[i].quota`` (the real quota) in place
    * - L2 behavior-level conversion
      - ``_to_targets(date, tm, suggestions, sys_weight, query)``
      - Weight -> the executable quantity of the parent account (in mode A the order quantity is converted by proportion; in mode B the sub-system instructions are passed through)
    * - L3 portfolio risk control
      - ``_check_risk(date, tm, suggestions, query)``
      - The portfolio-level risk control clipping (e.g. the ``max-single-position`` concentration upper limit)

``_to_targets`` and ``_check_risk`` both have default implementations, usually only ``_allocate`` needs to be overridden.

Common parameters (portfolio-level)
-----------------------------------

    * **weight-list** *(string|"")* : the L1 fixed weight list (comma separated); when it is not empty and its size matches the number of the sub-systems, it is used in order,
      otherwise it falls back to the equal weight, and it is normalized automatically when the weight sum is not 1.
    * **fixed-amount** *(float|0.0)* : the L2 fixed amount; in mode A it is the target market value of every instrument, in mode B it is the fixed quota of every sub-system.
    * **max-single-position** *(float|1.0)* : the L3 single-instrument concentration upper limit, the target position market value <= total assets x this value
      (<=0 or >=1 means no limit).

.. note::

   The parameters such as ``adjust_running_sys`` / ``auto_adjust_weight`` / ``ignore_zero_weight`` / ``reserve_percent`` in master
   have **no independent switch** in v5: the semantics of ``auto_adjust_weight`` is directly carried by the "**no normalization**" of ``AF_FixedWeight`` /
   ``AF_FixedWeightList``; the remaining parameters have no correspondence and need to be handled by the user themselves.


Built-in Asset Allocation Algorithms
------------------------------------

.. py:function:: AF_EqualWeight()

    The equal weight asset allocation, it allocates the selected assets in equal proportions (L1 equal weight 1/N).

    :return: the asset allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeight([weight = 0.1])

    The fixed proportion asset allocation, every selected asset only accounts for a fixed proportion of the total assets.

    L1 directly returns the fixed ``weight``, **without normalization** (equivalent to the master ``auto_adjust_weight=False``).

    :param float weight: the specified asset proportion (0, 1]
    :return: the asset allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedWeightList(weights)

    The fixed proportion asset allocation list, it allocates the assets to the selected systems by the specified weight list.

    L1 takes ``weights[i]`` one by one in the sub-system order, **without normalization**; when the quantity does not match it falls back to the equal weight.

    :param list weights: the specified asset proportion list
    :return: the asset allocation algorithm instance (``AFPtr``)

.. py:function:: AF_FixedAmount([amount = 20000.0])

    The fixed amount asset allocation (L1 equal weight + L2 fixed amount), it allocates the selected assets with an equal amount.

    .. note::

       The old master implementation asserts ``amount > 500`` and stops when the remaining cash ``< 0.6 x amount``;
       the v5 implementation requires ``fixed-amount >= 0`` and rebalances by the net amount, the **boundary behavior is different**.

    :param float amount: the maximum trade amount
    :return: the asset allocation algorithm instance (``AFPtr``)

.. py:function:: AF_MultiFactor()

    Create a MultiFactor scoring weight asset allocation algorithm instance, i.e. directly taking the scores returned by SE
    (``SubSystemContext.score``) as the weights.

    :return: the asset allocation algorithm instance (``AFPtr``)


Custom Asset Allocation Algorithm
---------------------------------

It is recommended to use ``crtAF`` to quickly construct a custom AF: only the L1 allocation callback is needed, L2 / L3 are injected as needed.

.. py:function:: crtAF(allocate_func[, params, name, to_targets_func, check_risk_func])

    Quickly create an asset allocation algorithm

    :param allocate_func: the L1 system-level allocation interface ``func(self, date, tm, contexts, query)``,
        returning ``{System: weight}``; in mode B it writes back ``contexts[i].quota`` in place and returns an empty table
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :param to_targets_func: the L2 behavior-level conversion interface ``func(self, date, tm, suggestions, sys_weight, query)``, optional
    :param check_risk_func: the L3 portfolio risk control interface ``func(self, date, tm, suggestions, query)``, optional
    :return: the custom asset allocation algorithm instance (``AFPtr``)

The following ``my_allocate`` demonstrates both mode A (returning the weight table) and mode B (writing back the quota):

.. code-block:: python

    from hikyuu import *

    def my_allocate(self, date, tm, contexts, query):
        # L1 [Required]: sub-system context -> weight (corresponding to the master _allocateWeight)
        #   Mode A: return {ctx.sys: weight}
        #   Mode B: write ctx.quota directly, and return an empty table
        if self.mode == "B":
            quota = tm.get_funds(date, query).total_assets / max(len(contexts), 1)
            for ctx in contexts:
                ctx.quota = quota
            return {}
        return {ctx.sys: 1.0 / len(contexts) for ctx in contexts}

    af = crtAF(my_allocate)

    # L2 [Optional] and L3 [Optional] are injected as needed (the base class default implementation is used when not injected):
    # af = crtAF(my_allocate, to_targets_func=my_to_targets, check_risk_func=my_check_risk)

The equivalent writing is to inherit :class:`AllocateFundsBase` directly, implement ``_clone`` and override
``_allocate`` / ``_to_targets`` / ``_check_risk`` as needed.

Migration comparison:

.. list-table::
    :header-rows: 1

    * - master
      - v5
    * - ``class MyAF(AllocateFundsBase)``
      - ``class MyAF(AllocateFundsBase)`` (the class of the same name is kept, the semantics is unchanged)
    * - ``_allocateWeight(date, se_list)``
      - ``_allocate(date, tm, contexts, query)`` (L1, the system-level weight / quota)
    * - (the normalization is post-processed by the framework)
      - ``_to_targets(...)`` (L2, in mode A it converts the parent account quantity; in mode B it passes through)
    * - —
      - ``_check_risk(...)`` (L3, the portfolio risk control clipping, e.g. ``max-single-position``)
    * - ``crtAF(func)``
      - ``crtAF(func)`` (only the callback signature is updated to ``_allocate(date, tm, contexts, query)``, and ``to_targets_func`` / ``check_risk_func`` can be added)


Asset Allocation Algorithm Base Class
-------------------------------------

.. py:class:: AllocateFundsBase

    The asset allocation algorithm (AF) base class, used by the aggregate system (MultiSystem) only.

    .. py:attribute:: name  Name
    .. py:attribute:: tm    Set or get the trade management object
    .. py:attribute:: query Set or get the query condition
    .. py:attribute:: mode  The allocation mode: ``"A"`` (signal aggregation + the parent uniform ordering) or ``"B"`` (quota allocation + the next-period quota write-back)

    .. py:method:: __init__(self[, name="AllocateFundsBase"])

        The initialization constructor

        :param str name: the name

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

    .. py:method:: have_param(self, name)

        Whether the specified parameter exists

        :param str name: the parameter name
        :return: whether it exists
        :rtype: bool

    .. py:method:: reset(self)

        The reset operation

    .. py:method:: clone(self)

        The clone operation

    .. py:method:: allocate(self, date, tm, suggestions, contexts, query)

        The unified entry of L1/L2/L3 (usually called internally by :class:`MultiSystem`): first execute ``_allocate`` to get the weights,
        then convert them into the executable quantity through ``_to_targets``, and finally clip them through ``_check_risk``.

        :param Datetime date: the allocation date
        :param TradeManager tm: the parent account trade management
        :param TradeSuggestionList suggestions: the sub-system trade suggestions (rewritten in place)
        :param list contexts: the sub-system contexts (``SubSystemContext``; in mode B the ``quota`` is written back in place)
        :param KQuery query: the query condition

    .. py:method:: _allocate(self, date, tm, contexts, query)

        [Overload interface] L1 system-level allocation: decide the weight of every sub-system by the sub-system context.

        Mode A returns ``{System: weight}``; mode B writes back ``contexts[i].quota`` in place and returns an empty table.

        :return: the weight table (``{System: weight}``)

    .. py:method:: _to_targets(self, date, tm, suggestions, sys_weight, query)

        [Overload interface] L2 behavior-level conversion: convert the weights into the executable quantity of the parent account, rewrite ``suggestions`` in place.

        Mode A converts the order quantity by ``assets_ratio`` (the over-allocation turns to the position reduction, SELL/CLEAR full close);
        mode B passes through the sub-system instructions and performs the defensive clipping. The base class default implementation is used when not overridden.

    .. py:method:: _check_risk(self, date, tm, suggestions, query)

        [Overload interface] L3 portfolio risk control clipping, rewrite the quantity of ``suggestions`` in place.

        The base class default implementation limits the single-instrument concentration by ``max-single-position`` (skipped in mode B).
        The base class default implementation is used when not overridden.

    .. py:method:: _reset(self)

        [Overload interface] The subclass reset interface, reset the internal private variables
