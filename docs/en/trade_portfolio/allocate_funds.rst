.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Asset Allocation Algorithm Component|AF
=======================================

The asset allocation algorithm component, used to allocate the assets to the selected systems.

Common parameters:

    * **adjust_running_sys** *(bool|True)* : Whether to adjust the positions of the strategies already holding positions. When not adjusting, only the current remaining funds of the total account are used for the allocation; otherwise the total market value is used.
    
        - True: actively increase or decrease the positions of the holding strategies according to the asset allocation, 
        - False: the holding strategies will not be forcibly increased or decreased according to the current allocated weights

    * **auto_adjust_weight** *(bool|True)* : Adjust the weights automatically; in this case the passed weights are considered to be the mutual ratios of the securities (see the ignore_zero_weight description). Otherwise, the passed weights are used as the specified weights without adjustment (in this case each passed weight needs to be less than 1).

    * **ignore_zero_weight** *(bool|False)* : This parameter takes effect when auto_adjust_weight is True. Whether to filter the 0 values (including those less than 0) and the nan values in the ratio weight list returned by the subclass.
   
        :: 
        
            E.g.: if the subclass returns the weight ratio list [6, 2, 0, 0, 0], then
               - filtering the 0 values, the actually adjusted weight is Xi / sum(Xi): [6/8, 2/8]
               - not filtering, let m be the number of the non-zero elements and n the total number of elements, (Xi / Sum(Xi)) * (m / n):
                  [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0], i.e. after keeping it divided into 5 shares, the relative ratio is kept only in 2 shares

    * **ignore_se_score_is_null** *(bool|False)* : Ignore the systems whose score is null in the selected system list. **Note: some SEs (e.g. SE_MultiFactor) may also have a similar control themselves**
    * **ignore_se_score_lt_zero** *(bool|False)* : Ignore the systems whose score is less than or equal to 0 in the selected system list
    * **reserve_percent** *(float|0.0)* : The reserved ratio of the asset proportion; the assets with a proportion smaller than this ratio will be ignored.
    * **trace** *(bool|False)* : Print the tracking information


Built-in Asset Allocation Algorithms
------------------------------------

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
                <td>Fixed-ratio asset allocation</td>
                <td>Every selected asset only takes a fixed proportion of the total assets</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_FixedAmount</a></td>
                <td>Fixed-amount asset allocation</td>
                <td>Every selected asset only takes a fixed amount of the total assets</td>
            </tr>            
            <tr>
                <td><a href="#target-section">AF_FixedWeightList</a></td>
                <td>Fixed-ratio asset allocation list</td>
                <td>Allocate the assets to the selected systems by the specified weight list</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_EqualWeight</a></td>
                <td>Fixed-ratio asset allocation</td>
                <td>Allocate the selected assets with an equal ratio</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_MultiFactor</a></td>
                <td>Multi-factor scoring weight asset allocation</td>
                <td>Allocate the assets by the system score; sort the selected systems by the score and allocate the assets from the highest score to the lowest; the systems with a score of 0 will be ignored.</td>
            </tr>    
        </tbody>
    </table>
    <p></p>

.. py:function:: AF_FixedWeight(weight)

    The fixed-ratio asset allocation; every selected asset only takes a fixed proportion of the total assets

    :param float weight:  the specified asset proportion [0, 1]

.. py:function:: AF_FixedAmount(amount)

    The fixed-amount asset allocation; every selected asset only takes a fixed amount of the total assets

    :param float amount:  the specified asset amount

.. py:function:: AF_FixedWeightList(weights)

    The fixed-ratio asset allocation list.

    :param float weights:  the specified asset proportion list


.. py:function:: AF_EqualWeight()

    The fixed-ratio asset allocation; allocate the selected assets with an equal ratio


.. py:function:: AF_MultiFactor()

    Allocate the assets by the system score; sort the selected systems by the score and allocate the assets from the highest score to the lowest; the systems with a score of 0 will be ignored.





System Weight Structure
-----------------------

.. py:class:: SystemWeight

    The system weight structure; during the asset allocation, it specifies the asset proportion coefficient of the corresponding system

    .. py:attribute:: sys The corresponding System instance
    .. py:attribute:: weight The corresponding weight coefficient, with the valid range [0, 1] 


.. py:class:: SystemWeightList

    The list composed of the system weight structures

    .. py:attribute:: sys  
    
        The corresponding System instance

    .. py::attribute weight

        The corresponding weight coefficient, with the valid range [0, 1]


Asset Allocation Algorithm Base Class
-------------------------------------

.. py:class:: AllocateFundsBase

    The asset allocation algorithm base class; the subclass interfaces:

    - _allocateWeight : [Required] The subclass asset allocation adjustment implementation
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables

    .. py:attribute:: name Name
    
    .. py:method:: __init__(self[, name="AllocateFundsBase])
    
        The initialization constructor
        
        :param str name: the name

    .. py:method:: have_param(self, name)

        Whether the specified parameter exists
        
        :param str name: the parameter name
        :return: True exists | False does not exist

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

    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: _calculate(self)
    
        [Overload interface] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Overload interface] The subclass reset interface, resetting the internal private variables

    .. py::method:: _allocate_weight(self, date, se_list)

        [Overload interface] The subclass weight allocation interface, getting the actually allocated system instances and their weights

        :param Datetime date: the current time
        :param SystemList se_list: the list of the currently selected systems
        :return: the list of the system weight allocation information
        :rtype: SystemWeightList
