.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

资产分配算法组件
================

内建资产分配算法
-----------------

系统权重系数结构
-----------------

.. py:class:: SystemWeight

    系统权重系数结构，在资产分配时，指定对应系统的资产占比系数

    .. py:attribute:: sys 对应的 System 实例
    .. py:attribute:: weight 对应的权重系数，有效范围为 [0, 1] 


.. py:class:: SystemWeightList

    由系统权重系数结构组成的列表


资产分配算法基类
------------------

.. py:class:: AllocateFundsBase

    资产分配算法基类
    
