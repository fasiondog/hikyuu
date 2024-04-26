# 已废弃接口

from .extend import *
from .util.mylog import spend_time


def deprecated_func(new_func, old_func_name, new_func_name):
    def wrap_deprecated_func(func):
        def wrapfunc(*args, **kwarg):
            print(
                # 'Deprecated warning: "{}" will be deprecated, please use: "{}"'.format(
                '警告: "{}" 函数即将废弃，请使用 "{}" 代替'.format(old_func_name, new_func_name)
            )
            return new_func(*args, **kwarg)

        return wrapfunc

    return wrap_deprecated_func


def deprecated_attr(name_dict):
    """ name_dict: {'old_property_name': 'new_property_name'}"""
    def wrap_deprecated_attr(func):
        def wrapfunc(self, name):
            clzname = self.__class__.__name__
            if name in name_dict:
                if name_dict[name] is None:
                    # print('Removed warning: the {}.{} will be removed!'.format(clzname, name))
                    print('警告: "{}.{}" 接口已被删除!'.format(clzname, name))
                else:
                    print(
                        # 'Deprecated warning: the "{}.{}" will be deprecated, please use: "{}.{}"'.
                        '警告: "{}.{}" 即将被废弃，请使用 "{}.{}" 代替'.format(clzname, name, clzname, name_dict[name])
                    )
                return func(self, name_dict[name])
            if name not in dir(self):
                raise AttributeError("'{}' object has no attribute '{}'".format(clzname, name))
            return func(self, name)

        return wrapfunc

    return wrap_deprecated_attr


# --------------------------------------------------------------------
#
# 待废弃函数
#
# --------------------------------------------------------------------


@deprecated_func(get_version, 'getVersion', 'get_version')
def getVersion():
    pass


@deprecated_func(get_stock, 'getStock', 'get_stock')
def getStock(querystr):
    pass


@deprecated_func(get_business_name, 'getBusinessName', 'get_business_name')
def getBusinessName(querystr):
    pass


@deprecated_func(get_system_part_name, 'getSystemPartName', 'get_system_part_name')
def getSystemPartName(querystr):
    pass


@deprecated_func(get_system_part_enum, 'getSystemPartEnum', 'get_system_part_enum')
def getSystemPartEnum(querystr):
    pass


@deprecated_func(Query, 'KQuery', 'Query')
def KQuery(*args, **kwargs):
    pass


@deprecated_func(Query, 'QueryByIndex', 'Query')
def QueryByIndex(*args, **kwargs):
    pass


@deprecated_func(Query, 'QueryByDate', 'Query')
def QueryByDate(*args, **kwargs):
    pass


@deprecated_func(get_date_range, 'getDateRange', 'get_date_range')
def getDateRange(*args, **kwargs):
    pass


@deprecated_func(SP_FixedPercent, 'SL_FixedPercent', 'SP_FixedPercent')
def SL_FixedPercent(*args, **kwargs):
    pass


@deprecated_func(SP_FixedValue, 'SL_FixedValue', 'SP_FixedValue')
def SL_FixedValue(*args, **kwargs):
    pass


# --------------------------------------------------------------------
#
# 待废弃属性
#
# --------------------------------------------------------------------


@deprecated_attr(
    {
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        '_allocateWeight': '_allocate_weight',
    }
)
def AllocateFundsBase_getattr(self, name):
    return getattr(self, name)


AllocateFundsBase.__getattr__ = AllocateFundsBase_getattr


@deprecated_attr({'size': '__len__', 'get': '__getitem__'})
def Block_getattr(self, name):
    return getattr(self, name)


Block.__getattr__ = Block_getattr


@deprecated_attr(
    {
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'isValid': 'is_valid',
        '_add_valid': '_add_valid',
        'setTO': 'to',
        'getTO': 'to',
        'setTM': 'tm',
        'getTM': 'tm',
        'setSG': 'sg',
        'getSG': 'sg'
    }
)
def ConditionBase_getattr(self, name):
    return getattr(self, name)


ConditionBase.__getattr__ = ConditionBase_getattr


@deprecated_attr(
    {
        'isNull': 'is_null',
        'dayOfWeek': 'day_of_week',
        'dayOfYear': 'day_of_year',
        'startOfDay': 'start_of_day',
        'endOfDay': 'end_of_day',
        'nextDay': 'next_day',
        'nextWeek': 'next_week',
        'nextMonth': 'next_month',
        'nextQuarter': 'next_quarter',
        'nextHalfyear': 'next_halfyear',
        'nextYear': 'next_year',
        'preDay': 'pre_day',
        'preWeek': 'pre_week',
        'preMonth': 'pre_month',
        'preQuarter': 'pre_quarter',
        'preHalfyear': 'pre_halfyear',
        'preYear': 'pre_year',
        'dateOfWeek': 'date_of_week',
        'startOfWeek': 'start_of_week',
        'endOfWeek': 'end_of_week',
        'startOfMonth': 'start_of_month',
        'endOfMonth': 'end_of_month',
        'startOfQuarter': 'start_of_quarter',
        'endOfQuarter': 'end_of_quarter',
        'startOfHalfyear': 'start_of_halfyear',
        'endOfHalfyear': 'end_of_halfyear',
        'startOfYear': 'start_of_year',
        'endOfYear': 'end_of_year'
    }
)
def Datetime_getattr(self, name):
    return getattr(self, name)


Datetime.__getattr__ = Datetime_getattr


@deprecated_attr(
    {
        'setQuery': 'query',
        'getQuery': 'query',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'isValid': 'is_valid',
        '_addValid': '_add_valid'
    }
)
def EnvironmentBase_getattr(self, name):
    return getattr(self, name)


EnvironmentBase.__getattr__ = EnvironmentBase_getattr


@deprecated_attr(
    {
        'setDiscard': 'set_discard',
        'size': '__len__',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'getResultNumber': 'get_result_num',
        'getByDate': 'get_by_datetime',
        'getPos': 'get_pos',
        'getDatetime': 'get_datetime',
        'getResult': 'get_result',
        'getResultAsPriceList': 'get_result_as_price_list',
        'getDatetimeList': 'get_datetime_list',
        'getContext': 'get_context',
        'setContext': 'set_context'
    }
)
def Indicator_getattr(self, name):
    return getattr(self, name)


Indicator.__getattr__ = Indicator_getattr


@deprecated_attr(
    {
        'getParameter': 'get_parameter',
        'getParam': 'get_param',
        'setParam': "set_param",
        'setDiscard': 'set_discard',
        '_readyBuffer': '_ready_buffer',
        'getResultNumber': 'get_result_num',
        'getResultAsPriceList': 'get_result_as_price_list',
        'isNeedContext': 'is_need_context'
    }
)
def IndicatorImp_getattr(self, name):
    return getattr(self, name)


IndicatorImp.__getattr__ = IndicatorImp_getattr


@deprecated_attr(
    {
        'startPos': 'start_pos',
        'endPos': 'end_pos',
        'lastPos': 'last_pos',
        'getPos': 'get_pos',
        'getDatetimeList': 'get_datetime_list',
        'getByDatetime': 'get_by_datetime',
        'getQuery': 'get_query',
        'getStock': 'get_stock',
    }
)
def KData_getattr(self, name):
    return getattr(self, name)


KData.__getattr__ = KData_getattr


@deprecated_attr(
    {
        'openPrice': 'open',
        'closePrice': 'close',
        'highPrice': 'high',
        'lowPrice': 'low',
        'transAmount': 'amount',
        'transCount': 'volume'
    }
)
def KRecord_getattr(self, name):
    return getattr(self, name)


KRecord.__getattr__ = KRecord_getattr


@deprecated_attr({'lastDate': 'last_datetime'})
def MarketInfo_getattr(self, name):
    return getattr(self, name)


MarketInfo.__getattr__ = MarketInfo_getattr


@deprecated_attr(
    {
        'setTM': 'tm',
        'getTM': 'tm',
        'setQuery': 'query',
        'getQuery': 'query',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'buyNotify': 'buy_notify',
        'sellNotify': 'sell_notify',
        'getBuyNumber': 'get_buy_num',
        'getSellNumber': 'get_sell_num',
        '_getBuyNumber': '_get_buy_num',
        '_getSellNumber': '_get_sell_num',
    }
)
def MoneyManagerBase_getattr(self, name):
    return getattr(self, name)


MoneyManagerBase.__getattr__ = MoneyManagerBase_getattr


@deprecated_attr(
    {
        'set': '__setitem__',
        'get': '__getitem__',
        'have': '__contains__',
        'getNameList': 'get_name_list',
        'getNameValueList': None
    }
)
def Parameter_getattr(self, name):
    return getattr(self, name)


Parameter.__getattr__ = Parameter_getattr


@deprecated_attr({'get': '__getitem__'})
def Performance_getattr(self, name):
    return getattr(self, name)


Performance.__getattr__ = Performance_getattr


@deprecated_attr({'getParam': 'get_param', 'setParam': 'set_param', 'haveParam': 'have_param'})
def Portfolio_getattr(self, name):
    return getattr(self, name)


Portfolio.__getattr__ = Portfolio_getattr


@deprecated_attr(
    {
        'takeDatetime': 'take_datetime',
        'cleanDatetime': 'clean_datetime',
        'goalPrice': 'goal_price',
        'totalNumber': 'total_number',
        'buyMoney': 'buy_money',
        'totalCost': 'total_cost',
        'totalRisk': 'total_risk',
        'sellMoney': 'sell_money'
    }
)
def PositionRecord_getattr(self, name):
    return getattr(self, name)


PositionRecord.__getattr__ = PositionRecord_getattr


@deprecated_attr(
    {
        'getTM': 'tm',
        'setTM': 'tm',
        'getTO': 'to',
        'setTO': 'to',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'buyNotify': 'buy_notify',
        'sellNotify': 'sell_notify',
        'getGoal': 'get_goal'
    }
)
def ProfitGoalBase_getattr(self, name):
    return getattr(self, name)


ProfitGoalBase.__getattr__ = ProfitGoalBase_getattr


@deprecated_attr(
    {
        'startDatetime': 'start_datetime',
        'endDatetime': 'end_datetime',
        'queryType': 'query_type',
        'kType': 'ktype',
        'recoverType': 'recover_type'
    }
)
def Query_getattr(self, name):
    return getattr(self, name)


Query.__getattr__ = Query_getattr


@deprecated_attr(
    {
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'getSelectedSystemList': 'get_selected_system_list',
        'addStock': 'add_stock',
        'addStockList': 'add_stock_list',
    }
)
def SelectorBase_getattr(self, name):
    return getattr(self, name)


@deprecated_attr(
    {
        'getTO': 'to',
        'setTO': 'to',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'shouldBuy': 'should_buy',
        'shouldSell': 'should_sell',
        'getBuySignal': 'get_buy_signal',
        'getSellSignal': 'get_sell_signal',
        '_addBuySignal': '_add_buy_signal',
        '_addSellSignal': '_add_sell_signal'
    }
)
def SignalBase_getattr(self, name):
    return getattr(self, name)


SignalBase.__getattr__ = SignalBase_getattr


@deprecated_attr(
    {
        'getTO': 'to',
        'setTO': 'to',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'getRealBuyPrice': 'get_real_buy_price',
        'getRealSellPrice': 'get_real_sell_price'
    }
)
def SlippageBase_getattr(self, name):
    return getattr(self, name)


SlippageBase.__getattr__ = SlippageBase_getattr


@deprecated_attr(
    {
        'startDatetime': 'start_datetime',
        'lastDatetime': 'last_datetime',
        'tickValue': 'tick_value',
        'minTradeNumber': 'min_trade_number',
        'maxTradeNumber': 'max_trade_number',
        'isNull': 'is_null',
        'isBuffer': 'is_buffer',
        'getKData': 'get_kdata',
        'getTimeLineList': 'get_timeline_list',
        'getTransList': 'get_trans_list',
        'getCount': 'get_count',
        'getMarketValue': 'get_market_value',
        'getKRecord': 'get_krecord',
        'getKRecordByDate': 'get_krecord',
        'getKRecordList': 'get_krecord_list',
        'getDatetimeList': 'get_datetime_list',
        'getFinanceInfo': 'get_finance_info',
        'realtimeUpdate': 'realtime_update',
        'getWeight': 'get_weight',
        'loadKDataToBuffer': 'load_kdata_to_buffer',
        'releaseKDataBuffer': 'release_kdata_buffer'
    }
)
def Stock_getattr(self, name):
    return getattr(self, name)


Stock.__getattr__ = Stock_getattr


@deprecated_attr(
    {
        'size': '__len__',
        'setKDataDriver': None,
        'getBaseInfoDriverParameter': 'get_base_info_parameter',
        'getBlockDriverParameter': 'get_block_parameter',
        'getKDataDriverParameter': 'get_kdata_parameter',
        'getPreloadParameter': 'get_preload_parameter',
        'getHikyuuParameter': 'get_hikyuu_parameter',
        'getAllMarket': "get_market_list",
        'getMarketInfo': 'get_market_info',
        'getStockTypeInfo': 'get_stock_type_info',
        'getStock': 'get_stock',
        'getBlock': 'get_block',
        'getBlockList': 'get_block_list',
        'getTradingCalendar': 'get_trading_calendar',
        'addTempCsvStock': 'add_temp_csv_stock',
        'removeTempCsvStock': 'remove_temp_csv_stock'
    }
)
def StockManager_getattr(self, name):
    return getattr(self, name)


StockManager.__getattr__ = StockManager_getattr


@deprecated_attr({
    'tickValue': 'tick_value',
    'minTradeNumber': 'min_trade_num',
    'maxTradeNumber': 'max_trade_num',
})
def StockTypeInfo_getattr(self, name):
    return getattr(self, name)


StockTypeInfo.__getattr__ = StockTypeInfo_getattr


@deprecated_attr(
    {
        'countAsGift': 'count_as_gift',
        'countForSell': 'count_for_sell',
        'priceForSell': 'price_for_sell',
        'totalCount': 'total_count',
        'freeCount': 'free_count',
    }
)
def StockWeight_getattr(self, name):
    return getattr(self, name)


StockWeight.__getattr__ = StockWeight_getattr


@deprecated_attr(
    {
        'getTO': 'to',
        'setTO': 'to',
        'getTM': 'tm',
        'setTM': 'tm',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'getPrice': 'get_price'
    }
)
def StoplossBase_getattr(self, name):
    return getattr(self, name)


StoplossBase.__getattr__ = StoplossBase_getattr


@deprecated_attr(
    {
        'getTO': 'to',
        'setTO': 'to',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'getStock': 'get_stock',
        'getTradeRecordList': 'get_trade_record_list',
        'getBuyTradeRequest': 'get_buy_trade_request',
        'getSellTradeRequest': 'get_sell_trade_request'
    }
)
def System_getattr(self, name):
    return getattr(self, name)


System.__getattr__ = System_getattr


@deprecated_attr(
    {
        'getParam': 'get_param',
        'setParam': 'set_param',
        'getBuyCost': 'get_buy_cost',
        'getSellCost': 'get_sell_cost',
    }
)
def TradeCostBase_getattr(self, name):
    return getattr(self, name)


TradeCostBase.__getattr__ = TradeCostBase_getattr


@deprecated_attr(
    {
        'initCash': 'init_cash',
        'currentCash': 'current_cash',
        'initDatetime': 'init_datetime',
        'firstDatetime': 'first_datetime',
        'lastDatetime': 'last_datetime',
        'costFunc': 'cost_func',
        'brokeLastDatetime': 'broker_last_datetime',
        'getParam': 'get_param',
        'setParam': 'set_param',
        'haveParam': 'have_param',
        'regBroker': 'reg_broker',
        'clearBroker': 'clear_broker',
        'getStockNumber': 'get_stock_num',
        'getHoldNumber': 'get_hold_num',
        'getTradeList': 'get_trade_list',
        'getPositionList': 'get_position_list',
        'getHistoryPositionList': 'get_history_position_list',
        'getPosition': 'get_position',
        'getBuyCost': 'get_buy_cost',
        'getSellCost': 'get_sell_cost',
        'getFunds': 'get_funds',
        'getFundsCurve': 'get_funds_curve',
        'getProfitCurve': 'get_profit_curve',
        'addTradeRecord': 'add_trade_record'
    }
)
def TradeManager_getattr(self, name):
    return getattr(self, name)


TradeManager.__getattr__ = TradeManager_getattr


@deprecated_attr({'planPrice': 'plan_price', 'realPrice': 'real_price', 'goalPrice': 'goal_price'})
def TradeRecord_getattr(self, name):
    return getattr(self, name)


TradeRecord.__getattr__ = TradeRecord_getattr
