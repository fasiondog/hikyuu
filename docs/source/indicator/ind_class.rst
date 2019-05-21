.. py:currentmodule:: hikyuu.indicator
.. highlightlang:: python

Indicator
============


.. py:class:: Indicator(name)

    Indicator 指标定义

    .. py:attribute:: name 名称

    .. py:attribute:: long_name 名称

    .. py:attribute:: discard 需抛弃的点数

    .. py:method:: getParam(self, name)

    .. py:method:: getParam(self, name)

        获取指定的参数
    
        :param str name: 参数名称
        :return: 参数值
        :raises out_of_range: 无此参数
        
    .. py:method:: setParam(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! 不支持的参数类型

    .. py:method:: clone(self)
    
        克隆操作

    .. py:method:: empty(self)

        是否为空

        :rtype: bool

    .. py:method:: formula(self)

        打印指标公式

        :rtype: str

    .. py:method:: getResultNumber(self)

        获取结果集数量

        :rtype: int

    .. py:method:: get(self, pos[, num=0])

        获取指定位置的值

        :param int pos: 指定的位置索引
        :param int num: 指定的结果集

    .. py:method:: getDatetime(self, pos)

        获取指定位置的日期

        :param int pos: 指定的位置索引

    .. py:method:: getByDate(self, date[, num=0])

        获取指定日期数值。如果对应日期无结果，返回 constant.null_price

        :param Datetime date: 指定日期
        :param int num: 指定的结果集
        :rtype: float

    .. py:method:: getResult(self, num)

        获取指定结果集

        :param int num: 指定的结果集
        :rtype: Indicator

    .. py:method:: getResultAsPriceList(self, num)

        获取指定结果集

        :param int num: 指定的结果集
        :rtype: PriceList

    .. py:method:: getDatetimeList(self)

        返回对应的日期列表

        :rtype: DatetimeList

    .. py:method:: getContext(self)

        获取上下文

        :rtype: KData

    .. py:method:: setContext(self, kdata)

        设置上下文

        :param KData kdata: 关联的上下文K线

    .. py:method:: setContext(self, stock, query)

        设置上下文

        :param Stock stock: 指定的 Stock
        :param Query query: 指定的查询条件