.. py:currentmodule:: hikyuu.indicator
.. highlightlang:: python

内建技术指标
============


.. py:function:: ABS([data])

    求绝对值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: AMA([data, n=10, fast_n=2, slow_n=30])

    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_
    
    :param Indicator data: 输入数据
    :param int n: 计算均值的周期窗口，必须为大于2的整数
    :param int fast_n: 对应快速周期N
    :param int slow_n: 对应慢速EMA线的N值
    :rtype: Indicator
    
    * result(0): AMA
    * result(1): ER

    
.. py:function:: AMO([data])

    获取成交金额，包装KData的成交金额成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator


.. py:function:: BETWEEN(a, b, c)

    介于(介于两个数之间)

    用法：BETWEEN(A,B,C)表示A处于B和C之间时返回1，否则返回0

    例如：BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5))表示收盘价介于5日均线和10日均线之间

    :param Indicator a: A
    :param Indicator b: B
    :param Indicator c: C
    :rtype: Indicator

    
.. py:function:: CLOSE([data])
   
    获取收盘价，包装KData的收盘价成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator
    
    
.. py:function:: CAPITAL(kdata)

   获取流通盘（单位：万股），同 LIUTONGPAN
   
   :param KData kdata: k线数据
   :rtype: Indicator
   

.. py:function:: CEIL([data])

    同  :py:func:`CEILING`

   
.. py:function:: CEILING([data])

    向上舍入(向数值增大方向舍入)取整
   
    用法：CEILING(A)返回沿A数值增大方向最接近的整数
   
    例如：CEILING(12.3)求得13；CEILING(-3.5)求得-3
   
    :param data: 输入数据
    :rtype: Indicator
   
    
.. py:function:: COUNT([data, n=20])

    统计满足条件的周期数。
    
    用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。
    
    例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数
    
    :param Indicator data: 条件
    :param int n: 周期
    :rtype: Indicator
    
    
.. py:function:: CVAL([data, value=0.0, discard=0])

    data 为 Indicator 实例，创建和 data 等长的常量指标，其值和为value，抛弃长度discard和data一样
    
    :param Indicator data: Indicator实例
    :param float value: 常数值
    :param int discard: 抛弃数量
    :rtype: Indicator
    
    
.. py:function:: DIFF([data])

    差分指标，即data[i] - data[i-1]
    
    :param Indicator data: 输入数据
    :rtype: Indicator
    
    
.. py:function:: EMA([data, n=22])

    指数移动平均线(Exponential Moving Average)

    :param Indicator data: 输入数据
    :param int n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator
    

.. py:function:: EXP([data])

    EXP(X)为e的X次幂

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: FLOOR([data])

    向下舍入(向数值减小方向舍入)取整
    
    用法：FLOOR(A)返回沿A数值减小方向最接近的整数
    
    例如：FLOOR(12.3)求得12

    :param data: 输入数据
    :rtype: Indicator


.. py:function:: HHV([data, n=20])

    N日内最高价，N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int n: N日时间窗口
    :rtype: Indicator
    

.. py:function:: HHVBARS([data, n=20])

    上一高点位置 求上一高点到当前的周期数。

    用法：HHVBARS(X,N):求N周期内X最高值到当前周期数N=0表示从第一个有效值开始统计

    例如：HHVBARS(HIGH,0)求得历史新高到到当前的周期数

    :param Indicator data: 输入数据
    :param int n: N日时间窗口
    :rtype: Indicator
    
.. py:function:: HIGH([data])

    获取最高价，包装KData的最高价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator
    
    
.. py:function:: HSL(kdata)

    获取换手率，等于 VOL(k) / CAPITAL(k)
    
    :param KData kdata: k线数据
    :rtype: Indicator
    
    
.. py:function:: IF(x, a, b)

    条件函数, 根据条件求不同的值。
    
    用法：IF(X,A,B)若X不为0则返回A,否则返回B
    
    例如：IF(CLOSE>OPEN,HIGH,LOW)表示该周期收阳则返回最高值,否则返回最低值
    
    :param Indicator x: 条件指标
    :param Indicator a: 待选指标 a
    :param Indicator b: 待选指标 b
    :rtype: Indicator
    
    
.. py:function:: KDATA([data])

    包装KData成Indicator，用于其他指标计算

    :param data: KData 或 具有6个返回结果的Indicator（如KDATA生成的Indicator）
    :rtype: Indicator

    
.. py:function:: KDATA_PART([data, kpart])

    根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL，如:KDATA_PART("CLOSE")等同于CLOSE()

    :param data: 输入数据（KData 或 Indicator） 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :rtype: Indicator


.. py:function:: LIUTONGPAN(kdata)

   获取流通盘（单位：万股），同 CAPITAL
   
   :param KData kdata: k线数据
   :rtype: Indicator

    
.. py:function:: LLV([data, n=20])

    N日内最低价，N=0则从第一个有效值开始。

    :param data: 输入数据
    :param int n: N日时间窗口
    :rtype: Indicator


.. py:function:: LLVBARS([data, n=20])

    上一低点位置 求上一低点到当前的周期数。
    
    用法：LLVBARS(X,N):求N周期内X最低值到当前周期数N=0表示从第一个有效值开始统计
    
    例如：LLVBARS(HIGH,20)求得20日最低点到当前的周期数
    
    :param data: 输入数据
    :param int n: N日时间窗口
    :rtype: Indicator


.. py:function:: LN([data])

    求自然对数, LN(X)以e为底的对数

    :param data: 输入数据
    :rtype: Indicator


.. py:function:: LOG([data])

    以10为底的对数

    :param data: 输入数据
    :rtype: Indicator

    
.. py:function:: LOW([data])

    获取最低价，包装KData的最低价成Indicator
    
    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator
    
    
.. py:function:: MA([data, n=22, type="SMA"])

    移动平均数包装，默认为简单平均数
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :param string type: "EMA"|"SMA"|"AMA"
    :rtype: Indicator

    
.. py:function:: MACD([data, n1=12, n2=26, n3=9])

    平滑异同移动平均线
    
    :param Indicator data: 输入数据
    :param int n1: 短期EMA时间窗
    :param int n2: 长期EMA时间窗
    :param int n3: （短期EMA-长期EMA）EMA平滑时间窗
    :rtype: 具有三个结果集的 Indicator

    * result(0): MACD_BAR：MACD直柱，即MACD快线－MACD慢线
    * result(1): DIFF: 快线,即（短期EMA-长期EMA）
    * result(2): DEA: 慢线，即快线的n3周期EMA平滑


.. py:function:: MAX(ind1, ind2)

    求最大值, MAX(A,B)返回A和B中的较大值。
    
    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator


.. py:function:: MIN(ind1, ind2)

    求最小值, MIN(A,B)返回A和B中的较小值。
    
    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator
    
    
.. py:function:: NOT([data])

    求逻辑非。NOT(X)返回非X,即当X=0时返回1，否则返回0。
    
    :param Indicator data: 输入数据
    :rtype: Indicator
    
    
.. py:function:: OPEN([data])

    获取开盘价，包装KData的开盘价成Indicator
    
    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator


.. py:function:: POW(data, n)

    乘幂
    
    用法：POW(A,B)返回A的B次幂
    
    例如：POW(CLOSE,3)求得收盘价的3次方
    
    :param data: 输入数据
    :param int n: 幂
    :rtype: Indicator


.. py:function:: PRICELIST(data[, result_num=0, discard=0])
    
    将 list、tuple、Indicator 转化为普通的 Indicator
    
    :param data: 输入数据，可以为 list、tuple、Indicator
    :param int result_num: 当data为Indicator实例时，指示Indicator的第几个结果集
    :param int discard: 在 data 为 Indicator类型时无效。表示前端抛弃的数据点数，抛弃的值使用 constant.null_price 填充
    :rtype: Indicator
 
 
.. py:function:: REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。
    
    用法：REF(X，A)　引用A周期前的X值。
    
    :param Indicator data: 输入数据
    :param int n: 引用n周期前的值，即右移n位
    :rtype: Indicator
   

.. py:function:: ROUND([data, ndigits=2])

    四舍五入

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator


.. py:function:: ROUNDDOWN([data, ndigits=2])

    向下截取，如10.1截取后为10

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator


.. py:function:: ROUNDUP([data, ndigits=2])

    向上截取，如10.1截取后为11

    :param data: 输入数据
    :param int ndigits: 保留的小数点后位数
    :rtype: Indicator


.. py:function:: SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_
    
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param Indicator data: 输入数据
    :param int n1: 计算平均噪音的回溯时间窗口
    :param int n2: 对初步止损线去n2日内的最高值
    :param float p: 噪音系数
    :rtype: Indicator
    

.. py:function:: SGN([data])

    求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: SMA([data, n=22])

    简单移动平均线
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator


.. py:function:: SQRT([data])

    开平方

    用法：SQRT(X)为X的平方根

    例如：SQRT(CLOSE)收盘价的平方根

    :param data: 输入数据
    :rtype: Indicator

    
.. py:function:: STDEV([data, n=10])

    计算N周期内样本标准差
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator
    

.. py:function:: SUM([data, n=20])

    求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :rtype: Indicator


.. py:function:: VIGOR([kdata, n=2])

    亚历山大.艾尔德力度指数 [BOOK2]_
    
    计算公式：（收盘价今－收盘价昨）＊成交量今
    
    :param KData data: 输入数据
    :param int n: EMA平滑窗口
    :rtype: Indicator
   
    
.. py:function:: VOL([data])

    获取成交量，包装KData的成交量成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :rtype: Indicator


.. py:function:: WEAVE(ind1, ind2)

    将ind1和ind2的结果组合在一起放在一个Indicator中。如ind = WEAVE(ind1, ind2), 则此时ind包含多个结果，按ind1、ind2的顺序存放。
    
    :param Indicator ind1: 指标1
    :param Indicator ind2: 指标2
    :rtype: Indicator

