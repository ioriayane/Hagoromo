@echo off

set CWD=%CD%
set QT_BIN_FOLDER=%1

if '%QT_BIN_FOLDER%'=='' goto HELP
goto MAIN

REM --- help -----------------------------
:HELP
echo build.bat QT_BIN_FOLDER
echo ex:
echo   build.bat path\to\Qt\5.15.2\msvc2019_64\bin

goto QUIT

REM --- main -----------------------------
:MAIN

set JOM_BIN_FOLDER=%QT_BIN_FOLDER%\..\..\..\Tools\QtCreator\bin\jom
IF "!VS_SETUP_BAT!"=="" set VS_SETUP_BAT="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
set BUILD_FOLDER=build-hagoromo

echo VS_SETUP_BAT=%VS_SETUP_BAT%

REM --- check path -------
qmake -v
if ERRORLEVEL 1 set path=%QT_BIN_FOLDER%;%PATH%
jom /version
if ERRORLEVEL 1 set path=%JOM_BIN_FOLDER%;%PATH%
nmake /? /c
if ERRORLEVEL 1 call %VS_SETUP_BAT%

REM --- make folder -------
if EXIST %BUILD_FOLDER% rmdir /s /q %BUILD_FOLDER%
if ERRORLEVEL 1 goto QUIT
mkdir %BUILD_FOLDER%
if ERRORLEVEL 1 goto QUIT

REM --- build -------
cd %BUILD_FOLDER%

qmake ..\Hagoromo.pro CONFIG+=debug
if ERRORLEVEL 1 goto QUIT

jom
if ERRORLEVEL 1 goto QUIT

cd %CWD%


REM --- deploy -------
REM windeployqt --qmldir app\qml %BUILD_FOLDER%\atprotocol_test\debug\atprotocol_test.exe

REM --- run -------
%BUILD_FOLDER%\tests\atprotocol_test\debug\atprotocol_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\chat_test\debug\chat_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\hagoromo_test\debug\hagoromo_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\hagoromo_test2\debug\hagoromo_test2.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\http_test\debug\http_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\log_test\debug\log_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\oauth_test\debug\oauth_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\realtime_test\debug\realtime_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\search_test\debug\search_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL
%BUILD_FOLDER%\tests\tools_test\debug\tools_test.exe
if not ERRORLEVEL 0 goto TEST_FAIL

goto QUIT
:TEST_FAIL
echo !!!!!!! Fail !!!!!!!
exit /b 1

:QUIT
