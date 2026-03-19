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
    - 12 位：YYYYMMDDHHmm（日线、分钟线）
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


def get_existing_stock_dates(dest_dir, market_prefix, data_type):
    """获取拆分目录下已有文件中每只股票的最后日期
    
    参数:
        dest_dir: 目标目录
        market_prefix: 市场前缀（如 sh, sz, bj）
        data_type: 数据类型（如 day, 5min, trans 等）
        
    返回:
        dict: {stock_name: last_datetime}，如果目录为空则返回空字典
    """
    import re
    
    stock_last_dates = {}
    
    # 检查目标目录是否存在
    if not os.path.exists(dest_dir):
        return stock_last_dates
    
    # 查找所有匹配的文件
    pattern = re.compile(rf'^{market_prefix}_{data_type}_\d{{4}}\.h5$')
    existing_files = []
    
    for filename in os.listdir(dest_dir):
        if pattern.match(filename):
            existing_files.append(os.path.join(dest_dir, filename))
    
    if not existing_files:
        return stock_last_dates
    
    print(f"\n发现 {len(existing_files)} 个已存在的拆分文件")
    
    # 打开每个文件，读取每只股票的最后日期
    for file_path in existing_files:
        try:
            f = tables.open_file(file_path, 'r')
            
            # 遍历/data 组下的所有股票表
            try:
                data_group = f.get_node('/data')
                for table in data_group:
                    if isinstance(table, tables.Table):
                        stock_name = table._v_name
                        
                        # 获取该表的最后一条记录
                        if table.nrows > 0:
                            last_row = table[table.nrows - 1]
                            last_datetime = last_row['datetime']
                            
                            # 更新该股票的最后日期（取最大值）
                            if stock_name not in stock_last_dates or last_datetime > stock_last_dates[stock_name]:
                                stock_last_dates[stock_name] = last_datetime
            except Exception as e:
                print(f"  警告：无法访问/data 组：{e}")
            
            f.close()
            
        except Exception as e:
            print(f"读取文件 {os.path.basename(file_path)} 时出错：{e}")
            continue
    
    if stock_last_dates:
        print(f"已加载 {len(stock_last_dates)} 只股票的历史最后日期")
    
    return stock_last_dates




def _split_data_by_year(src_file_name, dest_dir, data_type, table_desc):
    """
    通用的按年份拆分数据函数
    
    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
        data_type: 数据类型字符串，用于文件名
        table_desc: 表结构描述类
    """
    print(f"正在进行，请稍候.....")
    print(f"源文件：{src_file_name}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{data_type}")
    
    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)
    
    # 获取市场前缀
    market_prefix = os.path.basename(src_file_name).split('_')[0]
    
    # 获取已存在文件中股票的最后日期（增量拆分）
    print(f"\n检查增量拆分...")
    existing_stock_dates = get_existing_stock_dates(dest_dir, market_prefix, data_type)
    
    if existing_stock_dates:
        print(f"将进行增量拆分，只提取新增数据")
    else:
        print(f"未发现历史数据，将进行完整拆分")
    
    # 打开源文件
    src_hdf5 = tables.open_file(src_file_name, mode='r',
                               filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
    
    try:
        # 获取所有表
        all_tables = [x for x in src_hdf5.walk_nodes("/data")]
        total_tables = len(all_tables)
        
        # 特殊情况：如果是根目录下有 /data 节点，则显示其子节点数
        try:
            data_nodes = [node._v_name for node in src_hdf5.list_nodes('/data')]
            print(f"共找到 {len(data_nodes)} 个股票数据表")
        except:
            print(f"共找到 {total_tables - 1} 个股票数据表")
        
        # 统计总体进度
        total_stocks = total_tables - 1  # 减去/data 组本身
        stock_processed = 0
        
        # 遍历所有股票表，每次只处理一只股票
        for i in range(1, total_tables):
            src_table = all_tables[i]
            # 使用 _v_name 属性获取节点名称（股票代码）
            stock_name = src_table._v_name
            
            stock_processed += 1
            
            # 获取该股票在已存在文件中的最后日期
            existing_last_date = existing_stock_dates.get(stock_name, 0)
            
            # 打印该股票的处理情况
            if existing_last_date > 0:
                print(f"\n处理股票 {stock_name} ({stock_processed}/{total_stocks})，将进行增量拆分...")
            else:
                print(f"\n处理股票 {stock_name} ({stock_processed}/{total_stocks})...")
            
            # 为当前股票按年份收集数据
            year_data = {}  # {year: [records]}
            
            # 读取该股票的所有记录
            new_records_count = 0
            skipped_records_count = 0
            
            for row in src_table:
                try:
                    datetime_val = row['datetime']
                    year, month, day = extract_year_month_day(datetime_val)
                    
                    # 验证日期有效性
                    try:
                        datetime.date(year, month, day)
                    except ValueError:
                        continue
                    
                    # 增量拆分：跳过已有的数据
                    if existing_last_date > 0 and datetime_val <= existing_last_date:
                        skipped_records_count += 1
                        continue
                    
                    # 按年份归类
                    if year not in year_data:
                        year_data[year] = []
                    
                    year_data[year].append(row)
                    new_records_count += 1
                    
                except Exception:
                    continue
            
            # 打印该股票的读取情况
            if existing_last_date > 0:
                print(f"  股票 {stock_name}: 新增 {new_records_count:,} 条记录，跳过 {skipped_records_count:,} 条已有记录")
            else:
                print(f"  股票 {stock_name}: 读取 {len(src_table):,} 条有效记录")
            
            # 立即将该股票的数据写入各年份文件
            if year_data:
                print(f"  开始写入 {len(year_data)} 个年份的文件...")
                
                # 为每个年份创建/追加文件
                for year, records in sorted(year_data.items()):
                    # 构建目标文件名
                    dest_file_name = os.path.join(dest_dir, f"{market_prefix}_{data_type}_{year}.h5")
                    
                    # 检查文件是否已存在
                    file_exists = os.path.exists(dest_file_name)
                    
                    if file_exists:
                        # 以追加模式打开已有文件
                        dest_hdf5 = tables.open_file(dest_file_name, mode="a",
                                                   filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
                    else:
                        # 以写入模式创建新文件
                        dest_hdf5 = tables.open_file(dest_file_name, mode="w",
                                                   filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
                    
                    try:
                        # 获取或创建 data 组
                        if file_exists:
                            try:
                                dest_group = dest_hdf5.get_node("/data")
                            except Exception:
                                dest_group = dest_hdf5.create_group("/", "data")
                        else:
                            dest_group = dest_hdf5.create_group("/", "data")
                        
                        # 检查表是否已存在
                        table_exists = False
                        try:
                            existing_table = dest_hdf5.get_node(dest_group, stock_name)
                            table_exists = True
                        except Exception:
                            pass
                        
                        if table_exists:
                            print(f"  ! {stock_name} ({year}): 表已存在，跳过")
                            continue
                        
                        # 创建表并写入记录
                        dest_table = dest_hdf5.create_table(dest_group, stock_name, table_desc, title=stock_name)
                        
                        for src_row in records:
                            dest_row = dest_table.row
                            for col_name in dest_table.colnames:
                                try:
                                    dest_row[col_name] = src_row[col_name]
                                except KeyError:
                                    pass
                            dest_row.append()
                        
                        dest_table.flush()
                        
                        # 打印该股票的处理结果
                        print(f"  ✓ {stock_name} ({year}): {len(records):,} 条记录 -> {os.path.basename(dest_file_name)}")
                    
                    finally:
                        dest_hdf5.close()
                
                # 清空当前股票的数据，释放内存
                year_data.clear()
            
            # 打印完成消息
            print(f"  ✓ 股票 {stock_name} 处理完成")
        
        print(f"\n{'='*80}")
        print(f"拆分完成！共处理 {stock_processed} 只股票")
        
    finally:
        src_hdf5.close()


def split_kdata_by_year(src_file_name, dest_dir, data_type='day'):
    """
    将 K 线数据按年份拆分成独立文件
    
    参数:
        src_file_name: 源 HDF5 文件路径
        dest_dir: 目标目录
        data_type: 数据类型 ('day', '5min', '1min' 等)
    """
    print(f"\n正在进行，请稍候.....")
    print(f"源文件：{src_file_name}")
    print(f"目标目录：{dest_dir}")
    print(f"数据类型：{data_type}")
    
    # 确保目标目录存在
    os.makedirs(dest_dir, exist_ok=True)
    
    # 获取市场前缀
    market_prefix = os.path.basename(src_file_name).split('_')[0]
    
    # 获取已存在文件中股票的最后日期（增量拆分）
    print(f"\n检查增量拆分...")
    existing_stock_dates = get_existing_stock_dates(dest_dir, market_prefix, data_type)
    
    if existing_stock_dates:
        print(f"将进行增量拆分，只提取新增数据")
    else:
        print(f"未发现历史数据，将进行完整拆分")
    
    # 根据数据类型选择表结构描述类
    if data_type == 'day':
        table_desc = DayDataRecordH5File
    elif data_type == '5min':
        table_desc = FiveMinDataRecordH5File
    elif data_type == '1min':
        table_desc = OneMinDataRecordH5File
    else:
        raise ValueError(f"不支持的数据类型：{data_type}")
    
    # 打开源文件
    src_hdf5 = tables.open_file(src_file_name, mode='r',
                               filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
    
    try:
        # 获取所有表
        all_tables = [x for x in src_hdf5.walk_nodes("/data")]
        total_tables = len(all_tables)
        
        # 特殊情况：如果是根目录下有 /data 节点，则显示其子节点数
        try:
            data_nodes = [node._v_name for node in src_hdf5.list_nodes('/data')]
            print(f"共找到 {len(data_nodes)} 个股票数据表")
        except:
            print(f"共找到 {total_tables - 1} 个股票数据表")
        
        # 统计总体进度
        total_stocks = total_tables - 1  # 减去/data 组本身
        stock_processed = 0
        
        # 遍历所有股票表，每次只处理一只股票
        for i in range(1, total_tables):
            src_table = all_tables[i]
            # 使用 _v_name 属性获取节点名称（股票代码）
            stock_name = src_table._v_name
            
            stock_processed += 1
            
            # 获取该股票在已存在文件中的最后日期
            existing_last_date = existing_stock_dates.get(stock_name, 0)
            
            # 打印该股票的处理情况
            if existing_last_date > 0:
                print(f"\n处理股票 {stock_name} ({stock_processed}/{total_stocks})，将进行增量拆分...")
            else:
                print(f"\n处理股票 {stock_name} ({stock_processed}/{total_stocks})...")
            
            # 为当前股票按年份收集数据
            year_data = {}  # {year: [records]}
            
            # 读取该股票的所有记录
            new_records_count = 0
            skipped_records_count = 0
            
            for row in src_table:
                try:
                    datetime_val = row['datetime']
                    year, month, day = extract_year_month_day(datetime_val)
                    
                    # 验证日期有效性
                    try:
                        datetime.date(year, month, day)
                    except ValueError:
                        continue
                    
                    # 增量拆分：跳过已有的数据
                    if existing_last_date > 0 and datetime_val <= existing_last_date:
                        skipped_records_count += 1
                        continue
                    
                    # 按年份归类
                    if year not in year_data:
                        year_data[year] = []
                    
                    year_data[year].append(row)
                    new_records_count += 1
                    
                except Exception:
                    continue
            
            # 打印该股票的读取情况
            if existing_last_date > 0:
                print(f"  股票 {stock_name}: 新增 {new_records_count:,} 条记录，跳过 {skipped_records_count:,} 条已有记录")
            else:
                print(f"  股票 {stock_name}: 读取 {len(src_table):,} 条有效记录")
            
            # 立即将该股票的数据写入各年份文件
            if year_data:
                print(f"  开始写入 {len(year_data)} 个年份的文件...")
                
                # 为每个年份创建/追加文件
                for year, records in sorted(year_data.items()):
                    # 构建目标文件名
                    dest_file_name = os.path.join(dest_dir, f"{market_prefix}_{data_type}_{year}.h5")
                    
                    # 检查文件是否已存在
                    file_exists = os.path.exists(dest_file_name)
                    
                    if file_exists:
                        # 以追加模式打开已有文件
                        dest_hdf5 = tables.open_file(dest_file_name, mode="a",
                                                   filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
                    else:
                        # 以写入模式创建新文件
                        dest_hdf5 = tables.open_file(dest_file_name, mode="w",
                                                   filters=tables.Filters(complevel=9, complib='zlib', shuffle=True))
                    
                    try:
                        # 获取或创建 data 组
                        if file_exists:
                            try:
                                dest_group = dest_hdf5.get_node("/data")
                            except Exception:
                                dest_group = dest_hdf5.create_group("/", "data")
                        else:
                            dest_group = dest_hdf5.create_group("/", "data")
                        
                        # 检查表是否已存在
                        table_exists = False
                        try:
                            existing_table = dest_hdf5.get_node(dest_group, stock_name)
                            table_exists = True
                        except Exception:
                            pass
                        
                        if table_exists:
                            print(f"  ! {stock_name} ({year}): 表已存在，跳过")
                            continue
                        
                        # 创建表并写入记录
                        dest_table = dest_hdf5.create_table(dest_group, stock_name, table_desc, title=stock_name)
                        
                        for src_row in records:
                            dest_row = dest_table.row
                            for col_name in dest_table.colnames:
                                try:
                                    dest_row[col_name] = src_row[col_name]
                                except KeyError:
                                    pass
                            dest_row.append()
                        
                        dest_table.flush()
                        
                        # 打印该股票的处理结果
                        print(f"  ✓ {stock_name} ({year}): {len(records):,} 条记录 -> {os.path.basename(dest_file_name)}")
                    
                    finally:
                        dest_hdf5.close()
                
                # 清空当前股票的数据，释放内存
                year_data.clear()
            
            # 打印完成消息
            print(f"  ✓ 股票 {stock_name} 处理完成")
        
        print(f"\n{'='*80}")
        print(f"拆分完成！共处理 {stock_processed} 只股票")
        
    finally:
        src_hdf5.close()


def split_timeline_by_year(src_file_name, dest_dir):
    """
    将分时线数据文件按年份拆分
    
    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
    """
    _split_data_by_year(src_file_name, dest_dir, 'timeline', TimeLineRecordH5File)


def split_trans_by_year(src_file_name, dest_dir):
    """
    将分笔成交数据文件按年份拆分
    
    参数:
        src_file_name: 源文件路径
        dest_dir: 目标目录
    """
    _split_data_by_year(src_file_name, dest_dir, 'trans', TransRecordH5File)


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
        ttk.Checkbutton(type_frame, text="分时 (timeline)", variable=self.process_timeline).grid(row=0, column=3, padx=10)
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
        """启动日志处理器"""
        def process_logs():
            try:
                while True:
                    message = self.log_queue.get_nowait()
                    self.log_text.configure(state=tk.NORMAL)
                    self.log_text.insert(tk.END, message + "\n")
                    self.log_text.see(tk.END)
                    self.log_text.configure(state=tk.DISABLED)
            except queue.Empty:
                pass
            self.root.after(100, process_logs)
        
        self.root.after(100, process_logs)
    
    def _update_progress(self, value, status="", detail=""):
        """更新进度
        
        参数:
            value: 进度值 (0-100)
            status: 状态文本
            detail: 详细信息（如当前处理的股票名称等）
        """
        self.progress_var.set(value)
        if status:
            self.status_label.config(text=status)
        if detail:
            self.detail_label.config(text=detail)
    
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
            'timeline': ['*_timeline.h5', '*_TIMELINE.h5'],
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
    
    def _process_file(self, src_file, dest_dir, data_type):
        """处理单个文件"""
        try:
            self._log(f"\n处理文件：{src_file.name}")
            
            # 创建日志重定向器
            redirector = GUILogRedirector(self._log)
            redirector.start()
            
            try:
                if data_type == 'timeline':
                    split_timeline_by_year(str(src_file), str(dest_dir))
                elif data_type == 'trans':
                    split_trans_by_year(str(src_file), str(dest_dir))
                else:
                    split_kdata_by_year(str(src_file), str(dest_dir), data_type=data_type)
                
                self._log(f"✓ {src_file.name} 处理完成")
                return True
            finally:
                redirector.stop()
            
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
        """运行拆分任务"""
        try:
            self._log("=" * 60)
            self._log("开始处理...")
            self._update_progress(0, "正在扫描文件...")
            
            # 查找文件
            files = self._find_data_files()
            
            if not files:
                self._log("未找到任何数据文件！")
                self._update_progress(100, "未找到文件")
                messagebox.showinfo("提示", "源目录中未找到任何数据文件！")
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
            if self.process_timeline.get() and 'timeline' in files:
                to_process.extend([(f, 'timeline') for f in files['timeline']])
            if self.process_trans.get() and 'trans' in files:
                to_process.extend([(f, 'trans') for f in files['trans']])
            
            total_files = len(to_process)
            if total_files == 0:
                self._log("没有需要处理的文件！")
                self._update_progress(100, "无需处理")
                messagebox.showinfo("提示", "没有需要处理的文件！")
                return
            
            self._log(f"找到 {total_files} 个文件需要处理")
            
            # 处理每个文件
            success_count = 0
            for i, (src_file, data_type) in enumerate(to_process):
                if not self.is_processing:
                    self._log("\n用户取消处理")
                    break
                
                progress = int((i / total_files) * 100)
                self._update_progress(progress, f"处理中：{i+1}/{total_files}")
                
                # 创建目标目录（按市场）
                market = src_file.stem.split('_')[0]
                dest_subdir = Path(self.dest_dir.get()) / market / data_type
                dest_subdir.mkdir(parents=True, exist_ok=True)
                
                # 处理文件
                if self._process_file(src_file, dest_subdir, data_type):
                    success_count += 1
            
            # 完成
            if self.is_processing:
                self._update_progress(100, "处理完成")
                self._log("=" * 60)
                self._log(f"处理完成！成功：{success_count}/{total_files}")
                messagebox.showinfo("完成", f"数据拆分完成！\n成功：{success_count}/{total_files} 文件")
            else:
                self._update_progress(0, "已取消")
                self._log("=" * 60)
                self._log(f"处理已取消！成功：{success_count}/{total_files}")
                messagebox.showinfo("取消", f"处理已取消！\n成功：{success_count}/{total_files} 文件")
        
        except Exception as e:
            self._log(f"\n发生错误：{str(e)}")
            self._update_progress(0, "错误")
            messagebox.showerror("错误", f"处理过程中发生错误：\n{str(e)}")
        
        finally:
            self.is_processing = False
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
