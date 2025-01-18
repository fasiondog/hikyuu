from functools import wraps

from pyecharts import options as opts
from pyecharts.charts import Kline, Scatter, Line, Bar, Grid, Page
from pyecharts.charts.base import Base as ChartBase
from pyecharts.commons.utils import JsCode

from hikyuu.core import KData, System
from .common import get_draw_title
from hikyuu import *


class MultiLineTextChart(ChartBase):
    def __init__(self, init_opts: opts.InitOpts = opts.InitOpts()):
        super().__init__(init_opts=init_opts)
        self._last_x = 50  # 默认起始 x 坐标
        self._last_y = 5  # 默认起始 y 坐标
        self._line_height = 20  # 默认行高

    def reset_position(self, x=None, y=None):
        if x is not None:
            self._last_x = x
        if y is not None:
            self._last_y = y

    def add(self, text: str, x = None, y = None, text_style = None):
        """
        添加多行文本

        :param text: 要显示的文本内容
        :param x: 文本区域的左上角 x 坐标
        :param y: 文本区域的左上角 y 坐标
        :param text_style: 文本样式，如字体大小、颜色等
        """
        if text_style is None:
            text_style = {"fontSize": 14, "color": "#333", "fontFamily": "Arial"}

        graphic_elements = self.options.get('graphic', [])

        if x is None:
            x = self._last_x
        if y is None:
            y = self._last_y + self._line_height

        # 默认左右边距各50
        width = int(self.width.strip('px')) - 100
        if x is not None:
            width -= x
        lines = self._split_text(text, width, text_style.get("font_size", 14))

        for i, line in enumerate(lines):
            graphic_elements.append(
                opts.GraphicText(
                    graphic_item=opts.GraphicItem(
                        position=[x, y + i * (text_style.get("font_size", 14) + 5)],
                        z=100,
                    ),
                    graphic_textstyle_opts=opts.GraphicTextStyleOpts(
                        text=line,
                        font=text_style.get("fontFamily", "sans-serif"),
                        font_size=text_style.get("font_size", 14),
                        graphic_basicstyle_opts=opts.GraphicBasicStyleOpts(
                        fill=text_style.get("color", "#000"))
                    )
                )
            )
            self._last_x = x
            self._last_y = y + i * (text_style.get("font_size", 14) + 5)

        self.options.update(graphic=graphic_elements)


    def _split_text(self, text: str, width: int, font_size: int):
        """
        根据宽度和字体大小分割文本为多行

        :param text: 要分割的文本
        :param width: 文本区域的宽度
        :param font_size: 字体大小
        :return: 分割后的文本行列表
        """
        # 假设每个字符的宽度大致为字体大小的一半
        max_chars_per_line = int(width / (font_size / 2))
        lines = []
        current_line = ""
        for word in text.split():
            if len(current_line) + len(word) + 1 <= max_chars_per_line:
                current_line += " " + word if current_line else word
            else:
                if current_line:
                    lines.append(current_line)
                current_line = word
        if current_line:
            lines.append(current_line)
        return lines


# 控制k线图和volume图的位置
grid_pos = [
    opts.GridOpts(pos_left="5%", pos_right="1%", height="57%"),
    opts.GridOpts(pos_left="5%", pos_right="1%", pos_top="73%", height="10%")
]


def _get_js_data_var(kdata):
    datas = [[round(r.open, 2), round(r.close, 2), round(r.low, 2), round(r.high, 2), round(r.volume, 2)] for r in kdata]
    return datas


def _brush_chart(kdata, chart):
    datetimes_str = simple_datetime_list(kdata.get_datetime_list(), kdata.get_query().ktype)
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


def simple_datetime_list(datetimes: list, ktype:str=None):
    if ktype is None:
        return [
            sd.split(' ')[0] if sd.endswith('00:00:00') else
            sd.rsplit(':', 1)[0] if sd.endswith(':00') else
            sd
            for sd in (str(dt) for dt in datetimes)
        ]
    if ktype.startswith('MIN') or ktype.startswith('HOUR'):
        datetimes = [str(dt).rsplit(':', 1)[0] for dt in datetimes]
    else:
        datetimes = [str(dt).split(' ')[0] for dt in datetimes]
    return datetimes


def kplot_line(kdata: KData):
    datetimes = simple_datetime_list(kdata.get_datetime_list(), kdata.get_query().ktype)
    # order in OCLH
    kdata_points = [(round(r.open, 2), round(r.close, 2), round(r.low, 2), round(r.high, 2)) for r in kdata]
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
                    is_show=len(datetimes) > 200,
                    xaxis_index=[0, 1],  # 同时控制k线图和volume图的x轴缩放
                    type_="slider",
                    pos_top="85%",
                    range_start=(100 - 200 * 100 / len(datetimes)) if len(datetimes) > 200 else 0,
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
    datetimes = simple_datetime_list(kdata.get_datetime_list(), kdata.get_query().ktype)
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
def iplot(indicator, kref: KData = None, chart=None, ilongname=False):
    datetimes = indicator.get_datetime_list()
    py_indicator = [None if x == constant.null_price else round(x, 2) for x in indicator]
    datetimes = simple_datetime_list(datetimes)
    if not datetimes:
        datetimes = list(range(len(indicator)))
    if kref:
        datetimes = simple_datetime_list(kref.get_datetime_list(), kref.get_query().ktype)
    name = indicator.name
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
            title_opts=opts.TitleOpts(title="%s %.2f" % (indicator.long_name if ilongname else indicator.name, indicator[-1])),
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
            datazoom_opts=[
                opts.DataZoomOpts(
                    is_show=len(datetimes) > 200,
                    xaxis_index=0,
                    type_="slider",
                    pos_top="92%",
                    range_start=(100 - 200 * 100 / len(datetimes)) if len(datetimes) > 200 else 0,
                    range_end=100,
                )
            ],
        )
    return line


@_chart_html_wrap
def ibar(indicator, kref: KData = None, chart=None):
    datetimes = indicator.get_datetime_list()
    py_indicator = [None if x == constant.null_price else round(x, 2) for x in indicator]
    datetimes = simple_datetime_list(datetimes)
    if not datetimes:
        datetimes = list(range(len(indicator)))
    if kref:
        datetimes = simple_datetime_list(kref.get_datetime_list(), kref.get_query().ktype)
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
def kplot(kdata: KData, ind_main=None, ind_sub=None):
    kline = kplot_line(kdata)
    if ind_main is not None:
        if isinstance(ind_main, Indicator):
            ind_main = [ind_main]
        for ind in ind_main:
            kline = iplot(ind, kdata, kline)
    vol_bar = volume_barplot(kdata)
    if ind_sub is not None:
        if isinstance(ind_sub, Indicator):
            ind_sub = [ind_sub]
        for ind in ind_sub:
            vol_bar = iplot(ind, kdata, vol_bar)
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
    grid_chart.g1rootchart = kline
    grid_chart.g2rootchart = vol_bar
    _brush_chart(kdata, grid_chart)
    return grid_chart


@_chart_html_wrap
def sysplot(sys: System):
    kdata = sys.to
    datetimes = simple_datetime_list(kdata.get_datetime_list(), kdata.get_query().ktype)
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
    buy_dates = simple_datetime_list(buy_dates, kdata.get_query().ktype)
    sell_dates = simple_datetime_list(sell_dates, kdata.get_query().ktype)

    buy_pos = []
    sell_pos = []
    closep = [round(d.close, 2) for d in kdata]
    candle_gap = (max(closep) - min(closep)) * 0.05

    for d in buy_dates:
        if d not in date_index:
            continue
        buy_pos.append(kdata[date_index[d]].low - candle_gap)

    for d in sell_dates:
        if d not in date_index:
            continue
        sell_pos.append(kdata[date_index[d]].high + candle_gap)

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


def sys_performance(sys, ref_stk=None):
    if ref_stk is None:
        ref_stk = get_stock('sh000300')

    query = sys.query
    sh000001_k = get_kdata('sh000001', query)
    ref_dates = sh000001_k.get_datetime_list()

    ref_k = ref_stk.get_kdata(query)

    funds_list = sys.tm.get_funds_list(ref_dates)
    funds = [f.total_assets for f in funds_list]
    funds = VALUE(funds)
    funds_return = [f.total_assets / f.total_base if f.total_base != 0.0 else constant.null_price for f in funds_list]
    funds_return = VALUE(funds_return, align_dates=ref_dates)
    funds_return.name = "系统累积收益率"
    ref_return = ALIGN(ROCR(ref_k.close, 0), ref_dates)
    ref_return.name = f"{ref_stk.name}({ref_stk.market_code})"

    per = Performance()
    text = per.report(sys.tm, sh000001_k[-1].datetime)

    # 计算最大回撤
    max_pullback = min(MDD(funds).to_np())

    # 计算 sharp
    bond = ZHBOND10(ref_dates)
    sigma = STDEV(ROCP(funds), len(ref_dates))
    sigma = 15.874507866387544 * sigma[-1]  # 15.874 = sqrt(252)
    sharp = (per['帐户平均年收益率%'] - bond[-1]) * 0.01 / sigma if sigma != 0.0 else 0.0

    invest_total = per['累计投入本金'] + per['累计投入资产']
    cur_fund = per['当前总资产']
    t1 = '投入总资产: {:<.2f}    当前总资产: {:<.2f}    当前盈利: {:<.2f}'.format(
        invest_total, cur_fund, cur_fund - invest_total)
    t2 = '当前策略收益: {:<.2f}%    年化收益率: {:<.2f}%    最大回撤: {:<.2f}%'.format(
        funds_return[-1]*100 - 100, per["帐户平均年收益率%"], max_pullback)
    t3 = '系统胜率: {:<.2f}%    盈/亏比: 1 : {:<.2f}    夏普比率: {:<.2f}'.format(
        per['赢利交易比例%'], per['净赢利/亏损比例'], sharp)

    line = iplot(ref_return, ref_k)
    line = iplot(funds_return, ref_k, line)
    line.get_options().update(dataZoom=[
        opts.DataZoomOpts(
            is_show=len(ref_dates) > 500,
            xaxis_index=0,
            type_="slider",
            pos_top="92%",
            range_start=(100 - 500 * 100 / len(ref_dates)) if len(ref_dates) > 500 else 0,
            range_end=100,
        ),
    ])

    page = Page()
    page.add(line)

    tchart = MultiLineTextChart(opts.InitOpts(height='65px'))
    tchart.add(t1, y = 5)
    tchart.add(t2)
    tchart.add(t3)
    page.add(tchart)

    txchart = MultiLineTextChart(opts.InitOpts(height='550px'))
    tlines = text.splitlines()
    tlcount = len(tlines)
    for i in range(tlcount//2):
        txchart.add(tlines[i])
    txchart.reset_position(y=5)
    for i in range(tlcount//2, tlcount):
        txchart.add(tlines[i], x=400)
    page.add(txchart)
    return page
