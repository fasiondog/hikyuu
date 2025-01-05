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
            if self.name == "PYTA_OBV":
                if ind.get_result_num() < 2:
                    print("error: result_num must be >= 2!")
                    return
                inputs = {'close': ind.get_result(0).to_np(), 'volume': ind.get_result(1).to_np()}
            elif self.name in ("PYTA_BETA", "PYTA_CORREL"):
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

    def PYTA_AD(ind=None):
        imp = crtTaIndicatorImp(ta.AD, 'PYTA_AD', prices=['high', 'low', 'close', 'volume'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_AD.__doc__ = talib.AD.__doc__

    def PYTA_ADOSC(ind=None, fastperiod=3, slowperiod=10):
        imp = crtTaIndicatorImp(
            ta.ADOSC,
            'PYTA_ADOSC',
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod
            },
            prices=['high', 'low', 'close', 'volume']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ADOSC.__doc__ = talib.ADOSC.__doc__

    def PYTA_ADX(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.ADX, 'PYTA_ADX', params={'timeperiod': timeperiod}, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ADX.__doc__ = talib.ADX.__doc__

    def PYTA_ADXR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.ADXR, 'PYTA_ADXR', params={
                                'timeperiod': timeperiod}, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ADXR.__doc__ = talib.ADXR.__doc__

    def PYTA_APO(ind=None, fastperiod=12, slowperiod=26, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.APO, 'PYTA_APO', params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_APO.__doc__ = talib.APO.__doc__

    def PYTA_AROON(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AROON, 'PYTA_AROON', result_num=2, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_AROON.__doc__ = talib.AROON.__doc__

    def PYTA_AROONOSC(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AROONOSC, 'PYTA_AROONOSC', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_AROONOSC.__doc__ = talib.AROONOSC.__doc__

    def PYTA_ATR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.ATR, 'PYTA_ATR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ATR.__doc__ = talib.ATR.__doc__

    def PYTA_AVGPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.AVGPRICE,
            'PYTA_AVGPRICE',
            result_num=1,
            # params={'timeperiod': timeperiod},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_AVGPRICE.__doc__ = talib.AVGPRICE.__doc__

    def PYTA_BBANDS(ind=None, timeperiod=14, nbdevup=2, nbdevdn=2, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.BBANDS,
            'PYTA_BBANDS',
            result_num=3,
            params={
                'timeperiod': timeperiod,
                'nbdevup': nbdevup,
                'nbdevdn': nbdevdn,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_BBANDS.__doc__ = talib.BBANDS.__doc__

    def PYTA_BOP(ind=None):
        imp = crtTaIndicatorImp(ta.BOP, 'PYTA_BOP', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_BOP.__doc__ = talib.BOP.__doc__

    def PYTA_CCI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.CCI, 'PYTA_CCI', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CCI.__doc__ = talib.CCI.__doc__

    def PYTA_CMO(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.CMO, 'PYTA_CMO', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CMO.__doc__ = talib.CMO.__doc__

    def PYTA_DEMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.DEMA, 'PYTA_DEMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_DEMA.__doc__ = talib.DEMA.__doc__

    def PYTA_DX(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.DX, 'PYTA_DX', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_DX.__doc__ = talib.DX.__doc__

    def PYTA_EMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.EMA, 'PYTA_EMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_EMA.__doc__ = talib.EMA.__doc__

    def PYTA_HT_DCPERIOD(ind=None):
        imp = crtTaIndicatorImp(ta.HT_DCPERIOD, 'PYTA_HT_DCPERIOD', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_DCPERIOD.__doc__ = talib.HT_DCPERIOD.__doc__

    def PYTA_HT_DCPHASE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_DCPHASE, 'PYTA_HT_DCPHASE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_DCPHASE.__doc__ = talib.HT_DCPHASE.__doc__

    def PYTA_HT_PHASOR(ind=None):
        imp = crtTaIndicatorImp(ta.HT_PHASOR, 'PYTA_HT_PHASOR', result_num=2)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_PHASOR.__doc__ = talib.HT_PHASOR.__doc__

    def PYTA_HT_SINE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_SINE, 'PYTA_HT_SINE', result_num=2)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_SINE.__doc__ = talib.HT_SINE.__doc__

    def PYTA_HT_TRENDLINE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_TRENDLINE, 'PYTA_HT_TRENDLINE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_TRENDLINE.__doc__ = talib.HT_TRENDLINE.__doc__

    def PYTA_HT_TRENDMODE(ind=None):
        imp = crtTaIndicatorImp(ta.HT_TRENDMODE, 'PYTA_HT_TRENDMODE', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_HT_TRENDMODE.__doc__ = talib.HT_TRENDMODE.__doc__

    def PYTA_KAMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.KAMA, 'PYTA_KAMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_KAMA.__doc__ = talib.KAMA.__doc__

    def PYTA_LINEARREG(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.LINEARREG, 'PYTA_LINEARREG', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_LINEARREG.__doc__ = talib.LINEARREG.__doc__

    def PYTA_LINEARREG_ANGLE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_ANGLE, 'PYTA_LINEARREG_ANGLE', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_LINEARREG_ANGLE.__doc__ = talib.LINEARREG_ANGLE.__doc__

    def PYTA_LINEARREG_INTERCEPT(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_INTERCEPT, 'PYTA_LINEARREG_INTERCEPT', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_LINEARREG_INTERCEPT.__doc__ = talib.LINEARREG_INTERCEPT.__doc__

    def PYTA_LINEARREG_SLOPE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.LINEARREG_SLOPE, 'PYTA_LINEARREG_SLOPE', result_num=1, params={'timeperiod': timeperiod}
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_LINEARREG_SLOPE.__doc__ = talib.LINEARREG_SLOPE.__doc__

    def PYTA_MA(ind=None, timeperiod=30, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(ta.MA, 'PYTA_MA', result_num=1, params={'timeperiod': timeperiod, 'matype': matype})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MA.__doc__ = talib.MA.__doc__

    def PYTA_MACD(ind=None, fastperiod=12, slowperiod=26, signalperiod=9):
        imp = crtTaIndicatorImp(
            ta.MACD,
            'PYTA_MACD',
            result_num=3,
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'signalperiod': signalperiod
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MACD.__doc__ = talib.MACD.__doc__

    def PYTA_MACDEXT(
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
            'PYTA_MACDEXT',
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

    PYTA_MACDEXT.__doc__ = talib.MACDEXT.__doc__

    def PYTA_MACDFIX(ind=None, signalperiod=9):
        imp = crtTaIndicatorImp(ta.MACDFIX, 'PYTA_MACDFIX', result_num=3, params={'signalperiod': signalperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MACDFIX.__doc__ = talib.MACDFIX.__doc__

    def PYTA_MAMA(ind=None, fastlimit=0.5, slowlimit=0.05):
        imp = crtTaIndicatorImp(
            ta.MAMA, 'PYTA_MAMA', result_num=2, params={
                'fastlimit': fastlimit,
                'slowlimit': slowlimit
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MAMA.__doc__ = talib.MAMA.__doc__

    def PYTA_MAX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MAX, 'PYTA_MAX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MAX.__doc__ = talib.MAX.__doc__

    def PYTA_MAXINDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MAXINDEX, 'PYTA_MAXINDEX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MAXINDEX.__doc__ = talib.MAXINDEX.__doc__

    def PYTA_MEDPRICE(ind=None):
        imp = crtTaIndicatorImp(ta.MEDPRICE, 'PYTA_MEDPRICE', result_num=1, prices=['high', 'low'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MEDPRICE.__doc__ = talib.MEDPRICE.__doc__

    def PYTA_MIDPOINT(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.MIDPOINT, 'PYTA_MIDPOINT', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MIDPOINT.__doc__ = talib.MIDPRICE.__doc__

    def PYTA_MIDPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MIDPRICE, 'PYTA_MIDPRICE', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MIDPRICE.__doc__ = talib.MIDPRICE.__doc__

    def PYTA_MIN(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MIN, 'PYTA_MIN', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MIN.__doc__ = talib.MIN.__doc__

    def PYTA_MININDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MININDEX, 'PYTA_MININDEX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MININDEX.__doc__ = talib.MININDEX.__doc__

    def PYTA_MINMAX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MINMAX, 'PYTA_MINMAX', result_num=2, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MINMAX.__doc__ = talib.MINMAX.__doc__

    def PYTA_MINMAXINDEX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.MINMAXINDEX, 'PYTA_MINMAXINDEX', result_num=2, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MINMAXINDEX.__doc__ = talib.MINMAXINDEX.__doc__

    def PYTA_MINUS_DI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MINUS_DI,
            'PYTA_MINUS_DI',
            result_num=1,
            params={'timeperiod': timeperiod},
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MINUS_DI.__doc__ = talib.MINUS_DI.__doc__

    def PYTA_MINUS_DM(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.MINUS_DM, 'PYTA_MINUS_DM', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MINUS_DM.__doc__ = talib.MINUS_DM.__doc__

    def PYTA_MOM(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.MOM, 'PYTA_MOM', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_MOM.__doc__ = talib.MOM.__doc__

    def PYTA_NATR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.NATR, 'PYTA_NATR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_NATR.__doc__ = talib.NATR.__doc__

    def PYTA_PLUS_DI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.PLUS_DI, 'PYTA_PLUS_DI', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_PLUS_DI.__doc__ = talib.PLUS_DI.__doc__

    def PYTA_PLUS_DM(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.PLUS_DM, 'PYTA_PLUS_DM', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_PLUS_DM.__doc__ = talib.PLUS_DM.__doc__

    def PYTA_PPO(ind=None, fastperiod=12, slowperiod=26, matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.PPO,
            'PYTA_PPO',
            result_num=1,
            params={
                'fastperiod': fastperiod,
                'slowperiod': slowperiod,
                'matype': matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_PPO.__doc__ = talib.PPO.__doc__

    def PYTA_ROC(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROC, 'PYTA_ROC', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ROC.__doc__ = talib.ROC.__doc__

    def PYTA_ROCP(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCP, 'PYTA_ROCP', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ROCP.__doc__ = talib.ROCP.__doc__

    def PYTA_ROCR(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCR, 'PYTA_ROCR', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ROCR.__doc__ = talib.ROCR.__doc__

    def PYTA_ROCR100(ind=None, timeperiod=10):
        imp = crtTaIndicatorImp(ta.ROCR100, 'PYTA_ROCR100', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ROCR100.__doc__ = talib.ROCR100.__doc__

    def PYTA_RSI(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.RSI, 'PYTA_RSI', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_RSI.__doc__ = talib.RSI.__doc__

    def PYTA_SAR(ind=None, acceleration=0.02, maximum=0.2):
        imp = crtTaIndicatorImp(
            ta.SAR,
            'PYTA_SAR',
            result_num=1,
            params={
                'acceleration': acceleration,
                'maximum': maximum
            },
            prices=['high', 'low']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_SAR.__doc__ = talib.SAR.__doc__

    def PYTA_SAREXT(
        ind=None,
        startvalue=0.,
        offsetonreverse=0.,
        accelerationinitlong=0.02,
        accelerationlong=0.02,
        accelerationmaxlong=0.02,
        accelerationinitshort=0.02,
        accelerationshort=0.02,
        accelerationmaxshort=0.02
    ):
        imp = crtTaIndicatorImp(
            ta.SAREXT,
            'PYTA_SAREXT',
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

    PYTA_SAREXT.__doc__ = talib.SAREXT.__doc__

    def PYTA_SMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.SMA, 'PYTA_SMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_SMA.__doc__ = talib.SMA.__doc__

    def PYTA_STDDEV(ind=None, timeperiod=5, nbdev=1.0):
        imp = crtTaIndicatorImp(ta.STDDEV, 'PYTA_STDDEV', result_num=1, params={
                                'timeperiod': timeperiod, 'nbdev': nbdev})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_STDDEV.__doc__ = talib.STDDEV.__doc__

    def PYTA_STOCH(
        ind=None,
        fastk_period=5,
        slowk_period=3,
        slowk_matype=talib.MA_Type.SMA,
        slowd_period=3,
        slowd_matype=talib.MA_Type.SMA
    ):
        imp = crtTaIndicatorImp(
            ta.STOCH,
            'PYTA_STOCH',
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

    PYTA_STOCH.__doc__ = talib.STOCH.__doc__

    def PYTA_STOCHF(ind=None, fastk_period=5, fastd_period=3, fastd_matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.STOCHF,
            'PYTA_STOCHF',
            result_num=2,
            params={
                'fastk_period': fastk_period,
                'fastd_period': fastd_period,
                'fastd_matype': fastd_matype
            },
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_STOCHF.__doc__ = talib.STOCHF.__doc__

    def PYTA_STOCHRSI(ind=None, timeperiod=14, fastk_period=5, fastd_period=3, fastd_matype=talib.MA_Type.SMA):
        imp = crtTaIndicatorImp(
            ta.STOCHRSI,
            'PYTA_STOCHRSI',
            result_num=2,
            params={
                'timeperiod': timeperiod,
                'fastk_period': fastk_period,
                'fastd_period': fastd_period,
                'fastd_matype': fastd_matype
            }
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_STOCHRSI.__doc__ = talib.STOCHRSI.__doc__

    def PYTA_SUM(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.SUM, 'PYTA_SUM', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_SUM.__doc__ = talib.SUM.__doc__

    def PYTA_T3(ind=None, timeperiod=5, vfactor=0.7):
        imp = crtTaIndicatorImp(ta.T3, 'PYTA_T3', result_num=1, params={'timeperiod': timeperiod, 'vfactor': vfactor})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_T3.__doc__ = talib.T3.__doc__

    def PYTA_TEMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TEMA, 'PYTA_TEMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TEMA.__doc__ = talib.TEMA.__doc__

    def PYTA_TRANGE(ind=None):
        imp = crtTaIndicatorImp(ta.TRANGE, 'PYTA_TRANGE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TRANGE.__doc__ = talib.TRANGE.__doc__

    def PYTA_TRIMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TRIMA, 'PYTA_TRIMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TRIMA.__doc__ = talib.TRIMA.__doc__

    def PYTA_TRIX(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.TRIX, 'PYTA_TRIX', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TRIX.__doc__ = talib.TRIX.__doc__

    def PYTA_TSF(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.TSF, 'PYTA_TSF', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TSF.__doc__ = talib.TSF.__doc__

    def PYTA_TYPPRICE(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(ta.TYPPRICE, 'PYTA_TYPPRICE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_TYPPRICE.__doc__ = talib.TYPPRICE.__doc__

    def PYTA_ULTOSC(ind=None, timeperiod1=7, timeperiod2=14, timeperiod3=28):
        imp = crtTaIndicatorImp(
            ta.ULTOSC,
            'PYTA_ULTOSC',
            result_num=1,
            params={
                'timeperiod1': timeperiod1,
                'timeperiod2': timeperiod2,
                'timeperiod3': timeperiod3,
            },
            prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_ULTOSC.__doc__ = talib.ULTOSC.__doc__

    def PYTA_VAR(ind=None, timeperiod=5, nbdev=1.):
        imp = crtTaIndicatorImp(ta.VAR, 'PYTA_VAR', result_num=1, params={'timeperiod': timeperiod, 'nbdev': nbdev})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_VAR.__doc__ = talib.VAR.__doc__

    def PYTA_WCLPRICE(ind=None):
        imp = crtTaIndicatorImp(ta.WCLPRICE, 'PYTA_WCLPRICE', result_num=1, prices=['high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_WCLPRICE.__doc__ = talib.WCLPRICE.__doc__

    def PYTA_WILLR(ind=None, timeperiod=14):
        imp = crtTaIndicatorImp(
            ta.WILLR, 'PYTA_WILLR', result_num=1, params={'timeperiod': timeperiod}, prices=['high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_WILLR.__doc__ = talib.WILLR.__doc__

    def PYTA_WMA(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.WMA, 'PYTA_WMA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_WMA.__doc__ = talib.WMA.__doc__

    def PYTA_CDL2CROWS(ind=None):
        imp = crtTaIndicatorImp(ta.CDL2CROWS, 'PYTA_CDL2CROWS', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL2CROWS.__doc__ = talib.CDL2CROWS.__doc__

    def PYTA_CDL3BLACKCROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3BLACKCROWS, 'PYTA_CDL3BLACKCROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3BLACKCROWS.__doc__ = talib.CDL3BLACKCROWS.__doc__

    def PYTA_CDL3INSIDE(ind=None):
        imp = crtTaIndicatorImp(ta.CDL3INSIDE, 'PYTA_CDL3INSIDE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3INSIDE.__doc__ = talib.CDL3INSIDE.__doc__

    def PYTA_CDL3LINESTRIKE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3LINESTRIKE, 'PYTA_CDL3LINESTRIKE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3LINESTRIKE.__doc__ = talib.CDL3LINESTRIKE.__doc__

    def PYTA_CDL3OUTSIDE(ind=None):
        imp = crtTaIndicatorImp(ta.CDL3OUTSIDE, 'PYTA_CDL3OUTSIDE', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3OUTSIDE.__doc__ = talib.CDL3OUTSIDE.__doc__

    def PYTA_CDL3STARSINSOUTH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3STARSINSOUTH, 'PYTA_CDL3STARSINSOUTH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3STARSINSOUTH.__doc__ = talib.CDL3STARSINSOUTH.__doc__

    def PYTA_CDL3WHITESOLDIERS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDL3WHITESOLDIERS, 'PYTA_CDL3WHITESOLDIERS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDL3WHITESOLDIERS.__doc__ = talib.CDL3WHITESOLDIERS.__doc__

    def PYTA_CDLABANDONEDBABY(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLABANDONEDBABY,
            'PYTA_CDLABANDONEDBABY',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLABANDONEDBABY.__doc__ = talib.CDLABANDONEDBABY.__doc__

    def PYTA_CDLADVANCEBLOCK(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLADVANCEBLOCK, 'PYTA_CDLADVANCEBLOCK', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLADVANCEBLOCK = talib.CDLADVANCEBLOCK.__doc__

    def PYTA_CDLBELTHOLD(ind=None):
        imp = crtTaIndicatorImp(ta.CDLBELTHOLD, 'PYTA_CDLBELTHOLD', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLBELTHOLD.__doc__ = talib.CDLBELTHOLD.__doc__

    def PYTA_CDLBREAKAWAY(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLBREAKAWAY, 'PYTA_CDLBREAKAWAY', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLBREAKAWAY.__doc__ = talib.CDLBREAKAWAY.__doc__

    def PYTA_CDLCLOSINGMARUBOZU(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCLOSINGMARUBOZU, 'PYTA_CDLCLOSINGMARUBOZU', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLCLOSINGMARUBOZU.__doc__ = talib.CDLCLOSINGMARUBOZU.__doc__

    def PYTA_CDLCONCEALBABYSWALL(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCONCEALBABYSWALL, 'PYTA_CDLCONCEALBABYSWALL', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLCONCEALBABYSWALL.__doc__ = talib.CDLCONCEALBABYSWALL.__doc__

    def PYTA_CDLCOUNTERATTACK(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLCOUNTERATTACK, 'PYTA_CDLCOUNTERATTACK', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLCOUNTERATTACK.__doc__ = talib.CDLCOUNTERATTACK.__doc__

    def PYTA_CDLDARKCLOUDCOVER(ind=None, penetration=0.5):
        imp = crtTaIndicatorImp(
            ta.CDLDARKCLOUDCOVER,
            'PYTA_CDLDARKCLOUDCOVER',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLDARKCLOUDCOVER.__doc__ = talib.CDLDARKCLOUDCOVER.__doc__

    def PYTA_CDLDOJI(ind=None):
        imp = crtTaIndicatorImp(ta.CDLDOJI, 'PYTA_CDLDOJI', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLDOJI.__doc__ = talib.CDLDOJI.__doc__

    def PYTA_CDLDOJISTAR(ind=None):
        imp = crtTaIndicatorImp(ta.CDLDOJISTAR, 'PYTA_CDLDOJISTAR', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLDOJISTAR.__doc__ = talib.CDLDOJISTAR.__doc__

    def PYTA_CDLDRAGONFLYDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLDRAGONFLYDOJI, 'PYTA_CDLDRAGONFLYDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLDRAGONFLYDOJI.__doc__ = talib.CDLDRAGONFLYDOJI.__doc__

    def PYTA_CDLENGULFING(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLENGULFING, 'PYTA_CDLENGULFING', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLENGULFING.__doc__ = talib.CDLENGULFING.__doc__

    def PYTA_CDLEVENINGDOJISTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLEVENINGDOJISTAR,
            'PYTA_CDLEVENINGDOJISTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLEVENINGDOJISTAR.__doc__ = talib.CDLEVENINGDOJISTAR.__doc__

    def PYTA_CDLEVENINGSTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLEVENINGSTAR,
            'PYTA_CDLEVENINGSTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLEVENINGSTAR.__doc__ = talib.CDLEVENINGSTAR.__doc__

    def PYTA_CDLGAPSIDESIDEWHITE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLGAPSIDESIDEWHITE, 'PYTA_CDLGAPSIDESIDEWHITE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLGAPSIDESIDEWHITE.__doc__ = talib.CDLGAPSIDESIDEWHITE.__doc__

    def PYTA_CDLGRAVESTONEDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLGRAVESTONEDOJI, 'PYTA_CDLGRAVESTONEDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLGRAVESTONEDOJI.__doc__ = talib.CDLGRAVESTONEDOJI.__doc__

    def PYTA_CDLHAMMER(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHAMMER, 'PYTA_CDLHAMMER', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHAMMER.__doc__ = talib.CDLHAMMER.__doc__

    def PYTA_CDLHANGINGMAN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHANGINGMAN, 'PYTA_CDLHANGINGMAN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHANGINGMAN.__doc__ = talib.CDLHANGINGMAN.__doc__

    def PYTA_CDLHARAMI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHARAMI,
            'PYTA_CDLHARAMI',
            result_num=1,
            # params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHARAMI.__doc__ = talib.CDLHARAMI.__doc__

    def PYTA_CDLHARAMICROSS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHARAMICROSS, 'PYTA_CDLHARAMICROSS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHARAMICROSS.__doc__ = talib.CDLHARAMICROSS.__doc__

    def PYTA_CDLHIGHWAVE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHIGHWAVE, 'PYTA_CDLHIGHWAVE', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHIGHWAVE.__doc__ = talib.CDLHIGHWAVE.__doc__

    def PYTA_CDLHIKKAKE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLHIKKAKE, 'PYTA_CDLHIKKAKE', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHIKKAKE.__doc__ = talib.CDLHIKKAKE.__doc__

    def PYTA_CDLHIKKAKEMOD(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHIKKAKEMOD, 'PYTA_CDLHIKKAKEMOD', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHIKKAKEMOD.__doc__ = talib.CDLHIKKAKEMOD.__doc__

    def PYTA_CDLHOMINGPIGEON(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLHOMINGPIGEON, 'PYTA_CDLHOMINGPIGEON', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLHOMINGPIGEON.__doc__ = talib.CDLHOMINGPIGEON.__doc__

    def PYTA_CDLIDENTICAL3CROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLIDENTICAL3CROWS, 'PYTA_CDLIDENTICAL3CROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLIDENTICAL3CROWS.__doc__ = talib.CDLIDENTICAL3CROWS.__doc__

    def PYTA_CDLINNECK(ind=None):
        imp = crtTaIndicatorImp(ta.CDLINNECK, 'PYTA_CDLINNECK', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLINNECK.__doc__ = talib.CDLINNECK.__doc__

    def PYTA_CDLINVERTEDHAMMER(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLINVERTEDHAMMER, 'PYTA_CDLINVERTEDHAMMER', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLINVERTEDHAMMER.__doc__ = talib.CDLINVERTEDHAMMER.__doc__

    def PYTA_CDLKICKING(ind=None):
        imp = crtTaIndicatorImp(ta.CDLKICKING, 'PYTA_CDLKICKING', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLKICKING.__doc__ = talib.CDLKICKING.__doc__

    def PYTA_CDLKICKINGBYLENGTH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLKICKINGBYLENGTH, 'PYTA_CDLKICKINGBYLENGTH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLKICKINGBYLENGTH.__doc__ = talib.CDLKICKINGBYLENGTH.__doc__

    def PYTA_CDLLADDERBOTTOM(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLLADDERBOTTOM, 'PYTA_CDLLADDERBOTTOM', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLLADDERBOTTOM.__doc__ = talib.CDLLADDERBOTTOM.__doc__

    def PYTA_CDLLONGLEGGEDDOJI(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLLONGLEGGEDDOJI, 'PYTA_CDLLONGLEGGEDDOJI', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLLONGLEGGEDDOJI.__doc__ = talib.CDLLONGLEGGEDDOJI.__doc__

    def PYTA_CDLLONGLINE(ind=None):
        imp = crtTaIndicatorImp(ta.CDLLONGLINE, 'PYTA_CDLLONGLINE', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLLONGLINE.__doc__ = talib.CDLLONGLINE.__doc__

    def PYTA_CDLMARUBOZU(ind=None):
        imp = crtTaIndicatorImp(ta.CDLMARUBOZU, 'PYTA_CDLMARUBOZU', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLMARUBOZU.__doc__ = talib.CDLMARUBOZU.__doc__

    def PYTA_CDLMATCHINGLOW(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLMATCHINGLOW, 'PYTA_CDLMATCHINGLOW', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLMATCHINGLOW.__doc__ = talib.CDLMATCHINGLOW.__doc__

    def PYTA_CDLMATHOLD(ind=None, penetration=0.5):
        imp = crtTaIndicatorImp(
            ta.CDLMATHOLD,
            'PYTA_CDLMATHOLD',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLMATHOLD.__doc__ = talib.CDLMATHOLD.__doc__

    def PYTA_CDLMORNINGDOJISTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLMORNINGDOJISTAR,
            'PYTA_CDLMORNINGDOJISTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLMORNINGDOJISTAR.__doc__ = talib.CDLMORNINGDOJISTAR.__doc__

    def PYTA_CDLMORNINGSTAR(ind=None, penetration=0.3):
        imp = crtTaIndicatorImp(
            ta.CDLMORNINGSTAR,
            'PYTA_CDLMORNINGSTAR',
            result_num=1,
            params={'penetration': penetration},
            prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLMORNINGSTAR.__doc__ = talib.CDLMORNINGSTAR.__doc__

    def PYTA_CDLONNECK(ind=None):
        imp = crtTaIndicatorImp(ta.CDLONNECK, 'PYTA_CDLONNECK', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLONNECK.__doc__ = talib.CDLONNECK.__doc__

    def PYTA_CDLPIERCING(ind=None):
        imp = crtTaIndicatorImp(ta.CDLPIERCING, 'PYTA_CDLPIERCING', result_num=1,
                                prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLPIERCING.__doc__ = talib.CDLPIERCING.__doc__

    def PYTA_CDLRICKSHAWMAN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLRICKSHAWMAN, 'PYTA_CDLRICKSHAWMAN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLRICKSHAWMAN.__doc__ = talib.CDLRICKSHAWMAN.__doc__

    def PYTA_CDLRISEFALL3METHODS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLRISEFALL3METHODS, 'PYTA_CDLRISEFALL3METHODS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLRISEFALL3METHODS.__doc__ = talib.CDLRISEFALL3METHODS.__doc__

    def PYTA_CDLSEPARATINGLINES(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSEPARATINGLINES, 'PYTA_CDLSEPARATINGLINES', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSEPARATINGLINES.__doc__ = talib.CDLSEPARATINGLINES.__doc__

    def PYTA_CDLSHOOTINGSTAR(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSHOOTINGSTAR, 'PYTA_CDLSHOOTINGSTAR', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSHOOTINGSTAR.__doc__ = talib.CDLSHOOTINGSTAR.__doc__

    def PYTA_CDLSHORTLINE(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSHORTLINE, 'PYTA_CDLSHORTLINE', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSHORTLINE.__doc__ = talib.CDLSHORTLINE.__doc__

    def PYTA_CDLSPINNINGTOP(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSPINNINGTOP, 'PYTA_CDLSPINNINGTOP', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSPINNINGTOP.__doc__ = talib.CDLSPINNINGTOP.__doc__

    def PYTA_CDLSTALLEDPATTERN(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSTALLEDPATTERN, 'PYTA_CDLSTALLEDPATTERN', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSTALLEDPATTERN.__doc__ = talib.CDLSTALLEDPATTERN.__doc__

    def PYTA_CDLSTICKSANDWICH(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLSTICKSANDWICH, 'PYTA_CDLSTICKSANDWICH', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLSTICKSANDWICH.__doc__ = talib.CDLSTICKSANDWICH.__doc__

    def PYTA_CDLTAKURI(ind=None):
        imp = crtTaIndicatorImp(ta.CDLTAKURI, 'PYTA_CDLTAKURI', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLTAKURI.__doc__ = talib.CDLTAKURI.__doc__

    def PYTA_CDLTASUKIGAP(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLTASUKIGAP, 'PYTA_CDLTASUKIGAP', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLTASUKIGAP.__doc__ = talib.CDLTASUKIGAP.__doc__

    def PYTA_CDLTHRUSTING(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLTHRUSTING, 'PYTA_CDLTHRUSTING', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLTHRUSTING.__doc__ = talib.CDLTHRUSTING.__doc__

    def PYTA_CDLTRISTAR(ind=None):
        imp = crtTaIndicatorImp(ta.CDLTRISTAR, 'PYTA_CDLTRISTAR', result_num=1, prices=['open', 'high', 'low', 'close'])
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLTRISTAR.__doc__ = talib.CDLTRISTAR.__doc__

    def PYTA_CDLUNIQUE3RIVER(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLUNIQUE3RIVER, 'PYTA_CDLUNIQUE3RIVER', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLUNIQUE3RIVER.__doc__ = talib.CDLUNIQUE3RIVER.__doc__

    def PYTA_CDLUPSIDEGAP2CROWS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLUPSIDEGAP2CROWS, 'PYTA_CDLUPSIDEGAP2CROWS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLUPSIDEGAP2CROWS.__doc__ = talib.CDLUPSIDEGAP2CROWS.__doc__

    def PYTA_CDLXSIDEGAP3METHODS(ind=None):
        imp = crtTaIndicatorImp(
            ta.CDLXSIDEGAP3METHODS, 'PYTA_CDLXSIDEGAP3METHODS', result_num=1, prices=['open', 'high', 'low', 'close']
        )
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CDLXSIDEGAP3METHODS.__doc__ = talib.CDLXSIDEGAP3METHODS.__doc__

    def PYTA_BETA(ind=None, timeperiod=5):
        imp = crtTaIndicatorImp(ta.BETA, 'PYTA_BETA', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_BETA.__doc__ = talib.BETA.__doc__

    def PYTA_CORREL(ind=None, timeperiod=30):
        imp = crtTaIndicatorImp(ta.CORREL, 'PYTA_CORREL', result_num=1, params={'timeperiod': timeperiod})
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_CORREL.__doc__ = talib.CORREL.__doc__

    def PYTA_OBV(ind=None):
        imp = crtTaIndicatorImp(ta.OBV, 'PYTA_OBV', result_num=1)
        return Indicator(imp)(ind) if ind else Indicator(imp)

    PYTA_OBV.__doc__ = talib.OBV.__doc__

except:
    pass  # 非必须的，talib，不再打印告警，以免误解提问
    # print(
    #     "warning: can't import TA-Lib, will be ignored! You can fetch ta-lib "
    #     "from https://www.lfd.uci.edu/~gohlke/pythonlibs/#ta-lib"
    # )
