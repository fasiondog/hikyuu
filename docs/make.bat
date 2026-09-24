@ECHO OFF

pushd %~dp0

REM Command file for Sphinx documentation
REM   make.bat        构建中英两棵树 -> build\html\{en,zh}
REM   make.bat en     仅构建英文树   -> build\html\en
REM   make.bat zh     仅构建中文树   -> build\html\zh

if "%SPHINXBUILD%" == "" (
	set SPHINXBUILD=sphinx-build
)
set BUILDDIR=build\html

%SPHINXBUILD% >NUL 2>NUL
if errorlevel 9009 (
	echo.
	echo.The 'sphinx-build' command was not found. Make sure you have Sphinx
	echo.installed, then set the SPHINXBUILD environment variable to point
	echo.to the full path of the 'sphinx-build' executable. Alternatively,
	echo.you may add the Sphinx directory to PATH; if you don't have Sphinx
	echo.installed, grab it from https://www.sphinx-doc.org/
	exit /b 1
)

if "%1" == "" goto both

%SPHINXBUILD% -b html %1 %BUILDDIR%\%1 %SPHINXOPTS% %O%
goto end

:both
%SPHINXBUILD% -b html en %BUILDDIR%\en %SPHINXOPTS% %O%
%SPHINXBUILD% -b html zh %BUILDDIR%\zh %SPHINXOPTS% %O%
goto end

:end
popd
