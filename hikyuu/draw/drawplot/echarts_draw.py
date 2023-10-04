from functools import wraps

from pyecharts import options as opts
from pyecharts.charts import Kline, Scatter, Line, Bar, Grid
from pyecharts.charts.base import Base as ChartBase
from pyecharts.commons.utils import JsCode

from hikyuu.cpp.core import KData, System
from .common import get_draw_title
from hikyuu import *

# 控制k线图和volume图的位置
grid_pos = [
    opts.GridOpts(pos_left="5%", pos_right="1%", height="57%"),
    opts.GridOpts(pos_left="5%", pos_right="1%", pos_top="73%", height="10%")
]


def _get_js_data_var(kdata):
    datas = [[r.open, r.close, r.low, r.high, r.volume] for r in kdata]
    return datas


def _brush_chart(kdata, chart):
    datetimes_str = [str(dt) for dt in kdata.get_datetime_list()]
    brush_range = (datetimes_str[len(datetimes_str) // 2], datetimes_str[len(datetimes_str) * 3 // 4])
    if hasattr(chart, 'add_dispatch_actions'):  # PR https://github.com/pyecharts/pyecharts/pull/2244 not yet merged
        chart.add_dispatch_actions(
            """{
        type: 'brush',
        areas: [
        {
            brushType: 'lineX',
            coordRange: ['%s', '%s'],
            xAxisIndex: 0
        }
        ]
    }""" % (brush_range)  # use Python 2 string formating to avoid double curly braces
        )


def _chart_html_wrap(func):
    @wraps(func)
    def _func(*args, **kwargs):
        chart: ChartBase = func(*args, **kwargs)

        def _repr_html():
            return chart.render_notebook().__html__()

        def _html():
            return _repr_html()
        chart.__html__ = _html
        chart._repr_html_ = _repr_html
        return chart
    return _func


def get_series_name(kdata):
    stock = kdata.get_stock()
    if stock.is_null():
        return ""
    if stock.code == "":
        stitle = "Block(%s)" % (stock.id)
    else:
        stitle = stock.market + "/" + stock.code
    return stitle


def kplot_line(kdata: KData):
    datetimes = kdata.get_datetime_list()
    datetimes = [str(dt) for dt in datetimes]
    # order in OCLH
    kdata_points = [(r.open, r.close, r.low, r.high) for r in kdata]
    title = get_draw_title(kdata)
    last_record = kdata[-1]
    text = u'%s 开:%.2f 高:%.2f 低:%.2f 收:%.2f 涨幅:%.2f%%' % (
        last_record.datetime.number / 10000, last_record.open, last_record.high, last_record.low,
        last_record.close, 100 * (last_record.close - kdata[-2].close) / kdata[-2].close
    )

    kline = (
        Kline()
        .add_xaxis(xaxis_data=datetimes)
        .add_yaxis(
            series_name=get_series_name(kdata),
            y_axis=kdata_points,
            itemstyle_opts=opts.ItemStyleOpts(color="#ec0000", color0="#00da3c", border_color=None, border_color0=None),
        )
        .set_global_opts(
            legend_opts=opts.LegendOpts(
                is_show=True, pos_bottom=10, pos_left="center", border_width=0,
            ),
            datazoom_opts=[
                opts.DataZoomOpts(
                    is_show=True,
                    xaxis_index=[0, 1],  # 同时控制k线图和volume图的x轴缩放
                    type_="slider",
                    pos_top="85%",
                    range_start=0,
                    range_end=100,
                ),
            ],
            xaxis_opts=opts.AxisOpts(
                splitline_opts=opts.SplitLineOpts(is_show=False)
            ),
            yaxis_opts=opts.AxisOpts(
                is_scale=True,
                splitarea_opts=opts.SplitAreaOpts(
                    is_show=True, areastyle_opts=opts.AreaStyleOpts(opacity=1)
                ),
            ),
            tooltip_opts=opts.TooltipOpts(
                trigger="axis",
                axis_pointer_type="cross",
                background_color="rgba(245, 245, 245, 0.8)",
                border_width=1,
                border_color="#ccc",
                textstyle_opts=opts.TextStyleOpts(color="#000"),
                padding=10,
            ),
            visualmap_opts=opts.VisualMapOpts(
                is_show=False,
                dimension=2,
                series_index=5,
                is_piecewise=True,
                pieces=[
                    {"value": 1, "color": "#00da3c"},
                    {"value": -1, "color": "#ec0000"},
                ],
            ),
            axispointer_opts=opts.AxisPointerOpts(
                is_show=True,
                link=[{"xAxisIndex": "all"}],
                label=opts.LabelOpts(background_color="#777"),
            ),
            brush_opts=opts.BrushOpts(
                x_axis_index="all",
                brush_link="all",
                out_of_brush={"colorAlpha": 0.1},  # brush之外的candle会显示为淡色
            ),
            toolbox_opts=opts.ToolboxOpts(
                feature=opts.ToolBoxFeatureOpts(
                    save_as_image=opts.ToolBoxFeatureSaveAsImageOpts(is_show=True),  # 保存为图片
                    data_view=opts.ToolBoxFeatureDataViewOpts(is_show=False),  # 数据视图
                    magic_type=opts.ToolBoxFeatureMagicTypeOpts(is_show=False),  # 动态类型切换
                    data_zoom=opts.ToolBoxFeatureDataZoomOpts(yaxis_index=False),  # 区域缩放
                    brush=opts.ToolBoxFeatureBrushOpts(type_=["lineX"]),  # 选择组件, type_目前没有效果
                )
            ),
            title_opts=opts.TitleOpts(title=title),
            graphic_opts=opts.GraphicText(
                graphic_item=opts.GraphicItem(
                    right="10%",
                    top="22%"
                ),
                graphic_textstyle_opts=opts.GraphicTextStyleOpts(
                    text=text
                )
            )
        )
    )
    return kline


def volume_barplot(kdata: KData):
    datetimes = kdata.get_datetime_list()
    datetimes = [str(dt) for dt in datetimes]
    volumes = [r.volume for r in kdata]
    bar_vol = (
        Bar()
        .add_xaxis(xaxis_data=datetimes)
        .add_yaxis(
            series_name="Volume",
            y_axis=volumes,
            xaxis_index=1,
            yaxis_index=1,
            label_opts=opts.LabelOpts(is_show=False),
            itemstyle_opts=opts.ItemStyleOpts(
                # 处理volume bar的颜色，上涨为红色，下跌为绿色
                color=JsCode(
                    """
                function(params) {
                    var colorList;
                    if (barData[params.dataIndex][1] > barData[params.dataIndex][0]) {
                        colorList = '#ef232a';
                    } else {
                        colorList = '#14b143';
                    }
                    return colorList;
                }
                """
                )
            ),
        )
        .set_global_opts(
            xaxis_opts=opts.AxisOpts(
                type_="category",
                grid_index=1,
                axislabel_opts=opts.LabelOpts(is_show=False),
                axistick_opts=opts.AxisTickOpts(is_show=False),
                splitline_opts=opts.SplitLineOpts(is_show=False),
            ),
            yaxis_opts=opts.AxisOpts(
                axislabel_opts=opts.LabelOpts(is_show=False),
                axisline_opts=opts.AxisLineOpts(is_show=False),
                splitline_opts=opts.SplitLineOpts(is_show=False)
            ),
            legend_opts=opts.LegendOpts(is_show=False)
        )
    )
    return bar_vol


@_chart_html_wrap
def iplot(indicator, kref: KData = None, chart=None):
    datetimes = indicator.get_datetime_list()
    py_indicator = [None if x == constant.null_price else x for x in indicator]
    datetimes = [str(dt) for dt in datetimes]
    if not datetimes:
        datetimes = list(range(len(indicator)))
    if kref:
        datetimes = [str(dt) for dt in kref.get_datetime_list()]
    name = "%s %.2f" % (indicator.long_name, indicator[-1])
    line = (
        Line()
        .add_xaxis(xaxis_data=datetimes)
        .add_yaxis(
            series_name=name,
            y_axis=py_indicator,
            label_opts=opts.LabelOpts(is_show=False),
            is_symbol_show=False,
            linestyle_opts=opts.LineStyleOpts(width=2),
        )
    )
    if chart:
        return chart.overlap(line)
    else:
        line.set_global_opts(
            yaxis_opts=opts.AxisOpts(
                is_scale=True,
                splitarea_opts=opts.SplitAreaOpts(
                    is_show=True, areastyle_opts=opts.AreaStyleOpts(opacity=1)
                ),
            ),
            tooltip_opts=opts.TooltipOpts(
                trigger="axis",
                axis_pointer_type="cross",
                background_color="rgba(245, 245, 245, 0.8)",
                border_width=1,
                border_color="#ccc",
                textstyle_opts=opts.TextStyleOpts(color="#000"),
            ),
        )
    return line


@_chart_html_wrap
def ibar(indicator, kref: KData = None, chart=None):
    datetimes = indicator.get_datetime_list()
    py_indicator = [None if x == constant.null_price else x for x in indicator]
    datetimes = [str(dt) for dt in datetimes]
    if not datetimes:
        datetimes = list(range(len(indicator)))
    if kref:
        datetimes = [str(dt) for dt in kref.get_datetime_list()]
    name = "%s %.2f" % (indicator.long_name, indicator[-1])
    bar = (
        Bar()
        .add_xaxis(xaxis_data=datetimes)
        .add_yaxis(
            series_name=name,
            y_axis=py_indicator,
            label_opts=opts.LabelOpts(is_show=False)
        )
    )
    if chart:
        return chart.overlap(bar)
    else:
        bar.set_global_opts(
            yaxis_opts=opts.AxisOpts(
                is_scale=True,
                splitarea_opts=opts.SplitAreaOpts(
                    is_show=True, areastyle_opts=opts.AreaStyleOpts(opacity=1)
                ),
            ),
        )
    return bar


@_chart_html_wrap
def kplot(kdata: KData):
    kline = kplot_line(kdata)
    vol_bar = volume_barplot(kdata)
    grid_chart = Grid()
    grid_chart.add_js_funcs("var barData = {}".format(_get_js_data_var(kdata)))
    grid_chart.add(
        kline,
        grid_opts=grid_pos[0],
    )
    grid_chart.add(
        vol_bar,
        grid_opts=grid_pos[1],
    )
    _brush_chart(kdata, grid_chart)
    return grid_chart


@_chart_html_wrap
def sysplot(sys: System):
    kdata = sys.to
    datetimes = kdata.get_datetime_list()
    date_index = dict([(d, i) for i, d in enumerate(datetimes)])

    kline = kplot_line(kdata)

    tds = sys.tm.get_trade_list()
    buy_dates = []
    sell_dates = []
    for t in tds:
        if t.business in (BUSINESS.BUY, BUSINESS.BUY_SHORT):
            buy_dates.append(t.datetime)
        elif t.business in (BUSINESS.SELL, BUSINESS.SELL_SHORT):
            sell_dates.append(t.datetime)
        else:
            pass

    buy_pos = []
    sell_pos = []
    closep = [d.close for d in kdata]
    candle_gap = (max(closep) - min(closep)) * 0.05

    for d in buy_dates:
        if d not in date_index:
            continue
        buy_pos.append(kdata[date_index[d]].low - candle_gap)

    for d in sell_dates:
        if d not in date_index:
            continue
        sell_pos.append(kdata[date_index[d]].high + candle_gap)

    buy_dates = [str(dt) for dt in buy_dates]
    sell_dates = [str(dt) for dt in sell_dates]

    buy_scatter = (
        Scatter()
        .add_xaxis(buy_dates)
        .add_yaxis(
            series_name="buys",
            y_axis=buy_pos,
            label_opts=opts.LabelOpts(
                is_show=True,
                formatter="B",
                distance=5,
                position="bottom",
                color="#ec0000"
            ),
            symbol="arrow",
            symbol_size=[5, 10],
            color="#ec0000",
            symbol_rotate=0,
        )
    )
    sell_scatter = (
        Scatter()
        .add_xaxis(sell_dates)
        .add_yaxis(
            series_name="sells",
            y_axis=sell_pos,
            label_opts=opts.LabelOpts(
                is_show=True,
                formatter="S",
                distance=5,
                position="top",
                color="#00da3c"
            ),
            symbol="arrow",
            symbol_size=[5, 10],
            color="#00da3c",
            symbol_rotate=180,
        )
    )

    vol_bar = volume_barplot(kdata)
    kline = (
        kline
        .overlap(buy_scatter)
        .overlap(sell_scatter)
    )

    grid_chart = Grid()
    grid_chart.add_js_funcs("var barData = {}".format(_get_js_data_var(kdata)))
    grid_chart.add(
        kline,
        grid_opts=grid_pos[0],
    )
    grid_chart.add(
        vol_bar,
        grid_opts=grid_pos[1],
    )
    _brush_chart(kdata, grid_chart)
    return grid_chart
