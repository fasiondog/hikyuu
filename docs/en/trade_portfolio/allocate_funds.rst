.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Fund Allocation (AF)
====================

The fund allocation (AF) part, used to allocate capital among the selected systems.

Common parameters:

    * **adjust_running_sys** *(bool|True)* : Whether to adjust the positions of strategies that are already holding positions. When disabled, only the cash currently remaining in the account is allocated; otherwise the allocation is made against the total market value.

        - True: actively scale in or out of the already-held strategies according to the fund allocation,
        - False: do not forcibly add to or reduce the already-held strategies to match the currently allocated weights

    * **auto_adjust_weight** *(bool|True)* : Adjust the weights automatically. When enabled, the supplied weights are treated as relative ratios between the securities (see the description of ignore_zero_weight). Otherwise, the supplied weights are used directly as the target weights without adjustment (in which case each supplied weight must be less than 1).

    * **ignore_zero_weight** *(bool|False)* : This parameter takes effect only when auto_adjust_weight is True. Whether to filter out the zero values (including negative values) and NaN values from the ratio-weight list returned by the subclass.

        ::

            E.g., if the subclass returns the ratio-weight list [6, 2, 0, 0, 0], then:
               - with zero values filtered, the actual adjusted weights are Xi / sum(Xi): [6/8, 2/8]
               - without filtering, let m be the number of nonzero elements and n the total number of elements; each weight becomes (Xi / sum(Xi)) * (m / n):
                  [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0], i.e. the capital is still divided into 5 shares, but the relative ratios are preserved only within 2 of them

    * **ignore_se_score_is_null** *(bool|False)* : Ignore systems whose score is null in the selected system list. **Note: some SEs (e.g. SE_MultiFactor) may provide a similar control themselves**
    * **ignore_se_score_lt_zero** *(bool|False)* : Ignore systems whose score is less than or equal to 0 in the selected system list
    * **reserve_percent** *(float|0.0)* : The reserve threshold for the allocation ratio; any allocation smaller than this ratio is ignored.
    * **trace** *(bool|False)* : Print trace information


Built-in Fund Allocation Algorithms
-----------------------------------

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>Code</th>
                <th>Name</th>
                <th>Description</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><a href="#target-section">AF_FixedWeight</a></td>
                <td>Fixed-weight fund allocation</td>
                <td>Each selected system receives a fixed proportion of the total assets</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_FixedAmount</a></td>
                <td>Fixed-amount fund allocation</td>
                <td>Each selected system receives a fixed amount from the total assets</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_FixedWeightList</a></td>
                <td>Fixed-weight list fund allocation</td>
                <td>Allocate funds to the selected systems according to the specified weight list</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_EqualWeight</a></td>
                <td>Equal-weight fund allocation</td>
                <td>Allocate funds equally among the selected systems</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_MultiFactor</a></td>
                <td>Multi-factor score-weighted fund allocation</td>
                <td>Allocate funds according to the system scores: sort the selected systems by score and allocate funds from the highest score to the lowest; systems with a score of 0 are ignored.</td>
            </tr>
        </tbody>
    </table>
    <p></p>

.. py:function:: AF_FixedWeight(weight)

    Fixed-weight fund allocation: each selected system receives a fixed proportion of the total assets.

    :param float weight:  the specified proportion of assets, in [0, 1]

.. py:function:: AF_FixedAmount(amount)

    Fixed-amount fund allocation: each selected system receives a fixed amount from the total assets.

    :param float amount:  the specified fixed amount

.. py:function:: AF_FixedWeightList(weights)

    Fund allocation from a fixed weight list.

    :param float weights:  the specified list of asset proportions


.. py:function:: AF_EqualWeight()

    Equal-weight fund allocation: allocate funds equally among the selected systems.


.. py:function:: AF_MultiFactor()

    Allocate funds according to the system scores: sort the selected systems by score and allocate funds from the highest score to the lowest; systems with a score of 0 are ignored.




System Weight Structure
-----------------------

.. py:class:: SystemWeight

    The system weight structure; during fund allocation, it specifies the proportion coefficient of the corresponding system.

    .. py:attribute:: sys The corresponding System instance
    .. py:attribute:: weight The corresponding weight coefficient, with a valid range of [0, 1]


.. py:class:: SystemWeightList

    A list composed of system weight structures.

    .. py:attribute:: sys

        The corresponding System instance

    .. py::attribute weight

        The corresponding weight coefficient, with a valid range of [0, 1]


Fund Allocation Algorithm Base Class
------------------------------------

.. py:class:: AllocateFundsBase

    Base class for fund allocation algorithms. The subclass interface:

    - _allocateWeight : [Required] The subclass implementation of the fund allocation adjustment
    - _clone : [Required] Subclass clone hook
    - _reset : [Optional] Reset internal member variables

    .. py:attribute:: name Name

    .. py:method:: __init__(self[, name="AllocateFundsBase])

        Constructor.

        :param str name: the name

    .. py:method:: have_param(self, name)

        Check whether the specified parameter exists.

        :param str name: the parameter name
        :return: True if it exists | False if it does not

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter.

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: raised if no such parameter exists

    .. py:method:: set_param(self, name, value)

        Set the value of a parameter.

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! Raised when the parameter type is not supported

    .. py:method:: reset(self)

        Reset the part to its initial state.

    .. py:method:: clone(self)

        Create and return a copy of this instance.

    .. py:method:: _calculate(self)

        [Override hook] Subclass calculation hook.

    .. py:method:: _reset(self)

        [Override hook] Subclass reset hook, used to reset internal private state.

    .. py::method:: _allocate_weight(self, date, se_list)

        [Override hook] Subclass weight-allocation hook, returning the systems that actually receive funds and their weights.

        :param Datetime date: the current time
        :param SystemList se_list: the list of currently selected systems
        :return: the list of system weight allocations
        :rtype: SystemWeightList
