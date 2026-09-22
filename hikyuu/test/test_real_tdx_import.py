# -*- coding: utf-8 -*-
"""
测试模块：test_real_tdx_import.py
最后修改: 2025-08-07

本测试模块专用于验证通达信 (.day) 本地数据文件到 HDF5 格式的转换功能, 
其核心特点是 **使用真实数据文件** 进行端到端的集成测试。

设计哲学:
- **真实性**: 直接使用从通达信客户端获取的 .day 文件, 确保测试场景尽可能贴近实际使用情况。
- **健壮性**: 提供了多种对比策略, 包括全量对比和交集对比, 以适应源数据可能存在清洗或范围差异的情况。
- **自动化**: 通过 Pytest Fixture 管理测试环境的创建和销毁, 并通过 `skipif` 自动跳过不满足前置条件的测试。
- **故障注入**: 包含一个故障注入测试, 确保断言逻辑本身是可靠的, 能够在数据不一致时准确地失败。

与其他测试的区别:
- `test_tdx_import.py`: 主要使用 **模拟数据** 和 **Mock对象** 对导入流程进行单元测试, 关注的是代码逻辑路径的正确性, 运行速度快且不依赖外部文件。
- `test_tdx_real_data_import.py`: 使用 **真实数据** 对核心转换函数 `tdx_import_day_data_from_file` 进行集成测试, 关注的是数据转换结果的准确性, 运行速度稍慢且依赖于一个真实的 .day 文件。

使用前提:
- 需要一个真实的通达信 .day 文件。测试默认会寻找环境变量 `HKU_real_tdx_file_path` 指定的文件,
  如果未指定, 则会尝试读取 `/mnt/d1581/tdx/vipdoc/sh/lday/sh600000.day`。
  如果文件不存在, 所有测试将被自动跳过。
"""
import os
import sqlite3
import struct
import tempfile
from datetime import datetime

import pytest
import tables as tb

import unittest.mock
unittest.mock.patch('hikyuu.hku_cleanup', lambda: None).start()

from hikyuu.data.common_h5 import H5Record, get_h5table, open_h5file
# 假设要测试的函数位于以下模块
from hikyuu.data.tdx_to_h5 import tdx_import_day_data_from_file
from hikyuu.test.test_init import get_real_tdx_filepath

stcode = '600000'  # 示例股票代码


@pytest.fixture(scope="module")
def real_tdx_file_path():
    """为当前模块的测试提供 sh600000.day 的路径"""
    return get_real_tdx_filepath(stcode, market='sh')


# 用户提供的真实数据文件路径, 通过函数动态获取
# real_tdx_file_path = _get_real_tdx_filepath(stcode)


def _read_raw_tdx_day_file(filename):
    """
    一个辅助函数, 用于直接读取通达信官方的 .day 文件格式, 并将其解析为
    一个与 HDF5 存储格式相匹配的字典列表, 以便后续进行数据对比。
    """
    records = []
    with open(filename, 'rb') as f:
        while True:
            chunk = f.read(32)
            if not chunk:
                break
            if len(chunk) != 32:
                continue

            # 解析通达信.day文件格式 (32字节)
            # 字段: date, open, high, low, close, amount, volume, reserved
            # 类型: int, int, int, int, int, float, int, int
            # 字节:  4,   4,    4,    4,    4,     4,     4,    4
            raw_data = struct.unpack("iiiiifii", chunk)

            # 核心逻辑: 必须与 tdx_import_day_data_from_file 中的数据清洗逻辑完全一致
            # 如果开、高、低、收盘价中任何一个为0, 则该条记录被视为无效, 直接跳过
            if 0 in raw_data[1:5]:
                continue
            
            # 将解析出的原始数据转换为与HDF5中存储格式完全一致的字典
            record = {
                'datetime': int(datetime.strptime(str(raw_data[0]), "%Y%m%d").strftime("%Y%m%d0000")),
                'openPrice': raw_data[1] * 10,
                'highPrice': raw_data[2] * 10,
                'lowPrice': raw_data[3] * 10,
                'closePrice': raw_data[4] * 10,
                'transAmount': round(raw_data[5] * 0.001),
                'transCount': round(raw_data[6] * 0.01)
            }
            records.append(record)
    return records


@pytest.fixture
def imported_data(request, real_tdx_file_path):
    """
    一个 Module 级别的 Pytest Fixture, 它在当前测试模块的所有测试用例执行前, 
    仅运行一次。它负责:
    1. 创建一个临时的 HDF5 文件和内存中的 SQLite 数据库。
    2. 执行一次从 .day 文件到 HDF5 的导入操作。
    3. 使用 `yield` 将导入的结果 (如HDF5文件句柄、导入计数等) 提供给所有测试用例。
    4. 在所有测试用例执行完毕后, 自动清理临时文件和数据库连接。
    
    这种方式可以避免每个测试都重复执行耗时的导入操作, 显著提高测试效率。
    """
    # 1. 创建临时的数据库和数据目录
    db_connect = sqlite3.connect(":memory:")
    cur = db_connect.cursor()
    cur.execute(
        '''CREATE TABLE stock (
            stockid INTEGER PRIMARY KEY, marketid INTEGER, code TEXT,
            valid INTEGER, type INTEGER, startdate INTEGER, enddate INTEGER
        )'''
    )
    cur.execute(
        f"INSERT INTO stock (stockid, marketid, code, valid, type, startdate, enddate) VALUES (1, 1, '{stcode}', 1, 1, 0, 99999999)"
    )
    cur.execute("CREATE TABLE market (marketid INTEGER, lastdate INTEGER)")
    cur.execute("INSERT INTO market (marketid, lastdate) VALUES (1, 0)") # lastdate=0 确保从头导入
    db_connect.commit()

    temp_dir = tempfile.TemporaryDirectory()
    data_dir = temp_dir.name

    # 2. 执行导入
    market = 'SH'
    stock_code = stcode
    cur.execute(f"SELECT stockid, marketid, code, valid, type FROM stock WHERE code = '{stock_code}'")
    stock_record = cur.fetchone()
    h5file = open_h5file(data_dir, market, 'DAY')
    
    imported_count = tdx_import_day_data_from_file(
        connect=db_connect, filename=real_tdx_file_path, h5file=h5file, market=market, stock_record=stock_record
    )

    # 3. 使用 yield 将必要信息打包返回给测试用例
    yield {
        "h5file": h5file,
        "market": market,
        "stock_code": stock_code,
        "imported_count": imported_count,
    }

    # 4. 测试用例执行完毕后, 清理资源
    h5file.close()
    temp_dir.cleanup()
    db_connect.close()


# 使用 pytest.mark.skipif 装饰器, 如果指定的条件为真, 则跳过该测试。
# 这里用于检查真实的 .day 文件是否存在, 如果不存在, 测试将不会执行, 并给出友好提示。
def test_import_and_verify_output(imported_data, real_tdx_file_path):
    if not os.path.exists(real_tdx_file_path):
        pytest.skip(f"TDX file not found: {real_tdx_file_path}")
    """
    测试目标: 验证导入流程能成功执行, 并对导入结果进行一次快速的、人工友好的抽样检查。
    测试方法:
      - 检查导入的记录数是否大于0。
      - 打印 HDF5 文件中的第一条和最后一条记录的关键字段。
    预期结果:
      - 测试通过, 并在控制台输出清晰的首末记录, 方便人工核对。
    """
    print(f"\nSuccessfully imported {imported_data['imported_count']} records from {os.path.basename(real_tdx_file_path)}.")
    assert imported_data['imported_count'] > 0

    table = get_h5table(imported_data['h5file'], imported_data['market'], imported_data['stock_code'])
    print("\nPlease check whether the following data is correct:")
    print("=" * 30)
    
    first_record = table[0]
    print(f"First record: ")
    print(f"  datetime: {first_record['datetime']}")
    print(f"  open:   {first_record['openPrice'] / 1000.0}")
    print(f"  high:   {first_record['highPrice'] / 1000.0}")
    print(f"  low:    {first_record['lowPrice'] / 1000.0}")
    print(f"  close:  {first_record['closePrice'] / 1000.0}")
    print(f"  amount: {first_record['transAmount'] / 1000.0}")
    print(f"  volume: {first_record['transCount']}")

    last_record = table[-1]
    print(f"Last record: ")
    print(f"  datetime: {last_record['datetime']}")
    print(f"  open:   {last_record['openPrice'] / 1000.0}")
    print(f"  high:   {last_record['highPrice'] / 1000.0}")
    print(f"  low:    {last_record['lowPrice'] / 1000.0}")
    print(f"  close:  {last_record['closePrice'] / 1000.0}")
    print(f"  amount: {last_record['transAmount'] / 1000.0}")
    print(f"  volume: {last_record['transCount']}")

    print("-" * 30)
    print(f"{len(table)} records in total.")


def test_compare_imported_data_with_source(imported_data, real_tdx_file_path):
    if not os.path.exists(real_tdx_file_path):
        pytest.skip(f"TDX file not found: {real_tdx_file_path}")
    """
    测试目标: 对比 HDF5 中的数据和原始 .day 文件的数据, 确保 **全量数据** 的精确一致性。
    测试方法:
      - 分别读取 HDF5 和 .day 文件中的所有记录。
      - 首先断言两者记录数完全相等。
      - 然后逐条、逐字段地进行精确断言。
    预期结果:
      - 只有在两条数据源完全一致的情况下, 测试才能通过。
      - 如果导入过程中存在任何数据清洗或过滤, 此测试将会失败。
    """
    # 1. 从HDF5文件读取已导入的数据
    h5_table = get_h5table(imported_data['h5file'], imported_data['market'], imported_data['stock_code'])
    
    # 2. 从原始.day文件直接读取数据
    raw_records = _read_raw_tdx_day_file(real_tdx_file_path)
    
    # 3. 断言记录数量一致
    assert len(h5_table) == len(raw_records), "The number of records after import is inconsistent with the source file!"
    
    # 4. 逐条比较记录
    for i, h5_record in enumerate(h5_table):
        raw_record = raw_records[i]
        assert h5_record['datetime'] == raw_record['datetime'], f"Record {i+1}: datetime mismatch"
        assert h5_record['openPrice'] == raw_record['openPrice'], f"Record {i+1}: open price mismatch"
        assert h5_record['highPrice'] == raw_record['highPrice'], f"Record {i+1}: high price mismatch"
        assert h5_record['lowPrice'] == raw_record['lowPrice'], f"Record {i+1}: low price mismatch"
        assert h5_record['closePrice'] == raw_record['closePrice'], f"Record {i+1}: close price mismatch"
        assert h5_record['transAmount'] == raw_record['transAmount'], f"Record {i+1}: amount mismatch"
        assert h5_record['transCount'] == raw_record['transCount'], f"Record {i+1}: volume mismatch"

    print(f"\nSuccessfully verified {len(h5_table)} records; the HDF5 data is identical to the source file.")


def test_compare_common_data_with_source(imported_data, real_tdx_file_path):
    if not os.path.exists(real_tdx_file_path):
        pytest.skip(f"TDX file not found: {real_tdx_file_path}")
    """
    测试目标: 对比 HDF5 和 .day 文件的数据, 确保 **交集部分** 的数据精确一致性。
    测试方法:
      - 将两个数据源都读入以日期为键的字典中。
      - 找出两个日期集合的交集。
      - 只对交集中的数据进行逐条、逐字段的精确断言。
    预期结果:
      - 即使两个数据源的记录总数不同 (例如, HDF5中清洗了部分无效数据),
        只要它们共有的那部分数据是完全一致的, 测试就能通过。
      - 这是一个比全量对比更健壮的测试。
    """
    # 1. 从HDF5文件读取数据并转换为以日期为键的字典, 方便快速查找
    h5_table = get_h5table(imported_data['h5file'], imported_data['market'], imported_data['stock_code'])
    h5_records_map = {row['datetime']: {col: row[col] for col in h5_table.colnames} for row in h5_table.iterrows()}

    # 2. 从原始.day文件直接读取数据并也转换为字典
    raw_records_list = _read_raw_tdx_day_file(real_tdx_file_path)
    raw_records_map = {row['datetime']: row for row in raw_records_list}

    # 3. 打印数据源信息, 方便调试
    print("\n--- Data source info ---")
    if h5_records_map:
        h5_dates_list = sorted(h5_records_map.keys())
        print(f"HDF5      : {len(h5_records_map)} records, date range: {h5_dates_list[0]} -> {h5_dates_list[-1]}")
    else:
        print("HDF5      : 0 records")
    
    if raw_records_map:
        raw_dates_list = sorted(raw_records_map.keys())
        print(f"Raw .day  : {len(raw_records_map)} records, date range: {raw_dates_list[0]} -> {raw_dates_list[-1]}")
    else:
        print("Raw .day  : 0 records")

    # 4. 寻找两个日期集合的交集, 并进行比较
    h5_dates = set(h5_records_map.keys())
    raw_dates = set(raw_records_map.keys())
    common_dates = sorted(list(h5_dates.intersection(raw_dates)))
    
    print(f"Common    : {len(common_dates)} records for comparison.")
    print("--------------------")

    assert len(common_dates) > 0, "The HDF5 file and the raw file have no common data to compare!"

    # 5. 逐条比较共同的记录
    for date in common_dates:
        h5_record = h5_records_map[date]
        raw_record = raw_records_map[date]
        
        assert h5_record['openPrice'] == raw_record['openPrice'], f"Open price mismatch at {date}"
        assert h5_record['highPrice'] == raw_record['highPrice'], f"High price mismatch at {date}"
        assert h5_record['lowPrice'] == raw_record['lowPrice'], f"Low price mismatch at {date}"
        assert h5_record['closePrice'] == raw_record['closePrice'], f"Close price mismatch at {date}"
        assert h5_record['transAmount'] == raw_record['transAmount'], f"Amount mismatch at {date}"
        assert h5_record['transCount'] == raw_record['transCount'], f"Volume mismatch at {date}"

    print(f"\nSuccessfully verified {len(common_dates)} common records; the data is identical.")


def test_mismatched_data_is_detected(imported_data, real_tdx_file_path):
    if not os.path.exists(real_tdx_file_path):
        pytest.skip(f"TDX file not found: {real_tdx_file_path}")
    """
    测试目标: 验证我们的对比断言逻辑是可靠的, 能够在数据不一致时 **按预期失败**。
    测试方法 (故障注入):
      - 正常读取两份数据。
      - 在对比前, **故意修改** 原始数据中的一条记录 (例如, 修改开盘价)。
      - 使用 `pytest.raises(AssertionError)` 上下文管理器来执行对比逻辑。
    预期结果:
      - 对比逻辑在执行到被修改的那条数据时, 应该抛出 `AssertionError`。
      - `pytest.raises` 会捕获这个异常, 从而让整个测试用例 **通过**。
      - 如果对比逻辑有误, 没有抛出异常, `pytest.raises` 会反过来让测试 **失败**。
      - 同时, 还会检查捕获到的异常信息, 确保它是由我们注入的故障导致的。
    """
    # 1. 准备数据
    h5_table = get_h5table(imported_data['h5file'], imported_data['market'], imported_data['stock_code'])
    h5_records_map = {row['datetime']: {col: row[col] for col in h5_table.colnames} for row in h5_table.iterrows()}
    raw_records_list = _read_raw_tdx_day_file(real_tdx_file_path)
    raw_records_map = {row['datetime']: row for row in raw_records_list}
    common_dates = sorted(list(set(h5_records_map.keys()).intersection(set(raw_records_map.keys()))))
    
    assert len(common_dates) > 100, "Too little data to run this test"

    # 2. 故意修改一条数据以注入故障
    target_date = common_dates[100]  # 选择一条记录进行修改
    original_price = raw_records_map[target_date]['openPrice']
    modified_price = 99999999
    raw_records_map[target_date]['openPrice'] = modified_price
    
    print(f"\n[Fault injection] Change the raw open price of {target_date} ({original_price}) to {modified_price} to test the assertion.\n")

    # 3. 使用 pytest.raises 验证断言是否会按预期失败
    with pytest.raises(AssertionError) as excinfo:
        # 复用之前的比较逻辑
        for date in common_dates:
            h5_record = h5_records_map[date]
            raw_record = raw_records_map[date]
            assert h5_record['openPrice'] == raw_record['openPrice'], f"Open price mismatch at {date}"
            # ... (其他字段的断言在此处可以省略, 因为我们预期在 openPrice 就会失败)

    # 4. 验证失败信息是否包含了我们期望的内容, 确保断言是在正确的位置失败的
    error_message = str(excinfo.value)
    print(f"[Assertion check] Caught error: {error_message}")
    assert str(target_date) in error_message
    assert "Open price mismatch" in error_message
    print("[Verification passed] The assertion failed at the correct place for the correct reason.")
