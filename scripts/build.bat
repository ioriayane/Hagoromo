@echo off

set CWD=%CD%
set QTDIR=%1
set QT_BIN_FOLDER=%QTDIR%\bin

if '%QT_BIN_FOLDER%'=='' goto HELP
goto MAIN

REM --- help -----------------------------
:HELP
echo build.bat QT_BIN_FOLDER
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
set CMAKE_PLATFORM=x64
set OPENSSL_FOLDER=%QTDIR%/../../Tools/OpenSSLv3/Win_x64

echo VS_SETUP_BAT=%VS_SETUP_BAT%
echo VS_REDIST_FOLDER=%VS_REDIST_FOLDER%

REM --- build deps -------
cmd.exe /c %CWD%/scripts/build_zlib.bat

REM --- check path -------
qmake -v
if ERRORLEVEL 1 set path=%QT_BIN_FOLDER%;%PATH%
nmake /? /c
if ERRORLEVEL 1 call %VS_SETUP_BAT%


REM --- make folder -------
if EXIST %BUILD_FOLDER% rmdir /s /q %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %BUILD_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT

if EXIST %DEPLOY_FOLDER% rmdir /s /q %DEPLOY_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %DEPLOY_FOLDER%
if NOT ERRORLEVEL 0 goto QUIT
mkdir %DEPLOY_FOLDER%\hagoromo
if NOT ERRORLEVEL 0 goto QUIT



REM --- build -------
cd %BUILD_FOLDER%

cmake %SRC_FOLDER% -G "Visual Studio 17 2022" -DCMAKE_INSTALL_PREFIX="%DEPLOY_FOLDER%" -DHAGOROMO_RELEASE_BUILD=ON -DCMAKE_BUILD_TYPE:STRING=Release -A %CMAKE_PLATFORM%
if NOT ERRORLEVEL 0 goto QUIT

cmake --build . --target update_translations
if NOT ERRORLEVEL 0 goto QUIT

cmake --build . --config RELEASE --target INSTALL
if NOT ERRORLEVEL 0 goto QUIT


cd %CWD%


REM --- deploy -------
copy %BUILD_FOLDER%\app\release\Hagoromo.exe %DEPLOY_FOLDER%\hagoromo\
copy %BUILD_FOLDER%\app\release\*.dll %DEPLOY_FOLDER%\hagoromo\
copy zlib\bin\zlib.dll %DEPLOY_FOLDER%\hagoromo\
windeployqt --qmldir app\qml --openssl-root %OPENSSL_FOLDER% --no-translations %DEPLOY_FOLDER%\hagoromo\Hagoromo.exe
python3 scripts\copymsvcfiles.py %VS_REDIST_FOLDER% %DEPLOY_FOLDER%\hagoromo\

for /f "usebackq delims=" %%A in (`PowerShell -Command "((Get-Content app/main.cpp) -match 'app.setApplicationVersion' | Select-String -Pattern '[0-9]+\.[0-9]+\.[0-9]+' -AllMatches).Matches.Value"`) do set VERSION_NO=%%A
PowerShell -Command "Compress-Archive -Path deploy-hagoromo\hagoromo -DestinationPath deploy-hagoromo\hagoromo_%VERSION_NO%_windows.zip"

:QUIT
