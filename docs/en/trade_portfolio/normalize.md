# Factor Standardization and Neutralization|NORM

When calculating the factors, it is often necessary to standardize or normalize them on the cross-section, or to neutralize them by industry, market value, etc. This can be achieved by specifying a global standardization algorithm for the MF, or by specifying standardization and neutralization strategies for specific factors. Example:

```python
# Create the two factors ma20, ma60
ma20 = MA(CLOSE(), 20)
ma20.name = 'MA20'

ma60 = MA(CLOSE(), 60)
ma60.name = 'MA60'

# Specify the security list
stks = [s for s in blocka]

# Specify the query range, and create an equal-weight composed MF
query = Query(Datetime(20150101), Datetime(20251017))
mf = MF_EqualWeight([ma20, ma60], stks, query, ref_stk=sm["sh000001"])

# Without standardization, get the score list of a certain day after composing
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
# Add the global standardization
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
# Add the industry neutralization and the market neutralization (i.e. neutralizing by the market value style factor) for ma20
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

## Built-in Factor Standardization Algorithms

| Name                  | Description                                     | Parameters                                                                                                                                          |
| --------------------- | ---------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------- |
| NORM_MinMax           | Normalization                                  |                                                                                                                                                     |
| NORM_Zscore           | Normal distribution standardization            | **out_extreme**(false): remove the outliers<br />**nsigma**(3.0): the outlier judgment threshold (sigma)<br />**recursive**(false): whether to remove the outliers recursively |
| NORM_Quantile         | Quantile standardization                        | **quantile_min**(0.01): the minimum quantile<br />**quantile_max**(0.99): the maximum quantile                                                            |
| NORM_Quantile_Uniform | Quantile uniform distribution standardization   | **quantile_min**(0.01): the minimum quantile<br />**quantile_max**(0.99): the maximum quantile                                                            |
