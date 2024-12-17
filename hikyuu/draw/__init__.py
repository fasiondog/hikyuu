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

# ===============================================================================
# history:
# 1. 20100220, Added by fasiondog
# ===============================================================================

from .drawplot import (use_draw_engine,
                       get_current_draw_engine,
                       show_gcf,
                       create_figure,
                       ax_set_locator_formatter,
                       adjust_axes_show,
                       ax_draw_macd,
                       ax_draw_macd2,
                       gca,
                       gcf)
from .drawplot import (RGB, DRAWNULL, STICKLINE, DRAWBAND, PLOYLINE, DRAWLINE,
                       DRAWTEXT, DRAWNUMBER, DRAWTEXT_FIX, DRAWNUMBER_FIX, DRAWSL, DRAWIMG,
                       DRAWICON, DRAWBMP, SHOWICONS, DRAWRECTREL)

from . import volume as vl
from . import elder as el
from . import kaufman as kf


__all__ = [
    'vl', 'el', 'kf',
    'use_draw_engine',
    'get_current_draw_engine',
    'create_figure',
    'ax_set_locator_formatter',
    'adjust_axes_show',
    'ax_draw_macd',
    'ax_draw_macd2',
    'gcf', 'gca',
    'show_gcf',
    'DRAWNULL', 'STICKLINE',
    'DRAWBAND', 'RGB', 'PLOYLINE', 'DRAWLINE', 'DRAWTEXT', 'DRAWNUMBER', 'DRAWTEXT_FIX',
    'DRAWNUMBER_FIX', 'DRAWSL', 'DRAWIMG', 'DRAWICON', 'DRAWBMP', 'SHOWICONS', 'DRAWRECTREL'
]
