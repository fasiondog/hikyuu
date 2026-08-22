# -*- coding: utf-8 -*-

import unittest
from unittest.mock import patch

from hikyuu import Datetime
from hikyuu.data import common_clickhouse, common_mysql


BASE_RECORDS = [
    (202501310000, 10, 11, 9, 10.5, 100, 10),
    (202502030000, 20, 21, 19, 20.5, 200, 20),
]


class FakeMySQLCursor:
    def __init__(self, connect):
        self.connect = connect
        self.rows = []

    def execute(self, sql):
        if sql.strip().lower().startswith("select date, open"):
            self.rows = self.connect.base_records

    def executemany(self, sql, values):
        self.connect.writes.append((sql, list(values)))

    def __iter__(self):
        return iter(self.rows)

    def close(self):
        pass


class FakeMySQLConnect:
    def __init__(self, base_records):
        self.base_records = base_records
        self.writes = []

    def cursor(self):
        return FakeMySQLCursor(self)

    def commit(self):
        pass


class FakeClickHouseResult:
    def __init__(self, rows):
        self.result_rows = rows


class FakeClickHouseConnect:
    def __init__(self, base_records):
        self.base_records = base_records

    def query(self, _sql, settings=None):
        return FakeClickHouseResult(self.base_records)


class CommonSQLTest(unittest.TestCase):
    @staticmethod
    def fake_mysql_table(_connect, _market, _code, ktype):
        return ktype

    @staticmethod
    def fake_clickhouse_table(_connect, market, code, ktype):
        return (ktype, market, code)

    def test_mysql_period_data_does_not_include_next_period(self):
        connect = FakeMySQLConnect(BASE_RECORDS)

        def fake_lastdatetime(_connect, table):
            return BASE_RECORDS[-1][0] if table == "day" else None

        with patch.object(common_mysql, "get_table", side_effect=self.fake_mysql_table), \
                patch.object(common_mysql, "get_lastdatetime", side_effect=fake_lastdatetime):
            common_mysql.update_extern_data(connect, "SH", "600000", "day")

        month_rows = next(values for sql, values in connect.writes if "month" in sql)
        self.assertEqual(
            month_rows,
            [
                (202501310000, 10, 11, 9, 10.5, 100, 10),
                (202502280000, 20, 21, 19, 20.5, 200, 20),
            ],
        )

    def test_clickhouse_period_data_does_not_include_next_period(self):
        timestamp_records = [
            (Datetime(record[0]).timestamp_utc() // 1000000, *record[1:])
            for record in BASE_RECORDS
        ]
        connect = FakeClickHouseConnect(timestamp_records)

        def fake_lastdatetime(_connect, table):
            return Datetime(BASE_RECORDS[-1][0]) if table[0] == "day" else None

        with patch.object(
                common_clickhouse, "get_table", side_effect=self.fake_clickhouse_table
        ), patch.object(
                common_clickhouse, "get_lastdatetime", side_effect=fake_lastdatetime
        ):
            index_data = common_clickhouse.update_extern_data(connect, "SH", "600000", "day")

        month_rows = index_data["month"]
        self.assertEqual(
            month_rows,
            [
                (
                    "SH",
                    "600000",
                    Datetime(202501310000).timestamp_utc() // 1000000,
                    10,
                    11,
                    9,
                    10.5,
                    100,
                    10,
                ),
                (
                    "SH",
                    "600000",
                    Datetime(202502280000).timestamp_utc() // 1000000,
                    20,
                    21,
                    19,
                    20.5,
                    200,
                    20,
                ),
            ],
        )


def suite():
    return unittest.TestLoader().loadTestsFromTestCase(CommonSQLTest)


if __name__ == "__main__":
    unittest.main()
