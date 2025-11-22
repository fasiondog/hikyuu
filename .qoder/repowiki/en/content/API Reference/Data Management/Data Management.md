# Data Management

<cite>
**Referenced Files in This Document**   
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [DataDriverFactory.cpp](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.cpp)
- [KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
- [BaseInfoDriver.h](file://hikyuu_cpp/hikyuu/data_driver/BaseInfoDriver.h)
- [BlockInfoDriver.h](file://hikyuu_cpp/hikyuu/data_driver/BlockInfoDriver.h)
- [H5KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/hdf5/H5KDataDriver.h)
- [MySQLKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/mysql/MySQLKDataDriver.h)
- [SQLiteKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/sqlite/SQLiteKDataDriver.h)
- [DriverConnectPool.h](file://hikyuu_cpp/hikyuu/data_driver/DriverConnectPool.h)
- [common_h5.py](file://hikyuu/data/common_h5.py)
- [common_mysql.py](file://hikyuu/data/common_mysql.py)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Data Driver Architecture](#data-driver-architecture)
3. [Factory Pattern Implementation](#factory-pattern-implementation)
4. [Abstract Base Classes](#abstract-base-classes)
5. [Storage Backends](#storage-backends)
6. [Configuration and Usage](#configuration-and-usage)
7. [Connection Management](#connection-management)
8. [Thread Safety and Performance](#thread-safety-and-performance)
9. [Schema Structure](#schema-structure)
10. [Custom Data Drivers](#custom-data-drivers)
11. [Data Import/Export Operations](#data-importexport-operations)
12. [Batch Processing](#batch-processing)

## Introduction
The Hikyuu data management system provides a comprehensive framework for handling financial data through a flexible driver-based architecture. This documentation details the data driver architecture, including the factory pattern implementation, abstract base classes for different data types, and various storage backends. The system supports multiple data types including price data (KData), stock basic information (BaseInfo), and block data (BlockInfo), with storage options including HDF5, MySQL, SQLite, and ClickHouse.

## Data Driver Architecture
The data driver architecture in Hikyuu is designed around a modular, extensible framework that separates data access logic from storage implementation. The architecture consists of three main components: data drivers, a factory pattern for driver creation, and connection pooling for efficient resource management.

The system follows a layered approach where abstract base classes define the interface for data access operations, while concrete implementations handle specific storage backends. This design allows for easy extension and replacement of storage mechanisms without affecting the higher-level application logic.

```mermaid
graph TB
subgraph "Data Access Layer"
Client[Application Client]
end
subgraph "Driver Interface Layer"
KDataDriver[KDataDriver]
BaseInfoDriver[BaseInfoDriver]
BlockInfoDriver[BlockInfoDriver]
end
subgraph "Factory Layer"
DataDriverFactory[DataDriverFactory]
end
subgraph "Storage Layer"
HDF5[HDF5 Backend]
MySQL[MySQL Backend]
SQLite[SQLite Backend]
ClickHouse[ClickHouse Backend]
end
Client --> KDataDriver
Client --> BaseInfoDriver
Client --> BlockInfoDriver
DataDriverFactory --> KDataDriver
DataDriverFactory --> BaseInfoDriver
DataDriverFactory --> BlockInfoDriver
KDataDriver --> HDF5
KDataDriver --> MySQL
KDataDriver --> SQLite
KDataDriver --> ClickHouse
BaseInfoDriver --> MySQL
BaseInfoDriver --> SQLite
BlockInfoDriver --> MySQL
BlockInfoDriver --> SQLite
BlockInfoDriver --> Qianlong
**Diagram sources**
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
- [BaseInfoDriver.h](file://hikyuu_cpp/hikyuu/data_driver/BaseInfoDriver.h)
- [BlockInfoDriver.h](file://hikyuu_cpp/hikyuu/data_driver/BlockInfoDriver.h)
**Section sources**
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
## Factory Pattern Implementation
The DataDriverFactory class implements the factory pattern to manage the creation and lifecycle of data drivers. This pattern provides a centralized mechanism for driver instantiation, configuration, and resource management.
The factory maintains separate registries for different driver types (KData, BaseInfo, BlockInfo) and provides methods for registration, retrieval, and cleanup. It uses a prototype-based approach where driver instances are cloned to create new connections, ensuring consistent configuration across instances.
```mermaid
classDiagram
    class DataDriverFactory {
        +static init()
        +static release()
        +static regBaseInfoDriver(BaseInfoDriverPtr)
        +static removeBaseInfoDriver(string)
        +static getBaseInfoDriver(Parameter)
        +static regBlockDriver(BlockInfoDriverPtr)
        +static removeBlockDriver(string)
        +static getBlockDriver(Parameter)
        +static regKDataDriver(KDataDriverPtr)
        +static removeKDataDriver(string)
        +static getKDataDriverPool(Parameter)
    }
    
    class KDataDriverConnectPool {
        +KDataDriverConnectPool(DriverPtr, size_t, size_t)
        +getConnect() DriverConnectPtr
        +getPrototype() DriverPtr
        +count() size_t
        +idleCount() size_t
        +releaseIdleConnect()
    }
    
    DataDriverFactory --> KDataDriverConnectPool : "creates"
    DataDriverFactory "1" --> "many" BaseInfoDriver : "manages"
    DataDriverFactory "1" --> "many" BlockInfoDriver : "manages"
    DataDriverFactory "1" --> "many" KDataDriver : "manages"

**Diagram sources**
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [DriverConnectPool.h](file://hikyuu_cpp/hikyuu/data_driver/DriverConnectPool.h)

**Section sources**
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [DataDriverFactory.cpp](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.cpp)

## Abstract Base Classes
The data management system defines three abstract base classes that serve as interfaces for different types of financial data access.

### KDataDriver
The KDataDriver class provides the interface for accessing price and volume data (K-line data). It defines methods for retrieving K-line records, time series data, and transaction data based on market, code, and query parameters.

### BaseInfoDriver
The BaseInfoDriver class handles basic stock information, including market details, stock type information, and historical financial data. It provides methods for retrieving stock information, market information, and financial metrics.

### BlockInfoDriver
The BlockInfoDriver class manages block data, which represents groups of securities (such as industry sectors or custom portfolios). It provides methods for retrieving block information, membership, and category data.

```mermaid
classDiagram
class KDataDriver {
+KDataDriver()
+KDataDriver(Parameter)
+KDataDriver(string)
+name() string
+init(Parameter) bool
+clone() KDataDriverPtr
+_clone() KDataDriverPtr
+_init() bool
+isIndexFirst() bool
+canParallelLoad() bool
+getCount(string, string, KType) size_t
+getIndexRangeByDate(string, string, KQuery, size_t, size_t) bool
+getKRecordList(string, string, KQuery) KRecordList
+getTimeLineList(string, string, KQuery) TimeLineList
+getTransList(string, string, KQuery) TransList
+isColumnFirst() bool
}
class BaseInfoDriver {
+BaseInfoDriver(string)
+name() string
+init(Parameter) bool
+_init() bool
+getAllStockInfo() vector~StockInfo~
+getStockInfo(string, string) StockInfo
+getStockWeightList(string, string, Datetime, Datetime) StockWeightList
+getAllStockWeightList() map~string, StockWeightList~
+getHistoryFinance(string, string, Datetime, Datetime) vector~HistoryFinanceInfo~
+getHistoryFinanceField() vector~pair~size_t,string~~
+getFinanceInfo(string, string) Parameter
+getMarketInfo(string) MarketInfo
+getAllMarketInfo() vector~MarketInfo~
+getAllStockTypeInfo() vector~StockTypeInfo~
+getStockTypeInfo(uint32_t) StockTypeInfo
+getAllHolidays() set~Datetime~
+getAllZhBond10() ZhBond10List
+getAllHistoryFinance() map~string, vector~HistoryFinanceInfo~~
}
class BlockInfoDriver {
+BlockInfoDriver(string)
+name() string
+init(Parameter) bool
+getStockBelongs(Stock, string) BlockList
+load()
+_init() bool
+getAllCategory() StringList
+getBlock(string, string) Block
+getBlockList(string) BlockList
+getBlockList() BlockList
+save(Block)
+remove(string, string)
}
KDataDriver <|-- H5KDataDriver
KDataDriver <|-- MySQLKDataDriver
KDataDriver <|-- SQLiteKDataDriver
BaseInfoDriver <|-- MySQLBaseInfoDriver
BaseInfoDriver <|-- SQLiteBaseInfoDriver
BlockInfoDriver <|-- MySQLBlockInfoDriver
BlockInfoDriver <|-- SQLiteBlockInfoDriver
BlockInfoDriver <|-- QLBlockInfoDriver
**Diagram sources**
- [KDataDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
- [BaseInfoDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/BaseInfoDriver.h)
- [BlockInfoDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/BlockInfoDriver.h)
**Section sources**
- [KDataDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
- [BaseInfoDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/BaseInfoDriver.h)
- [BlockInfoDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/BlockInfoDriver.h)
## Storage Backends
The Hikyuu data management system supports multiple storage backends for different use cases and performance requirements.
### HDF5 Implementation
The HDF5 backend provides high-performance storage for large datasets, particularly suitable for time-series data. The H5KDataDriver implementation leverages HDF5's hierarchical data format and compression capabilities to efficiently store and retrieve K-line data.
Key features :
- Column-oriented storage for efficient data access
- Built-in compression (zlib with level 9)
- Thread safety when compiled with HDF5 thread-safe library
- Index-based data access for optimal performance
### MySQL Implementation
The MySQL backend offers a robust relational database solution for structured financial data. The MySQLKDataDriver and related implementations use MyISAM storage engine for optimal read performance.
Key features :
- Schema-per-market and schema-per-KType organization
- Automatic database and table creation
- Support for complex queries and joins
- Transaction support for data integrity
### SQLite Implementation
The SQLite backend provides a lightweight, file-based database solution suitable for local development and smaller datasets. The SQLiteKDataDriver implementation offers a balance between performance and simplicity.
Key features :
- Single-file database storage
- Zero-configuration deployment
- ACID-compliant transactions
- Cross-platform compatibility
### ClickHouse Implementation
Although not directly visible in the provided code, the system appears to support ClickHouse as a columnar database backend for analytical workloads. This would provide high-performance querying capabilities for large financial datasets.
```mermaid
graph TD
    subgraph "Storage Backends"
        HDF5[HDF5 Backend]
        MySQL[MySQL Backend]
        SQLite[SQLite Backend]
        ClickHouse[ClickHouse Backend]
    end
    
    subgraph "Data Types"
        KData[KDataDriver]
        BaseInfo[BaseInfoDriver]
        BlockInfo[BlockInfoDriver]
    end
    
    KData --> HDF5
    KData --> MySQL
    KData --> SQLite
    BaseInfo --> MySQL
    BaseInfo --> SQLite
    BlockInfo --> MySQL
    BlockInfo --> SQLite
    BlockInfo --> Qianlong[Qianlong Backend]
    
    HDF5 -->|High-performance| TimeSeries[Time Series Data]
    MySQL -->|Relational| Structured[Structured Data]
    SQLite -->|Lightweight| Local[Local Storage]
    ClickHouse -->|Analytical| Analytics[Large-scale Analytics]

**Diagram sources**
- [H5KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/hdf5/H5KDataDriver.h)
- [MySQLKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/mysql/MySQLKDataDriver.h)
- [SQLiteKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/sqlite/SQLiteKDataDriver.h)
- [common_h5.py](file://hikyuu/data/common_h5.py)
- [common_mysql.py](file://hikyuu/data/common_mysql.py)

**Section sources**
- [H5KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/hdf5/H5KDataDriver.h)
- [MySQLKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/mysql/MySQLKDataDriver.h)
- [SQLiteKDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/kdata/sqlite/SQLiteKDataDriver.h)

## Configuration and Usage
The data drivers are configured through parameter objects that specify connection details and behavior options. Configuration can be done in both C++ and Python, with consistent interfaces across languages.

### C++ Usage Example
```cpp
// Create parameter object
Parameter param;
param.set<string>("type", "HDF5");
param.set<string>("data_dir", "/path/to/data");

// Get driver instance from factory
auto driver_pool = DataDriverFactory::getKDataDriverPool(param);
auto connection = driver_pool->getConnect();

// Use the connection to retrieve data
KQuery query(Datetime(20200101), Datetime(20201231), KQuery::DAY);
auto k_data = connection->getKRecordList("SH", "000001", query);
```

### Python Usage Example
```python
from hikyuu import *

# Create parameter object
param = Parameter()
param.type = "HDF5"
param.data_dir = "/path/to/data"

# Get driver instance from factory
driver_pool = DataDriverFactory.getKDataDriverPool(param)
connection = driver_pool.getConnect()

# Use the connection to retrieve data
query = KQuery(Datetime(20200101), Datetime(20201231), KQuery.DAY)
k_data = connection.getKRecordList("SH", "000001", query)
```

**Section sources**
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)
- [KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)

## Connection Management
The system implements a sophisticated connection management strategy using connection pooling to optimize resource utilization and performance.

### DriverConnectPool
The DriverConnectPool template class manages a pool of driver connections, providing:
- Configurable maximum connection limits
- Maximum idle connection management
- Thread-safe access to connections
- Automatic resource cleanup

The pool uses a queue-based approach to manage available connections, with blocking behavior when the maximum connection limit is reached. This ensures that applications can efficiently reuse connections without the overhead of repeated initialization.

### Connection Lifecycle
Connections follow a specific lifecycle:
1. Application requests a connection from the pool
2. Pool provides an existing idle connection or creates a new one
3. Application uses the connection for data operations
4. Connection is automatically returned to the pool when out of scope
5. Pool manages the physical connection, either keeping it idle or closing it based on configuration

```mermaid
sequenceDiagram
participant Application
participant DriverPool
participant DriverConnection
Application->>DriverPool : getConnect()
alt Pool has idle connection
DriverPool->>Application : Return idle connection
else Pool needs new connection
DriverPool->>DriverPool : Create new connection
DriverPool->>Application : Return new connection
end
Application->>DriverConnection : Perform data operations
DriverConnection-->>Application : Return data
Application->>DriverPool : Connection goes out of scope
DriverPool->>DriverPool : Return connection to idle pool
alt Exceeds max idle connections
DriverPool->>DriverPool : Close connection
else Within idle limits
DriverPool->>DriverPool : Keep connection idle
end
**Diagram sources**
- [DriverConnectPool.h](file : //hikyuu_cpp/hikyuu/data_driver/DriverConnectPool.h)
**Section sources**
- [DriverConnectPool.h](file : //hikyuu_cpp/hikyuu/data_driver/DriverConnectPool.h)
## Thread Safety and Performance
The data management system incorporates several design considerations for thread safety and performance optimization.
### Thread Safety
- HDF5 backend : Thread-safe when compiled with HDF5 thread-safe library (H5_HAVE_THREADSAFE)
- MySQL backend : Connection-level thread safety with connection pooling
- SQLite backend : Thread-safe with proper configuration
- Connection pools : Thread-safe using mutex protection
### Performance Characteristics
- HDF5 : Optimized for sequential reads and large dataset access
- MySQL : Optimized for complex queries and relational operations
- SQLite : Optimized for single-threaded access and small datasets
- Connection pooling : Reduces connection initialization overhead
### Performance Optimization Features
- Index-based data access in HDF5 backend
- Query optimization in MySQL backend
- Memory mapping in SQLite backend
- Parallel loading support in HDF5 and MySQL backends
**Section sources**
- [H5KDataDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/kdata/hdf5/H5KDataDriver.h)
- [MySQLKDataDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/kdata/mysql/MySQLKDataDriver.h)
- [SQLiteKDataDriver.h](file : //hikyuu_cpp/hikyuu/data_driver/kdata/sqlite/SQLiteKDataDriver.h)
- [DriverConnectPool.h](file : //hikyuu_cpp/hikyuu/data_driver/DriverConnectPool.h)
## Schema Structure
The data management system uses different schema structures for each storage backend and data type.
### HDF5 Schema
- File-per-market and KType (e.g., sh_day.h5, sh_min.h5)
- Group structure : /data, /week, /month, /quarter, /halfyear, /year
- Table naming : MARKETCODE (e.g., SH000001)
- Data types :
- H5Record : Basic K-line data (date, open, high, low, close, amount, count)
- H5Index : Extended K-line index (date, start)
- H5Transaction : Transaction data (date, price, vol, buyorsell)
- H5TransactionIndex : Transaction index (date, start)
- H5MinuteTime : Minute-level time series (date, price, vol)
### MySQL Schema
- Database-per-market and KType (e.g., sh_day, sz_min)
- Table naming : lowercase code (e.g., 000001)
- Column structure :
- date : BIGINT(20) UNSIGNED (primary key)
- open, high, low, close : DOUBLE UNSIGNED
- amount, count : DOUBLE UNSIGNED
- Storage engine : MyISAM for optimal read performance
### SQLite Schema
- Single database file per market
- Table-per-security with naming convention : market_code (e.g., sh_000001)
- Similar column structure to MySQL implementation
**Section sources**
- [common_h5.py](file : //hikyuu/data/common_h5.py)
- [common_mysql.py](file : //hikyuu/data/common_mysql.py)
## Custom Data Drivers
The system supports extension through custom data drivers that can be registered with the DataDriverFactory.
### Creating Custom Drivers
To create a custom driver :
1. Inherit from the appropriate base class (KDataDriver, BaseInfoDriver, or BlockInfoDriver)
2. Implement the required virtual methods
3. Register the driver with the factory
### Example Custom Driver
```cpp
class CustomKDataDriver : public KDataDriver {
public:
    CustomKDataDriver() : KDataDriver("CUSTOM") {}
    
    virtual KDataDriverPtr _clone() override {
        return std::make_shared<CustomKDataDriver>();
    }
    
    virtual bool _init() override {
        // Custom initialization logic
        return true;
    }
    
    virtual bool isIndexFirst() override {
        return true;
    }
    
    virtual bool canParallelLoad() override {
        return true;
    }
    
    virtual size_t getCount(const string& market, const string& code,
                          const KQuery::KType& kType) override {
        // Implementation
    }
    
    // Implement other required methods
};

// Register with factory
auto driver = make_shared<CustomKDataDriver>();
DataDriverFactory::regKDataDriver(driver);
```

**Section sources**
- [KDataDriver.h](file://hikyuu_cpp/hikyuu/data_driver/KDataDriver.h)
- [DataDriverFactory.h](file://hikyuu_cpp/hikyuu/data_driver/DataDriverFactory.h)

## Data Import/Export Operations
The system provides utilities for importing and exporting data between different formats and storage backends.

### HDF5 Data Operations
The common_h5.py module provides functions for:
- Creating and opening HDF5 files
- Getting tables for specific securities
- Updating external data indices (weekly, monthly, etc.)
- Managing transaction and time series data

### MySQL Data Operations
The common_mysql.py module provides functions for:
- Database creation and version management
- Table creation and schema management
- Data import and export
- Holiday calendar management
- External data index updates

### Data Migration
The system supports data migration between backends through:
- Direct export/import scripts
- Batch processing capabilities
- Schema conversion utilities

**Section sources**
- [common_h5.py](file://hikyuu/data/common_h5.py)
- [common_mysql.py](file://hikyuu/data/common_mysql.py)

## Batch Processing
The data management system includes capabilities for batch processing of financial data.

### Batch Data Loading
- Parallel loading support in HDF5 and MySQL backends
- Connection pooling for efficient resource utilization
- Configurable batch sizes and threading

### Batch Operations
- Bulk data import/export
- Schema updates and migrations
- Index regeneration
- Data validation and cleanup

### Performance Considerations
- Optimal batch sizes vary by backend
- Memory usage should be monitored during large batch operations
- Connection pool settings should be tuned for batch workloads

**Section sources**
- [common_h5.py](file://hikyuu/data/common_h5.py)
- [common_mysql.py](file://hikyuu/data/common_mysql.py)