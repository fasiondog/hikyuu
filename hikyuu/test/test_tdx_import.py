# -*- coding: utf-8 -*-
"""
测试模块: test_tdx_import.py

本测试模块使用 unittest.TestCase 结构, 旨在验证通达信(TDX)本地数据
导入HDF5文件的核心流程。

由于导入过程涉及到多进程、文件I/O和数据库操作, 为了实现一个稳定、可靠的
单元测试, 本模块采取了以下策略:
1.  **环境隔离**: 使用 unittest 的 setUp 和 tearDown 方法, 在系统的临时
    目录中为每个测试创建独立、干净的运行环境, 并在测试结束后自动清理,
    避免对开发环境造成污染。
2.  **依赖模拟 (Mocking)**: 通过 unittest.mock.patch, 模拟了所有外部
    依赖, 包括:
    -   数据库初始化 (sqlite_create_database)
    -   网络数据获取 (search_best_tdx, TdxHq_API)
    -   其他数据导入函数 (sqlite_import_stock_name 等)
    这样可以确保测试只关注于导入逻辑本身, 不受网络状况或外部API变动的影响。
3.  **同步执行**: 导入线程 (UseTdxImportToH5Thread) 的多进程执行逻辑
    (_run 方法) 被一个自定义的内部测试类 (TestImporterThread) 覆盖,
    将异步的多进程操作转换为同步的单进程操作。这彻底解决了在测试过程中
    因进程间文件读写延迟而导致的"竞争条件" (Race Condition) 问题,
    是确保测试稳定性的关键。
"""
import os
import sys
import time
import struct
import unittest
import sqlite3
import tempfile
import shutil
from configparser import ConfigParser
from unittest.mock import patch, MagicMock

# 确保项目根目录在Python的模块搜索路径中, 以便正确导入hikyuu库
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

from hikyuu.gui.data.UseTdxImportToH5Thread import UseTdxImportToH5Thread
from hikyuu.gui.data.ImportTdxToH5Task import ImportTdxToH5Task

# 引入 get_real_tdx_filepath, 用于支持使用真实的测试数据
_current_dir = os.path.dirname(os.path.abspath(__file__))
if _current_dir not in sys.path:
    sys.path.append(_current_dir)
from test_init import get_real_tdx_filepath

# ------------------------------------------------------------------------------
# Test Helper Classes
# ------------------------------------------------------------------------------

class MockRunner:
    """
    模拟 PyQt 的信号接收器 (Slot), 用于在非GUI环境中捕获并记录导入线程
    发出的完成信号。这是连接测试主线程和导入线程状态的关键。
    """
    def __init__(self):
        """初始化状态，标记导入任务尚未完成。"""
        self.finished = False

    def on_message_from_thread(self, msg):
        """
        这是一个模拟的槽函数, 用于接收信号。当接收到表示任务结束或失败的
        特定消息时, 将 finished 标志位设为 True。
        """
        # 检查消息是否为表示任务结束的特定格式: ['HDF5_IMPORT', 'THREAD', 'FINISHED' 或 'FAILURE']
        if msg and len(msg) > 2 and msg[0] == 'HDF5_IMPORT' and msg[1] == 'THREAD' and msg[2] in ('FINISHED', 'FAILURE'):
            self.finished = True

class MockSignal:
    """
    模拟 PyQt 的 pyqtSignal, 允许在非GUI的测试环境中"发送"信号。
    它通过直接调用已连接的处理器 (handler) 来实现。
    """
    def __init__(self, handler):
        """
        初始化信号, 将其与一个处理器 (如 MockRunner.on_message_from_thread) 绑定。
        """
        self.handler = handler

    def emit(self, msg):
        """模拟发送信号, 实际是直接调用绑定的处理器函数。"""
        self.handler(msg)

def create_dummy_tdx_day_file(filepath, data):
    """
    创建一个符合通达信官方 .day 文件格式的二进制虚拟文件。
    这使得测试不依赖于真实的、可能变动的TDX客户端数据。
    """
    # 结构体格式:
    # < : 小端字节序 (Little-endian)
    # I : unsigned int (4字节), 用于日期和成交量
    # i : int (4字节), 用于价格 (开、高、低、收)
    # f : float (4字节), 用于成交金额
    # I : unsigned int (4字节), 用于保留字段
    record = struct.pack("<IiiiifII", data['date'], int(data['open'] * 100), int(data['high'] * 100),
                         int(data['low'] * 100), int(data['close'] * 100), data['amount'],
                         data['volume'], 0)
    with open(filepath, 'wb') as f:
        f.write(record)

# ------------------------------------------------------------------------------
# Main Test Class
# ------------------------------------------------------------------------------

class TestTdxImport(unittest.TestCase):
    """
    封装了所有针对TDX本地数据导入HDF5功能的测试用例。
    通过 setUp 和 tearDown 方法确保每个测试都在独立、干净的环境中运行。
    """
    def setUp(self):
        """
        在每个测试用例执行前, 初始化一个全新的、隔离的测试环境。
        这包括创建临时目录、模拟的TDX数据文件、HDF5目标目录以及一个
        包含必要元数据的SQLite数据库。
        """
        # 1. 创建一个唯一的临时目录, 避免测试间干扰和对本地环境的污染
        self.temp_dir = tempfile.mkdtemp(prefix="test_tdx_import_")
        
        # 2. 在临时目录中创建通达信源数据和HDF5目标数据的目录结构
        self.tdx_source_dir = os.path.join(self.temp_dir, "tdx_source")
        self.hdf5_dest_dir = os.path.join(self.temp_dir, "hdf5_dest")
        sh_dir = os.path.join(self.tdx_source_dir, "vipdoc", "sh", "lday")
        os.makedirs(sh_dir)
        os.makedirs(self.hdf5_dest_dir)
        
        # 3. 创建一个虚拟的通达信 .day 文件, 作为导入的数据源
        #    如果找到真实的通达信文件, 则优先使用, 否则创建虚拟文件
        dest_day_file = os.path.join(sh_dir, 'sh600000.day')
        real_day_file = get_real_tdx_filepath(code='600000', market='sh')
        if os.path.exists(real_day_file):
            shutil.copy(real_day_file, dest_day_file)
        else:
            dummy_data_sh = {'date': 20230103, 'open': 10.0, 'high': 11.5, 'low': 9.8, 'close': 11.2, 'amount': 550000.0, 'volume': 50000}
            create_dummy_tdx_day_file(dest_day_file, dummy_data_sh)

        # 4. 创建一个导入任务必需的、结构完整、内容正确的SQLite数据库
        #    这个数据库为导入过程提供了股票代码和市场等元数据。
        db_path = os.path.join(self.hdf5_dest_dir, "stock.db")
        con = sqlite3.connect(db_path)
        cur = con.cursor()
        cur.execute("CREATE TABLE market (marketid INTEGER, market TEXT, name TEXT, lastdate INTEGER)")
        cur.execute("INSERT INTO market VALUES (1, 'SH', '上海证券交易所', 0)")
        cur.execute("CREATE TABLE stock (stockid INTEGER, marketid INTEGER, code TEXT, valid INTEGER, type INTEGER, name TEXT, startdate INTEGER, enddate INTEGER)")
        cur.execute("INSERT INTO stock VALUES (1, 1, '600000', 1, 1, '浦发银行', 19991110, 99999999)")
        con.commit()
        con.close()

        # 5. 创建一个指向临时环境的配置对象 (ConfigParser)
        #    这个配置在运行时传递给导入线程, 指导其在哪里找到源文件和存放目标文件。
        self.config = ConfigParser()
        self.config.add_section('hdf5'); self.config.set('hdf5', 'enable', 'True'); self.config.set('hdf5', 'dir', self.hdf5_dest_dir)
        self.config.add_section('tdx'); self.config.set('tdx', 'enable', 'True'); self.config.set('tdx', 'dir', self.tdx_source_dir)
        self.config.add_section('ktype'); self.config.set('ktype', 'day', 'True'); self.config.set('ktype', 'min', 'False')
        self.config.add_section('quotation'); self.config.set('quotation', 'stock', 'True'); self.config.set('quotation', 'fund', 'False')
        self.config.add_section('weight'); self.config.set('weight', 'enable', 'False');
        self.config.add_section('finance'); self.config.set('finance', 'enable', 'False')
        self.config.add_section('pytdx'); self.config.set('pytdx', 'enable', 'False')

    def tearDown(self):
        """
        在每个测试用例执行后, 无论成功与否, 都会彻底清理临时目录和其中的所有文件,
        确保测试环境的纯净性。
        """
        shutil.rmtree(self.temp_dir)

    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.TdxHq_API')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_new_holidays')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_index_name')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_stock_name')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_create_database')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.search_best_tdx')
    def test_import_process_completes(self, mock_search, mock_create, mock_stock, mock_index, mock_holiday, mock_api):
        """
        冒烟测试 (Smoke Test): 验证导入流程可以被完整触发, 并且能够正常运行结束,
        最终发出 'FINISHED' 信号, 全程不发生崩溃或死锁。
        
        此测试的核心是验证流程的稳定性, 不关心导入数据的具体内容是否正确。
        通过模拟所有外部依赖, 确保测试的焦点仅在代码的执行路径上。
        """
        # 定义一个自定义的导入线程类, 其唯一目的是覆盖_run方法以实现同步执行
        class TestImporterThread(UseTdxImportToH5Thread):
            def _run(self):
                # 在测试中直接、同步地运行导入任务, 避免多进程带来的复杂性和竞争条件
                for task in self.tasks:
                    if isinstance(task, ImportTdxToH5Task) and task.market == 'SH':
                        task()
                # 任务完成后, 发送结束信号
                self.send_message(['THREAD', 'FINISHED'])

        # 配置mock, 模拟 search_best_tdx 返回符合其真实数据结构的虚拟服务器地址
        mock_search.return_value = [(True, 0.0, '127.0.0.1', 7709)]
        
        # 配置mock, 模拟 TdxHq_API 的行为
        mock_api_instance = MagicMock()
        mock_api_instance.connect.return_value = True
        mock_api.return_value = mock_api_instance

        # 实例化各个模拟组件和我们自定义的测试导入器
        runner = MockRunner()
        importer = TestImporterThread(None, self.config)
        importer.message = MockSignal(runner.on_message_from_thread)
        
        # 启动导入流程
        importer.run()

        # 等待导入结束信号, 设置10秒超时以防止测试在失败时永久挂起
        start_time = time.time()
        while not runner.finished and time.time() - start_time < 10:
            time.sleep(0.01)
        
        # 断言: 确认导入过程已正常结束
        self.assertTrue(runner.finished, "导入过程未能按预期在10秒内发出完成信号, 可能发生死锁或意外错误。")

    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.TdxHq_API')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_new_holidays')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_index_name')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_import_stock_name')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.sqlite_create_database')
    @patch('hikyuu.gui.data.UseTdxImportToH5Thread.search_best_tdx')
    def test_import_data_verification(self, mock_search, mock_create, mock_stock, mock_index, mock_holiday, mock_api):
        """
        核心功能测试: 验证从模拟的 .day 文件导入到HDF5文件中的数据,
        其数值和结构是否完全正确。这是确保导入逻辑正确性的关键测试。
        """
        try:
            import h5py
            import numpy as np
        except ImportError:
            self.skipTest("h5py 或 numpy 未安装, 跳过此核心功能测试")

        # 同样地, 定义一个用于测试的同步导入线程
        class TestImporterThread(UseTdxImportToH5Thread):
            def _run(self):
                for task in self.tasks:
                    if isinstance(task, ImportTdxToH5Task) and task.market == 'SH':
                        task()
                self.send_message(['THREAD', 'FINISHED'])

        # 配置mock, 模拟 search_best_tdx 返回符合其真实数据结构的虚拟服务器地址
        mock_search.return_value = [(True, 0.0, '127.0.0.1', 7709)]
        
        # 配置mock, 模拟 TdxHq_API 的行为
        mock_api_instance = MagicMock()
        mock_api_instance.connect.return_value = True
        mock_api.return_value = mock_api_instance

        # 实例化并运行导入器
        runner = MockRunner()
        importer = TestImporterThread(None, self.config)
        importer.message = MockSignal(runner.on_message_from_thread)
        importer.run()

        # 等待导入完成
        start_time = time.time()
        while not runner.finished and time.time() - start_time < 10:
            time.sleep(0.01)
        self.assertTrue(runner.finished, "导入过程未能按预期在10秒内完成, 数据校验无法进行。")

        # --- 数据校验阶段 ---
        h5_file_path = os.path.join(self.hdf5_dest_dir, 'sh_day.h5')
        self.assertTrue(os.path.exists(h5_file_path), f"目标HDF5文件未被创建于路径: {h5_file_path}")

        with h5py.File(h5_file_path, 'r') as f:
            # 1. 检查HDF5文件内部结构是否符合预期
            self.assertIn('data', f, "HDF5文件中必须存在顶层组 '/data'")
            data_group = f['/data']
            
            # 2. 检查对应股票的数据集是否存在 (注意: 导入时会自动转为大写)
            stock_code = 'SH600000'
            self.assertIn(stock_code, data_group, f"数据集中应存在股票代码为 '{stock_code}' 的表")
            
            dset = data_group[stock_code]
            
            # 如果记录数为1, 则认为是虚拟数据, 执行严格的逐字段校验
            if dset.shape[0] == 1:
                record = dset[0]
                # 3. 逐字段验证数据是否经过了正确的转换和存储
                expected_datetime = 202301030000
                self.assertEqual(record['datetime'], expected_datetime, f"日期时间戳不匹配. 预期: {expected_datetime}, 实际: {record['datetime']}")
                
                # 注意: HDF5中价格为放大1000倍的整数, 成交额单位为千元, 成交量单位为百股
                expected_open = 10.0
                actual_open = record['openPrice'] / 1000.0
                self.assertTrue(np.isclose(actual_open, expected_open), f"开盘价不匹配. 预期: {expected_open}, 实际: {actual_open}")

                expected_high = 11.5
                actual_high = record['highPrice'] / 1000.0
                self.assertTrue(np.isclose(actual_high, expected_high), f"最高价不匹配. 预期: {expected_high}, 实际: {actual_high}")

                expected_low = 9.8
                actual_low = record['lowPrice'] / 1000.0
                self.assertTrue(np.isclose(actual_low, expected_low), f"最低价不匹配. 预期: {expected_low}, 实际: {actual_low}")

                expected_close = 11.2
                actual_close = record['closePrice'] / 1000.0
                self.assertTrue(np.isclose(actual_close, expected_close), f"收盘价不匹配. 预期: {expected_close}, 实际: {actual_close}")

                expected_amount = 550  # 550000.0 / 1000
                actual_amount = record['transAmount']
                self.assertTrue(np.isclose(actual_amount, expected_amount), f"成交金额不匹配. 预期: {expected_amount} (千元), 实际: {actual_amount}")

                expected_volume = 500 # 50000 / 100
                actual_volume = record['transCount']
                self.assertTrue(np.isclose(actual_volume, expected_volume), f"成交量不匹配. 预期: {expected_volume} (百股), 实际: {actual_volume}")
            
            # 如果记录数大于1, 则认为是真实的通达信数据, 只做基本的存在性检查
            else:
                self.assertTrue(dset.shape[0] > 1, f"数据记录数量不正确. 预期: > 1, 实际: {dset.shape[0]}")

if __name__ == '__main__':
    # 允许此文件作为独立的脚本直接运行, 方便单独调试
    unittest.main()