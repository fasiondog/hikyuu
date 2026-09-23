from hikyuu.interactive import *
# !/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2025-10-16
#    Author: fasiondog


stks = [s for s in blocka if s.valid]
print('股票数: ', len(stks))

query = Query(Datetime(20150101), Datetime(20251017))

ma20 = MA(CLOSE(), 20)
ma20.name = 'MA20'

ma60 = MA(CLOSE(), 60)
ma60.name = 'MA60'

mf = MF_EqualWeight([ma20, ma60], stks, query, ref_stk=sm["sh000001"])

scores = mf.get_scores(Datetime(20251016))
df = scores.to_df()

print(df)

mf.set_normalize(NORM_Zscore())
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())

mf.add_special_normalize("MA20", NORM_Zscore(), category="行业板块")
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())

mf.add_special_normalize("MA20", NORM_Zscore(), style_inds=[LOG(CLOSE()*LIUTONGPAN())])
scores = mf.get_scores(Datetime(20251016))
print(scores.to_df())
