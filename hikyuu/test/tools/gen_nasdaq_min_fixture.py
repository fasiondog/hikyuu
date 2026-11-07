# coding: utf-8
"""NASDAQ representative index minute-bar fixture generator (end-to-end carrier of the
multi-market read-side unified expression)

Purpose: generate test_data/nasdaq_5min.h5 with two groups:
  - "data"  : 5-minute bars (H5Record); datetime is the bucket-end time of the **New York
              local wall clock** (09:35 ... 16:00);
  - "min15" : 15-minute index table (H5Index{datetime, start}); datetime is the New York
              local bucket-end time (09:45 ... 16:00 every 15 minutes), start points to the
              index of the first 5-minute bar of that bucket inside "data".

Why these two groups: the H5 driver registers the same `{market}_min5` config key for
MIN5/MIN15/MIN30/MIN60/HOUR2 (see H5KDataDriver::_init); MIN15 reads the "min15" index
table, and missing groups silently return empty. Hence this file only creates data + min15
and leaves the other periods empty on purpose.

Why bucket boundaries are computed locally: common_h5.getMin15Date hardcodes the A-share
session boundaries (945/1000/.../1500), which is wrong for non-A-share markets (the
driver's minute aggregation is a recorded capability boundary, untouched in this batch),
so the fixture generates correct buckets per the New York session. The bucket-end
convention matches A-share: advance from the session start by one period
(09:30 + 15m = 09:45 onwards).

Window 2016-10-24 ~ 2016-11-10 spans the US DST end on 2016-11-06 (Sunday), to verify:
  - around the DST end, the same wall-clock time (09:45) maps to different Beijing times
    (+12h vs +13h);
  - the normalized series stays strictly monotonic (the premise of buffer bisection and
    getPos);
  - day and coarser labels do not participate in the conversion.
The window also overlaps the actual coverage of nasdaq_day.h5 in the same library
(2014-02 ~ 2017-02, the earliest real NDX availability), so the same trading days assert
both day labels and minute buckets.

The on-disk format is identical to hikyuu/data/common_h5.py: dataset name = MARKET + code
concatenated verbatim (HDF5 is case-sensitive; the C++ side looks up by market + code
directly); the four prices = real price x 1000 as integers (the driver read side is fixed
at x0.001, so only 3 decimals can be stored); transAmount is an approximation (close x
vol) and its absolute value is not asserted by unit tests. Data is synthetic arithmetic,
network-free, and byte-identical across reruns.

Usage:
    python hikyuu/test/tools/gen_nasdaq_min_fixture.py
    python hikyuu/test/tools/gen_nasdaq_min_fixture.py --dest test_data
"""

import argparse
import datetime
import os

import tables as tb

MARKET = "NASDAQ"
CODE = "NDX"
PRECISION = 2
SCALE = 1000  # h5 stores prices multiplied by 1000
FILENAME = "nasdaq_5min.h5"

# Window spans the US DST end on 2016-11-06 (Sunday) and overlaps the nasdaq_day.h5 coverage
START = datetime.date(2016, 10, 24)
END = datetime.date(2016, 11, 10)

# New York regular session 09:30-16:00 (local wall clock); bars are labeled by bucket-end time
SESSION_START_MIN = 9 * 60 + 30
SESSION_END_MIN = 16 * 60


class KMinRecord(tb.IsDescription):
    """Same layout as common_h5.H5Record (the C++ side reads by field name, order-free)"""
    datetime = tb.UInt64Col()
    openPrice = tb.UInt32Col()
    highPrice = tb.UInt32Col()
    lowPrice = tb.UInt32Col()
    closePrice = tb.UInt32Col()
    transAmount = tb.UInt64Col()
    transCount = tb.UInt64Col()


class KIndexRecord(tb.IsDescription):
    """Same layout as common_h5.H5Index: bucket-end datetime + base offset start"""
    datetime = tb.UInt64Col()
    start = tb.UInt64Col()


def session_ends(step):
    """Bucket-end times (HH:MM) advanced by step minutes within the session"""
    out = []
    t = SESSION_START_MIN
    while t + step <= SESSION_END_MIN:
        t += step
        out.append("%02d:%02d" % (t // 60, t % 60))
    return out


MIN5_ENDS = session_ends(5)
MIN15_ENDS = session_ends(15)


def trading_days(start, end):
    """Weekdays within the window (no US holidays in this window)"""
    d = start
    while d <= end:
        if d.weekday() < 5:
            yield d
        d += datetime.timedelta(days=1)


def day_min5(d):
    """5-minute local times of one trading day (YYYYMMDDHHMM integers)"""
    day = int(d.strftime("%Y%m%d"))
    return [day * 10000 + int(e.replace(":", "")) for e in MIN5_ENDS]


def day_min15_index(min5_of_day, base_offset):
    """15-minute index rows of one trading day: bucket-end local time + index of the bucket's first 5-minute bar"""
    per_bucket = 15 // 5
    rows = []
    for k, end in enumerate(MIN15_ENDS):
        first = k * per_bucket
        if first >= len(min5_of_day):
            break
        rows.append((min5_of_day[first] // 10000 * 10000 + int(end.replace(":", "")),
                     base_offset + first))
    return rows


def price_at(i):
    return round(BASE_PRICE + (i % 400) * STEP, PRECISION)


BASE_PRICE = 2200.00   # starting level (synthetic, unrelated to the real index)
STEP = 0.05            # arithmetic step per bar
BASE_VOL = 1000


def build():
    """Return (bars, index_rows): bars = [(dt_local, o, h, l, c, vol)]"""
    bars, index_rows = [], []
    for d in trading_days(START, END):
        min5 = day_min5(d)
        base_offset = len(bars)
        for dt in min5:
            i = len(bars)
            c = price_at(i)
            o = price_at(i - 1) if i else round(BASE_PRICE - STEP, PRECISION)
            vol = BASE_VOL + (i % 7) * 10
            bars.append((dt, o, max(o, c), min(o, c), c, vol))
        index_rows.extend(day_min15_index(min5, base_offset))
    return bars, index_rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dest", default="test_data", help="h5 output directory")
    args = ap.parse_args()

    filename = os.path.join(args.dest, FILENAME)
    os.makedirs(args.dest, exist_ok=True)

    bars, index_rows = build()
    filters = tb.Filters(complevel=9, complib="zlib", shuffle=True)
    table_name = MARKET + CODE
    with tb.open_file(filename, "w", filters=filters) as h5f:
        t = h5f.create_table(h5f.create_group("/", "data"), table_name, KMinRecord,
                             expectedrows=len(bars))
        row = t.row
        for dt, o, hi, lo, c, vol in bars:
            row["datetime"] = dt
            row["openPrice"] = int(round(o * SCALE))
            row["highPrice"] = int(round(hi * SCALE))
            row["lowPrice"] = int(round(lo * SCALE))
            row["closePrice"] = int(round(c * SCALE))
            row["transAmount"] = int(round(c * vol))
            row["transCount"] = int(vol)
            row.append()
        t.flush()

        it = h5f.create_table(h5f.create_group("/", "min15"), table_name, KIndexRecord,
                              expectedrows=len(index_rows))
        irow = it.row
        for dt, start in index_rows:
            irow["datetime"] = dt
            irow["start"] = start
            irow.append()
        it.flush()

    days = len(list(trading_days(START, END)))
    print("%s: %d trading days / data %d 5-minute bars / min15 %d buckets (%d per day)"
          % (filename, days, len(bars), len(index_rows), len(MIN15_ENDS)))
    print("first bucket %s / last bucket %s (market-local wall clock; the window contains the DST switch)"
          % (index_rows[0][0], index_rows[-1][0]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
