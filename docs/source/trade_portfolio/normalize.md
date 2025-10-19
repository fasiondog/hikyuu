# 因子标准化与中性化|NORM

因子计算时，往往需要在时间截面上进行标准化或归一化，或者进行行业中性化、市值中性化等。可以通过为 MF 指定全局的标准化算法，或针对特定因子指定标准化和中性化策略，来达到此目的。示例：

```python
# 创建两个因子 ma20, ma60
ma20 = MA(CLOSE(), 20)
ma20.name = 'MA20'

ma60 = MA(CLOSE(), 60)
ma60.name = 'MA60'

# 指定证券列表
stks = [s for s in blocka]

# 指定查询范围，并创建一个等权组合的 MF
query = Query(Datetime(20150101), Datetime(20251017))
mf = MF_EqualWeight([ma20, ma60], stks, query, ref_stk=sm["sh000001"])

# 没有标准化时，获取合成后某日的评分列表
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())
```

```python
证券代码  证券名称        score
0     SH600519  贵州茅台  1460.640867
1     SH603444    吉比特   500.309125
2     SZ002371  北方华创   395.552042
3     BJ920982  锦波生物   300.004475
4     SH605499  东鹏饮料   295.006675
...        ...       ...          ...
3974  BJ872808  曙光数创          NaN
3975  BJ873527    夜光明          NaN
3976  BJ832471  美邦科技          NaN
3977  BJ430478  峆一药业          NaN
3978  SZ000675  ST 银 山          NaN

[3979 rows x 3 columns]

```

```python
# 添加全局标准化
mf.set_normalize(NORM_Zscore())
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())

证券代码  证券名称      score
0     SH600519  贵州茅台  43.150291
1     SH603444    吉比特  14.382820
2     SZ002371  北方华创  11.265517
3     BJ920982  锦波生物   8.427688
4     SH605499  东鹏饮料   8.272032
...        ...       ...        ...
3974  BJ872808  曙光数创        NaN
3975  BJ873527    夜光明        NaN
3976  BJ832471  美邦科技        NaN
3977  BJ430478  峆一药业        NaN
3978  SZ000675  ST 银 山        NaN

[3979 rows x 3 columns]
```

```python
# 为 ma20 添加行业中性化以及市场中性化(即按市值风格因子中性化)
mf.add_special_normalize("MA20", NORM_Zscore(), category="行业板块", style_inds=[LOG(CLOSE()*LIUTONGPAN())])
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())

证券代码  证券名称      score
0     SH600519  贵州茅台  43.114519
1     SH603444    吉比特  14.403012
2     SZ002371  北方华创  11.271072
3     BJ920982  锦波生物   8.407414
4     SH605499  东鹏饮料   8.233678
...        ...       ...        ...
3974  BJ872808  曙光数创        NaN
3975  BJ873527    夜光明        NaN
3976  BJ832471  美邦科技        NaN
3977  BJ430478  峆一药业        NaN
3978  BJ832566    梓橦宫        NaN

[3979 rows x 3 columns]
```

## 内建的因子标准化算法

| 名称                  | 说明                 | 参数                                                                                                                                                |
| --------------------- | -------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| NORM_MinMax           | 归一化               |                                                                                                                                                     |
| NORM_Zscore           | 正态分布标准化       | **out_extreme**(false): 去除去除异常值<br />**nsigma**(3.0): 异常值判断界限(西格玛)<br />**recursive**(false): 是否递归去除异常值 |
| NORM_Quantile         | 分位数标准化         | **quantile_min**(0.01): 最小分位数<br />**quantile_max**(0.99): 最大分位数                                                            |
| NORM_Quantile_Uniform | 分位数均匀分布标准化 | **quantile_min**(0.01): 最小分位数<br />**quantile_max**(0.99): 最大分位数                                                            |
