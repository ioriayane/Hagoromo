@echo off

set CWD=%CD%
set QTDIR=%1

if '%QTDIR%'=='' goto HELP
goto MAIN

REM --- help -----------------------------
:HELP
echo build.bat QTDIR
echo ex:
echo   build.bat path\to\Qt\6.8.1\msvc2019_64

goto QUIT

REM --- main -----------------------------
:MAIN

IF not defined  VS_SETUP_BAT set VS_SETUP_BAT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
IF not defined  VS_REDIST_FOLDER set VS_REDIST_FOLDER="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC"
set SRC_FOLDER=..
set BUILD_FOLDER=build-hagoromo
set DEPLOY_FOLDER=deploy-hagoromo
set OPENSSL_FOLDER=%QTDIR%/../../Tools/OpenSSLv3/Win_x64

echo VS_SETUP_BAT=%VS_SETUP_BAT%
echo VS_REDIST_FOLDER=%VS_REDIST_FOLDER%

REM --- build deps -------
cmd.exe /c %CWD%/scripts/build_zlib.bat

REM --- check path -------
nmake /? /c
if ERRORLEVEL 1 call %VS_SETUP_BAT%

set PATH=%QTDIR%\lib;%PATH%
set PATH=%QTDIR%\bin;%PATH%
set PATH=%OPENSSL_FOLDER%\bin;%PATH%
set PATH=%CWD%\zlib\bin;%PATH%

REM --- make folder -------
if EXIST %BUILD_FOLDER% rmdir /s /q %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT

if EXIST %DEPLOY_FOLDER% rmdir /s /q %DEPLOY_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %DEPLOY_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT


REM --- build -------
cd %BUILD_FOLDER%

cmake .. -G Ninja -DCMAKE_PREFIX_PATH:PATH='%QTDIR%' ^
    -DCMAKE_INSTALL_PREFIX:PATH='..\..\%DEPLOY_FOLDER%\hagoromo' ^
    -DCMAKE_INSTALL_LIBDIR:PATH='.' ^
    -DCMAKE_INSTALL_BINDIR:PATH='.' ^
    -DHAGOROMO_UNIT_TEST_BUILD=ON ^
    -DCMAKE_BUILD_TYPE:STRING=Debug
if not ERRORLEVEL 0 goto TEST_FAIL

cmake --build . --target tests\all

REM --- run -------
ctest --test-dir tests -C Debug -j 2

if not ERRORLEVEL 0 goto TEST_FAIL

cd %CWD%

goto QUIT
:TEST_FAIL
echo !!!!!!! Fail !!!!!!!
exit /b 1

:QUIT
