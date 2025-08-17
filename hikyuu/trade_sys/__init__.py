#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import pandas as pd
from .trade_sys import *
from hikyuu.core import (ScoreRecordList, scorerecords_to_np, scorerecords_to_df,
                         SystemWeightList, systemweights_to_np, systemweights_to_df)


ScoreRecordList.to_np = lambda self: scorerecords_to_np(self)
ScoreRecordList.to_df = lambda self: scorerecords_to_df(self)
ScoreRecordList.to_numpy = ScoreRecordList.to_np
ScoreRecordList.to_pandas = ScoreRecordList.to_df


SystemWeightList.to_np = lambda self: systemweights_to_np(self)
SystemWeightList.to_df = lambda self: systemweights_to_df(self)
SystemWeightList.to_numpy = SystemWeightList.to_np
SystemWeightList.to_pandas = SystemWeightList.to_df
