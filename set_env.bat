@REM 此处需要修改为实际boost所在路径
@set BOOST_ROOT=d:\src\boost

@set BOOST_LIB=%BOOST_ROOT%\stage\lib
@set PATH=%PATH%;%BOOST_ROOT%
@set HIKYUU=%cd%

@REM 如果已经有其他的PYTHONPATH需要保留，需要象下面这样
@REM set PYTHONPATH=%PYTHONPATH%;%HIKYUU%\tools
@set PYTHONPATH=%HIKYUU%\tools