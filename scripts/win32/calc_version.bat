@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "FILE=%~dp0..\..\SQLiteStudio3\coreSQLiteStudio\sqlitestudio.cpp"

if not exist "%FILE%" (
    echo ERROR: File not found:
    echo   %FILE%
    exit /b 1
)

set "LINE="
for /f "usebackq delims=" %%L in (`findstr /R /C:"sqlitestudioVersion[ ]*=[^;]*;" "%FILE%"`) do (
    if not defined LINE set "LINE=%%L"
)

if not defined LINE (
    echo ERROR: sqlitestudioVersion not found
    exit /b 2
)

for /f "tokens=2 delims==" %%A in ("!LINE!") do set "TMP=%%A"

set "VERSION=!TMP:;=!"
set "VERSION=!VERSION: =!"

set /a MAJOR=VERSION/10000
set /a MINOR=(VERSION/100)%%100
set /a PATCH=VERSION%%100

echo %MAJOR%.%MINOR%.%PATCH% %VERSION%
