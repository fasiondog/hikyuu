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

from hikyuu import *

load_hikyuu()

blocka = Block("A", "ALL")
for s in sm:
    if s.type in (constant.STOCKTYPE_A, constant.STOCKTYPE_A_BJ):
        blocka.add(s)
zsbk_a = blocka
blocksh = Block("A", "SH")
for s in blocka:
    if s.market == "SH":
        blocksh.add(s)
zsbk_sh = blocksh
blocksz = Block("A", "SZ")
for s in blocka:
    if s.market == "SZ":
        blocksz.add(s)
zsbk_sz = blocksz
blockbj = Block("A", "BJ")
for s in blocka:
    if s.market == "BJ":
        blockbj.add(s)
zsbk_bj = blockbj
blockg = Block("G", "创业板")
for s in sm:
    if s.type == constant.STOCKTYPE_GEM:
        blockg.add(s)
zsbk_cyb = blockg
blockstart = Block("START", "科创板")
for s in sm:
    if s.type == constant.STOCKTYPE_START:
        blockstart.add(s)
blockzxb = Block("A", "中小板")
for s in blocksz:
    if s.code[:3] == "002":
        blockzxb.add(s)
zsbk_zxb = blockzxb
zsbk_sh50 = sm.get_block("指数板块", "上证50")
zsbk_sh180 = sm.get_block("指数板块", "上证180")
zsbk_hs300 = sm.get_block("指数板块", "沪深300")
zsbk_zz100 = sm.get_block("指数板块", "中证100")
