@echo off
goto Start

:Not_found_compiler
echo Can't found compiler
goto :eof

:Start
if NOT exist boost-build.jam copy boost-build.jam.win boost-build.jam
if NOT exist Jamroot copy Jamroot.win Jamroot

REM GUESS PROCESSOR_ARCHITECTURE
if "_%PROCESSOR_ARCHITECTURE%_" == "_AMD64_" (
    set ADDRESS_MODEL=address-model=64
    goto Guess_compiler)

:Guess_compiler	
REM GUESS COMPILER
if NOT "_%VS100COMNTOOLS%_" == "__" (
    set BUILD_OUTPUT_PATH=msvc-10.0\release\address-model-64\threading-multi
	set TOOLSET=toolset=msvc-10.0
	goto Start_build) 
if NOT "_%VS90COMNTOOLS%_" == "__" (
    set BUILD_OUTPUT_PATH="msvc-9.0\release\address-model-64\threading-multi"
	set TOOLSET=toolset=msvc-9.0
	goto Start_build) 
goto Not_found_compiler

:Start_build
@echo on
b2 -j 2 link=shared threading=multi %ADDRESS_MODEL% 

@if NOT exist .\bin mkdir .\bin

copy .\build\extern-libs\%BUILD_OUTPUT_PATH%\*.dll .\bin
copy .\build\hikyuu\%BUILD_OUTPUT_PATH%\*.dll .\bin
copy .\build\hikyuu_utils\%BUILD_OUTPUT_PATH%\*.dll .\bin

copy .\bin\*.dll C:\Windows\System32

copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\*.pyd .\tools\hikyuu
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\indicator\*.pyd .\tools\hikyuu\indicator
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\trade_manage\*.pyd .\tools\hikyuu\trade_manage
copy .\build\hikyuu_python\%BUILD_OUTPUT_PATH%\trade_sys\*.pyd .\tools\hikyuu\trade_sys

cd ./test
bjam -j 2 link=shared threading=multi %ADDRESS_MODEL% 
cd ..

python .\tools\hikyuu\test\test.py

::copy \workspace\fasiondog\trunk\build\cstock\msvc-10.0\release\\address-model-64\threading-multi\indicator\*.pyd \workspace\fasiondog\trunk\tools\cstock\cstock\indicator
::copy \workspace\fasiondog\trunk\build\cstock\msvc-10.0\release\\address-model-64\threading-multi\instance\*.pyd \workspace\fasiondog\trunk\tools\cstock\cstock\instance
::copy \workspace\fasiondog\trunk\build\cstock\msvc-10.0\release\\address-model-64\threading-multi\tradesys\*.pyd \workspace\fasiondog\trunk\tools\cstock\cstock\tradesys

::copy /Y \lib\*.dll c:\windows\system32
