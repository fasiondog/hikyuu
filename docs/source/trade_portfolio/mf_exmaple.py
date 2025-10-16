from hikyuu.interactive import *
# !/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-10-16
#    Author: fasiondog


stks = [s for s in blocka if s.valid]
print('股票数: ', len(stks))

query = Query(start=20180101, end=20251016)

mf = MF_EqualWeight([MA(CLOSE(), 20)], stks, query, ref_stk=sm["sh000001"])

scores = mf.get_scores(Datetime(20251016))
df = scores.to_df()

print(df)
