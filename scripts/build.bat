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
set VS_SETUP_BAT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
set VS_REDIST_FOLDER="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC"
set BUILD_FOLDER=build-hagoromo
set DEPLOY_FOLDER=deploy-hagoromo


REM --- check path -------
qmake -v
if ERRORLEVEL 1 set path=%QT_BIN_FOLDER%;%PATH%
jom /version
if ERRORLEVEL 1 set path=%JOM_BIN_FOLDER%;%PATH%
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

qmake ..\Hagoromo.pro CONFIG+=HAGOROMO_RELEASE_BUILD
if NOT ERRORLEVEL 0 goto QUIT

jom
if NOT ERRORLEVEL 0 goto QUIT

cd %CWD%


REM --- deploy -------
copy %BUILD_FOLDER%\app\release\Hagoromo.exe %DEPLOY_FOLDER%\hagoromo\
copy %BUILD_FOLDER%\app\release\*.dll %DEPLOY_FOLDER%\hagoromo\
windeployqt --qmldir app\qml %DEPLOY_FOLDER%\hagoromo\Hagoromo.exe
python3 scripts\copymsvcfiles.py %VS_REDIST_FOLDER% %DEPLOY_FOLDER%\hagoromo\

copy %BUILD_FOLDER%\app\release\translations\*.qm %DEPLOY_FOLDER%\hagoromo\translations\

for /f "usebackq delims=" %%A in (`PowerShell -Command "((Get-Content app/main.cpp) -match 'app.setApplicationVersion' | Select-String -Pattern '[0-9]+\.[0-9]+\.[0-9]+' -AllMatches).Matches.Value"`) do set VERSION_NO=%%A
PowerShell -Command "Compress-Archive -Path deploy-hagoromo\hagoromo -DestinationPath deploy-hagoromo\hagoromo_%VERSION_NO%_windows.zip"

python3 scripts\updateweb.py "web\layouts\shortcodes\download_link.html" %VERSION_NO%

:QUIT
