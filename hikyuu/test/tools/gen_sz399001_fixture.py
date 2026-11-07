# -*- coding: utf-8 -*-
# cp936
"""SZ399001 (Shenzhen Component Index) representative index fixture generator

Registration, trading-calendar and dimension assertions need the SZ representative index
to have data in sz_day/sz_1min/sz_5min/sz_time/sz_trans; sh_trans additionally gets
SH000001. All offline deterministic synthesis; existing instrument data is not modified.

Dimension consistency: day.transAmount == sum(1min) == sum(5min),
sum(1min/5min.transCount) == day.transCount * 100.

Usage (repo root):
    python hikyuu/test/tools/gen_sz399001_fixture.py
"""

import tables

DAY_COUNT = 500
MINUTE_DAYS = 10
BASE_PRICE = 3000.0
PRICE_SCALE = 1000  # HDF5 stores prices multiplied by 1000
DAY_VOL = 5000      # day volume in lots (100 shares per lot)


def write_dataset(h5f, name, rows):
    """Overwrite-write a dataset, reusing the column layout of an existing table in the same file"""
    group = h5f.root.data
    if name in group:
        h5f.remove_node(group, name)
    src = group.SZ000001 if name.startswith("SZ") else group.SH600000
    t = h5f.create_table(group, name, src.description, expectedrows=len(rows))
    row = t.row
    for r in rows:
        for k, v in r.items():
            row[k] = v
        row.append()
    t.flush()


def distribute(total, n):
    """Split integer total into n even parts, remainder folded into the last one (sum == total)"""
    base = total // n
    out = [base] * n
    out[-1] += total - base * n
    return out


def minute_labels(day):
    """A-share 1min/5min bar labels, consistent with the existing data"""
    hhmm1 = [(9, m) for m in range(31, 60)] + [(10, m) for m in range(0, 60)] + \
        [(11, 0)] + [(11, m) for m in range(1, 31)] + \
        [(13, m) for m in range(1, 60)] + [(14, m) for m in range(0, 60)] + [(15, 0)]
    min1 = [day * 10000 + hh * 100 + mm for hh, mm in hhmm1]
    min5 = [day * 10000 + e for e in
            [935, 940, 945, 950, 955, 1000, 1005, 1010, 1015, 1020, 1025, 1030,
             1035, 1040, 1045, 1050, 1055, 1100, 1105, 1110, 1115, 1120, 1125, 1130,
             1305, 1310, 1315, 1320, 1325, 1330, 1335, 1340, 1345, 1350, 1355, 1400,
             1405, 1410, 1415, 1420, 1425, 1430, 1435, 1440, 1445, 1450, 1455, 1500]]
    return min1, min5


def main():
    day_file = tables.open_file("test_data/sz_day.h5", "a")
    dates = sorted(r["datetime"] for r in day_file.root.data.SZ000001.iterrows())[-DAY_COUNT:]
    min_days = dates[-MINUTE_DAYS:]

    # Build minute details first; day amount/count aggregate from them: day lots x 100 = total detail shares
    min1_rows, min5_rows, day_amounts, day_counts = {}, {}, {}, {}
    with tables.open_file("test_data/sz_1min.h5", "a") as f:
        for i, day in enumerate(min_days):
            day_key = day // 10000
            min1, min5 = minute_labels(day_key)
            day_shares = DAY_VOL * 100
            day_amount = 0
            rows1 = []
            counts = distribute(day_shares, len(min1))
            for j, dt in enumerate(min1):
                price = int(round(BASE_PRICE * PRICE_SCALE) + ((i * 240 + j) % 200) * 5)
                rows1.append({"datetime": dt, "openPrice": price - 500, "highPrice": price + 500,
                              "lowPrice": price - 800, "closePrice": price,
                              "transAmount": price * counts[j], "transCount": counts[j]})
                day_amount += price * counts[j]
            rows5 = []
            counts5 = distribute(day_shares, len(min5))
            amounts5 = distribute(day_amount, len(min5))
            for j, dt in enumerate(min5):
                price = int(round(BASE_PRICE * PRICE_SCALE) + ((i * 48 + j) % 200) * 5)
                rows5.append({"datetime": dt, "openPrice": price - 500, "highPrice": price + 500,
                              "lowPrice": price - 800, "closePrice": price,
                              "transAmount": amounts5[j], "transCount": counts5[j]})
            day_amounts[day] = day_amount
            day_counts[day] = day_shares // 100
            min1_rows[day] = rows1
            min5_rows[day] = rows5
        write_dataset(f, "SZ399001", sum(min1_rows.values(), []))

    with tables.open_file("test_data/sz_5min.h5", "a") as f:
        write_dataset(f, "SZ399001", sum(min5_rows.values(), []))

    # Day bars: days with minute details use the aggregated values
    day_rows = []
    for i, day in enumerate(dates):
        price = int(round(BASE_PRICE * PRICE_SCALE) + (i % 200) * 50)
        amount = day_amounts.get(day, price * DAY_VOL)
        count = day_counts.get(day, DAY_VOL)
        day_rows.append({"datetime": day, "openPrice": price - 1000, "highPrice": price + 1500,
                         "lowPrice": price - 2000, "closePrice": price,
                         "transAmount": amount, "transCount": count})
    write_dataset(day_file, "SZ399001", day_rows)
    day_file.close()

    # time uses the last trading day (only needs to be non-empty); trans datetime is YYYYMMDDHHMMSS
    last_key = min_days[-1] // 10000
    with tables.open_file("test_data/sz_time.h5", "a") as f:
        rows = [{"datetime": last_key * 10000 + hm, "price": int(BASE_PRICE * PRICE_SCALE) + i * 500,
                 "vol": 100 + i} for i, hm in enumerate([930, 1000, 1030, 1400, 1430, 1500])]
        write_dataset(f, "SZ399001", rows)
    with tables.open_file("test_data/sz_trans.h5", "a") as f:
        rows = [{"buyorsell": i % 2, "datetime": last_key * 1000000 + hm * 100,
                 "price": int(BASE_PRICE * PRICE_SCALE) + i * 500, "vol": 50 + i}
                for i, hm in enumerate([930, 1001, 1030, 1400, 1430, 1500])]
        write_dataset(f, "SZ399001", rows)

    # sh_trans had only single stocks before; add SH000001 or the sh_trans keys would be empty
    with tables.open_file("test_data/sh_trans.h5", "a") as f:
        dayf = tables.open_file("test_data/sh_day.h5", "r")
        sh_day = dayf.root.data.SH000001
        tail = {r["datetime"]: r["closePrice"] for r in sh_day.iterrows()}
        dayf.close()
        rows = []
        for day in sorted(tail)[-5:]:
            close = tail[day]
            day_key = day // 10000
            for i, hm in enumerate([930, 1000, 1030, 1400, 1430, 1500]):
                rows.append({"buyorsell": i % 2, "datetime": day_key * 1000000 + hm * 100,
                             "price": close + i * 100, "vol": 80 + i})
        write_dataset(f, "SH000001", rows)

    print("SZ399001: day %d / 1min %d days / 5min %d days / time+trans last day; "
          "SH000001 trans 5 days" % (len(day_rows), len(min_days), len(min_days)))


if __name__ == "__main__":
    main()
