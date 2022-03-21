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