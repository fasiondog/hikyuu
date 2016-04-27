.. py:currentmodule:: indicator
.. highlightlang:: python

内建技术指标
============

.. py:function:: AMA([data, n = 10, fast_n = 2, slow_n = 30])

    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_
    
    :param int n: 计算均值的周期窗口，必须为大于2的整数
    :param int fast_n: 对应快速周期N
    :param int slow_n: 对应慢速EMA线的N值

    
.. py:function:: AMO([data])

    包装KData的成交金额成Indicator

    
.. py:function:: CLOSE([data])
   
    包装KData的收盘价成Indicator

    
.. py:function:: DIFF([data])

    差分指标，即data[i] - data[i-1]

    
.. py:function:: EMA([data, n = 22])

    指数移动平均线(Exponential Moving Average)

    :param int n: 计算均值的周期窗口，必须为大于0的整数 

    
.. py:function:: HHV([data, n = 20])

    N日内最高价

    :param int n: N日时间窗口

    
.. py:function:: HIGH([data])

    包装KData的最高价成Indicator    

    
.. py:function:: KDATA([data])

    包装KData成Indicator，用于其他指标计算

    :param data: KData 或 具有6个返回结果的Indicator（如KDATA生成的Indicator）

    
.. py:function:: KDATA_PART([data, kpart])

    根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL，如:KDATA_PART("CLOSE")等同于CLOSE()

    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL

    
.. py:function:: LLV([data, n = 20])

    N日内最低价

    :param int n: N日时间窗口

    
.. py:function:: LOW([data])

    包装KData的最低价成Indicator

    
.. py:function:: MA([data, n = 22, type = "SMA"])

    移动平均数包装，默认为简单平均数
    
    :param int n: 时间窗口
    :param string type: "EMA"|"SMA"|"AMA"

    
.. py:function:: MACD([data, n1 = 12, n2 = 26, n3 = 9])

    平滑异同移动平均线
    
    * result(0): MACD BAR： MACD直柱，即MACD快线－MACD慢线
    * result(1): DIFF: 快线,即（短期EMA-长期EMA）
    * result(2): 慢线，即快线的n3周期EMA平滑
    
    :param int n1: 短期EMA时间窗
    :param int n2: 长期EMA时间窗
    :param int n3: （短期EMA-长期EMA）EMA平滑时间窗
    
    
.. py:function:: OPEN([data])

    包装KData的开盘价成Indicator


.. py:function:: PRICELIST(data[, result_num = 0])

    将某指标转化为PRICELIST
    
    :param Indicator data: 源数据
    :param int result_num: 源数据中指定的结果集

    
.. py:function:: PRICELIST(data[, discard = 0])
    
    包装PriceList成Indicator
    
    :param PriceList data: 源数据
    :param int discard: 前端抛弃的数据点数，抛弃的值使用 constant.null_price 填充
 
 
.. py:function:: REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。
    
    用法：REF(X，A)　引用A周期前的X值。
    
    :param int n: 引用n周期前的值，即右移n位
    

.. py:function:: SAFTYLOSS([data, n1 = 10, n2 = 3, p = 2.0])

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_
    
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param int n1: 计算平均噪音的回溯时间窗口
    :param int n2: 对初步止损线去n2日内的最高值
    :param double p: 噪音系数
    
    
.. py:function:: SMA([data, int = 22])

    简单移动平均线
    
    :param int n: 时间窗口

    
.. py:function:: STDEV([data, n = 10])

    计算N周期内样本标准差
    
    :param int n: 时间窗口
    

.. py:function:: VIGOR(data[, n = 2])

    亚历山大.艾尔德力度指数 [BOOK2]_
    
    计算公式：（收盘价今－收盘价昨）＊成交量今
    
    :param KData data: K线数据
    :param int n: EMA平滑窗口
    
    
.. py:function:: VOL([data])

    包装KData的成交量成Indicator


