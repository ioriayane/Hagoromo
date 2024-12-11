@echo off

set CWD=%CD%

REM --- main -----------------------------

set VS_SETUP_BAT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
set SRC_FOLDER=../zlib
set BUILD_FOLDER=build-zlib
set DEPLOY_FOLDER=../../zlib
set CMAKE_PLATFORM=x64


REM --- check path -------
nmake /? /c
if ERRORLEVEL 1 call %VS_SETUP_BAT%

REM --- make folder -------
cd 3rdparty
if EXIST %BUILD_FOLDER% rmdir /s /q %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT


REM --- build -------
cd %BUILD_FOLDER%

cmake %SRC_FOLDER% -G "Visual Studio 17 2022" -DCMAKE_INSTALL_PREFIX="%DEPLOY_FOLDER%" -A %CMAKE_PLATFORM%
if NOT ERRORLEVEL 0 goto QUIT

cmake --build . --config RELEASE --target INSTALL
if NOT ERRORLEVEL 0 goto QUIT
cmake --build . --config DEBUG --target INSTALL
if NOT ERRORLEVEL 0 goto QUIT

cd %CWD%

REM --- clean up -----
cd 3rdparty/zlib
git checkout .
cd %CWD%

:QUIT
