# AGENTS.md — Hikyuu Project AI Development Guide

> This file provides the core context needed to work in the hikyuu repository for AI coding agents (and newly joined developers):
> the project structure, the build/test commands, the code conventions and the common caveats. **Read this file first, before changing any code.**

## 1. Project Overview

- **Hikyuu** is an open-source ultra-high-speed quantitative trading research framework based on **C++/Python**, focusing on the strategy analysis, the backtesting and the live trading capability extensions (deeply adapted to the domestic China A-share market).
- The core capabilities: the trading model development, the ultra-fast computing engine, the efficient backtesting system and the live trading extensions.
- The project composition: the **high-performance C++ core library** (`hikyuu_cpp`) + the **pybind11 binding layer** (`hikyuu_pywrap`) + the **Python interface layer** (the `hikyuu` package) + the **interactive exploration tools** (`hikyuu.interactive`).
- License: Apache License 2.0; the default branch is `master`, plus the `release`, `bugfix` and `feature/*` branches.
- Project documentation: [https://hikyuu.readthedocs.io/zh-cn/latest/index.html](https://hikyuu.readthedocs.io/zh-cn/latest/index.html) (Sphinx, mainly in Chinese).

## 2. Repository Structure

```
hikyuu/
├── xmake.lua                 # The top-level build script (an xmake project, defining the global options/dependencies)
├── copy_dependents.lua       # The task that copies the third-party dependency headers/libraries
├── requirements.txt          # The Python-side dependencies
├── setup.py / sub_setup.py   # The Python package installation scripts
├── hikyuu_cpp/               # The C++ core engine library
│   ├── hikyuu/               #   ├─ The core code: Stock/KData/Indicator/StockManager...
│   │   ├── data_driver/      #   ├─ The data driver engine (HDF5/MySQL/SQLite/TDX)
│   │   ├── trade_sys/        #   ├─ The trading system components (SG/MM/ST/CN/EV/PG/SP/PF/SE/AF/MF)
│   │   ├── trade_manage/     #   ├─ The trade management (TradeManager/OrderBroker)
│   │   ├── indicator/        #   ├─ The indicator library (including indicator_talib)
│   │   ├── strategy/         #   ├─ The strategy context
│   │   ├── factor/           #   ├─ The multi-factor
│   │   ├── utilities/        #   ├─ The basic utilities (logging/configuration/networking, etc.)
│   │   └── xmake.lua         #   └─ The core target("hikyuu") definition
│   ├── unit_test/            # The C++ unit tests (doctest), targets: unit-test / small-test / real-test
│   └── demo/                 # The C++ demos
├── hikyuu_pywrap/            # The pybind11 bindings (the target "core" → core.so / core.pyd)
│   ├── main.cpp              # The binding registration entry
│   ├── indicator/ trade_sys/ trade_manage/ data_driver/ ...   # The bindings of each module
│   └── xmake.lua
├── hikyuu/                   # The Python interface package
│   ├── __init__.py           # The package entry: loading the compiled core.so and the dependency libraries
│   ├── core.py / extend.py   # The core object imports and extensions
│   ├── indicator/            # The Python-side indicator extensions (the .pyi stubs and the implementations)
│   ├── trade_sys/ trade_manage/ analysis/ data/ draw/ util/
│   ├── fetcher/ gui/ strategy/ interactive.py / hub.py
│   ├── plugin/               # The runtime plugins (the data import, the market data, etc.)
│   ├── cpp/                  # The compiled artifacts directory: core310~core313.so, lib*.dylib, etc. (gitignored)
│   ├── test/                 # The Python tests (test.py is the entry)
│   └── examples/             # The examples and the notebook tutorials
├── docs/                     # The Sphinx documentation (dual-source: docs/zh Chinese + docs/en English; docs/make.sh builds)
├── test_data/                # The C++ test data (copied automatically when running the tests)
├── i18n/                     # The internationalization/language files
├── docker/                   # The containerization configuration
└── .github/workflows/        # The CI: ubuntu.yml / windows.yml / macosx.yml
```

## 3. Build System (xmake)

- Build tool: **xmake** (the top-level `set_xmakever("3.0.0")`, and the CI uses 3.0.8). The C++ standard is **C++20**; Windows uses clang-cl.
- All the third-party dependencies are pulled through the xmake package management (`add_requires`): boost, hdf5, mysql, fmt, spdlog, sqlite3, flatbuffers, nng, nlohmann_json, eigen, xxhash, utf8proc, ta-lib, mimalloc, pybind11, doctest, etc.; the external repository is `hikyuu-extern-libs` (github/gitee).
- The key configuration items (the `xmake f` options): `mysql`, `hdf5`, `sqlite`, `tdx`, `ta_lib`, `low_precision`, `omp`, `serialize`, `leak_check`, `stacktrace`, `log_level`, `async_log`, `feedback`, `spend_time`, etc.
- The artifacts are output to `build/{mode}/{plat}/{arch}/lib`; the `core.so` and the dependency libraries needed by the Python package at runtime must be copied to `hikyuu/cpp/` (see the workflow below).

### Common Commands

```bash
# Configure (the first time, or after changing the dependencies/options)
xmake f -k shared -y -vD

# Build the C++ core library
xmake -b core

# Build and run the C++ unit tests (doctest; small-test does not depend on the real data)
xmake r small-test

# Run the full unit tests (covering most modules such as indicator/trade_sys)
xmake r unit-test

# The real data test (requiring HKU_USE_REAL_DATA_TEST and the real market data; usually run only in the CI or locally with the data)
xmake r real-test

# The debug/coverage mode
xmake f -m debug -y          # debug
xmake f -m coverage -y       # coverage (generating the lcov/genhtml reports)
```

> Note: when running the `xmake r` series tests, the build system automatically copies `test_data`, `hikyuu/plugin` and `i18n` to the directory of the executable (see the `prepare_run` in `hikyuu_cpp/unit_test/xmake.lua`).

### IDE / LSP Indexing (clangd)

The sources of `hikyuu_cpp/hikyuu/` generally use `#include "hikyuu/xxx.h"` (relying on the `add_includedirs("..")` of each target, i.e. with `hikyuu_cpp` as the include root), while `hikyuu_pywrap/` uses `#include <hikyuu/xxx.h>` (relying on `add_includedirs("../hikyuu_cpp")`). Therefore, **clangd must get the compilation database**, otherwise it will degrade to the fallback arguments (with the compilation directory = the directory of the file itself), reporting masses of `Unknown type name 'XXX'` and `'hikyuu/xxx.h' file not found` for the newly created files — such errors are **an indexing problem, not a code problem**; do not try to "fix" them by changing the code.

```bash
# Generate compile_commands.json in the project root (the locations discovered natively by clangd)
xmake project -k compile_commands --lsp=clangd
```

- Re-run it after adding/deleting the source files or changing the `xmake f` options (the dependencies/switches); the generated artifacts are gitignored (`.vscode` and `.clangd` are both in the ignore list), do not commit them.
- Do not use the `-I.` of `.clangd` to replace the compilation database: the relative paths are resolved against the compilation directory, and for the fallback commands they point to the directory of the source file rather than the include root — **verified to be ineffective**.
- If you still want to put the database in `.vscode/` or another subdirectory, you can use `clangd.arguments: --compile-commands-dir=<dir>` to specify the directory.
- Manually editing `hikyuu_cpp/hikyuu/config.h` and `version.h` has no effect (both are generated at the build time by `add_configfiles` from `config.h.in`/`version.h.in`, and are gitignored); the recognition of the `HKU_*` conditional compilation macros by clangd also depends on the `-D` definitions carried in the compilation database, so you must regenerate `compile_commands.json` after changing them.

## 4. Testing

### Python Tests (hikyuu/test/)

```bash
export PYTHONPATH=.
python3 hikyuu/test/test.py     # the entry used by the CI
```

- The independent test files of each module: `Indicator.py`, `KData.py`, `Signal.py`, `MoneyManager.py`, `Stoploss.py`, `AllocateFunds.py`, `Datetime.py`, `Parameter.py`, etc., which can be run individually (e.g. `python3 hikyuu/test/Indicator.py`).
- The new Python features should add the corresponding tests under `hikyuu/test/`.

### C++ Tests (hikyuu_cpp/unit_test/)

Based on **doctest**, the directory structure corresponds one-to-one with the core library modules. The test project organization must follow the principles below:

#### The Organization Principles

1. **Physical isolation with a parallel structure**: the test project and the source code project are physically isolated, using completely independent parallel directories (`hikyuu_cpp/unit_test/hikyuu/…` against `hikyuu_cpp/hikyuu/…`), with a consistent internal directory structure.
2. **One module, one suite**: for one module (usually one class), establish a test suite, named `test_<module>_suite`, e.g. `test_iniparser_suite`, all in lowercase. Declare the suite at the top of the file with `@defgroup` / `@ingroup`, see the existing files such as `test_iniparser.cpp` and `test_Stock.cpp`.
3. **One suite, one file**: each test suite uses an independent test file, named `test_<module>.cpp`, e.g. `test_iniparser.cpp` and `test_Stock.cpp`.
4. **One function/method, one case**: for each function or class member method, establish an independent test case (`TEST_CASE`), named `test_<function_name>` or `test_<class_name>_<method_name>`. When the names collide, you can add `_case` or another identifier after them to distinguish.
5. **Cover the public interfaces as much as possible**: the public interfaces should have tests added as much as possible; the scenarios that must be mocked to simulate may be skipped.
6. **Mark the test points with @arg**: within each test case, use the `/** @arg … */` comments to clearly mark each test point, for the code review and the quick locating. Example:

```cpp
TEST_CASE("test_IniParser_hasSection") {
    IniParser ini_parser;
    // …prepare the data…
    /** @arg the specified section exists */
    CHECK_UNARY(ini_parser.hasSection("test1"));
    /** @arg the specified section does not exist */
    CHECK_UNARY(!ini_parser.hasSection("test2"));
}
```

7. **The boundary conditions must be covered**: in the tests of each function/method, the boundary conditions must be covered, paying special attention to:

   - **The loop boundaries**: 0, 1, exactly N, N-1, N+1 iterations (e.g. an empty container, a single element, multiple elements).
   - **The extreme value boundaries**: the minimum/maximum, an empty string, an empty range, `Null<T>()`, an out-of-bounds index, zero, a negative value (if allowed).
   - **The branch boundaries**: each branch of `if/else` and `switch`, both sides of the ternary expressions, and the paths of the early `return` / `break` / `continue`.
   - **The error/exception paths**: the invalid inputs, the missing files, the malformed formats, etc., which should trigger the exceptions.
8. **The coverage requirements**: overall, aim for the branch coverage, with the line coverage as the minimum requirement. **The code paths that must be mocked to simulate are exempted** (e.g. the branches that can only be triggered by the external dependencies such as the network, the database and the live trading connections). `xmake f -m coverage -y` can generate the lcov coverage report for a self-check.

#### The Run Targets

- `unit-test`: the complete unit test set covering most modules.
- `small-test`: the minimal regression set, used by the CI by default.
- `real-test`: requires the real market data, used together with `HKU_USE_REAL_DATA_TEST`.

## 5. Code Conventions

| Language | Convention                                    | Key points                                                                                                                              |
| -------- | --------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| C++      | `.clang-format` (Google style as the base)    | a 4-space indent, a column width of 100, the attached braces; the warning switches such as `-Wno-sign-compare` are in xmake.lua |
| Python   | `hikyuu/.style.yapf` (yapf) + `.flake8`       | a 4-space indent, a column width of 120 (flake8 `max-line-length=120`)                                                                  |
| Lua      | `.lua-format`                                 | format the build scripts                                                                                                                 |

- Format the changed files with `clang-format` / `yapf` before committing, to avoid deviating from the existing style.
- Adding a new public API requires maintaining the `.pyi` stubs (`hikyuu/__init__.pyi`, `core.pyi`, `extend.pyi` and `hikyuu/cpp/core3xx.pyi`) and the documentation (`docs/zh/` and `docs/en/`; the two trees must be updated in pairs with a consistent structure) synchronously.

### Naming Conventions (C++)

The conventions below are distilled from the existing code of `hikyuu_cpp/hikyuu/`; the new/modified code must follow them:

| The identifier category                    | The convention                                                                                                                  | The examples                                                                                                                                                               |
| ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Namespace                                  | all lowercase                                                                                                                    | `namespace hku;`                                                                                                                                                           |
| Class / struct                             | `PascalCase`, the business domain + the core concept; the exported classes carry the `HKU_API` macro                             | `class HKU_API StockManager`, `class SignalBase`, `struct ParamItemRecord`                                                                                                 |
| Public member functions                    | `camelCase`, starting with a verb (`should/get/set/is/has/reload…`)                                                              | `shouldBuy()`, `getBuyValue()`, `reloadWith()`, `isIpcClientMode()`, `setTO()`, `nextTimeShouldBuy()`                                                                      |
| Protected / private member functions       | the `_` prefix + `camelCase` (the hooks that the subclasses need to override start with `_`)                                     | `_calculate()`, `_reset()`, `_clone()`, `_addBuySignal()`, `_testingSetIpcClientMode()`                                                                                    |
| Member variables                           | the `m_` prefix + `camelCase`                                                                                                    | `m_name`, `m_kdata`, `m_is_python_object`, `m_buySig`, `m_cycle_start`, `m_ipc_client_mode`                                                                                |
| Class static member variables              | the `ms_` prefix + `camelCase` (distinguished from the non-static `m_`)                                                          | `ms_sm`, `ms_init_mutex`, `ms_stockDict` (note: the old code of `StockManager` still uses `m_sm`/`m_init_mutex`/`m_stockDict` as a historical legacy; all the newly added static members use `ms_`) |
| Global variables / file-scope statics      | the `g_` prefix + `camelCase`                                                                                                    | `g_load_event`, `g_shm_server_role`, `g_all_base_ktype`, `g_ktype2min`, `g_log_level`                                                                                      |
| The static local variables in functions    | the `g_` prefix + `camelCase` (consistent with the global variables, easy to identify the long-lived storage)                    | `static std::once_flag g_tz_set;`, `static long int g_timezone;`                                                                                                           |
| Type aliases / smart pointer aliases       | the business object name +`Ptr` (`typedef shared_ptr<T> XPtr;`)                                                                  | `typedef shared_ptr<SignalBase> SignalPtr;`                                                                                                                                |
| Enumeration types / enumeration values     | the enumeration type in `PascalCase`; the enumeration values in all uppercase + underscores                                       | `KQuery::QueryType { INDEX, DATE, INVALID }`                                                                                                                               |
| Macros / compilation switches / constants  | all uppercase + underscores                                                                                                       | `HKU_API`, `HKU_SUPPORT_SERIALIZATION`, `HKU_ENABLE_NODE`, `IND_EQ_THRESHOLD`                                                                                              |
| Function parameters                        | `camelCase`                                                                                                                       | `baseInfoParam`, `kdataParam`, `datetime`, `context`                                                                                                                       |
| Local variables                            | `camelCase`                                                                                                                       | the local variable style inside `initParam()`                                                                                                                              |
| Header/source file names                   | `PascalCase`; **one `class` per file** (the class name matches the file name); the flat `struct`/POD/small utility types may coexist in one file | `StockManager.h`, `SignalBase.h`; `SG_Cross.h`, `MM_FixedPercent.h`, `ST_FixedPercent.h`, `SP_Normal.h`                                                                    |
| The files of the derived implementations   | `PascalCase`, placed in the `imp/` subdirectory of the corresponding module                                                       | `imp/CrossSignal.h`, `imp/FixedPercentMoneyManager.h`                                                                                                                      |

> Note: `hku` is the only top-level namespace of the entire C++ core library; the newly added public classes must carry the `HKU_API` export macro; the factory constructors are centralized in the `crt/` subdirectory of each module, and the derived implementations are centralized in the `imp/` subdirectory.
>
> **The file organization constraint**: each `class` must exclusively occupy one header/source file (the file name matches the class name); only the lightweight definitions such as the flat `struct`s, the PODs, the small utility structures, the enumerations and the typedef aliases may coexist with the other types in the same file. E.g. the coexistence of `struct ParamItemRecord` and `class Parameter` in `Parameter.h` is a compliant exception, but the core business classes such as `class StockManager` and `class SignalBase` are each in their own file.

### Naming Conventions (Python)

The conventions below are distilled from the existing code of the `hikyuu/` package (excluding the `cpp/` compiled artifacts and `test/`):

| The identifier category              | The convention                                                                                         | The examples                                                                                            |
| ------------------------------------ | ------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| Package / module file names          | `snake_case`                                                                                            | `indicator/`, `trade_manage/`, `trade_sys/`, `util/singleton.py`, `draw/drawplot/matplotlib_draw.py`     |
| Classes                              | `PascalCase`                                                                                            | `class Spot`, `class OrderBrokerWrap`, `class SingletonType`, `class System`                             |
| Public functions / methods           | `snake_case`, starting with a verb                                                                      | `concat_to_df()`, `df_to_ind()`, `run_in_strategy()`, `get_part()`                                       |
| Private / internal methods           | a single underscore prefix + `snake_case` (conventionally for internal use, not strictly enforced)      | `_buy()`, `_sell()`, `_get_asset_info()`, `_clone()`                                                     |
| Magic methods                        | wrapped in double underscores, the Python standard                                                      | `__init__`, `__iter__`, `__str__`, `__repr__`                                                            |
| Module-level constants               | `SCREAMING_SNAKE_CASE`                                                                                  | `BASE_DIR`, `DRAWNULL`, `HDF5_COMPRESS_LEVEL`, `KDATA`, `CLOSE`, `OPEN`, `HIGH`, `LOW`                   |
| The short global context variables   | a single uppercase letter (the abbreviations of the K-line fields/objects)                              | `O`, `C`, `H`, `L`, `A`, `V`, `D`, `K`, `Q` (as the global convenience aliases in `hikyuu/__init__.py`)  |
| Class variables / enumeration constants | all uppercase                                                                                        | `System.ENVIRONMENT`, `System.SIGNAL`, `System.STOPLOSS`                                                 |
| Instance variables                   | `snake_case`; the private ones start with a single underscore                                            | `self._name`, `self._params`, `self._broker`, `self._instance_lock`, `self._stop_event`                  |
| Local variables                      | `snake_case`                                                                                            | `df`, `ind_list`, `head_stock_code`, `params`, `cloned`                                                  |
| Function parameters                  | `snake_case`; the annotated variable names are also `snake_case`                                        | `head_stock_code`, `col_name`, `col_date`, `allocate_weight_func`, `get_real_buy_price`                  |
| Custom decorators                    | the `hku_` prefix + `snake_case`                                                                        | `@hku_catch`, `@hku_check_ignore`                                                                        |
| property / accessors                 | `snake_case` (the `PascalCase` accessors in `flat/Spot.py` are the flatbuffers generated code, **not** this convention) | the accessor methods of the `hikyuu/` code itself are mainly `snake_case`                                |

> Note: the stubs generated by pybind11-stubgen, such as `hikyuu/cpp/core3xx.pyi`, may contain the naming inconsistent with the above; they belong to the binding layer generated artifacts and are not regarded as the Python-side handwritten conventions.

### Generating the .pyi Stubs (pybind11-stubgen)

The `.pyi` stubs of the C++ binding layer (`core.so` / `core.pyd`) are generated with **pybind11-stubgen**:

```bash
# 1. Install pybind11-stubgen (if it is not installed)
pip install pybind11-stubgen

# 2. Generate the stubs (run it in the repository root, outputting to the current directory; make sure the project directory
#    is in the PYTHONPATH, and the compiled artifacts are under hikyuu/cpp/ so that import hikyuu works)
pybind11-stubgen -o . hikyuu
```

- Do not generate the stubs by hand; only generate them when releasing or when requested manually.
- After modifying the bindings under `hikyuu_pywrap/` (adding/changing the classes, the functions and the parameters), the corresponding stubs should be regenerated and synchronized.

## 6. Architecture and Key Components

The core components of the systematic trading framework (independently replaceable and freely composable); the C++ implementations are in `hikyuu_cpp/hikyuu/trade_sys/`, and the Python counterparts are in `hikyuu/trade_sys/`, `hikyuu/trade_manage/`, etc.:

| The level          | The components                                                                                               | The description                                                                                            |
| ------------------ | ------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- |
| The portfolio level | PortfolioPF / SelectorSE / AllocateFundsAF / MultiFactorMF                                                    | the multi-system scheduling, the strategy screening, the fund allocation, the multi-factor                 |
| The trading system SYS | EnvironmentEV / ConditionCN / SignalSG / Stoploss·StopprofitST / MoneyManagerMM / ProfitGoalPG / SlippageSP | the market environment, the valid condition, the signal, the risk control, the money management, the profit goal, the slippage |
| The trade management | TradeManagerTM / OrderBrokerOB                                                                              | the account funds and position records, the live trading order connection                                   |
| The data level     | StockManagerSM / KDataKD / QueryQ                                                                             | the security management, the K-line data, the time range query                                              |

- The data storage supports: HDF5 (the default) / MySQL / ClickHouse / SQLite / TDX.
- For the new indicators/new strategy components, it is recommended to implement them in the C++ core first (including the bindings and the unit tests), and then expose them at the Python layer; the pure Python extensions go into the corresponding subpackages of `hikyuu/`.

## 7. Documentation

- Sphinx + myst_parser, **dual-source and bilingual**: `docs/zh/` (Chinese) and `docs/en/` (English) are two **independent Sphinx trees**, each with its own `conf.py`, without using gettext.
- The files are still a mix of `.rst` and `.md` (the new files prefer `.md`).
- The local build: `cd docs && ./make.sh` (building both trees → `build/html/{en,zh}`); `./make.sh en` / `./make.sh zh` build only one tree.
- **They must be maintained in pairs**: when changing the documentation of either language, synchronize the other tree within the same PR, keeping the file sets / the toctree / the heading levels / the labels / the images / the code blocks consistent.
- When modifying the public interfaces/adding the components, synchronize the corresponding sections under **both trees** (`indicator/`, `trade_sys/`, `trade_manage/`, `stock_manager.rst`, `factor.md`, etc.).
- The RTD hosting configuration: `docs/en/.readthedocs.yaml`, `docs/zh/.readthedocs.yaml` (the configuration files are **not** placed at the repository root); the cross-language jumps are provided by the RTD Flyout, and hardcoding the `/en/`, `/zh-cn/` links in the sources is forbidden.
- **When doing Chinese-English translation (covering the C++/Python comment anglicization, the docstrings, the bilingual docs, the README, etc.), the wording must refer to the glossary `docs/tools/glossary.zh-en.md`**; new terms must be registered in the glossary first (via PR review), and then be used — do not invent synonymous translations.

## 8. The AI Development Workflow and Caveats

1. **Locate the code**: the C++ logic → `hikyuu_cpp/hikyuu/`; the bindings → `hikyuu_pywrap/`; the Python layer → `hikyuu/`; the tests → `hikyuu_cpp/unit_test/` and `hikyuu/test/`.
2. **After modifying the C++ code, you must recompile and let the Python package load the new artifacts**:

   ```bash
   xmake -b core
   # Synchronize the compiled artifacts to hikyuu/cpp/ (for import hikyuu to load)
   ```

   The Python package entry `hikyuu/__init__.py` loads `core.so` and the dependency libraries from `hikyuu/cpp/` (mac/linux sets the `LD_LIBRARY_PATH`).
3. **When only changing the Python layer, there is no need to recompile the C++**, but note that the `.pyi` stubs must stay in sync with the implementations, and `hikyuu/core.py`/`extend.py` carry the core object exports.
4. **Do not commit the compiled artifacts**: `*.so`, `*.pyd`, `*.dll`, `build/` are all in `.gitignore`; the `core3xx.so`, etc. under `hikyuu/cpp/` are the local build artifacts.
5. **Adding new dependencies**: the C++ dependencies go into `xmake.lua` with `add_requires` (note the platform differences and the versions, e.g. hdf5 is 1.13.3 on Windows, and mysql varies by platform); the Python dependencies go into `requirements.txt`.
6. **Tests first**: when the change involves the C++ core, run at least `xmake r unit-test` + `python3 hikyuu/test/test.py`; when a specific module is involved, run its corresponding test file.
7. **The CI will verify**: the three pipelines of ubuntu (aarch64/x86_64), windows and macosx under `.github/workflows/`; the PRs must pass the builds and the tests before merging into `master`.
8. **The commit messages uniformly use English**: in the conventional commits style, e.g. `fix(data): fix cross-period aggregation of derived K-lines in the SQL backend`; the historical early commits have Chinese messages, but all the new commits use English, and the body text is also in English.
9. **The AI must not commit proactively**: an AI coding agent is forbidden to execute `git commit`, and should also avoid `git add`; after completing each step, list "the list of the files to be committed + the suggested English commit message (a directly copyable `git commit -m "..."`)" and inform the user, letting the user decide the commit timing and the granularity.
10. **Handle with care**: `hikyuu_pywrap` uses a unity build (`c++.unity_build`); pay attention to the unity_group grouping when adding the .cpp files; after modifying `xmake.lua`, you need to reconfigure with `xmake f`.

## 9. The Quick Self-check Checklist (before committing)

- [ ] The changed files have been formatted with `clang-format` / `yapf`
- [ ] The C++ changes have compiled successfully and the Python side can `import hikyuu` normally
- [ ] The related unit tests have been run (C++: `xmake r small-test`; Python: `python3 hikyuu/test/test.py`)
- [ ] No compiled artifacts/local data files have been committed
