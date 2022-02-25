.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

资产分配算法组件
================

内建资产分配算法
------------------

.. py:function:: AF_FixedWeight(weight)

    固定比例资产分配，每个选中的资产都只占总资产固定的比例

    :param float weight:  指定的资产比例 [0, 1]


.. py:function:: AF_EqualWeight()

    固定比例资产分配，对选中的资产进行等比例分配


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

    
    
