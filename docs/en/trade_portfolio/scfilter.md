# Cross-section Scores Filter|SCFilter

After the MF gets the cross-section factor data (score records), it is usually necessary to filter and select the targets according to the scores, for the PF to adjust the portfolio positions. Implementing this function mainly relies on SE_MultiFactor2. But there are many filtering ways; hikyuu provides the built-in SCFilter (a new Filter can also be customized), which SE_MultiFactor uses to implement the filtering, e.g.:

```python
# For an SE_MultiFactor2 instance, set the filter: the score is not Nan | split into 10 groups and take group 0 |
# the price is greater than or equal to 10 yuan | the amount is not in the last 20% of the daily ranking | take the top 10
se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)|SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))
```

SCFilter generates a new Filter with the | operator, and the filtering of the score records is performed in order.

The | operator returns an independent combined Filter: both operands are copied, never modified and never shared as part of the combination. So the same named filter can be safely reused in multiple combinations, and changing an original filter afterwards does not affect combinations already built, e.g.:

```python
f = SCFilter_TopN(10)
se1.set_scores_filter(f | SCFilter_Price(10.))
se2.set_scores_filter(f | SCFilter_AmountLimit(0.2))  # the chain of se1 is not affected
```

| Name                      | Description                                                                                     | Parameters                                                                                           |
| ------------------------- | ----------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| SCFilter_IgnoreNan        | Ignore the Nan values in the cross-section                                                                      |                                                                                                |
| SCFilter_LessOrEqualValue | Filter out the cross-sections whose score is less than or equal to the specified value                                                           | value(double): defaults to 0.0                                                                       |
| SCFilter_TopN             | Only get the targets ranked in the top TopN of the score list<br />Note: it is related to the sorting mode specified by the MF itself, which is descending by default   | topn(int): defaults to 10                                                                            |
| SCFilter_Group            | Group the cross-section scores and select the specified group                                                       | group(int): the number of the groups, defaults to 10<br />group_index(int): the specified group index (starting from 0), defaults to 0          |
| SCFilter_AmountLimit      | Filter out the given percentage of the targets that have the smallest amount<br />Note: it is unrelated to the order of the passed score list, and the output keeps the score order of the input<br />At least one record is always kept, so a limit of 1.0 keeps the one with the largest amount | min_amount_percent_limit(double): defaults to 0.1, i.e. only keep the top 90%                                       |
| SCFilter_Price            | Filter by the target price, keeping only the targets whose price meets the condition<br /> between [min_price, max_price]                | min_price(double): the minimum price limit (defaults to 10.0)<br />max_price(double): the maximum price limit (defaults to 100000.0) |
