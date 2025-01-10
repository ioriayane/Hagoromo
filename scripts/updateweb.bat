@echo off

for /f "usebackq delims=" %%A in (`PowerShell -Command "((Get-Content app/main.cpp) -match 'app.setApplicationVersion' | Select-String -Pattern '[0-9]+\.[0-9]+\.[0-9]+' -AllMatches).Matches.Value"`) do set VERSION_NO=%%A
python3 scripts\updateweb.py "web\layouts\shortcodes\download_link.html" %VERSION_NO%
