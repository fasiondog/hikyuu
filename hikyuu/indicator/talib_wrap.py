#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2017 fasiondog
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
# 作者：fasiondog
# 历史：1）20170923, Added by fasiondog
# ===============================================================================

from .indicator import Indicator, IndicatorImp

try:
    import talib
    import talib.abstract as ta
    import numpy as np

    def tawrap_init(self, tafunc, name, params, result_num=1, prices=None):
        super(self.__class__, self).__init__(name, result_num)
        for k, v in params.items():
            self.set_param(k, v)
        self._tafunc = tafunc
        self._prices = prices
        self._params = params
        self._result_num = result_num

    def tawrap_calculate(self, ind):
        result_num = self.get_result_num()

        if result_num < 1:
            print("error: result_num must be >= 1!")
            return

        if not self._prices:
            if self.name == "TA_OBV":
                if ind.get_result_num() < 2:
                    print("error: result_num must be >= 2!")
                    return
                inputs = {'close': ind.get_result(0).to_np(), 'volume': ind.get_result(1).to_np()}
            elif self.name in ("TA_BETA", "TA_CORREL"):
                if ind.get_result_num() < 2:
                    print("error: result_num must be >= 2!")
                    return
                inputs = {'high': ind.get_result(0).to_np(), 'low': ind.get_result(1).to_np()}
            else:
                inputs = {'close': ind.to_np()}
        else:
            if ind.name != 'KDATA':
                print("error: ind must KDATA")
                return

            inputs = {
                'open': ind.get_result(0).to_np(),
                'high': ind.get_result(1).to_np(),
                'low': ind.get_result(2).to_np(),
                'close': ind.get_result(3).to_np(),
                'volume': ind.get_result(5).to_np()
            }

        params = self.get_parameter()
        param_names = params.get_name_list()
        func_params = {}
        for name in param_names:
            if name != "kdata":
                func_params[name] = self.get_param(name)

        self._tafunc.set_parameters(func_params)

        outputs = self._tafunc(inputs, prices=self._prices) if self._prices else self._tafunc(inputs)
        if result_num == 1:
            for i, val in enumerate(outputs):
                if not np.isnan(val):
                    self._set(float(val), i)
            self.set_discard(self._tafunc.lookback)

        else:
            for i, out in enumerate(outputs):
                for j, val in enumerate(out):
                    if not np.isnan(val):
                        self._set(float(val), j, i)
            self.set_discard(self._tafunc.lookback)

    def check_all_true(self):
        return True

    def tawrap_support_ind_param(self):
        return False

    def tawrap_clone(self):
        return crtTaIndicatorImp(
            self._tafunc, self.name, self._params, self._result_num, self._prices, check=self.check
        )

    def crtTaIndicatorImp(tafunc, name, params={}, result_num=1, prices=None, check=check_all_true):
        meta_x = type(
            name, (IndicatorImp, ), {
                '__init__': tawrap_init,
                'check': check,
                '_clone': tawrap_clone,
                '_calculate': tawrap_calculate,
                'support_ind_param': tawrap_support_ind_param,
            }
        )
        return meta_x(tafunc, name, params, result_num, prices)

    def TA_AD(ind=None):
        imp = crtTaIndicatorImp(ta.AD, 'TA_AD', prices=['high', 'low', 'close', 'volume'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_AD.__doc__ = talib.AD.__doc__

    def TA_ADOSC(ind=None, fastperiod=3, slowperiod=10):
        imp = crtTaIndicatorImp(
            ta.ADOSC,
            'TA_ADOSC',
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod
            },
            prices=['high', 'low', 'close', 'volume']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ADOSC.__doc__ = talib.ADOSC.__doc__

    def TA_ADX(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.ADX, 'TA_ADX', params={'timeperiod': timeperiod}, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ADX.__doc__ = talib.ADX.__doc__

    def TA_ADXR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.ADXR, 'TA_ADXR', params={'timeperiod': timeperiod}, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ADXR.__doc__ = talib.ADXR.__doc__

    def TA_APO(ind=None, fastperiod=12, slowperiod=26, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.APO, 'TA_APO', params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_APO.__doc__ = talib.APO.__doc__

    def TA_AROON(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AROON, 'TA_AROON', result_num=2, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_AROON.__doc__ = talib.AROON.__doc__

    def TA_AROONOSC(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AROONOSC, 'TA_AROONOSC', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_AROONOSC.__doc__ = talib.AROONOSC.__doc__

    def TA_ATR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.ATR, 'TA_ATR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ATR.__doc__ = talib.ATR.__doc__

    def TA_AVGPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AVGPRICE,
            'TA_AVGPRICE',
            result_num=1,
            # params={'timeperiod': timeperiod},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_AVGPRICE.__doc__ = talib.AVGPRICE.__doc__

    def TA_BBANDS(ind=None, timeperiod=14, nbdevup=2, nbdevdn=2, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.BBANDS,
            'TA_BBANDS',
            result_num=3,
            params={
                'timeperiod': timeperiod,
                'nbdevup': nbdevup,
                'nbdevdn': nbdevdn,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_BBANDS.__doc__ = talib.BBANDS.__doc__

    def TA_BOP(ind=None):
        imp = crtTaIndicatorImp(ta.BOP, 'TA_BOP', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_BOP.__doc__ = talib.BOP.__doc__

    def TA_CCI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.CCI, 'TA_CCI', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CCI.__doc__ = talib.CCI.__doc__

    def TA_CMO(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.CMO, 'TA_CMO', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CMO.__doc__ = talib.CMO.__doc__

    def TA_DEMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.DEMA, 'TA_DEMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_DEMA.__doc__ = talib.DEMA.__doc__

    def TA_DX(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.DX, 'TA_DX', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_DX.__doc__ = talib.DX.__doc__

    def TA_EMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.EMA, 'TA_EMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_EMA.__doc__ = talib.EMA.__doc__

    def TA_HT_DCPERIOD(ind=None):
        imp = crtTaIndicatorImp(ta.HT_DCPERIOD, 'TA_HT_DCPERIOD', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_DCPERIOD.__doc__ = talib.HT_DCPERIOD.__doc__

    def TA_HT_DCPHASE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_DCPHASE, 'TA_HT_DCPHASE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_DCPHASE.__doc__ = talib.HT_DCPHASE.__doc__

    def TA_HT_PHASOR(ind=None):
        imp = crtTaIndicatorImp(ta.HT_PHASOR, 'TA_HT_PHASOR', result_num=2)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_PHASOR.__doc__ = talib.HT_PHASOR.__doc__

    def TA_HT_SINE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_SINE, 'TA_HT_SINE', result_num=2)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_SINE.__doc__ = talib.HT_SINE.__doc__

    def TA_HT_TRENDLINE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_TRENDLINE, 'TA_HT_TRENDLINE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_TRENDLINE.__doc__ = talib.HT_TRENDLINE.__doc__

    def TA_HT_TRENDMODE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_TRENDMODE, 'TA_HT_TRENDMODE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_HT_TRENDMODE.__doc__ = talib.HT_TRENDMODE.__doc__

    def TA_KAMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.KAMA, 'TA_KAMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_KAMA.__doc__ = talib.KAMA.__doc__

    def TA_LINEARREG(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.LINEARREG, 'TA_LINEARREG', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_LINEARREG.__doc__ = talib.LINEARREG.__doc__

    def TA_LINEARREG_ANGLE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_ANGLE, 'TA_LINEARREG_ANGLE', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_LINEARREG_ANGLE.__doc__ = talib.LINEARREG_ANGLE.__doc__

    def TA_LINEARREG_INTERCEPT(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_INTERCEPT, 'TA_LINEARREG_INTERCEPT', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_LINEARREG_INTERCEPT.__doc__ = talib.LINEARREG_INTERCEPT.__doc__

    def TA_LINEARREG_SLOPE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_SLOPE, 'TA_LINEARREG_SLOPE', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_LINEARREG_SLOPE.__doc__ = talib.LINEARREG_SLOPE.__doc__

    def TA_MA(ind=None, timeperiod=30, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(ta.MA, 'TA_MA', result_num=1, params={'timeperiod': timeperiod, 'matype': matype})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MA.__doc__ = talib.MA.__doc__

    def TA_MACD(ind=None, fastperiod=12, slowperiod=26, signalperiod=9):
        imp = crtTaIndicatorImp(
            ta.MACD,
            'TA_MACD',
            result_num=3,
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'signalperiod': signalperiod
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MACD.__doc__ = talib.MACD.__doc__

    def TA_MACDEXT(
        ind=None,
        fastperiod=12,
        fastmatype=talib.MA_Type.SMA,
        slowperiod=26,
        slowmatype=talib.MA_Type.SMA,
        signalperiod=9,
        signalmatype=talib.MA_Type.SMA
    ):
        imp = crtTaIndicatorImp(
            ta.MACDEXT,
            'TA_MACDEXT',
            result_num=3,
            params={
                'fastperiod': fastperiod,
                'fastmatype': fastmatype,
                'slowperiod': slowperiod,
                'slowmatype': slowmatype,
                'signalperiod': signalperiod,
                'signalmatype': signalmatype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MACDEXT.__doc__ = talib.MACDEXT.__doc__

    def TA_MACDFIX(ind=None, signalperiod=9):
        imp = crtTaIndicatorImp(ta.MACDFIX, 'TA_MACDFIX', result_num=3, params={'signalperiod': signalperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MACDFIX.__doc__ = talib.MACDFIX.__doc__

    def TA_MAMA(ind=None, fastlimit=0.5, slowlimit=0.05):
        imp = crtTaIndicatorImp(
            ta.MAMA, 'TA_MAMA', result_num=2, params={
                'fastlimit': fastlimit,
                'slowlimit': slowlimit
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MAMA.__doc__ = talib.MAMA.__doc__

    def TA_MAX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MAX, 'TA_MAX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MAX.__doc__ = talib.MAX.__doc__

    def TA_MAXINDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MAXINDEX, 'TA_MAXINDEX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MAXINDEX.__doc__ = talib.MAXINDEX.__doc__

    def TA_MEDPRICE(ind=None):
        imp = crtTaIndicatorImp(ta.MEDPRICE, 'TA_MEDPRICE', result_num=1, prices=['high', 'low'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MEDPRICE.__doc__ = talib.MEDPRICE.__doc__

    def TA_MIDPOINT(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.MIDPOINT, 'TA_MIDPOINT', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MIDPOINT.__doc__ = talib.MIDPRICE.__doc__

    def TA_MIDPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MIDPRICE, 'TA_MIDPRICE', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MIDPRICE.__doc__ = talib.MIDPRICE.__doc__

    def TA_MIN(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MIN, 'TA_MIN', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MIN.__doc__ = talib.MIN.__doc__

    def TA_MININDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MININDEX, 'TA_MININDEX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MININDEX.__doc__ = talib.MININDEX.__doc__

    def TA_MINMAX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MINMAX, 'TA_MINMAX', result_num=2, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MINMAX.__doc__ = talib.MINMAX.__doc__

    def TA_MINMAXINDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MINMAXINDEX, 'TA_MINMAXINDEX', result_num=2, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MINMAXINDEX.__doc__ = talib.MINMAXINDEX.__doc__

    def TA_MINUS_DI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MINUS_DI,
            'TA_MINUS_DI',
            result_num=1,
            params={'timeperiod': timeperiod},
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MINUS_DI.__doc__ = talib.MINUS_DI.__doc__

    def TA_MINUS_DM(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MINUS_DM, 'TA_MINUS_DM', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MINUS_DM.__doc__ = talib.MINUS_DM.__doc__

    def TA_MOM(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.MOM, 'TA_MOM', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_MOM.__doc__ = talib.MOM.__doc__

    def TA_NATR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.NATR, 'TA_NATR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_NATR.__doc__ = talib.NATR.__doc__

    def TA_PLUS_DI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.PLUS_DI, 'TA_PLUS_DI', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_PLUS_DI.__doc__ = talib.PLUS_DI.__doc__

    def TA_PLUS_DM(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.PLUS_DM, 'TA_PLUS_DM', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_PLUS_DM.__doc__ = talib.PLUS_DM.__doc__

    def TA_PPO(ind=None, fastperiod=12, slowperiod=26, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.PPO,
            'TA_PPO',
            result_num=1,
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_PPO.__doc__ = talib.PPO.__doc__

    def TA_ROC(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROC, 'TA_ROC', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ROC.__doc__ = talib.ROC.__doc__

    def TA_ROCP(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCP, 'TA_ROCP', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ROCP.__doc__ = talib.ROCP.__doc__

    def TA_ROCR(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCR, 'TA_ROCR', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ROCR.__doc__ = talib.ROCR.__doc__

    def TA_ROCR100(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCR100, 'TA_ROCR100', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ROCR100.__doc__ = talib.ROCR100.__doc__

    def TA_RSI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.RSI, 'TA_RSI', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_RSI.__doc__ = talib.RSI.__doc__

    def TA_SAR(ind=None, acceleration=0.02, maximum=0.2):
        imp = crtTaIndicatorImp(
            ta.SAR,
            'TA_SAR',
            result_num=1,
            params={
                'acceleration': acceleration,
                'maximum': maximum
            },
            prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_SAR.__doc__ = talib.SAR.__doc__

    def TA_SAREXT(
        ind=None,
        startvalue=0,
        offsetonreverse=0,
        accelerationinitlong=0.02,
        accelerationlong=0.02,
        accelerationmaxlong=0.02,
        accelerationinitshort=0.02,
        accelerationshort=0.02,
        accelerationmaxshort=0.02
    ):
        imp = crtTaIndicatorImp(
            ta.SAREXT,
            'TA_SAREXT',
            result_num=1,
            params={
                'startvalue': startvalue,
                'offsetonreverse': offsetonreverse,
                'accelerationinitlong': accelerationinitlong,
                'accelerationlong': accelerationlong,
                'accelerationmaxlong': accelerationmaxlong,
                'accelerationinitshort': accelerationinitshort,
                'accelerationshort': accelerationshort,
                'accelerationmaxshort': accelerationmaxshort
            },
            prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_SAREXT.__doc__ = talib.SAREXT.__doc__

    def TA_SMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.SMA, 'TA_SMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_SMA.__doc__ = talib.SMA.__doc__

    def TA_STDDEV(ind=None, timeperiod=5, nbdev=1):
        imp = crtTaIndicatorImp(ta.STDDEV, 'TA_STDDEV', result_num=1, params={'timeperiod': timeperiod, 'nbdev': nbdev})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_STDDEV.__doc__ = talib.STDDEV.__doc__

    def TA_STOCH(
        ind=None,
        fastk_period=5,
        slowk_period=3,
        slowk_matype=talib.MA_Type.SMA,
        slowd_period=3,
        slowd_matype=talib.MA_Type.SMA
    ):
        imp = crtTaIndicatorImp(
            ta.STOCH,
            'TA_STOCH',
            result_num=2,
            params={
                'fastk_period': fastk_period,
                'slowk_period': slowk_period,
                'slowk_matype': slowk_matype,
                'slowd_period': slowd_period,
                'slowd_matype': slowd_matype
            },
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_STOCH.__doc__ = talib.STOCH.__doc__

    def TA_STOCHF(ind=None, fastk_period=5, fastd_period=3, fastd_matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.STOCHF,
            'TA_STOCHF',
            result_num=2,
            params={
                'fastk_period': fastk_period,
                'fastd_period': fastd_period,
                'fastd_matype': fastd_matype
            },
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_STOCHF.__doc__ = talib.STOCHF.__doc__

    def TA_STOCHRSI(ind=None, timeperiod=14, fastk_period=5, fastd_period=3, fastd_matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.STOCHRSI,
            'TA_STOCHRSI',
            result_num=2,
            params={
                'timeperiod': timeperiod,
                'fastk_period': fastk_period,
                'fastd_period': fastd_period,
                'fastd_matype': fastd_matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_STOCHRSI.__doc__ = talib.STOCHRSI.__doc__

    def TA_SUM(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.SUM, 'TA_SUM', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_SUM.__doc__ = talib.SUM.__doc__

    def TA_T3(ind=None, timeperiod=5, vfactor=0.7):
        imp = crtTaIndicatorImp(ta.T3, 'TA_T3', result_num=1, params={'timeperiod': timeperiod, 'vfactor': vfactor})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_T3.__doc__ = talib.T3.__doc__

    def TA_TEMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TEMA, 'TA_TEMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TEMA.__doc__ = talib.TEMA.__doc__

    def TA_TRANGE(ind=None):
        imp = crtTaIndicatorImp(ta.TRANGE, 'TA_TRANGE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TRANGE.__doc__ = talib.TRANGE.__doc__

    def TA_TRIMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TRIMA, 'TA_TRIMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TRIMA.__doc__ = talib.TRIMA.__doc__

    def TA_TRIX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TRIX, 'TA_TRIX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TRIX.__doc__ = talib.TRIX.__doc__

    def TA_TSF(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.TSF, 'TA_TSF', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TSF.__doc__ = talib.TSF.__doc__

    def TA_TYPPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.TYPPRICE, 'TA_TYPPRICE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_TYPPRICE.__doc__ = talib.TYPPRICE.__doc__

    def TA_ULTOSC(ind=None, timeperiod1=7, timeperiod2=14, timeperiod3=28):
        imp = crtTaIndicatorImp(
            ta.ULTOSC,
            'TA_ULTOSC',
            result_num=1,
            params={
                'timeperiod1': timeperiod1,
                'timeperiod2': timeperiod2,
                'timeperiod3': timeperiod3,
            },
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_ULTOSC.__doc__ = talib.ULTOSC.__doc__

    def TA_VAR(ind=None, timeperiod=5, nbdev=1):
        imp = crtTaIndicatorImp(ta.VAR, 'TA_VAR', result_num=1, params={'timeperiod': timeperiod, 'nbdev': nbdev})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_VAR.__doc__ = talib.VAR.__doc__

    def TA_WCLPRICE(ind=None):
        imp = crtTaIndicatorImp(ta.WCLPRICE, 'TA_WCLPRICE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_WCLPRICE.__doc__ = talib.WCLPRICE.__doc__

    def TA_WILLR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.WILLR, 'TA_WILLR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_WILLR.__doc__ = talib.WILLR.__doc__

    def TA_WMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.WMA, 'TA_WMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_WMA.__doc__ = talib.WMA.__doc__

    def TA_CDL2CROWS(ind=None):
        imp = crtTaIndicatorImp(ta.CDL2CROWS, 'TA_CDL2CROWS', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL2CROWS.__doc__ = talib.CDL2CROWS.__doc__

    def TA_CDL3BLACKCROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3BLACKCROWS, 'TA_CDL3BLACKCROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3BLACKCROWS.__doc__ = talib.CDL3BLACKCROWS.__doc__

    def TA_CDL3INSIDE(ind=None):
        imp = crtTaIndicatorImp(ta.CDL3INSIDE, 'TA_CDL3INSIDE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3INSIDE.__doc__ = talib.CDL3INSIDE.__doc__

    def TA_CDL3LINESTRIKE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3LINESTRIKE, 'TA_CDL3LINESTRIKE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3LINESTRIKE.__doc__ = talib.CDL3LINESTRIKE.__doc__

    def TA_CDL3OUTSIDE(ind=None):
        imp = crtTaIndicatorImp(ta.CDL3OUTSIDE, 'TA_CDL3OUTSIDE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3OUTSIDE.__doc__ = talib.CDL3OUTSIDE.__doc__

    def TA_CDL3STARSINSOUTH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3STARSINSOUTH, 'TA_CDL3STARSINSOUTH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3STARSINSOUTH.__doc__ = talib.CDL3STARSINSOUTH.__doc__

    def TA_CDL3WHITESOLDIERS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3WHITESOLDIERS, 'TA_CDL3WHITESOLDIERS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDL3WHITESOLDIERS.__doc__ = talib.CDL3WHITESOLDIERS.__doc__

    def TA_CDLABANDONEDBABY(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLABANDONEDBABY,
            'TA_CDLABANDONEDBABY',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLABANDONEDBABY.__doc__ = talib.CDLABANDONEDBABY.__doc__

    def TA_CDLADVANCEBLOCK(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLADVANCEBLOCK, 'TA_CDLADVANCEBLOCK', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLADVANCEBLOCK = talib.CDLADVANCEBLOCK.__doc__

    def TA_CDLBELTHOLD(ind=None):
        imp = crtTaIndicatorImp(ta.CDLBELTHOLD, 'TA_CDLBELTHOLD', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLBELTHOLD.__doc__ = talib.CDLBELTHOLD.__doc__

    def TA_CDLBREAKAWAY(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLBREAKAWAY, 'TA_CDLBREAKAWAY', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLBREAKAWAY.__doc__ = talib.CDLBREAKAWAY.__doc__

    def TA_CDLCLOSINGMARUBOZU(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCLOSINGMARUBOZU, 'TA_CDLCLOSINGMARUBOZU', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLCLOSINGMARUBOZU.__doc__ = talib.CDLCLOSINGMARUBOZU.__doc__

    def TA_CDLCONCEALBABYSWALL(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCONCEALBABYSWALL, 'TA_CDLCONCEALBABYSWALL', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLCONCEALBABYSWALL.__doc__ = talib.CDLCONCEALBABYSWALL.__doc__

    def TA_CDLCOUNTERATTACK(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCOUNTERATTACK, 'TA_CDLCOUNTERATTACK', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLCOUNTERATTACK.__doc__ = talib.CDLCOUNTERATTACK.__doc__

    def TA_CDLDARKCLOUDCOVER(ind=None, penetration=0.5):
        imp = crtTaIndicatorImp(
            ta.CDLDARKCLOUDCOVER,
            'TA_CDLDARKCLOUDCOVER',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLDARKCLOUDCOVER.__doc__ = talib.CDLDARKCLOUDCOVER.__doc__

    def TA_CDLDOJI(ind=None):
        imp = crtTaIndicatorImp(ta.CDLDOJI, 'TA_CDLDOJI', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLDOJI.__doc__ = talib.CDLDOJI.__doc__

    def TA_CDLDOJISTAR(ind=None):
        imp = crtTaIndicatorImp(ta.CDLDOJISTAR, 'TA_CDLDOJISTAR', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLDOJISTAR.__doc__ = talib.CDLDOJISTAR.__doc__

    def TA_CDLDRAGONFLYDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLDRAGONFLYDOJI, 'TA_CDLDRAGONFLYDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLDRAGONFLYDOJI.__doc__ = talib.CDLDRAGONFLYDOJI.__doc__

    def TA_CDLENGULFING(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLENGULFING, 'TA_CDLENGULFING', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLENGULFING.__doc__ = talib.CDLENGULFING.__doc__

    def TA_CDLEVENINGDOJISTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLEVENINGDOJISTAR,
            'TA_CDLEVENINGDOJISTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLEVENINGDOJISTAR.__doc__ = talib.CDLEVENINGDOJISTAR.__doc__

    def TA_CDLEVENINGSTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLEVENINGSTAR,
            'TA_CDLEVENINGSTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLEVENINGSTAR.__doc__ = talib.CDLEVENINGSTAR.__doc__

    def TA_CDLGAPSIDESIDEWHITE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLGAPSIDESIDEWHITE, 'TA_CDLGAPSIDESIDEWHITE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLGAPSIDESIDEWHITE.__doc__ = talib.CDLGAPSIDESIDEWHITE.__doc__

    def TA_CDLGRAVESTONEDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLGRAVESTONEDOJI, 'TA_CDLGRAVESTONEDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLGRAVESTONEDOJI.__doc__ = talib.CDLGRAVESTONEDOJI.__doc__

    def TA_CDLHAMMER(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHAMMER, 'TA_CDLHAMMER', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHAMMER.__doc__ = talib.CDLHAMMER.__doc__

    def TA_CDLHANGINGMAN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHANGINGMAN, 'TA_CDLHANGINGMAN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHANGINGMAN.__doc__ = talib.CDLHANGINGMAN.__doc__

    def TA_CDLHARAMI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHARAMI,
            'TA_CDLHARAMI',
            result_num=1,
            # params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHARAMI.__doc__ = talib.CDLHARAMI.__doc__

    def TA_CDLHARAMICROSS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHARAMICROSS, 'TA_CDLHARAMICROSS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHARAMICROSS.__doc__ = talib.CDLHARAMICROSS.__doc__

    def TA_CDLHIGHWAVE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHIGHWAVE, 'TA_CDLHIGHWAVE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHIGHWAVE.__doc__ = talib.CDLHIGHWAVE.__doc__

    def TA_CDLHIKKAKE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHIKKAKE, 'TA_CDLHIKKAKE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHIKKAKE.__doc__ = talib.CDLHIKKAKE.__doc__

    def TA_CDLHIKKAKEMOD(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHIKKAKEMOD, 'TA_CDLHIKKAKEMOD', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHIKKAKEMOD.__doc__ = talib.CDLHIKKAKEMOD.__doc__

    def TA_CDLHOMINGPIGEON(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHOMINGPIGEON, 'TA_CDLHOMINGPIGEON', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLHOMINGPIGEON.__doc__ = talib.CDLHOMINGPIGEON.__doc__

    def TA_CDLIDENTICAL3CROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLIDENTICAL3CROWS, 'TA_CDLIDENTICAL3CROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLIDENTICAL3CROWS.__doc__ = talib.CDLIDENTICAL3CROWS.__doc__

    def TA_CDLINNECK(ind=None):
        imp = crtTaIndicatorImp(ta.CDLINNECK, 'TA_CDLINNECK', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLINNECK.__doc__ = talib.CDLINNECK.__doc__

    def TA_CDLINVERTEDHAMMER(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLINVERTEDHAMMER, 'TA_CDLINVERTEDHAMMER', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLINVERTEDHAMMER.__doc__ = talib.CDLINVERTEDHAMMER.__doc__

    def TA_CDLKICKING(ind=None):
        imp = crtTaIndicatorImp(ta.CDLKICKING, 'TA_CDLKICKING', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLKICKING.__doc__ = talib.CDLKICKING.__doc__

    def TA_CDLKICKINGBYLENGTH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLKICKINGBYLENGTH, 'TA_CDLKICKINGBYLENGTH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLKICKINGBYLENGTH.__doc__ = talib.CDLKICKINGBYLENGTH.__doc__

    def TA_CDLLADDERBOTTOM(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLLADDERBOTTOM, 'TA_CDLLADDERBOTTOM', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLLADDERBOTTOM.__doc__ = talib.CDLLADDERBOTTOM.__doc__

    def TA_CDLLONGLEGGEDDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLLONGLEGGEDDOJI, 'TA_CDLLONGLEGGEDDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLLONGLEGGEDDOJI.__doc__ = talib.CDLLONGLEGGEDDOJI.__doc__

    def TA_CDLLONGLINE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLLONGLINE, 'TA_CDLLONGLINE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLLONGLINE.__doc__ = talib.CDLLONGLINE.__doc__

    def TA_CDLMARUBOZU(ind=None):
        imp = crtTaIndicatorImp(ta.CDLMARUBOZU, 'TA_CDLMARUBOZU', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLMARUBOZU.__doc__ = talib.CDLMARUBOZU.__doc__

    def TA_CDLMATCHINGLOW(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLMATCHINGLOW, 'TA_CDLMATCHINGLOW', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLMATCHINGLOW.__doc__ = talib.CDLMATCHINGLOW.__doc__

    def TA_CDLMATHOLD(ind=None, penetration=0.5):
        imp = crtTaIndicatorImp(
            ta.CDLMATHOLD,
            'TA_CDLMATHOLD',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLMATHOLD.__doc__ = talib.CDLMATHOLD.__doc__

    def TA_CDLMORNINGDOJISTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLMORNINGDOJISTAR,
            'TA_CDLMORNINGDOJISTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLMORNINGDOJISTAR.__doc__ = talib.CDLMORNINGDOJISTAR.__doc__

    def TA_CDLMORNINGSTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLMORNINGSTAR,
            'TA_CDLMORNINGSTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLMORNINGSTAR.__doc__ = talib.CDLMORNINGSTAR.__doc__

    def TA_CDLONNECK(ind=None):
        imp = crtTaIndicatorImp(ta.CDLONNECK, 'TA_CDLONNECK', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLONNECK.__doc__ = talib.CDLONNECK.__doc__

    def TA_CDLPIERCING(ind=None):
        imp = crtTaIndicatorImp(ta.CDLPIERCING, 'TA_CDLPIERCING', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLPIERCING.__doc__ = talib.CDLPIERCING.__doc__

    def TA_CDLRICKSHAWMAN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLRICKSHAWMAN, 'TA_CDLRICKSHAWMAN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLRICKSHAWMAN.__doc__ = talib.CDLRICKSHAWMAN.__doc__

    def TA_CDLRISEFALL3METHODS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLRISEFALL3METHODS, 'TA_CDLRISEFALL3METHODS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLRISEFALL3METHODS.__doc__ = talib.CDLRISEFALL3METHODS.__doc__

    def TA_CDLSEPARATINGLINES(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSEPARATINGLINES, 'TA_CDLSEPARATINGLINES', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSEPARATINGLINES.__doc__ = talib.CDLSEPARATINGLINES.__doc__

    def TA_CDLSHOOTINGSTAR(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSHOOTINGSTAR, 'TA_CDLSHOOTINGSTAR', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSHOOTINGSTAR.__doc__ = talib.CDLSHOOTINGSTAR.__doc__

    def TA_CDLSHORTLINE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSHORTLINE, 'TA_CDLSHORTLINE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSHORTLINE.__doc__ = talib.CDLSHORTLINE.__doc__

    def TA_CDLSPINNINGTOP(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSPINNINGTOP, 'TA_CDLSPINNINGTOP', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSPINNINGTOP.__doc__ = talib.CDLSPINNINGTOP.__doc__

    def TA_CDLSTALLEDPATTERN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSTALLEDPATTERN, 'TA_CDLSTALLEDPATTERN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSTALLEDPATTERN.__doc__ = talib.CDLSTALLEDPATTERN.__doc__

    def TA_CDLSTICKSANDWICH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSTICKSANDWICH, 'TA_CDLSTICKSANDWICH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLSTICKSANDWICH.__doc__ = talib.CDLSTICKSANDWICH.__doc__

    def TA_CDLTAKURI(ind=None):
        imp = crtTaIndicatorImp(ta.CDLTAKURI, 'TA_CDLTAKURI', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLTAKURI.__doc__ = talib.CDLTAKURI.__doc__

    def TA_CDLTASUKIGAP(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLTASUKIGAP, 'TA_CDLTASUKIGAP', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLTASUKIGAP.__doc__ = talib.CDLTASUKIGAP.__doc__

    def TA_CDLTHRUSTING(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLTHRUSTING, 'TA_CDLTHRUSTING', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLTHRUSTING.__doc__ = talib.CDLTHRUSTING.__doc__

    def TA_CDLTRISTAR(ind=None):
        imp = crtTaIndicatorImp(ta.CDLTRISTAR, 'TA_CDLTRISTAR', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLTRISTAR.__doc__ = talib.CDLTRISTAR.__doc__

    def TA_CDLUNIQUE3RIVER(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLUNIQUE3RIVER, 'TA_CDLUNIQUE3RIVER', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLUNIQUE3RIVER.__doc__ = talib.CDLUNIQUE3RIVER.__doc__

    def TA_CDLUPSIDEGAP2CROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLUPSIDEGAP2CROWS, 'TA_CDLUPSIDEGAP2CROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLUPSIDEGAP2CROWS.__doc__ = talib.CDLUPSIDEGAP2CROWS.__doc__

    def TA_CDLXSIDEGAP3METHODS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLXSIDEGAP3METHODS, 'TA_CDLXSIDEGAP3METHODS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CDLXSIDEGAP3METHODS.__doc__ = talib.CDLXSIDEGAP3METHODS.__doc__

    def TA_BETA(ind=None, timeperiod=5):
        imp = crtTaIndicatorImp(ta.BETA, 'TA_BETA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_BETA.__doc__ = talib.BETA.__doc__

    def TA_CORREL(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.CORREL, 'TA_CORREL', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_CORREL.__doc__ = talib.CORREL.__doc__

    def TA_OBV(ind=None):
        imp = crtTaIndicatorImp(ta.OBV, 'TA_OBV', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    TA_OBV.__doc__ = talib.OBV.__doc__

except:
    pass  # 非必须的，talib，不再打印告警，以免误解提问
    # print(
    #     "warning: can't import TA-Lib, will be ignored! You can fetch ta-lib "
    #     "from https://www.lfd.uci.edu/~gohlke/pythonlibs/#ta-lib"
    # )
