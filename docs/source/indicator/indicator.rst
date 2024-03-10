.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

内建技术指标
============


.. py:function:: ABS([data])

    求绝对值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: ACOS([data])

    反余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: AD(kdata)

   累积/派发线
   
   :param KData kdata: k线数据
   :rtype: Indicator


.. py:function:: ADVANCE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    上涨家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :rtype: Indicator


.. py:function:: ALIGN(data, ref[, fill_null=True])

    按指定的参考日期对齐

    :param Indicator data: 输入数据
    :param DatetimeList|Indicator|KData ref: 指定做为日期参考的 DatetimeList、Indicator 或 KData
    :param bool fill_null: 缺失数据使用 nan 填充; 否则使用小于对应日期且最接近对应日期的数据
    :retype: Indicator

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


.. py:function:: ASIN([data])

    反正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: ATAN([data])

    反正切值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: ATR([data, n=14])

    平均真实波幅(Average True Range)

    :param Indicator data 待计算的源数据
    :param int n: 计算均值的周期窗口，必须为大于1的整数
    :rtype: Indicator


.. py:function:: AVEDEV(data[, n=22])

    平均绝对偏差，求X的N日平均绝对偏差

    :param Indicator data: 输入数据
    :param int|Indicator n: 时间窗口
    :rtype: Indicator


.. py:function:: BACKSET([data, n=2])

    向前赋值将当前位置到若干周期前的数据设为1。

    用法：BACKSET(X,N),X非0,则将当前位置到N周期前的数值设为1。

    例如：BACKSET(CLOSE>OPEN,2)若收阳则将该周期及前一周期数值设为1,否则为0

    :param Indicator data: 输入数据
    :param int n|Indicator|IndParam: N周期
    :rtype: Indicator


.. py:function:: BARSCOUNT([data])

    有效值周期数, 求总的周期数。

    用法：BARSCOUNT(X)第一个有效数据到当前的天数。

    例如：BARSCOUNT(CLOSE)对于日线数据取得上市以来总交易日数，对于1分钟线取得当日交易分钟数。

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: BARSLAST([data])

    上一次条件成立位置 上一次条件成立到当前的周期数。

    用法：BARSLAST(X): 上一次 X 不为 0 到现在的天数。

    例如：BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) 表示上一个涨停板到当前的周期数

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: BARSSINCE([data])

    第一个条件成立位置到当前的周期数。

    用法：BARSSINCE(X):第一次X不为0到现在的天数。

    例如：BARSSINCE(HIGH>10)表示股价超过10元时到当前的周期数

    :param Indicator data: 输入数据
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
   

.. py:function:: CORR(ind1, ind2, n)

    计算 ind1 和 ind2 的样本相关系数与协方差。返回中存在两个结果，第一个为相关系数，第二个为协方差。

    :param Indicator ind1: 指标1
    :param Indicator ind2: 指标2
    :param int n: 按指定 n 的长度计算两个 ind 直接数据相关系数。如果为0，使用输入的ind长度。
    :rtype: Indicator    


.. py:function:: COS([data])

    余弦值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: COST(k[, x=10.0])

    成本分布

    用法：COST(k, X) 表示X%获利盘的价格是多少

    例如：COST(k, 10),表示10%获利盘的价格是多少，即有10%的持仓量在该价格以下，其余90%在该价格以上，为套牢盘 该函数仅对日线分析周期有效

    :param KData k: 关联的K线数据
    :param float x: x%获利价格, 0~100
    :rtype: Indicator


.. py:function:: COUNT([data, n=20])

    统计满足条件的周期数。
    
    用法：COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。
    
    例如：COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数
    
    :param Indicator data: 条件
    :param int|Indicator|IndParam n: 周期
    :rtype: Indicator


.. py:function:: CROSS(x, y)

    交叉函数

    :param x: 变量或常量，判断交叉的第一条线
    :param y: 变量或常量，判断交叉的第二条线
    :rtype: Indicator


.. py:function:: CVAL([data, value=0.0, discard=0])

    data 为 Indicator 实例，创建和 data 等长的常量指标，其值和为value，抛弃长度discard和data一样
    
    :param Indicator data: Indicator实例
    :param float value: 常数值
    :param int discard: 抛弃数量
    :rtype: Indicator
    

.. py:function:: DATE([data])

    取得该周期从1900以来的年月日。用法: DATE 例如函数返回1000101，表示2000年1月1日。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: DAY([data])

    取得该周期的日期。用法: DAY 函数返回有效值范围为(1-31)。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: DECLINE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    下跌家数。当存在指定上下文且 ignore_context 为 false 时，将忽略 query, market, stk_type 参数。

    :param Query query: 查询条件
    :param str market: 所属市场，等于 "" 时，获取所有市场
    :param int stk_type: 证券类型, 大于 constant.STOCKTYPE_TMP 时，获取所有类型证券
    :param bool ignore_context: 是否忽略上下文。忽略时，强制使用 query, market, stk_type 参数。
    :rtype: Indicator


.. py:function:: DEVSQ([data, n=10])

    数据偏差平方和，求X的N日数据偏差平方和

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: DIFF([data])

    差分指标，即data[i] - data[i-1]
    
    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: DMA(ind, a)

    动态移动平均

    用法：DMA(X,A),求X的动态移动平均。

    算法：若Y=DMA(X,A) 则 Y=A*X+(1-A)*Y',其中Y'表示上一周期Y值。

    例如：DMA(CLOSE,VOL/CAPITAL)表示求以换手率作平滑因子的平均价

    :param Indicator ind: 输入数据
    :param Indicator a: 动态系数
    :rtype: Indicator


.. py:function:: DROPNA([data])

    删除 nan 值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: DOWNNDAY(data[, n=3])

    连跌周期数, DOWNNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator

    
.. py:function:: EMA([data, n=22])

    指数移动平均线(Exponential Moving Average)

    :param data: 输入数据
    :param int|Indciator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator
    

.. py:function:: EVERY([data, n=20])

    一直存在

    用法：EVERY (X,N) 表示条件X在N周期一直存在

    例如：EVERY(CLOSE>OPEN,10) 表示前10日内一直是阳线

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator


.. py:function:: EXIST([data, n=20])

    存在, EXIST(X,N) 表示条件X在N周期有存在

    :param data: 输入数据
    :param int|Indicator|IndParam n: 计算均值的周期窗口，必须为大于0的整数 
    :rtype: Indicator


.. py:function:: EXP([data])

    EXP(X)为e的X次幂

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: FILTER([data, n=5])

    信号过滤, 过滤连续出现的信号。

    用法：FILTER(X,N): X 满足条件后，删除其后 N 周期内的数据置为 0。

    例如：FILTER(CLOSE>OPEN,5) 查找阳线，5 天内再次出现的阳线不被记录在内。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 过滤周期
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
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator
    

.. py:function:: HHVBARS([data, n=20])

    上一高点位置 求上一高点到当前的周期数。

    用法：HHVBARS(X,N):求N周期内X最高值到当前周期数N=0表示从第一个有效值开始统计

    例如：HHVBARS(HIGH,0)求得历史新高到到当前的周期数

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator
    
.. py:function:: HIGH([data])

    获取最高价，包装KData的最高价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator
    
    
.. py:function:: HOUR([data])

    取得该周期的小时数。用法：HOUR 函数返回有效值范围为(0-23)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: HSL(kdata)

    获取换手率，等于 VOL(k) / CAPITAL(k)
    
    :param KData kdata: k线数据
    :rtype: Indicator
    
.. py:function:: IC(ind, stks, query, n, ref_stk)

    计算指定的因子相对于参考证券的 IC （实际为 RankIC）
    
    :param sequence(stock)|Block stks 证券组合
    :param Query query: 查询条件
    :param int n: 时间窗口
    :param Stock ref_stk: 参照证券，通常使用 sh000300 沪深300
    :rtype: Indicator


.. py:function:: ICIR(ic[,n])

    计算 IC 因子 IR = IC的多周期均值/IC的标准方差

    :param Indicator: ic 已经计算出的 ic 值
    :param int n: 时间窗口
    :rtype: Indicator


.. py:function:: IR(p, b[, n=100])

    信息比率（Information Ratio，IR）

    公式: (P-B) / TE
    P: 组合收益率
    B: 比较基准收益率
    TE: 投资周期中每天的 p 和 b 之间的标准差
    实际使用时，P 一般为 TM 的资产曲线，B 为沪深 3000 收盘价，如:
    ref_k = sm["sh000300"].get_kdata(query)
    funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
    ir = IR(PRICELIST(funds), ref_k.close, 0)

    如果希望计算因子 IC 的 IR 值，请使用 ICIR 指标

    :param Indicator p:
    :param Indicator b:
    :param int n: 时间窗口（默认100），如果只想使用最后的值，可以使用 0, 或 len(p),len(b) 指定
    :rtype: Indicator

    
.. py:function:: IF(x, a, b)

    条件函数, 根据条件求不同的值。
    
    用法：IF(X,A,B)若X不为0则返回A,否则返回B
    
    例如：IF(CLOSE>OPEN,HIGH,LOW)表示该周期收阳则返回最高值,否则返回最低值
    
    :param Indicator x: 条件指标
    :param Indicator a: 待选指标 a
    :param Indicator b: 待选指标 b
    :rtype: Indicator
    

.. py:function:: INTPART([data])

    取整(绝对值减小取整，即取得数据的整数部分)
    
    :param data: 输入数据
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


.. py:function:: KDJ(kdata[, n=9, m12=3, m2=3])

    经典 KDJ 随机指标

    :param KData kdata: 关联的K线数据
    :param int n:
    :param int m1:
    :param int m2:
    :return: k, d, j

.. py:function:: LIUTONGPAN(kdata)

   获取流通盘（单位：万股），同 CAPITAL
   
   :param KData kdata: k线数据
   :rtype: Indicator


.. py:function:: LAST([data, m=10, n=5])

    区间存在。

    用法：LAST (X,M,N) 表示条件 X 在前 M 周期到前 N 周期存在。

    例如：LAST(CLOSE>OPEN,10,5) 表示从前10日到前5日内一直阳线。

    :param data: 输入数据
    :param int m: m周期
    :param int n: n周期
    :rtype: Indicator


.. py:function:: LLV([data, n=20])

    N日内最低价，N=0则从第一个有效值开始。

    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator


.. py:function:: LLVBARS([data, n=20])

    上一低点位置 求上一低点到当前的周期数。
    
    用法：LLVBARS(X,N):求N周期内X最低值到当前周期数N=0表示从第一个有效值开始统计
    
    例如：LLVBARS(HIGH,20)求得20日最低点到当前的周期数
    
    :param data: 输入数据
    :param int|Indicator|IndParam n: N日时间窗口
    :rtype: Indicator


.. py:function:: LN([data])

    求自然对数, LN(X)以e为底的对数

    :param data: 输入数据
    :rtype: Indicator


.. py:function:: LOG([data])

    以10为底的对数

    :param data: 输入数据
    :rtype: Indicator


.. py:function:: LONGCROSS(a, b[, n=3])

    两条线维持一定周期后交叉

    用法：LONGCROSS(A,B,N)表示A在N周期内都小于B，本周期从下方向上穿过B时返 回1，否则返回0

    例如：LONGCROSS(MA(CLOSE,5),MA(CLOSE,10),5)表示5日均线维持5周期后与10日均线交金叉

    :param Indicator a:
    :param Indicator b:
    :param int n:
    :rtype: Indicator

    
.. py:function:: LOW([data])

    获取最低价，包装KData的最低价成Indicator
    
    :param data: 输入数据（KData 或 Indicator） 
    :rtype: Indicator
    
    
.. py:function:: MA([data, n=22])

    简单移动平均
    
    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
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


.. py:function:: MDD([ind])

    当前价格相对历史最高值的回撤百分比，通常用于计算最大回撤


.. py:function:: MIN(ind1, ind2)

    求最小值, MIN(A,B)返回A和B中的较小值。
    
    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator
    

.. py:function:: MINUTE([data])

    取得该周期的分钟数。用法：MINUTE 函数返回有效值范围为(0-59)，对于日线及更长的分析周期值为0。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: MOD(ind1, ind2)

    取整后求模。该函数仅为兼容通达信。实际上，指标求模可直接使用 % 操作符

    用法：MOD(A,B)返回A对B求模

    例如：MOD(26,10) 返回 6

    :param Indicator ind1:
    :param Indicator ind2:
    :rtype: Indicator


.. py:function:: MONTH([data])

    取得该周期的月份。用法: MONTH 函数返回有效值范围为(1-12)。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: MRR([ind])

    当前价格相对历史最低值的盈利百分比


.. py:function:: NDAY(x, y[, n=3])

    连大, NDAY(X,Y,N)表示条件X>Y持续存在N个周期

    :param Indicator x:
    :param Indicator y:
    :param int n: 时间窗口
    :rtype: Indicator


.. py:function:: NOT([data])

    求逻辑非。NOT(X)返回非X,即当X<=0时返回1，否则返回0。
    
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
    :param int|Indicator|IndParam n: 幂
    :rtype: Indicator


.. py:function:: PRICELIST(data[, result_index=0, discard=0])
    
    将 list、tuple、Indicator 转化为普通的 Indicator
    
    :param data: 输入数据，可以为 list、tuple、Indicator
    :param int result_index: 当data为Indicator实例时，指示Indicator的第几个结果集
    :param int discard: 在 data 为 Indicator类型时无效。表示前端抛弃的数据点数，抛弃的值使用 constant.null_price 填充
    :rtype: Indicator
 
 
.. py:function:: REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。
    
    用法：REF(X，A)　引用A周期前的X值。
    
    :param Indicator data: 输入数据
    :param int n: 引用n周期前的值，即右移n位
    :rtype: Indicator


.. py:function:: REVERSE([data])

    求相反数，REVERSE(X)返回-X

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: ROC([data, n=10])

    变动率指标: ((price / prevPrice)-1)*100

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: ROCP([data, n=10])

    变动率指标: (price - prevPrice) / prevPrice

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: ROCR([data, n=10])

    变动率指标: (price / prevPrice)

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: ROCR100([data, n=10])

    变动率指标: (price / prevPrice) * 100

    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
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


.. py:function:: RSI(kdata=None, N1=6, N2=12, N3=24)

    相对强弱指标

    :param KData kdata: 关联的K线数据
    :param int N1: 参数N1
    :param int N2: 参数N1
    :param int N3: 参数N1
    :return: rsi1, rsi2, rsi3
    

.. py:function:: SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_
    
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param Indicator data: 输入数据
    :param int n1: 计算平均噪音的回溯时间窗口
    :param int n2: 对初步止损线去n2日内的最高值
    :param float p: 噪音系数
    :rtype: Indicator
    

.. py:function:: SIN([data])

    正弦值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: SGN([data])

    求符号值, SGN(X)，当 X>0, X=0, X<0分别返回 1, 0, -1。

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: SLICE(data, start, end, result_index=0)

    获取某指标中指定范围 [start, end) 的数据，生成新的指标

    :param Indicator|sequence data: 输入数据
    :param int start: 起始位置
    :param int end: 终止位置（不包含本身）
    :param int result_index: 原输入数据中的结果集


.. py:function:: SLOPE(data, n=22)

    计算线性回归斜率，N支持变量

    :param Indicator|sequence data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: SMA([data, n=22, m=2])

    求移动平均

    用法：若Y=SMA(X,N,M) 则 Y=[M*X+(N-M)*Y')/N,其中Y'表示上一周期Y值
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :param float m: 系数
    :rtype: Indicator


.. py:function:: SPEARMAN(ind1, ind2, n)

    Spearman 相关系数

    :param Indicator ind1: 输入参数1
    :param Indicator ind2: 输入参数2
    :param int n: 滚动窗口(大于2 或 等于0)，等于0时，代表 n 实际使用 ind 的长度


.. py:function:: SQRT([data])

    开平方

    用法：SQRT(X)为X的平方根

    例如：SQRT(CLOSE)收盘价的平方根

    :param data: 输入数据
    :rtype: Indicator


.. py:function:: STD([data, n=10])

    计算N周期内样本标准差
    
    :param Indicator data: 输入数据
    :param int n|Indicator|IndParam: 时间窗口
    :rtype: Indicator


.. py:function:: STDEV([data, n=10])

    计算N周期内样本标准差
    
    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator
    

.. py:function:: STDP([data, n=10])

    总体标准差，STDP(X,N)为X的N日总体标准差
    
    :param data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: SUM([data, n=20])

    求总和。SUM(X,N),统计N周期中X的总和,N=0则从第一个有效值开始。

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: SUMBARS([data,] a)

    累加到指定周期数, 向前累加到指定值到现在的周期数

    用法：SUMBARS(X,A):将X向前累加直到大于等于A,返回这个区间的周期数

    例如：SUMBARS(VOL,CAPITAL)求完全换手到现在的周期数

    :param Indicator data: 输入数据
    :param float|Indicator|IndParam a: 指定累加和
    :rtype: Indicator


.. py:function:: TAN([data])

    正切值

    :param Indicator data: 输入数据
    :rtype: Indicator


.. py:function:: TIME([data])

    取得该周期的时分秒。用法: TIME 函数返回有效值范围为(000000-235959)。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: TIMELINE([k])

    分时价格数据

    :param KData k: 上下文
    :rtype: Indicator
    

.. py:function:: TIMELINEVOL([k])

    分时成交量数据

    :param KData k: 上下文
    :rtype: Indicator


.. py:function:: UPNDAY(data[, n=3])

    连涨周期数, UPNDAY(CLOSE,M)表示连涨M个周期

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: VAR([data, n=10])

    估算样本方差, VAR(X,N)为X的N日估算样本方差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
    :rtype: Indicator


.. py:function:: VARP([data, n=10])

    总体样本方差, VARP(X,N)为X的N日总体样本方差

    :param Indicator data: 输入数据
    :param int|Indicator|IndParam n: 时间窗口
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


.. py:function:: WEEK([data])

    取得该周期的星期数。用法：WEEK 函数返回有效值范围为(0-6)，0表示星期天。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: YEAR([data])

    取得该周期的年份。

    :param data: 输入数据 KData
    :rtype: Indicator


.. py:function:: ZHBOND10([data, default_val])

    获取10年期中国国债收益率

    :param DatetimeList|KDate|Indicator data: 输入的日期参考，优先使用上下文中的日期
    :param float default_val: 如果输入的日期早于已有国债数据的最早记录，则使用此默认值


.. py:function:: ZSCORE([data, out_extreme, nsigma, recursive])

    对数据进行标准化（归一），可选进行极值排除

    注：非窗口滚动，如需窗口滚动的标准化，直接 (x - MA(x, n)) / STDEV(x, n) 即可。
    
    :param Indicator data: 待剔除异常值的数据
    :param bool outExtreme: 指示剔除极值，默认 False
    :param float nsigma: 剔除极值时使用的 nsigma 倍 sigma ,默认 3.0
    :param bool recursive: 是否进行递归剔除极值, 默认 False
    :rtype: Indicator
