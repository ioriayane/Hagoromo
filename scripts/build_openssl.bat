@echo off

set CWD=%CD%

REM --- main -----------------------------

IF not defined  VS_SETUP_BAT set VS_SETUP_BAT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
set SRC_FOLDER=..\openssl
set BUILD_FOLDER=build-openssl
set DEPLOY_FOLDER=%CWD%\openssl

echo VS_SETUP_BAT=%VS_SETUP_BAT%

REM --- check path -------
nmake /? /c
if ERRORLEVEL 1 call %VS_SETUP_BAT%

REM --- make folder -------
cd 3rdparty
if EXIST %BUILD_FOLDER% rmdir /s /q %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT_ERROR
mkdir %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT_ERROR


REM --- build -------
cd %BUILD_FOLDER%

perl %SRC_FOLDER%\Configure VC-WIN64A no-asm ^
                            zlib-dynamic ^
                            --with-zlib-include="%CWD%\zlib\include" ^
                            --with-zlib-lib="%CWD%\zlib\lib\zlib.lib" ^
                            --prefix="%DEPLOY_FOLDER%" ^
                            --openssldir="%DEPLOY_FOLDER%"
if ERRORLEVEL 1 goto QUIT_ERROR

nmake
if ERRORLEVEL 1 goto QUIT_ERROR
nmake install
if ERRORLEVEL 1 goto QUIT_ERROR

cd %CWD%

REM --- clean up -----
cd 3rdparty/openssl
git checkout .
cd %CWD%


:QUIT
exit /b 0

:QUIT_ERROR
exit /b 1
