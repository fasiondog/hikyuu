.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

资产分配算法组件
================

资产分配算法组件，用于对选中的系统进行资产分配。

公共参数:

    * **adjust_running_sys** *(bool|True)* : 是否调整之前已经持仓策略的持仓。不调整时，仅使用总账户当前剩余资金进行分配，否则将使用总市值进行分配。
    
        - True: 主动根据资产分配对已持仓策略进行增减仓, 
        - False: 不会根据当前分配权重对已持仓策略进行强制加减仓

    * **auto_adjust_weight** *(bool|True)* : 自动调整权重，此时认为传入的权重为各证券的相互比例（详见ignore_zero_weight说明）。否则，以传入的权重为指定权重不做调整（此时传入的各个权重需要小于1）。

    * **ignore_zero_weight** *(bool|False)* : 该参数在 auto_adjust_weight 为 True 时生效。是否过滤子类返回的比例权重列表中的 0 值（包含小于0）和 nan 值。
   
        :: 
        
            如: 子类返回权重比例列表 [6, 2, 0, 0, 0], 则
               - 过滤 0 值, 则实际调整后的权重为 Xi / sum(Xi): [6/8, 2/8]
               - 不过滤, m 设为非零元素个数, n为总元素个数, (Xi / Sum(Xi)) * (m / n):
                  [(6/8)*(2/5), (2/8)*(2/5), 0, 0, 0] 即保留分为5份后, 仅在2份中保持相对比例

    * **ignore_se_score_is_null** *(bool|False)* : 忽略选中系统列表中的系统得分为 null 的系统。 **注意: 某些SE(如SE_MultiFactor)本身可能也存在类似控制**
    * **ignore_se_score_lt_zero** *(bool|False)* : 忽略选中系统列表中的系统得分小于等于 0 的系统
    * **reserve_percent** *(float|0.0)* : 资产占比保留比例，小于该比例的资产将被忽略。
    * **trace** *(bool|False)* : 打印跟踪信息


内建资产分配算法
------------------

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>代码</th>
                <th>名称</th>
                <th>描述</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><a href="#target-section">AF_FixedWeight</a></td>
                <td>固定比例资产分配</td>
                <td>每个选中的资产都只占总资产固定的比例</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_FixedWeightList</td>
                <td>固定比例资产分配列表</td>
                <td>按指定的权重列表对选中系统进行资产分配</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_EqualWeight</td>
                <td>固定比例资产分配</td>
                <td>对选中的资产进行等比例分配</td>
            </tr>
            <tr>
                <td><a href="#target-section">AF_MultiFactor</td>
                <td>多因子评分权重资产分配</td>
                <td>根据系统得分进行资产分配，对选中的系统进行得分排序，按得分从高到低进行资产分配，得分为0的系统将被忽略。</td>
            </tr>    
        </tbody>
    </table>
    <p></p>

.. py:function:: AF_FixedWeight(weight)

    固定比例资产分配，每个选中的资产都只占总资产固定的比例

    :param float weight:  指定的资产比例 [0, 1]

.. py:function:: AF_FixedWeightList(weights)

    固定比例资产分配列表.

    :param float weights:  指定的资产比例列表


.. py:function:: AF_EqualWeight()

    固定比例资产分配，对选中的资产进行等比例分配


.. py:function:: AF_MultiFactor()

    根据系统得分进行资产分配，对选中的系统进行得分排序，按得分从高到低进行资产分配，得分为0的系统将被忽略。


系统权重系数结构
-----------------

.. py:class:: SystemWeight

    系统权重系数结构，在资产分配时，指定对应系统的资产占比系数

    .. py:attribute:: sys 对应的 System 实例
    .. py:attribute:: weight 对应的权重系数，有效范围为 [0, 1] 


.. py:class:: SystemWeightList

    由系统权重系数结构组成的列表

    .. py:attribute:: sys  
    
        对应的 System 实例

    .. py::attribute weight

        对应的权重系数，有效范围为 [0, 1]


资产分配算法基类
------------------

.. py:class:: AllocateFundsBase

    资产分配算法基类, 子类接口：

    - _allocateWeight : 【必须】子类资产分配调整实现
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量

    .. py:attribute:: name 名称
    
    .. py:method:: __init__(self[, name="AllocateFundsBase])
    
        初始化构造函数
        
        :param str name: 名称

    .. py:method:: have_param(self, name)

        指定的参数是否存在
        
        :param str name: 参数名称
        :return: True 存在 | False 不存在

    .. py:method:: get_param(self, name)

        获取指定的参数
        
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: set_param(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: reset(self)
    
        复位操作
    
    .. py:method:: clone(self)
    
        克隆操作        
        
    .. py:method:: _calculate(self)
    
        【重载接口】子类计算接口
    
    .. py:method:: _reset(self)
    
        【重载接口】子类复位接口，复位内部私有变量

    .. py::method:: _allocate_weight(self, date, se_list)

        【重载接口】子类分配权重接口，获取实际分配资产的系统实例及其权重

        :param Datetime date: 当前时间
        :param SystemList se_list: 当前选中的系统列表
        :return: 系统权重分配信息列表
        :rtype: SystemWeightList