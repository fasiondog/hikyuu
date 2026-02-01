.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Indicator
============


.. py:class:: Indicator(name)

    Indicator 指标定义

    .. py:attribute:: name 名称

    .. py:attribute:: long_name 名称

    .. py:attribute:: discard 需抛弃的点数

    .. py:method:: have_param(self, name)

        是否存在指定参数

        :param str name: 参数名称

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

    .. py:method:: support_ind_param(self)

        是否支持动态指标参数

    .. py:method:: have_ind_param(self, name)

        是否存在指定参数

        :param str name: 参数名称

    .. py:method:: get_ind_param(self, name)

        获取指定的动态指标参数

        :param str name: 参数名称
        :rtype: IndParam

    .. py::method:: set_ind_param(self, name, val)

        设置动态指标参数

        :param str name: 参数名称
        :param Indicator|IndParam: 参数值（可为 Indicator 或 IndParam 实例）    

    .. py:method:: clone(self)
    
        克隆操作

    .. py:method:: empty(self)

        是否为空

        :rtype: bool

    .. py:method:: formula(self)

        打印指标公式

        :rtype: str

    .. py:method:: get_result_num(self)

        获取结果集数量

        :rtype: int

    .. py:method:: get(self, result_index[, num=0])

        获取指定位置的值

        :param int pos: 指定的位置索引
        :param int num: 指定的结果集

    .. py::method:: get_pos(self, date):

        获取指定日期相应的索引位置

        :param Datetime date: 指定日期
        :rtype: int

    .. py:method:: get_datetime(self, pos)

        获取指定位置的日期

        :param int pos: 指定的位置索引

    .. py:method:: get_by_datetime(self, datetime[, result_index=0])

        获取指定日期数值。如果对应日期无结果，返回 constant.null_price

        :param Datetime datetime: 指定日期
        :param int num: 指定的结果集
        :rtype: float

    .. py:method:: get_result(self, result_index)

        获取指定结果集

        :param int result_index: 指定的结果集
        :rtype: Indicator

    .. py:method:: get_result_as_price_list(self, result_index)

        获取指定结果集

        :param int result_index: 指定的结果集
        :rtype: list

    .. py:method:: get_datetime_list(self)

        返回对应的日期列表

        :rtype: DatetimeList

    .. py:method:: get_context(self)

        获取上下文

        :rtype: KData

    .. py:method:: set_context(self, kdata)

        设置上下文

        :param KData kdata: 关联的上下文K线

        set_context(self, stock, query)

        设置上下文

        :param Stock stock: 指定的 Stock
        :param Query query: 指定的查询条件

    .. py:method:: extend(self)

        在有上下文时，自动将上下文扩展至当前最新数据并进行增量计算

        增量计算时，相当于当前query的起点不变，向后扩展至新的结束点。该特性对部分递归型指标对初值敏感，如AMA等可能存在部分差异

        主要用途：实盘时，快速获取最新数据进行计算

    .. py:method:: to_array(self, result_index=0)

        将指定结果集转换为 numpy.array

    .. py:method:: to_numpy(self)

        转换为 numpy 数组, 如果为时间序列，将包含 datetime 列

        :rtype: numpy.ndarray

    .. py:method:: value_to_numpy(self)

        转换为 numpy 数组, 仅包含 value

    .. py:method:: to_pandas(self)

        转换为 pandas DataFrame, 如果为时间序列，将包含 datetime 列

    .. py:method:: value_to_pandas(self)

        转换为 pandas DataFrame, 仅包含 value

    .. py:method:: to_pyarrow(self)

        转换为 pyarrow Table, 如果为时间序列，将包含 datetime 列

        :rtype: pyarrow.Table

    .. py:method:: value_to_pyarrow(self)

        转换为 pyarrow Table, 仅包含 value

        :rtype: pyarrow.Table


    
