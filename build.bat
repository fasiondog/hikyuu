@call set_env

@echo off
goto Start

:Not_found_compiler
echo Can't found compiler
goto :eof

:Start
if NOT exist boost-build.jam copy boost-build.jam.win boost-build.jam
if NOT exist Jamroot copy Jamroot.win Jamroot

set ADDRESS_MODEL_OUTPUT=
REM GUESS PROCESSOR_ARCHITECTURE
if "_%PROCESSOR_ARCHITECTURE%_" == "_AMD64_" (
    set ADDRESS_MODEL=address-model=64
    set ADDRESS_MODEL_OUTPUT=address-model-64
    goto Guess_compiler)

:Guess_compiler	
REM GUESS COMPILER
REM if NOT "_%VS140COMNTOOLS%_" == "__" (
REM     set BUILD_OUTPUT_PATH=msvc-14.0\release\address-model-64\threading-multi
REM    	set TOOLSET=toolset=msvc-14.0
REM    	goto Start_build) 
REM if NOT "_%VS120COMNTOOLS%_" == "__" (
REM     set BUILD_OUTPUT_PATH=msvc-12.0\release\address-model-64\threading-multi
REM 	set TOOLSET=toolset=msvc-12.0
REM 	goto Start_build) 
REM if NOT "_%VS150COMNTOOLS%_" == "__" (
REM     set BUILD_OUTPUT_PATH=msvc-14.1\release\address-model-64\threading-multi
REM    	set TOOLSET=toolset=msvc-14.1
REM    	goto Start_build) 
if NOT "_%VS100COMNTOOLS%_" == "__" (
    set BUILD_OUTPUT_PATH=msvc-10.0\release\%ADDRESS_MODEL_OUTPUT%\threading-multi
	set TOOLSET=toolset=msvc-10.0
	goto Start_build) 
if NOT "_%VS90COMNTOOLS%_" == "__" (
    set BUILD_OUTPUT_PATH="msvc-9.0\release\%ADDRESS_MODEL_OUTPUT%\threading-multi"
	set TOOLSET=toolset=msvc-9.0
	goto Start_build) 
goto Not_found_compiler

:Start_build
@echo on
%BOOST_ROOT%\b2 -j 4 link=shared threading=multi %ADDRESS_MODEL%
REM %TOOLSET%  需要利用Jamroot消除告警

@if NOT exist .\bin mkdir .\bin

del .\tools\hikyuu\*.dll
del .\tools\hikyuu\*.exe
del .\tools\hikyuu\*.pyd /S

copy .\extern-libs\boost\*.dll .\tools\hikyuu
copy .\extern-libs\hdf5\dll\*.dll .\tools\hikyuu
copy .\extern-libs\log4cplus\dll\*.dll .\tools\hikyuu
copy .\extern-libs\mysql\lib\*.dll .\tools\hikyuu
copy .\extern-libs\msvc\*.dll .\tools\hikyuu

copy .\build\extern-libs\%BUILD_OUTPUT_PATH%\*.dll .\tools\hikyuu
copy .\build\hikyuu\%BUILD_OUTPUT_PATH%\*.dll .\tools\hikyuu
copy .\build\hikyuu_utils\%BUILD_OUTPUT_PATH%\*.dll .\tools\hikyuu
copy .\build\importdata\%BUILD_OUTPUT_PATH%\*.exe .\tools\hikyuu

copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\*.pyd .\tools\hikyuu
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\indicator\*.pyd .\tools\hikyuu\indicator
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\trade_manage\*.pyd .\tools\hikyuu\trade_manage
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\trade_sys\*.pyd .\tools\hikyuu\trade_sys
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\trade_instance\*.pyd .\tools\hikyuu\trade_instance
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\data_driver\*.pyd .\tools\hikyuu\data_driver

cd ./test
%BOOST_ROOT%\b2 -j 4 link=shared threading=multi %ADDRESS_MODEL% 
cd ..

python .\tools\hikyuu\test\test.py
