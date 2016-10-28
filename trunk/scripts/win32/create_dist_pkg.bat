@echo off

set OLDDIR=%CD%

rem Find Qt
where /q qmake
IF "%errorlevel%" == "0" (
	for /f "delims=" %%a in ('where qmake') do @set QMAKE=%%a
	for %%F in ("%QMAKE%") do set QT_DIR="%%~dpF"
	echo INFO: Qt found at %QT_DIR%
) else (
	echo ERROR: Cannot find Qt
	GOTO:EOF
)

rem Find 7zip
set USE_ZIP=0
where /q 7z
IF "%errorlevel%" == "1" (
	echo INFO: No 7z.exe. *.zip packages will not be created, only a runnable distribution.
	GOTO AfterZip
)
for /f "delims=" %%a in ('where 7z') do @set ZIP=%%a
if exist %ZIP% (
	echo INFO: 7zip found at %ZIP%
	set USE_ZIP=1
)
:AfterZip

cd %OLDDIR%
cd ..\..
set parent_dir="%CD%"

rem Clean up
echo INFO: Cleaning up...
cd %parent_dir%\output
rmdir /s /q portable

rem Create a copy
echo INFO: Creating a portable distribution
mkdir portable\SQLiteStudio
xcopy SQLiteStudio portable\SQLiteStudio /s /e /q > nul

rem Remove .a files from app dir
cd portable\SQLiteStudio
del /q *.a
set PORTABLE=%CD%

rem Remove .a files from plugins dir
cd plugins
del /q *.a
rem Copy Qt files
cd %QT_DIR%
set QT_LIB_LIST=Qt5Core Qt5Gui Qt5Network Qt5PrintSupport Qt5Script Qt5Svg Qt5Widgets Qt5Xml icudt53 icuin53 icuuc53 libgcc_s_dw2-1 libstdc++-6 libwinpthread-1
for %%i in (%QT_LIB_LIST%) do (
	copy "%%i.dll" %PORTABLE% > nul
)
copy qt.conf %PORTABLE% > nul

mkdir %PORTABLE%\iconengines %PORTABLE%\imageformats %PORTABLE%\platforms %PORTABLE%\printsupport
cd %QT_DIR%\..\plugins

copy iconengines\qsvgicon.dll %PORTABLE%\iconengines > nul
copy platforms\qwindows.dll %PORTABLE%\platforms > nul
copy printsupport\windowsprintersupport.dll %PORTABLE%\printsupport > nul
for %%i in (qdds qgif qicns qico qjpeg qsvg qtga qtiff qwbmp) do (
	copy imageformats\%%i.dll %PORTABLE%\imageformats > nul
)

rem Copy app-specific deps
cd %parent_dir%\..\lib
copy *.dll %PORTABLE% > nul

call:getAppVersion
cd %PORTABLE%\..
if "%USE_ZIP%" == "1" "%ZIP%" a -r sqlitestudio-%APP_VERSION%.zip SQLiteStudio > nul

rem Incremental package
echo INFO: Creating incremental update package
cd %PORTABLE%\..
mkdir incremental\SQLiteStudio
xcopy SQLiteStudio incremental\SQLiteStudio /s /e /q > nul
cd incremental\SQLiteStudio
del /q Qt5*.dll
rem del /q icu*.dll > nul
del /q libgcc* libstdc* libwinpthread*
rmdir /s /q iconengines imageformats platforms printsupport plugins

cd %PORTABLE%\..\incremental
if "%USE_ZIP%" == "1" "%ZIP%" a -r sqlitestudio-%APP_VERSION%.zip SQLiteStudio > nul

rem Plugin packages
echo INFO: Creating plugin updates
cd %PORTABLE%\..
for /f "delims=" %%p in ('SQLiteStudio\SQLiteStudio.exe --list-plugins') do (
	call:preparePlugin %%p
)

cd %OLDDIR%

echo INFO: Portable distribution created at %PORTABLE%
GOTO:EOF

:preparePlugin
	set plugin=%~1
	set plugin_ver=%~2
	if exist SQLiteStudio\plugins\%plugin%.dll (
		echo INFO: Creating plugin update: %plugin%
		mkdir plugins\%plugin%\SQLiteStudio\plugins
		copy SQLiteStudio\plugins\%plugin%.dll plugins\%plugin%\SQLiteStudio\plugins > nul

		cd plugins\%plugin%
		if "%USE_ZIP%" == "1" "%ZIP%" a -r ..\%plugin%-%plugin_ver%.zip SQLiteStudio > nul
		cd ..\..
	)
GOTO:EOF

:getAppVersion
	pushd
	cd %PORTABLE%
	for /f "delims=" %%v in ('sqlitestudiocli --version') do (
		call:getAppVersionFromSecondArgument %%v
	)
	popd
GOTO:EOF

:getAppVersionFromSecondArgument
	set APP_VERSION=%~2
GOTO:EOF
