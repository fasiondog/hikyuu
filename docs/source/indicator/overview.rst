.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

技术指标总览
============

**辅助类指标**

* :py:func:`ALIGN` - 按指定的参考日期对齐
* :py:func:`CVAL` - 创建指定长度的固定数值指标
* :py:func:`DROPNA` - 删除 nan 值
* :py:func:`PRICELIST` - 将PriceList或Indicator的结果集包装为Indicator，同名 VALUE
* :py:func:`WEAVE` - 将两个ind的结果合并到一个ind中


**行情指标**

* :py:func:`KDATA` - 包装KData成Indicator，用于其他指标计算
* :py:func:`KDATA_PART` - 根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL
* :py:func:`OPEN`  - 包装KData的开盘价成Indicator
* :py:func:`HIGH`  - 包装KData的最高价成Indicator
* :py:func:`LOW`   - 包装KData的最低价成Indicator
* :py:func:`CLOSE` - 包装KData的收盘价成Indicator
* :py:func:`AMO`   - 包装KData的成交金额成Indicator
* :py:func:`VOL`   - 包装KData的成交量成Indicator
* :py:func:`HSL` - 换手率
* :py:func:`CAPITAL` - 流通盘，同名：LIUTONGPAN
* :py:func:`TIMELINE`   - 分时价格
* :py:func:`TIMELINEVOL`   - 分时成交量
    

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


**技术指标**

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
* :py:func:`RSI` - 相对强弱指标
* :py:func:`SMA` - 移动平均线
* :py:func:`SAFTYLOSS` - 亚历山大 艾尔德安全地带止损线
* :py:func:`SUM` - 求总和
* :py:func:`SUMBARS` - 累加到指定周期数, 向前累加到指定值到现在的周期数
* :py:func:`VIGOR` - 亚历山大.艾尔德力度指数


**Ta-lib指标**

以下指标计算方法同 Ta-lib

* :py:func:`AD` - 累积/派发线
* :py:func:`ROC` - 变动率指标: ((price / prevPrice)-1)*100
* :py:func:`ROCP` - 变动率指标: (price - prevPrice) / prevPrice
* :py:func:`ROCR` - 变动率指标: (price / prevPrice)
* :py:func:`ROCR100` - 变动率指标: (price / prevPrice) * 100
