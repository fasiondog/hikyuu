#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

import tables
import datetime
import sys
import math
import os
from pathlib import Path
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import threading
import queue
import time
import gc


class GUILogRedirector:
    """将控制台输出重定向到 GUI 日志窗口的类"""

    def __init__(self, log_func):
        """
        初始化日志重定向器

        参数:
            log_func: 用于写入日志的函数
        """
        self.log_func = log_func
        self.original_stdout = sys.stdout
        self.original_stderr = sys.stderr

    def write(self, text):
        """写入日志"""
        if text.strip():  # 忽略空行
            self.log_func(text.rstrip())

    def flush(self):
        """刷新缓冲区"""
        pass

    def start(self):
        """开始重定向"""
        sys.stdout = self
        sys.stderr = self

    def stop(self):
        """停止重定向"""
        sys.stdout = self.original_stdout
        sys.stderr = self.original_stderr


class ProgressCallback:
    """进度回调类，用于从底层函数更新 GUI 进度"""

    def __init__(self, update_func):
        """
        初始化进度回调

        参数:
            update_func: 更新函数，签名：update_func(current, total, stock_name)
        """
        self.update_func = update_func

    def update(self, current, total, stock_name=""):
        """更新进度"""
        if self.update_func:
            self.update_func(current, total, stock_name)


class ProgressBar:
    """增强的进度条类，支持显示详细信息和自适应宽度"""

    def __init__(self, description="进度", width=60):
        """
        初始化进度条

        参数:
            description: 进度描述文本
            width: 进度条宽度（字符数）
        """
        self.description = description
        self.width = width
        self.last_printed = ""

    def update(self, current, total, sub_description=None):
        """
        更新进度条显示

        参数:
            current: 当前进度值
            total: 总数值
            sub_description: 子描述信息（如股票名称、年份等）
        """
        percent = current / total * 100
        filled_length = int(self.width * current / total)
        bar = '=' * filled_length + '-' * (self.width - filled_length)

        # 构建显示文本
        if sub_description:
            display_text = f"\r{self.description}: [{bar}] {percent:5.1f}% ({current}/{total}) - {sub_description}"
        else:
            display_text = f"\r{self.description}: [{bar}] {percent:5.1f}% ({current}/{total})"

        # 只在实际内容变化时打印，避免闪烁
        if display_text != self.last_printed:
            sys.stdout.write(display_text)
            sys.stdout.flush()
            self.last_printed = display_text

    def finish(self, message="完成"):
        """显示完成消息"""
        print(f"\n{message}")
        self.last_printed = ""


class FiveMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class OneMinDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class DayDataRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    openPrice = tables.UInt32Col()       # IGNORE:E1101
    highPrice = tables.UInt32Col()       # IGNORE:E1101
    lowPrice = tables.UInt32Col()        # IGNORE:E1101
    closePrice = tables.UInt32Col()      # IGNORE:E1101
    transAmount = tables.UInt64Col()     # IGNORE:E1101
    transCount = tables.UInt64Col()      # IGNORE:E1101


class TimeLineRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    price = tables.UInt32Col()           # IGNORE:E1101
    vol = tables.UInt64Col()             # IGNORE:E1101


class TransRecordH5File(tables.IsDescription):
    datetime = tables.UInt64Col()        # IGNORE:E1101
    price = tables.UInt32Col()           # IGNORE:E1101
    vol = tables.UInt64Col()             # IGNORE:E1101
    buyorsell = tables.UInt8Col()        # IGNORE:E1101


def extract_year_month_day(datetime_value):
    """从 datetime 值中提取年月日

    支持不同长度的 datetime 格式：
    - 12 位：YYYYMMDDHHmm（日线、分钟线、分时）
    - 14 位：YYYYMMDDHHmmss（分笔数据）

    参数:
        datetime_value: datetime 值

    返回:
        tuple: (year, month, day)
    """
    dt_str = str(datetime_value)
    if len(dt_str) == 12:
        year = int(dt_str[:4])
        month = int(dt_str[4:6])
        day = int(dt_str[6:8])
    elif len(dt_str) >= 14:
        year = int(dt_str[:4])
        month = int(dt_str[4:6])
        day = int(dt_str[6:8])
    else:
        year, month, day = 0, 0, 0
    return year, month, day


def extract_year(datetime_value, data_type='time'):
    """从 datetime 值中提取年份
    
    支持不同长度的 datetime 格式：
    - 12 位：YYYYMMDDHHmm（日线、分钟线、分时）→ 直接除 100000000
    - 14 位：YYYYMMDDHHmmss（分笔数据）→ 直接除 10000000000
    
    参数:
        datetime_value: datetime 值
        data_type: 数据类型 ('time', 'trans', 'day', '1min', '5min')
        
    返回:
        int: 年份
    """
    # 对于分笔数据（14 位），使用不同的除数
    if data_type == 'trans':
        return datetime_value // 10000000000  # 14 位：YYYYMMDDHHmmss
    else:
        return datetime_value // 100000000    # 12 位：YYYYMMDDHHmm


def get_existing_stock_dates(dest_dir, market_prefix, data_type, stop_flag=None):
    """获取拆分目录下已有文件中每只股票的最大 datetime 值

    参数:
        dest_dir: 目标目录
        market_prefix: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型（如 day, 5min, trans 等）
        stop_flag: 停止标志检查函数，返回 True 表示应该停止

    返回:
        dict: {stock_name: max_datetime}，如果目录为空则返回空字典

    重要说明:
        必须遍历所有年份文件，找到每只股票的最大 datetime 值，
        因为某些年份文件可能不完整（如只有部分月份的数据）。
        不能假设"某年的最后一条记录"就是"该年的最大 datetime"。
    """
    import re

    stock_max_dates = {}

    # 检查目标目录是否存在
    if not os.path.exists(dest_dir):
        return stock_max_dates

    # 查找所有匹配的文件
    pattern = re.compile(rf'^{market_prefix}_{data_type}_\d{{4}}\.h5$')
    existing_files = []

    for filename in os.listdir(dest_dir):
        # 在扫描文件列表时也要检查停止标志
        if stop_flag and stop_flag():
            print(f"\n用户取消扫描已存在文件")
            raise InterruptedError("用户取消拆分操作")

        if pattern.match(filename):
            existing_files.append(os.path.join(dest_dir, filename))

    if not existing_files:
        return stock_max_dates

    print(f"\n发现 {len(existing_files)} 个已存在的拆分文件")

    # 打开每个文件，读取每只股票的所有记录的 datetime 值，找出最大值
    files_processed = 0
    for file_path in existing_files:
        # 在打开每个文件前检查停止标志
        if stop_flag and stop_flag():
            print(f"\n用户取消扫描已存在文件，已处理 {files_processed}/{len(existing_files)} 个文件")
            raise InterruptedError("用户取消拆分操作")

        try:
            f = tables.open_file(file_path, 'r')

            # 遍历/data 组下的所有股票表
            try:
                data_group = f.get_node('/data')
                stocks_in_file = 0

                for table in data_group:
                    if isinstance(table, tables.Table):
                        stock_name = table._v_name
                        stocks_in_file += 1

                        # 每处理 10 只股票检查一次停止标志
                        if stocks_in_file % 10 == 0 and stop_flag and stop_flag():
                            print(f"\n用户取消扫描，当前文件：{os.path.basename(file_path)}，已处理 {stocks_in_file} 只股票")
                            f.close()
                            raise InterruptedError("用户取消拆分操作")

                        # 读取该股票表的所有记录，找出最大 datetime
                        if table.nrows > 0:
                            max_datetime = 0
                            records_checked = 0

                            for row in table:
                                dt = row['datetime']
                                if dt > max_datetime:
                                    max_datetime = dt

                                records_checked += 1
                                # 每处理 1000 条记录检查一次停止标志
                                if records_checked % 1000 == 0 and stop_flag and stop_flag():
                                    print(f"\n用户取消扫描，当前股票：{stock_name}，已检查 {records_checked} 条记录")
                                    f.close()
                                    raise InterruptedError("用户取消拆分操作")

                            # 更新该股票的最大日期（取所有年份文件中的最大值）
                            if stock_name not in stock_max_dates or max_datetime > stock_max_dates[stock_name]:
                                stock_max_dates[stock_name] = max_datetime
            except Exception as e:
                print(f"  警告：无法访问/data 组：{e}")

            f.close()
            files_processed += 1

            # 每处理完一个文件，打印进度
            if files_processed % 5 == 0 or files_processed == len(existing_files):
                print(f"  已扫描 {files_processed}/{len(existing_files)} 个文件...")

        except Exception as e:
            print(f"读取文件 {os.path.basename(file_path)} 时出错：{e}")
            continue

    if stock_max_dates:
        print(f"已加载 {len(stock_max_dates)} 只股票的最大 datetime 值（跨所有年份文件）")

    return stock_max_dates


def _split_data_by_year(src_file_name, dest_dir, data_type, table_desc, stop_flag=None, progress_callback=None):
    """
    通用的按年份拆分数据函数（流式处理版本）

    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
        data_type: 数据类型字符串，用于文件名
        table_desc: 表结构描述类
        stop_flag: 停止标志检查函数
        progress_callback: 进度回调对象，用于更新 GUI
    """
    print(f"正在进行，请稍候.....")
    print(f"源文件：{src_file_name}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{data_type}")

    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)

    # 获取市场前缀
    market_prefix = os.path.basename(src_file_name).split('_')[0]

    # 直接进行完整拆分
    print(f"\n将进行完整拆分")
    existing_stock_dates = {}

    # 打开源文件
    src_hdf5 = tables.open_file(src_file_name, mode='r',
                                filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))

    try:
        # 第一步：统计股票总数（使用 walk_nodes 迭代器，惰性求值）
        # walk_nodes 返回生成器，不会一次性加载所有节点到内存
        stock_tables_iter = src_hdf5.walk_nodes('/data')
        
        # 跳过 /data 组本身（第一个节点）
        first_node = next(stock_tables_iter, None)
        
        # 统计总数（这会消耗一次遍历）
        total_stocks = sum(1 for _ in stock_tables_iter)
        
        # 如果第一个节点不是/data 组，说明它是股票表，需要计入总数
        if first_node is not None and not (hasattr(first_node, '_v_name') and first_node._v_name == 'data'):
            total_stocks += 1
        
        print(f"共找到 {total_stocks} 个股票数据表")
        
        # 第二步：按年份分组处理（外层循环是年份，内层循环是股票）
        # 这样每个年份文件只打开一次！
        
        # 先收集所有需要处理的年份范围
        print(f"\n分析数据年份范围...")
        min_year = float('inf')
        max_year = float('-inf')
        
        # 重新创建迭代器遍历年份范围
        stock_tables_iter = src_hdf5.walk_nodes('/data')
        next(stock_tables_iter, None)  # 跳过 /data 组
        
        for src_table in stock_tables_iter:
            if hasattr(src_table, '_v_name') and src_table._v_name == 'data':
                continue  # 跳过 /data 组
            
            stock_name = src_table._v_name
            existing_last_date = existing_stock_dates.get(stock_name, 0)
            
            # 快速扫描该股票的年份范围（只检查首尾记录）
            if len(src_table) > 0:
                # 第一条记录的年份
                first_datetime = src_table[0]['datetime']
                first_year = extract_year(first_datetime, data_type)
                
                # 最后一条记录的年份
                last_datetime = src_table[-1]['datetime']
                last_year = extract_year(last_datetime, data_type)
                
                # 考虑增量更新
                if existing_last_date > 0:
                    existing_last_year = extract_year(existing_last_date, data_type)
                    if first_year >= existing_last_year:
                        continue  # 所有数据都已存在
                    # 只需要处理到 existing_last_year 之前的数据
                    last_year = min(last_year, existing_last_year - 1)
                
                min_year = min(min_year, first_year)
                max_year = max(max_year, last_year)
        
        if min_year == float('inf'):
            print(f"没有需要处理的数据")
            return
        
        print(f"数据年份范围：{min_year} - {max_year}")
        
        # 第三步：按年份处理（外层循环）
        processed_stocks = set()
        stock_processed = 0
        total_process_count = 0  # 记录总处理次数（包括同一股票的不同年份）
        
        # 计算预计的总任务数（年份数 × 股票数）
        estimated_total_tasks = (max_year - min_year + 1) * total_stocks
        
        for year in range(min_year, max_year + 1):
            print(f"\n{'='*80}")
            print(f"处理年份：{year}")
            
            # 打开/创建该年份的文件（整个年份只打开一次！）
            dest_file_name = os.path.join(dest_dir, f"{market_prefix}_{data_type}_{year}.h5")
            file_exists = os.path.exists(dest_file_name)
            
            if file_exists:
                dest_hdf5 = tables.open_file(dest_file_name, mode="a",
                                             filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
            else:
                dest_hdf5 = tables.open_file(dest_file_name, mode="w",
                                             filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
                # 创建 /data 组
                dest_group = dest_hdf5.create_group("/", "data", "Stock Data")
            
            try:
                # 内层循环：遍历所有股票，处理该年份的数据
                # 注意：需要重新创建迭代器，因为 walk_nodes 是消耗型的
                stock_tables_iter_inner = src_hdf5.walk_nodes('/data')
                next(stock_tables_iter_inner, None)  # 跳过 /data 组
                
                for src_table in stock_tables_iter_inner:
                    if hasattr(src_table, '_v_name') and src_table._v_name == 'data':
                        continue  # 跳过 /data 组
                    
                    stock_name = src_table._v_name
                    
                    # 从头开始处理所有数据
                    start_row_index = 0
                    
                    # 检查停止标志
                    if stop_flag and stop_flag():
                        print(f"\n用户取消处理，当前股票：{stock_name}")
                        raise InterruptedError("用户取消拆分操作")
                    
                    # 获取或创建股票表
                    try:
                        dest_table = dest_hdf5.get_node("/data", stock_name)
                    except tables.NoSuchNodeError:
                        # 新建股票表
                        table_title = f"{table_desc.__doc__ or 'Stock Data'}"
                        dest_table = dest_hdf5.create_table("/data", name=stock_name,
                                                            description=table_desc, title=table_title)
                    
                    # 处理该股票在该年份的数据
                    new_records_count = 0
                    for idx in range(start_row_index, len(src_table)):
                        row = src_table[idx]
                        try:
                            datetime_val = row['datetime']
                            row_year = extract_year(datetime_val, data_type)
                            
                            # 只处理当前年份的数据
                            if row_year != year:
                                continue
                            
                            # 复制记录的所有字段值
                            dest_row = dest_table.row
                            for col_name in src_table.colnames:
                                try:
                                    dest_row[col_name] = row[col_name]
                                except KeyError:
                                    pass
                            dest_row.append()
                            
                            new_records_count += 1
                            
                            # 每处理 100 条记录，检查一次停止标志
                            if new_records_count % 100 == 0 and stop_flag and stop_flag():
                                print(f"\n用户取消处理，当前股票：{stock_name}，已处理 {new_records_count} 条记录")
                                raise InterruptedError("用户取消拆分操作")
                        
                        except InterruptedError:
                            raise
                        except Exception:
                            continue
                    
                    # ✓ 修复：无论是否为新股票，都要更新进度显示
                    # 原始问题：只有第一年（新股票）才更新进度，导致后续年份进度条不动
                    # 修复方案：每次处理股票都更新进度，让用户看到持续的进度反馈
                    
                    # 增加总处理计数
                    total_process_count += 1
                    
                    # 如果是新股票，增加已处理股票计数
                    is_new_stock = stock_name not in processed_stocks
                    if is_new_stock:
                        processed_stocks.add(stock_name)
                        stock_processed += 1
                    
                    # 更新进度回调（显示当前处理的股票和年份）
                    # 使用 total_process_count 计算进度百分比，确保进度条持续增长
                    if progress_callback:
                        # 计算当前进度的百分比（基于总处理次数）
                        current_progress_pct = int((total_process_count / estimated_total_tasks) * 100) if estimated_total_tasks > 0 else 0
                        
                        # 调用回调，传入基于实际处理次数的进度
                        progress_callback.update(
                            total_process_count,           # 当前已处理的总次数
                            estimated_total_tasks,         # 预计的总任务数
                            f"{stock_name} ({year})"      # 显示股票名和年份
                        )
                    
                    # 打印该股票的处理情况（仅当年有数据时）
                    if new_records_count > 0:
                        if is_new_stock:
                            if existing_last_date > 0:
                                print(f"  股票 {stock_name}: 新增 {new_records_count:,} 条记录 ({year})")
                            else:
                                print(f"  股票 {stock_name}: 处理 {new_records_count:,} 条记录 ({year})")
                        else:
                            # 已处理过的股票（不同年份）
                            print(f"  {year}年 - {stock_name}: {new_records_count:,} 条记录")
                
            finally:
                # 关闭该年份文件
                if dest_hdf5 is not None:
                    dest_hdf5.close()
        
        print(f"\n{'='*80}")
        print(f"拆分完成！共处理 {len(processed_stocks)} 只股票")
        gc.collect()

    except InterruptedError:
        # 重新抛出 InterruptedError，让外层处理
        raise
    except Exception as e:
        # 记录其他异常
        print(f"\n处理过程中发生错误：{type(e).__name__}: {str(e)}")
        import traceback
        print(traceback.format_exc())
        raise
    finally:
        # 确保无论是否发生异常，都会关闭文件
        src_hdf5.close()


def split_kdata_by_year(src_file_name, dest_dir, data_type='day', stop_flag=None, progress_callback=None):
    """
    将 K 线数据文件（日 K、分钟线）按年份拆分（流式处理版本）

    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
        data_type: 数据类型（'day', '1min', '5min'）
        stop_flag: 停止标志检查函数
        progress_callback: 进度回调对象，用于更新 GUI
    """
    print(f"\n{'='*80}")
    print(f"正在进行 K 线数据拆分...")
    print(f"源文件：{src_file_name}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{data_type}")

    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)

    # 获取市场前缀
    market_prefix = os.path.basename(src_file_name).split('_')[0]

    # 根据数据类型选择表结构描述类
    if data_type == 'day':
        table_desc = DayDataRecordH5File
    elif data_type == '5min':
        table_desc = FiveMinDataRecordH5File
    elif data_type == '1min':
        table_desc = OneMinDataRecordH5File
    else:
        raise ValueError(f"不支持的数据类型：{data_type}")

    # 复用 _split_data_by_year 的实现，确保一致性
    return _split_data_by_year(
        src_file_name=src_file_name,
        dest_dir=dest_dir,
        data_type=data_type,
        table_desc=table_desc,
        stop_flag=stop_flag,
        progress_callback=progress_callback
    )


def split_timeline_by_year(src_file_name, dest_dir, stop_flag=None, progress_callback=None):
    """
    将分时线数据文件按年份拆分

    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
        stop_flag: 停止标志检查函数
        progress_callback: 进度回调对象，用于更新 GUI
    """
    _split_data_by_year(src_file_name, dest_dir, 'time', TimeLineRecordH5File,
                        stop_flag=stop_flag, progress_callback=progress_callback)


def split_trans_by_year(src_file_name, dest_dir, stop_flag=None, progress_callback=None):
    """
    将分笔成交数据文件按年份拆分

    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
        stop_flag: 停止标志检查函数
        progress_callback: 进度回调对象，用于更新 GUI
    """
    _split_data_by_year(src_file_name, dest_dir, 'trans', TransRecordH5File,
                        stop_flag=stop_flag, progress_callback=progress_callback)

# 移除了 split_single_file 函数，因为我们现在统一按年份合并所有股票


class SplitApp:
    """数据文件拆分 GUI 应用"""

    def __init__(self, root):
        self.root = root
        self.root.title("Hikyuu 数据文件按年份拆分工具")
        self.root.geometry("800x600")

        # 变量
        self.src_dir = tk.StringVar()
        self.dest_dir = tk.StringVar()
        self.process_day = tk.BooleanVar(value=True)
        self.process_1min = tk.BooleanVar(value=False)
        self.process_5min = tk.BooleanVar(value=False)
        self.process_timeline = tk.BooleanVar(value=False)
        self.process_trans = tk.BooleanVar(value=False)

        self.log_queue = queue.Queue()
        self.is_processing = False

        self._create_widgets()
        self._start_log_processor()

    def _create_widgets(self):
        """创建界面组件"""
        # 主框架
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        # 源目录选择
        src_frame = ttk.LabelFrame(main_frame, text="源数据目录", padding="5")
        src_frame.grid(row=0, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Entry(src_frame, textvariable=self.src_dir, width=60).grid(row=0, column=0, padx=5)
        ttk.Button(src_frame, text="浏览...", command=self._browse_src).grid(row=0, column=1)

        # 目标目录选择
        dest_frame = ttk.LabelFrame(main_frame, text="目标目录", padding="5")
        dest_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Entry(dest_frame, textvariable=self.dest_dir, width=60).grid(row=0, column=0, padx=5)
        ttk.Button(dest_frame, text="浏览...", command=self._browse_dest).grid(row=0, column=1)

        # 数据类型选择
        type_frame = ttk.LabelFrame(main_frame, text="数据类型", padding="5")
        type_frame.grid(row=2, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        ttk.Checkbutton(type_frame, text="日线 (day)", variable=self.process_day).grid(row=0, column=0, padx=10)
        ttk.Checkbutton(type_frame, text="1 分钟线 (1min)", variable=self.process_1min).grid(row=0, column=1, padx=10)
        ttk.Checkbutton(type_frame, text="5 分钟线 (5min)", variable=self.process_5min).grid(row=0, column=2, padx=10)
        ttk.Checkbutton(type_frame, text="分时 (time)", variable=self.process_timeline).grid(row=0, column=3, padx=10)
        ttk.Checkbutton(type_frame, text="分笔 (trans)", variable=self.process_trans).grid(row=0, column=4, padx=10)

        # 全选/全不选按钮
        select_frame = ttk.Frame(type_frame)
        select_frame.grid(row=1, column=0, columnspan=5, pady=5)

        ttk.Button(select_frame, text="全选", command=self._select_all).pack(side=tk.LEFT, padx=5)
        ttk.Button(select_frame, text="全不选", command=self._select_none).pack(side=tk.LEFT, padx=5)

        # 开始按钮
        button_frame = ttk.Frame(main_frame)
        button_frame.grid(row=3, column=0, columnspan=3, pady=10)

        self.start_button = ttk.Button(button_frame, text="开始拆分", command=self._start_split, width=20)
        self.start_button.pack(side=tk.LEFT, padx=5)

        self.stop_button = ttk.Button(button_frame, text="停止", command=self._stop_split, width=20, state=tk.DISABLED)
        self.stop_button.pack(side=tk.LEFT, padx=5)

        # 进度条
        progress_frame = ttk.LabelFrame(main_frame, text="进度", padding="5")
        progress_frame.grid(row=4, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=5)

        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(progress_frame, variable=self.progress_var, maximum=100, mode='determinate')
        self.progress_bar.grid(row=0, column=0, sticky=(tk.W, tk.E))

        self.status_label = ttk.Label(progress_frame, text="就绪", font=('Arial', 9))
        self.status_label.grid(row=1, column=0, sticky=tk.W, pady=5)

        # 详细信息标签（显示当前处理的文件、股票等信息）
        self.detail_label = ttk.Label(progress_frame, text="", font=('Arial', 8), foreground='gray')
        self.detail_label.grid(row=2, column=0, sticky=tk.W, pady=2)

        # 日志窗口
        log_frame = ttk.LabelFrame(main_frame, text="处理日志", padding="5")
        log_frame.grid(row=5, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=5)

        self.log_text = tk.Text(log_frame, height=15, width=80, state=tk.DISABLED)
        self.log_text.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        scrollbar = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.log_text.yview)
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        self.log_text.configure(yscrollcommand=scrollbar.set)

        # 配置网格权重
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(5, weight=1)
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)
        progress_frame.columnconfigure(0, weight=1)

    def _browse_src(self):
        """浏览源目录"""
        directory = filedialog.askdirectory(title="选择源数据目录")
        if directory:
            self.src_dir.set(directory)
            self._log(f"源目录：{directory}")

    def _browse_dest(self):
        """浏览目标目录"""
        directory = filedialog.askdirectory(title="选择目标目录")
        if directory:
            self.dest_dir.set(directory)
            self._log(f"目标目录：{directory}")

    def _select_all(self):
        """全选"""
        self.process_day.set(True)
        self.process_1min.set(True)
        self.process_5min.set(True)
        self.process_timeline.set(True)
        self.process_trans.set(True)

    def _select_none(self):
        """全不选"""
        self.process_day.set(False)
        self.process_1min.set(False)
        self.process_5min.set(False)
        self.process_timeline.set(False)
        self.process_trans.set(False)

    def _log(self, message):
        """添加日志"""
        self.log_queue.put(message)

    def _start_log_processor(self):
        """启动日志处理器（批量处理，提高响应性）"""
        def process_logs():
            try:
                messages = []
                # 批量获取队列中的所有消息
                while True:
                    try:
                        message = self.log_queue.get_nowait()
                        messages.append(message)
                    except queue.Empty:
                        break

                # 批量更新 GUI
                if messages:
                    self.log_text.configure(state=tk.NORMAL)
                    for message in messages:
                        self.log_text.insert(tk.END, message + "\n")
                    self.log_text.see(tk.END)
                    self.log_text.configure(state=tk.DISABLED)

            except Exception:
                pass  # 忽略错误

            # 缩短检查间隔，提高响应性
            self.root.after(50, process_logs)

        self.root.after(50, process_logs)

    def _update_progress(self, value, status="", detail=""):
        """更新进度（确保在主线程执行）"""
        # 使用 after 确保在主线程更新
        self.root.after(0, lambda: self._do_update_progress(value, status, detail))

    def _do_update_progress(self, value, status="", detail=""):
        """实际执行进度更新（内部方法）"""
        try:
            self.progress_var.set(value)
            if status:
                self.status_label.config(text=status)
            if detail:
                self.detail_label.config(text=detail)
        except Exception:
            # 忽略 GUI 更新错误，避免影响处理流程
            pass

    def _validate_selection(self):
        """验证选择"""
        if not self.src_dir.get():
            messagebox.showerror("错误", "请选择源数据目录！")
            return False

        if not self.dest_dir.get():
            messagebox.showerror("错误", "请选择目标目录！")
            return False

        if not os.path.isdir(self.src_dir.get()):
            messagebox.showerror("错误", "源目录不存在！")
            return False

        # 检查是否至少选择了一种数据类型
        if not any([self.process_day.get(), self.process_1min.get(),
                   self.process_5min.get(), self.process_timeline.get(),
                   self.process_trans.get()]):
            messagebox.showerror("错误", "请至少选择一种数据类型！")
            return False

        return True

    def _find_data_files(self):
        """查找源目录下的数据文件（仅扫描源目录本身，不递归子目录）"""
        files = {}
        src_path = Path(self.src_dir.get())

        # 查找各种类型的数据文件（只扫描源目录本身）
        patterns = {
            'day': ['*_day.h5', '*_DAY.h5'],
            '1min': ['*_1min.h5', '*_1MIN.h5'],
            '5min': ['*_5min.h5', '*_5MIN.h5'],
            'time': ['*_time.h5', '*_TIME.h5'],
            'trans': ['*_trans.h5', '*_TRANS.h5']
        }

        for data_type, pattern_list in patterns.items():
            for pattern in pattern_list:
                # 使用 iterdir() 只遍历源目录本身，不递归子目录
                for item in src_path.iterdir():
                    if item.is_file() and item.match(pattern):
                        if data_type not in files:
                            files[data_type] = []
                        files[data_type].append(item)

        return files

    def _process_file(self, src_file, dest_dir, data_type, stop_flag=None, progress_callback=None):
        """处理单个文件（支持可中断和日志重定向）"""
        try:
            self._log(f"\n处理文件：{src_file.name}")

            # 创建日志重定向器
            redirector = GUILogRedirector(self._log)
            redirector.start()

            try:
                if data_type == 'time':
                    split_timeline_by_year(str(src_file), str(dest_dir), stop_flag=stop_flag,
                                           progress_callback=progress_callback)
                elif data_type == 'trans':
                    split_trans_by_year(str(src_file), str(dest_dir), stop_flag=stop_flag,
                                        progress_callback=progress_callback)
                else:
                    split_kdata_by_year(str(src_file), str(dest_dir), data_type=data_type,
                                        stop_flag=stop_flag, progress_callback=progress_callback)

                self._log(f"✓ {src_file.name} 处理完成")
                return True
            finally:
                redirector.stop()

        except InterruptedError:
            # 用户主动取消
            self._log(f"✗ {src_file.name} 处理已取消")
            return False
        except Exception as e:
            self._log(f"✗ {src_file.name} 处理失败：{str(e)}")
            import traceback
            self._log(traceback.format_exc())
            return False

    def _start_split(self):
        """开始拆分"""
        if not self._validate_selection():
            return

        if self.is_processing:
            messagebox.showwarning("警告", "正在处理中，请稍候...")
            return

        self.is_processing = True
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)

        # 在新线程中执行
        thread = threading.Thread(target=self._run_split, daemon=True)
        thread.start()

    def _run_split(self):
        """运行拆分任务（支持可中断和实时进度）"""
        try:
            self._log("=" * 60)
            self._log("开始处理...")
            self._update_progress(0, "正在扫描文件...")

            # 查找文件
            files = self._find_data_files()

            if not files:
                self._log("未找到任何数据文件！")
                self._update_progress(100, "未找到文件")
                # 在主线程显示对话框
                self.root.after(0, lambda: messagebox.showinfo("提示", "源目录中未找到任何数据文件！"))
                return

            # 统计要处理的文件
            total_files = 0
            to_process = []

            if self.process_day.get() and 'day' in files:
                to_process.extend([(f, 'day') for f in files['day']])
            if self.process_1min.get() and '1min' in files:
                to_process.extend([(f, '1min') for f in files['1min']])
            if self.process_5min.get() and '5min' in files:
                to_process.extend([(f, '5min') for f in files['5min']])
            if self.process_timeline.get() and 'time' in files:
                to_process.extend([(f, 'time') for f in files['time']])
            if self.process_trans.get() and 'trans' in files:
                to_process.extend([(f, 'trans') for f in files['trans']])

            total_files = len(to_process)
            if total_files == 0:
                self._log("没有需要处理的文件！")
                self._update_progress(100, "无需处理")
                self.root.after(0, lambda: messagebox.showinfo("提示", "没有需要处理的文件！"))
                return

            self._log(f"找到 {total_files} 个文件需要处理")

            # 处理每个文件
            success_count = 0
            for i, (src_file, data_type) in enumerate(to_process):
                # 检查停止标志
                if not self.is_processing:
                    self._log("\n用户取消处理")
                    break

                # 更新进度（每 5% 更新一次，避免过于频繁）
                progress = int(((i + 1) / total_files) * 100)
                self._update_progress(progress, f"处理中：{i+1}/{total_files}", f"{src_file.name}")

                # 创建目标目录（按市场）
                market = src_file.stem.split('_')[0]
                dest_subdir = Path(self.dest_dir.get()) / market / data_type
                dest_subdir.mkdir(parents=True, exist_ok=True)

                # 处理文件（传入停止标志检查函数和进度回调）
                # 注意：必须在循环外部定义 lambda，避免闭包问题
                def stop_check(): return not self.is_processing

                # ✓ 修复：使用默认参数捕获当前 i 的值，避免闭包问题
                current_file_index = i

                # 创建进度回调函数，用于更新股票进度
                def on_stock_progress(current, total, stock_name="", file_index=current_file_index):
                    if current > 0 and total > 0:
                        # ✓ 修复：使用传入的 current 和 total 计算进度
                        # current: 当前已处理的总次数（包括不同年份）
                        # total: 预计的总任务数（年份数 × 股票数）
                        progress_pct = int((current / total) * 100)
                        
                        # 限制最大进度为 99%，避免提前显示 100%
                        display_progress = min(progress_pct, 99)
                        
                        self._update_progress(
                            display_progress,
                            f"处理 {current}/{total}",
                            f"{src_file.name} - {stock_name}"
                        )
                    else:
                        # 回退到文件进度
                        current_progress = int(((file_index + 1) / total_files) * 100)
                        self._update_progress(
                            current_progress,
                            f"处理中：{file_index+1}/{total_files}",
                            f"{src_file.name}"
                        )

                callback = ProgressCallback(on_stock_progress)

                if self._process_file(src_file, dest_subdir, data_type, stop_flag=stop_check, progress_callback=callback):
                    success_count += 1

                # 每处理完一个文件，让出一点时间给 GUI
                import time
                time.sleep(0.001)  # 仅 1 毫秒

            # 完成
            if self.is_processing:
                self._update_progress(100, "处理完成")
                self._log("=" * 60)
                self._log(f"处理完成！成功：{success_count}/{total_files}")
                # 在主线程显示对话框
                self.root.after(0, lambda: messagebox.showinfo("完成", f"数据拆分完成！\n成功：{success_count}/{total_files} 文件"))
            else:
                self._update_progress(0, "已取消")
                self._log("=" * 60)
                self._log(f"处理已取消！成功：{success_count}/{total_files}")
                self.root.after(0, lambda: messagebox.showinfo("取消", f"处理已取消！\n成功：{success_count}/{total_files} 文件"))

        except Exception as e:
            self._log(f"\n发生错误：{str(e)}")
            self._update_progress(0, "错误")
            import traceback
            self._log(traceback.format_exc())
            # 在主线程显示对话框
            self.root.after(0, lambda: messagebox.showerror("错误", f"处理过程中发生错误：\n{str(e)}"))

        finally:
            self.is_processing = False
            # 恢复按钮状态
            self.root.after(0, lambda: self.start_button.config(state=tk.NORMAL))
            self.root.after(0, lambda: self.stop_button.config(state=tk.DISABLED))

    def _stop_split(self):
        """停止拆分"""
        if messagebox.askyesno("确认", "确定要停止当前处理吗？"):
            self.is_processing = False
            self._log("\n正在停止...")


def main():
    """主函数"""
    root = tk.Tk()
    app = SplitApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
