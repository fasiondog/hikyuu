动态周期K线
=============


.. py:function:: register_extra_ktype(ktype, basetype, nbars|minutes[, get_phase_end])
          
    注册扩展K线类型, 实现自定义动态周期K线。

    方式一: 严格基于Bar数量进行合成, 如: register_extra_ktype("DAY4, "DAY", 4)

    方式二: 基于时间段进行合成, 需定义当前时间计算周期结束时间的转换函数, 如定义7分钟K线:

    ::

      def get_min3_phase_end(datetime):
          m = datetime.minute
          if m % 3 == 0:
              return datetime
          m = (m / 3 + 1) * 3
          return Datetime(datetime.year, datetime.month, datetime.day, datetime.hour, m)

      register_extra_ktype("MIN3", "MIN", 3, get_min3_phase_end)

    .. note::
    
        注意：
        
        1. Hikyuu 已内建了基于Bar数量的的DAY3、DAY5、DAY7, 和基于时间转换的 MIN3 扩展K线
        2. 建议使用 hub c++ 部件的方式创建自定义K线类型周期结束点计算转换函数, 因为 python 带有 GIL 锁, 创建转换函数时将可能导致无法使用多线程进行计算
        3. 注册动态K线是非线程安全的, 请在注册完毕后再进行其他操作
    
    :param str ktype: 扩展K线类型名称
    :param str basetype: 基础K线类型名称
    :param int nbars: (or) minutes: 基础K线类型对应的基础周期数或分钟数
    :param func get_phase_end: 日期转换函数，参数为日期，返回该日期对应的周期结束日期
    :return: None