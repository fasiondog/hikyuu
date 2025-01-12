.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

技术指标总览
============

**辅助类指标**

* :py:func:`ALIGN` - 按指定的参考日期对齐
* :py:func:`CVAL` - 创建指定长度的固定数值指标
* :py:func:`CONTEXT` - 独立上下文
* :py:func:`DROPNA` - 删除 nan 值
* :py:func:`ISNA` - 判断是否为 nan 值
* :py:func:`ISINF` - 判断是否为 +inf 值
* :py:func:`ISINFA` - 判断是否为 -inf 值
* :py:func:`PRICELIST` - 将PriceList或Indicator的结果集包装为Indicator，同名 VALUE
* :py:func:`REPLACE` - 替换指标中指定值，默认为替换 nan 值为 0.0
* :py:func:`RESULT` - 以指标公式的方式返回指定指标中相应的结果集
* :py:func:`WEAVE` - 将两个ind的结果合并到一个ind中
* :py:func:`ZSCORE` - ZScore 标准化


**行情指标**

* :py:func:`KDATA` - 包装KData成Indicator，用于其他指标计算
* :py:func:`KDATA_PART` - 根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL
* :py:func:`OPEN`  - 包装KData的开盘价成Indicator
* :py:func:`HIGH`  - 包装KData的最高价成Indicator
* :py:func:`LOW`   - 包装KData的最低价成Indicator
* :py:func:`CLOSE` - 包装KData的收盘价成Indicator
* :py:func:`AMO`   - 包装KData的成交金额成Indicator
* :py:func:`VOL`   - 包装KData的成交量成Indicator
* :py:func:`RECOVER_FORWARD` - 前向复权
* :py:func:`RECOVER_BACKWARD` - 后向复权
* :py:func:`RECOVER_EQUAL_FORWARD` - 等比前向复权
* :py:func:`RECOVER_EQUAL_BACKWARD` - 等比后向复权
* :py:func:`FINANCE` - 历史财务信息
* :py:func:`HSL` - 换手率
* :py:func:`CAPITAL` - 流通盘，同名：LIUTONGPAN
* :py:func:`TIMELINE`   - 分时价格
* :py:func:`TIMELINEVOL`   - 分时成交量
* :py:func:`ZHBOND10` - 10年期中国国债收益率
* :py:func:`ZONGGUBEN` - 总股本
    

**大盘指标**

* :py:func:`ADVANCE` - 上涨家数
* :py:func:`DECLINE` - 下跌家数

**逻辑算术函数**

指标本身直接支持 "+"、"-"、"*" 、"/"、"&"（与）、"|"（或）、"<"、">"、"<="、">="、"=="、"!=" 操作。

* :py:func:`BETWEEN` - 介于(介于两个数之间)
* :py:func:`CEILING` - 向上舍入(向数值增大方向舍入)取整
* :py:func:`CROSS` - 交叉函数
* :py:func:`DOWNNDAY` - 连跌周期数
* :py:func:`EVERY` - 一直存在
* :py:func:`EXIST` - 存在, EXIST(X,N) 表示条件X在N周期有存在
* :py:func:`FLOOR` - 向下舍入(向数值减小方向舍入)取整
* :py:func:`IF` - 根据条件求不同的值
* :py:func:`INTPART` - 取整(绝对值减小取整，即取得数据的整数部分)
* :py:func:`LAST` - 区间存在
* :py:func:`LONGCROSS` - 两条线维持一定周期后交叉
* :py:func:`NOT` - 求逻辑非
* :py:func:`UPNDAY` - 连涨周期数
* :py:func:`NDAY` - 连大


**数学指标**

* :py:func:`ABS` - 求绝对值
* :py:func:`ACOS` - 反余弦值
* :py:func:`ASIN` - 反正弦值
* :py:func:`ATAN` - 反正切值
* :py:func:`COS` - 余弦值
* :py:func:`EXP` - e的X次幂
* :py:func:`LN` - 求自然对数, LN(X)以e为底的对数
* :py:func:`LOG` - 以10为底的对数
* :py:func:`MAX` - 最大值
* :py:func:`MIN` - 最小值
* :py:func:`MOD` - 取整后求模。该函数仅为兼容通达信。实际上，指标求模可直接使用 % 操作符。
* :py:func:`POW` - 乘幂
* :py:func:`REVERSE` - 求相反数
* :py:func:`ROUND` - 四舍五入
* :py:func:`ROUNDUP` -  向上截取，如10.1截取后为11
* :py:func:`ROUNDDOWN` - 向下截取，如10.1截取后为10
* :py:func:`SIN` - 正弦值
* :py:func:`SGN` - 求符号值
* :py:func:`SLOPE` - 计算线性回归斜率
* :py:func:`SQRT` - 开平方
* :py:func:`TAN` - 正切值

**统计指标**

* :py:func:`AVEDEV` - 平均绝对偏差
* :py:func:`DEVSQ` - 数据偏差平方和
* :py:func:`STD` - 估算标准差，同 STDEV
* :py:func:`STDEV` - 计算N周期内样本标准差
* :py:func:`STDP` - 总体标准差
* :py:func:`VAR` - 估算样本方差
* :py:func:`VARP` - 总体样本方差
* :py:func:`CORR` - 样本相关系数与协方差
* :py:func:`SPEARMAN` - Spearman相关系数

**横向统计**

* :py:func:`BLOCKSETNUM` - 返回板块股个数
* :py:func:`INSUM` - 返回板块各成分该指标相应输出按计算类型得到的计算值.计算类型:0-累加,1-平均数,2-最大值,3-最小值.


**技术指标**

* :py:func:`AD` - 累积/派发线
* :py:func:`BACKSET` - 向前赋值将当前位置到若干周期前的数据设为1
* :py:func:`BARSCOUNT` - 有效值周期数, 求总的周期数。
* :py:func:`BARSLAST` - 上一次条件成立位置, 上一次条件成立到当前的周期数
* :py:func:`BARSSINCE` - 第一个条件成立位置到当前的周期数
* :py:func:`COUNT` - 统计满足条件的周期数
* :py:func:`COST` - 成本分布
* :py:func:`DIFF` - 差分指标，即data[i] - data[i-1]
* :py:func:`DMA` - 动态移动平均
* :py:func:`FILTER` - 信号过滤, 过滤连续出现的信号
* :py:func:`HHV` - N日内最高价
* :py:func:`HHVBARS` - 上一高点位置 求上一高点到当前的周期数
* :py:func:`KDJ` - 经典随机指标
* :py:func:`LLV` - N日内最低价
* :py:func:`LLVBARS` - 上一低点位置 求上一低点到当前的周期数
* :py:func:`MA`  - 简单移动平均数
* :py:func:`MACD` - 平滑异同移动平均线
* :py:func:`AMA` - 佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_
* :py:func:`EMA` - 指数移动平均线(Exponential Moving Average)
* :py:func:`REF` - 向前引用 （即右移），引用若干周期前的数据
* :py:func:`ROC` - 变动率指标: ((price / prevPrice)-1)*100
* :py:func:`ROCP` - 变动率指标: (price - prevPrice) / prevPrice
* :py:func:`ROCR` - 变动率指标: (price / prevPrice)
* :py:func:`ROCR100` - 变动率指标: (price / prevPrice) * 100
* :py:func:`RSI` - 相对强弱指标
* :py:func:`SMA` - 移动平均线
* :py:func:`SAFTYLOSS` - 亚历山大 艾尔德安全地带止损线
* :py:func:`SUM` - 求总和
* :py:func:`SUMBARS` - 累加到指定周期数, 向前累加到指定值到现在的周期数
* :py:func:`VIGOR` - 亚历山大.艾尔德力度指数


**时间指标**

* :py:func:`DATE` - 取得该周期从1900以来的年月日
* :py:func:`TIME` - 取得该周期的时分秒
* :py:func:`YEAR` - 取得该周期的年份
* :py:func:`MONTH` - 取得该周期的月份
* :py:func:`WEEK` - 取得该周期的星期数，函数返回有效值范围为(0-6)，0表示星期天
* :py:func:`DAY` - 取得该周期的日期
* :py:func:`HOUR` - 取得该周期的小时数
* :py:func:`MINUTE` - 取得该周期的分钟数

**因子类指标**

* :py:func:`IC` - 计算因子 IC 值
* :py:func:`IR` - 用于计算账户收益与参照收益的IR
* :py:func:`ICIR` - 计算因子 IC 的 IR 值


**其他转换辅助**

* :py:func:`concat_to_df` - 合并指标列表为 DateFrame
* :py:func:`df_to_ind` - 将 DataFrame 指定列转为指标

.. py:function:: concat_to_df(dates, ind_list[, head_stock_code=True, head_ind_name=False])
    将列表中的指标至合并在一张 pandas DataFrame 中

    :param DatetimeList dates: 指定的日期列表
    :param sequence ind_list: 已计算的指标列表
    :param bool head_ind_name: 表标题是否使用指标名称
    :param bool head_stock_code: 表标题是否使用证券代码
    :return: 合并后的 DataFrame, 以 dates 为 index（注: dates列 为 Datetime 类型）

::

    示例:
        query = Query(-200)
        k_list = [stk.get_kdata(query) for stk in [sm['sz000001'], sm['sz000002']]]
        ma_list = [MA(k) for k in k_list]
        concat_to_df(sm.get_trading_calendar(query), ma_list, head_stock_code=True, head_ind_name=False)
        df

                date	SZ000001	SZ000002
        0	2023-05-12 00:00:00	12.620000	15.060000
        1	2023-05-15 00:00:00	12.725000	15.060000
        2	2023-05-16 00:00:00	12.690000	15.010000
        3	2023-05-17 00:00:00	12.640000	14.952500
        4	2023-05-18 00:00:00	12.610000	14.886000
        ...	...	...	...
        195	2024-03-01 00:00:00	9.950455	9.837273
        196	2024-03-04 00:00:00	9.995909	9.838182
        197	2024-03-05 00:00:00	10.038182	9.816364
        198	2024-03-06 00:00:00	10.070455	9.776818
        199	2024-03-07 00:00:00	10.101364	9.738182




.. py:function:: df_to_ind(df, col_name, col_date=None):
    
    将 pandas.DataFrame 指定列转化为 Indicator

    :param df: pandas.DataFrame
    :param col_name: 指定列名
    :param col_date: 指定日期列名 (为None时忽略, 否则该列为对应参考日期)
    :return: Indicator

::

    示例, 从 akshare 获取美国国债10年期收益率:
        import akshare as ak
        df = ak.bond_zh_us_rate("19901219")
        x = df_to_ind(df, '美国国债收益率10年', '日期')