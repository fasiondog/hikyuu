# 截面评分过滤|SCFilter

MF获取时间截面因子数据（评分记录)后，通常需要根据评分情况，进行过滤选择标的，供 PF 进行组合调仓等。实现这个功能主要靠 SE_MultitFactor2。但过滤的方式有很多中，hikyuu 提供了内置的 SCFilter（也可自定义新的Filter)，供 SE_MultiFactor 实现过滤，如：

```python
# 如为 SE_MultiFactor2 实例，设置过滤：分值不为Nan|分成10组取第0组|价格大于等于10元|成交金额不在当日排名末尾20%之内|取前10
se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))
```

SCFilter 通过 | 操作符生成新的 Filter，且对评分记录的过滤是按顺序进行的。

| 名称                      | 说明                                                                                     | 参数                                                                                           |
| ------------------------- | ---------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| SCFilter_IgnoreNan        | 忽略截面中的 Nan 值                                                                      |                                                                                                |
| SCFilter_LessOrEqualValue | 过滤掉评分小于等于指定值的截面                                                           | value(double)：默认值0.0                                                                       |
| SCFilter_TopN             | 仅获取评分列表中排在前TopN位的标的<br />注意：和 MF 本身指定的排序方式相关，默认为降序   | topn(int): 默认值10                                                                            |
| SCFilter_Group            | 对截面评分进行分组，并选定指定分组                                                       | group(int): 分组数量，默认10组<br />group_index(int): 指定分组序号（从0开始)，默认为0          |
| SCFilter_AmountLimit      | 过滤掉成交金额在评分列表末尾百分比范围内的标的，<br />即保证成交金额在指定排名百分比之前 | min_amount_percent_limit(double)：默认0.1，即仅保留前90%                                       |
| SCFilter_Price            | 按标的价格过滤，仅保留价格符合条件的标的<br /> [min_price, max_price]之间                | min_price(double): 最低价格限制（默认10.0)<br />max_price(double): 最高价格限制（默认100000.0) |
