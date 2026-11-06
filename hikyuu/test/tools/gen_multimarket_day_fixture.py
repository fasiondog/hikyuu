# coding: utf-8
"""多市场代表指数日线 fixture 生成器

用途：为多市场单测准备 test_data/{market}_day.h5（hdf5 后端、只做日线）。
落盘格式与 hikyuu/data/tdx_to_h5.py + common_h5.py 完全一致：
  - 数据集名 = MARKET + code 原样拼接（HDF5 大小写敏感，C++ 侧按 market + code 直取）；
  - datetime = 交易日 yyyymmdd0000（日线为纯日期标签，不做时区换算，见设计文档 G1）；
  - 四价 = 真实价 x 1000 的整数（h5 驱动读侧固定 x0.001，与 precision 无关；故 h5 只能存 3 位小数）；
  - transAmount 为近似值（close x volume），非真实成交额：未乘合约乘数/每手股数，
    指数类的 volume 为成分股合计、与点位相乘无经济含义；单测不断言其绝对值。
    transCount 日线直接存 API 值（手）。

尽力而为：目标窗口取不到时顺着候选源链尝试，仍不覆盖则退到"该源实际可得起点 + 3 年"，
并在 manifest 里登记偏差；彻底取不到数的市场标记为未覆盖（真实取数属 W3-a 导入程序职责）。

用法：
    python hikyuu/test/tools/gen_multimarket_day_fixture.py              # 优先用 CSV 缓存，缺失才取数
    python hikyuu/test/tools/gen_multimarket_day_fixture.py --refresh    # 强制重新取数
    python hikyuu/test/tools/gen_multimarket_day_fixture.py --markets SHF,HK

CSV 缓存目录 test_data/downloads/multi_market/ 不入库；manifest（取数审计记录）与生成的 .h5 一并入库。
"""

import argparse
import csv
import datetime
import os
import sys

import tables as tb

CACHE_DIR = os.path.join("test_data", "downloads", "multi_market")
MANIFEST = os.path.join("test_data", "multi_market_manifest.csv")  # 入库，审计偏差与未覆盖原因
SANE_MIN = datetime.date(1970, 1, 1)
FALLBACK_YEARS = 3

# 目标窗口与 A 股 fixture 对齐（sh_day.h5 止于 2011-12-06）；2011 后才有的品种从真实上市日起。
SPEC = [
    # market, code, precision, start, end, 源候选链
    ("SHF", "rb0000", 2, "20090327", "20111206", [("fut", "RB0"), ("fut_zh", "RB0")]),
    ("CZC", "SR0000", 2, "20060106", "20111206", [("fut", "SR0"), ("fut_zh", "SR0")]),
    ("CFE", "IF0000", 2, "20100416", "20111206", [("fut_zh", "IF0"), ("fut", "IF0")]),
    ("DCE", "i0000", 2, "20131018", "20161017", [("fut_zh", "I0")]),
    ("INE", "sc0000", 2, "20180326", "20210325", [("fut_zh", "SC0")]),
    ("GFE", "si0000", 2, "20221220", "20241219", [("fut_zh", "SI0")]),
    ("HK", "HSI", 2, "20050104", "20111206", [("hk_idx", "HSI")]),
    # 代表指数必须与 Market.code 身份一致：.IXIC（综合指数）历史更长但不可冒名替代 NDX
    ("NASDAQ", "NDX", 2, "20050104", "20111206", [("us_idx", ".NDX")]),
    ("NYSE", "DJI", 2, "20050104", "20111206", [("us_idx", ".DJI")]),
    ("FX", "EURUSD", 5, "20050104", "20111206",
     [("fx", "EURUSD"), ("fx_boc", "欧元"), ("ccy_ts", "EUR")]),
    ("BINANCE", "BTCUSDT", 6, "20170817", "20200816", [("crypto", "BTCUSD")]),
]

CN_COLS = {"日期": "date", "开盘价": "open", "最高价": "high", "最低价": "low",
           "收盘价": "close", "现价": "close", "成交量": "volume"}


class KDayRecord(tb.IsDescription):
    """与 common_h5.H5Record 同构（C++ 侧按字段名取值，顺序无关）"""
    datetime = tb.UInt64Col()
    openPrice = tb.UInt32Col()
    highPrice = tb.UInt32Col()
    lowPrice = tb.UInt32Col()
    closePrice = tb.UInt32Col()
    transAmount = tb.UInt64Col()
    transCount = tb.UInt64Col()


def to_num(v):
    s = str(v).replace(",", "").strip()
    if s in ("", "nan", "None", "-"):
        return 0.0
    return float(s)


def parse_date(v):
    if v is None:
        return None
    s = str(v).strip().replace("/", "-")[:10]
    fmt = "%Y-%m-%d" if "-" in s else "%Y%m%d"
    try:
        d = datetime.datetime.strptime(s, fmt).date()
    except ValueError:
        return None
    return d if d >= SANE_MIN else None


def ymd(d):
    return d.year * 10000 + d.month * 100 + d.day


def plus_years(d, years):
    try:
        return d.replace(year=d.year + years)
    except ValueError:  # 2/29
        return d.replace(year=d.year + years, day=d.day - 1)


def fetch(kind, symbol):
    """返回升序 (date, open, high, low, close, volume)；源只提供成交量的，量记 0"""
    import akshare as ak

    if kind == "crypto":
        raise ValueError("crypto_js_spot 只有实时价、无日线历史，待导入程序提供历史源")

    if kind == "fx_boc":
        # 中行牌价：折算价为每 100 外币兑人民币，除以 100 得单位价；买价作低、卖价作高
        df = ak.currency_boc_sina(symbol=symbol, start_date="20040101", end_date="20251231")
        df = df.rename(columns={"日期": "date"})
        rows = []
        for r in df.itertuples(index=False):
            d = parse_date(getattr(r, "date", None))
            mid = to_num(getattr(r, "中行折算价", 0)) / 100.0
            bid = to_num(getattr(r, "中行汇买价", 0)) / 100.0
            ask = to_num(getattr(r, "中行汇卖价", 0)) / 100.0
            if d is None or mid <= 0:
                continue
            lo = bid if bid > 0 else mid
            hi = ask if ask > 0 else mid
            rows.append((d, mid, max(mid, hi), min(mid, lo), mid, 0))
        rows.sort(key=lambda x: x[0])
        if not rows:
            raise ValueError("空结果")
        return rows

    if kind == "ccy_ts":
        df = ak.currency_time_series(symbol, ["USD"], "20040101", "20251231")
        df = df.rename(columns={"时间": "date", "收盘价": "close"})
        for c in ("open", "high", "low", "volume"):
            df[c] = df["close"] if c != "volume" else 0
        api = df
    else:
        api = {"fut": lambda: ak.futures_main_sina(symbol=symbol),
               "fut_zh": lambda: ak.futures_zh_daily_sina(symbol=symbol),
               "hk_idx": lambda: ak.stock_hk_index_daily_sina(symbol=symbol),
               "us_idx": lambda: ak.index_us_stock_sina(symbol=symbol),
               "fx": lambda: ak.forex_hist_em(symbol=symbol)}[kind]()

    api = api.rename(columns=CN_COLS)
    cols = list(api.columns)
    if "close" not in cols:
        raise KeyError("无法识别的列名: %s" % cols)

    rows = []
    for r in api.itertuples(index=False):
        d = parse_date(getattr(r, "date", None) if "date" in cols else r[0])
        if d is None:
            continue

        def pick(name, pos):
            v = getattr(r, name, None) if name in cols else (r[pos] if pos < len(r) else None)
            return to_num(v)

        o, h, l, c = pick("open", 1), pick("high", 2), pick("low", 3), pick("close", 4)
        v = pick("volume", 6)
        rows.append((d, o, h, l, c, v))
    rows.sort(key=lambda x: x[0])
    if not rows:
        raise ValueError("空结果")
    return rows


def cache_path(market, code):
    return os.path.join(CACHE_DIR, "%s_%s.csv" % (market, code))


def load_cache(path):
    rows = []
    with open(path, newline="", encoding="utf8") as f:
        for r in csv.reader(f):
            if len(r) < 6:
                continue
            rows.append((datetime.date(int(r[0][:4]), int(r[0][4:6]), int(r[0][6:8])),
                         float(r[1]), float(r[2]), float(r[3]), float(r[4]), float(r[5])))
    return sorted(rows, key=lambda x: x[0])


def save_cache(path, rows):
    with open(path, "w", newline="", encoding="utf8") as f:
        csv.writer(f).writerows([("%08d" % ymd(r[0]),) + r[1:] for r in rows])


def src_path(market, code):
    """缓存旁注真实来源，避免缓存命中时把 source 记成 "cache"""
    return os.path.join(CACHE_DIR, "%s_%s.src" % (market, code))


def sanitize(rows):
    """坏 bar 的统一出口（缓存与在线取数都过这一道）：四价齐全、high >= max(open,close)、
    low <= min(open,close)、剔除周末。指数/期货/股票类源均无周末交易；将来接入 FX/CRYPTO
    的周末时段需放宽此规则。"""
    out = []
    for r in rows:
        d, o, h, l, c = r[0], r[1], r[2], r[3], r[4]
        if 0 in (o, h, l, c):
            continue
        if h < l or h < max(o, c) or l > min(o, c):
            continue
        if d.weekday() >= 5:
            continue
        out.append(r)
    return out


def pick_window(market, code, d1, d2, sources, refresh):
    """按候选源链取窗口数据。返回 (rows, 说明, 偏差)；窗口不覆盖时退到可得起点 + 3 年"""
    path = cache_path(market, code)
    errs = []
    longest = None  # (rows, 源说明)

    if os.path.exists(path) and not refresh:
        rows = load_cache(path)
        if not rows:
            raise RuntimeError("缓存为空: %s（用 --refresh 重新取数）" % path)
        clean = sanitize(rows)
        if len(clean) != len(rows):  # 旧缓存可能早于过滤逻辑，清洗后回写
            save_cache(path, clean)
        src = "cache"
        if os.path.exists(src_path(market, code)):
            with open(src_path(market, code), encoding="utf8") as f:
                src = f.read().strip() or "cache"
        cands = [(clean, src)]
    else:
        cands = []
        for kind, symbol in sources:
            try:
                cands.append((sanitize(fetch(kind, symbol)), "%s(%s)" % (kind, symbol)))
            except Exception as e:  # noqa: BLE001
                errs.append("%s(%s): %s" % (kind, symbol, str(e)[:70]))
        if not cands and os.path.exists(path):
            # 刷新失败时不倒退：已有缓存仍可产出 fixture
            rows = sanitize(load_cache(path))
            if rows:
                errs.append("刷新失败，回退旧缓存")
                cands = [(rows, "cache")]
        if cands:
            # 缓存取覆盖面最广的一份，保证离线重跑与 --refresh 语义成立
            best = max(cands, key=lambda x: len(x[0]))
            if not best[0]:
                raise RuntimeError("源返回空数据: %s" % best[1])
            save_cache(path, best[0])
            with open(src_path(market, code), "w", encoding="utf8") as f:
                f.write(best[1])

    for rows, src in cands:
        if longest is None or len(rows) > len(longest[0]):
            longest = (rows, src)
        picked = [r for r in rows if d1 <= r[0] <= d2]
        if picked:
            # 起点、终点对称检查：任一侧不满足目标窗口都记入偏差
            notes = []
            if (picked[0][0] - d1).days > 7:
                notes.append("起点 %.0f 天延迟" % (picked[0][0] - d1).days)
            if (d2 - picked[-1][0]).days > 7:
                notes.append("终点提前 %.0f 天" % (d2 - picked[-1][0]).days)
            return picked, src, "; ".join(notes)

    if longest is None:
        raise RuntimeError("; ".join(errs) or "无可用源")
    rows, src = longest
    if not rows:
        raise RuntimeError("无可用数据: %s" % src)
    s = rows[0][0]
    e = plus_years(s, FALLBACK_YEARS)
    picked = [r for r in rows if s <= r[0] <= e]
    dev = "窗口 %s~%s 未覆盖，退到可得起点 %s + %d 年" % (ymd(d1), ymd(d2), ymd(s), FALLBACK_YEARS)
    return picked, src, dev


def build_h5(market, code, rows, dest_dir):
    """h5 驱动读侧固定 x0.001（H5KDataDriver.cpp:619-622），故价格必须存 price x 1000，
    与 stocktypeinfo.precision 无关（precision 只影响显示位数）。因此 h5 后端价格上限 3 位小数，
    源价多于 3 位小数的字段会被舍入 —— 属后端能力边界，在生成时给出精度丢失计数。"""
    filename = os.path.join(dest_dir, "%s_day.h5" % market.lower())
    if os.path.exists(filename):
        os.remove(filename)
    scale = 1000
    # 按实际数据统计被舍入的价格字段数（源数据的真实小数位与 SPEC.precision 无关）
    lost = 0
    for _, o, h, l, c, _ in rows:
        for v in (o, h, l, c):
            raw = v * scale
            if abs(raw - round(raw)) > 1e-9:
                lost += 1
    filters = tb.Filters(complevel=9, complib="zlib", shuffle=True)
    with tb.open_file(filename, "w", filters=filters) as h5f:
        group = h5f.create_group("/", "data")
        table = h5f.create_table(group, market + code, KDayRecord, expectedrows=len(rows))
        row = table.row
        for d, o, h, l, c, vol in rows:
            row["datetime"] = ymd(d) * 10000
            row["openPrice"] = int(round(o * scale))
            row["highPrice"] = int(round(h * scale))
            row["lowPrice"] = int(round(l * scale))
            row["closePrice"] = int(round(c * scale))
            # 成交额为近似口径：未乘合约乘数/每手股数（unit 由品种级覆盖），
            # 存千元（驱动 x0.1 -> API 万元）；单测不断言 amount 绝对值
            row["transAmount"] = int(round(c * vol))
            row["transCount"] = int(vol)
            row.append()
        table.flush()
    note = "" if not lost else "h5 精度上限 3 位小数，%d 个价格字段被舍入（%d 根 bar）" % (lost, len(rows))
    return filename, note


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--refresh", action="store_true", help="忽略 CSV 缓存重新取数")
    ap.add_argument("--markets", default="", help="逗号分隔市场名，缺省全部")
    ap.add_argument("--dest", default="test_data", help="h5 输出目录")
    ap.add_argument("--list", action="store_true", help="仅列出配置")
    args = ap.parse_args()

    if args.list:
        for m, c, p, s, e, srcs in SPEC:
            print("%-8s %-8s prec=%d %s~%s %s" % (m, c, p, s, e, srcs))
        return 0

    wanted = {x.strip().upper() for x in args.markets.split(",") if x.strip()}
    os.makedirs(CACHE_DIR, exist_ok=True)
    os.makedirs(args.dest, exist_ok=True)

    manifest = []
    summary = []
    for market, code, precision, start, end, sources in SPEC:
        if wanted and market not in wanted:
            continue
        d1 = datetime.date(int(start[:4]), int(start[4:6]), int(start[6:8]))
        d2 = datetime.date(int(end[:4]), int(end[4:6]), int(end[6:8]))
        try:
            rows, src, dev = pick_window(market, code, d1, d2, sources, args.refresh)
            filename, note = build_h5(market, code, rows, args.dest)
            dev = "; ".join(x for x in (dev, note) if x)
            manifest.append([market, code, len(rows), ymd(rows[0][0]), ymd(rows[-1][0]), src, dev])
            summary.append("OK   %-8s %-8s rows=%-5d %s~%s%s"
                           % (market, code, len(rows), rows[0][0], rows[-1][0],
                              "  [偏差] " + dev if dev else ""))
        except Exception as e:  # noqa: BLE001
            manifest.append([market, code, 0, "", "", "", "未覆盖: %s" % e])
            summary.append("FAIL %-8s %-8s %s" % (market, code, e))

    with open(MANIFEST, "w", newline="", encoding="utf8") as f:
        w = csv.writer(f)
        w.writerow(["market", "code", "rows", "start", "end", "source", "deviation"])
        w.writerows(manifest)

    print("\n".join(summary))
    print("\nmanifest -> %s" % MANIFEST)
    return 1 if any(s.startswith("FAIL") for s in summary) else 0


if __name__ == "__main__":
    sys.exit(main())
