.. py:currentmodule:: hikyuu
.. highlight:: python

杂项或辅助
=============

函数
------

.. py:function:: select()

    示例::
    
        #选出涨停股
        C = CLOSE()
        x = select(C / REF(C, 1) - 1 >= 0.0995))

    :param Indicator cond: 条件指标
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :param bool print_out: 打印选中的股票
    :rtype: 选中的股票列表

.. py:function:: get_log_level()

    获取当前日志打印级别
    
    :rtype: LOG_LEVEL
    
.. py:function:: set_log_level(level)

    设置日志打印级别
    
    :param LOG_LEVEL level: 指定的日志打印级别

.. py:function:: hku_save(var, filename)

    序列化，将hikyuu内建类型的变量（如Stock、TradeManager等）保存在指定的文件中，格式为XML。
    
    :param var: hikyuu内建类型的变量
    :param str filename: 指定的文件名

.. py:function:: hku_load(filename)

    将通过 hku_save 保存的变量，读取到var中。
    
    :param str filename: 待载入的序列化文件。
    :return: 之前被序列化保存的文件
    
.. py:function:: roundUp(arg1[, arg2=0])

    向上截取，如10.1截取后为11
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据


.. py:function:: roundDown(arg1[, arg2=0])

    向下截取，如10.1截取后为10
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据
    
    
.. py:function:: getDateRange(start, end)

    获取指定 [start, end) 日期时间范围的自然日日历日期列表，仅支持到日
    
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :rtype: DatetimeList

    
.. py:function:: toPriceList(arg)

    将Python的可迭代对象如 list、tuple 转化为 PriceList（该函数实际已废弃，可以不再使用）
    
    :param arg: 待转化的Python序列
    :rtype: list
    

.. py:function:: set_global_context(stk, query)

    设置全局的 context

    :param Stock stk: 指定的全局Stock
    :param Query query: 指定的查询条件
    
    
.. py:function:: get_global_context()

    获取当前全局默认上下文
    
    :rtype: KData

    
类
-----------

.. py:class:: Parameter

    参数类
    
    .. py:method:: get(self, name)
        
        获取指定参数
        
        :param str name: 参数名称
        :return: 参数值
        
    .. py:method:: set(self, name, value)
    
        设置参数
        
        :param str name: 参数名称
        :param value: 参数值（仅支持 int | float | str | bool 类型）
    
    
.. py:class:: PriceList

    价格序列，其中价格使用double表示，对应C++中的std::vector<double>。

    .. py:method:: to_np(self)

        仅在安装了numpy模块时生效，转换为numpy.array

    .. py:method:: to_df(self)

        仅在安装了pandas模块时生效，转换为pandas.DataFrame
        
        
.. py:class:: DatetimeList

    日期序列，对应C++中的std::vector<Datetime>
    
    .. py:method:: append(self, datetime)
    
        向列表末端加入元素
        
        :param Datetime datetime: 待加入的元素
    
    .. py:method:: to_np(self)

        仅在安装了numpy模块时生效，转换为numpy.array

    .. py:method:: to_df(self)

        仅在安装了pandas模块时生效，转换为pandas.DataFrame


.. py:class:: StringList

    字符串列表，对应C++中的std::vector<String>
    
    
.. py:class:: KRecordList

    C++ std::vector<KRecord>包装
    
    .. py:method:: append(self, krecord)
    
        向列表末端加入元素
        
        :param KRecord krecord: 待加入的元素
    
    .. py:method:: to_np(self)

        仅在安装了numpy模块时生效，转换为numpy.array

    .. py:method:: to_df(self)

        仅在安装了pandas模块时生效，转换为pandas.DataFrame
    

.. py:class:: BlockList

    C++ std::vector<Block>包装
    
    
.. py:class:: OstreamRedirect

    重定向C++ std::cout、std::cerr至python。在非命令行方式下，某些App无法显示C++ iostream的输出信息，如Jupyter notebook。默认构造时，只是指定是否需要重定向std::cout或std::cerr，必须使用open方法或with语法才会启用重定向。
    
    使用with示例：
    
    .. code-block:: python
    
        with OstreamRedirect():
            your_function() #被封装的C++函数，其中使用了std::iostream输出
    

    .. py:method:: init(self[, stdout=True, stderr=True])
        
        :param bool stdout: 是否重定向C++ std::cout
        :param bool stderr: 是否重定向C++ std::cerr

    .. py:method:: open(self)
    
        启用重定向
        
    .. py:method:: close(self)
    
        关闭重定向
    
    
枚举
-----------    

.. py:class:: LOG_LEVEL

    - DEBUG
    - TRACE
    - INFO
    - WARN
    - ERROR
    - FATAL
    - NO_PRINT

