@echo off

where /q qmake
if "%errorlevel%" == "0" (
	for /f "delims=" %%a in ('where qmake') do @set QMAKE=%%a
	for %%F in (%QMAKE%) do set QT_DIR=%%~dpF
	echo INFO: Qt found at %QT_DIR%
) else (
	echo ERROR: Cannot find Qt
	GOTO:EOF
)

where /q mingw32-make
if "%errorlevel%" == "0" (
	for /f "delims=" %%a in ('where mingw32-make') do @set MAKE=%%a
	for %%F in (%MAKE%) do set MINGW_DIR=%%~dpF
	echo INFO: MinGW32 found in %MINGW_DIR%
) else (
	echo ERROR: Cannot find MinGW32 [mingw32-make.exe].
	GOTO:EOF
)

set cdir="%CD%"
cd ..\..
set parent_dir="%CD%"
cd %cdir%

set /p new_cpu_cores=Number of CPU cores to use for compiling (hit enter to use %NUMBER_OF_PROCESSORS%): 
if [%new_cpu_cores%] == [] (
	set cpu_cores=%NUMBER_OF_PROCESSORS%
) else (
	set cpu_cores=%new_cpu_cores%
)

set output="%parent_dir%\output"

if exist %output% (
	set /p yn=Directory %output% already exists. The script will delete and recreate it. Is that okay? [y/N]: 
	if "%yn%" == "y" (
	    del /S /Q /F %output%
		for /f %%f in ('dir /ad /b %output%') do rd /s /q %output%\%%f
		rd /s /q %output%
	) else (
	    echo ERROR: Aborted
		GOTO:EOF
	)
)

cd %parent_dir%
md output output\build output\build\Plugins

cd output\build
%QMAKE% ../../SQLiteStudio3
%MAKE% -j %cpu_cores%

cd Plugins
%QMAKE% ../../../Plugins
%MAKE% -j %cpu_cores%

cd %cdir%