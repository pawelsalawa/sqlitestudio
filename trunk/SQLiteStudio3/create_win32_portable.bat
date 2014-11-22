@echo off

set QT_DIR=c:\Qt\5.3\mingw482_32\bin
set ZIP="c:\Program Files (x86)\7-Zip\7z.exe"

set QMAKE=%QT_DIR%\qmake.exe
set OLDDIR=%CD%

rem Find Qt
if exist %QMAKE% (
	echo Qt found at %QT_DIR%
) else (
	echo Cannot find Qt
	GOTO:EOF
)

rem Find 7zip
if exist %ZIP% (
	echo 7zip found at %ZIP%
) else (
	echo Cannot find 7zip
	GOTO:EOF
)

cd %OLDDIR%

rem Clean up
echo Cleaning up...
cd ..\output
rmdir /s /q portable

rem Create a copy
echo Creating a portable distribution
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
set QT_LIB_LIST=Qt5Core Qt5Gui Qt5Network Qt5PrintSupport Qt5Script Qt5Svg Qt5Widgets Qt5Xml icudt52 icuin52 icuuc52 libgcc_s_dw2-1 libstdc++-6 libwinpthread-1
for %%i in (%QT_LIB_LIST%) do (
	copy "%%i.dll" %PORTABLE% > nul
)

mkdir %PORTABLE%\iconengines %PORTABLE%\imageformats %PORTABLE%\platforms %PORTABLE%\printsupport
cd %QT_DIR%\..\plugins

copy iconengines\qsvgicon.dll %PORTABLE%\iconengines > nul
copy platforms\qwindows.dll %PORTABLE%\platforms > nul
copy printsupport\windowsprintersupport.dll %PORTABLE%\printsupport > nul
for %%i in (qdds qgif qicns qico qjpeg qsvg qtga qtiff qwbmp) do (
	copy imageformats\%%i.dll %PORTABLE%\imageformats > nul
)

rem Copy app-specific deps
cd %OLDDIR%\..\..\lib
copy *.dll %PORTABLE% > nul

call:getAppVersion
cd %PORTABLE%\..
%ZIP% a -r sqlitestudio-%APP_VERSION%.zip SQLiteStudio > nul

rem Incremental package
echo Creating incremental update package
cd %PORTABLE%\..
mkdir incremental\SQLiteStudio
xcopy SQLiteStudio incremental\SQLiteStudio /s /e /q > nul
cd incremental\SQLiteStudio
del /q Qt5*.dll
del /q icu*.dll
del /q libgcc* libstdc* libwinpthread*
rmdir /s /q iconengines imageformats platforms printsupport plugins

cd %PORTABLE%\..\incremental
%ZIP% a -r sqlitestudio-%APP_VERSION%.zip SQLiteStudio > nul

rem Plugin packages
echo Creating plugin updates
cd %PORTABLE%\..
for /f "delims=" %%p in ('SQLiteStudio\SQLiteStudio.exe --list-plugins') do (
	call:preparePlugin %%p
)

cd %OLDDIR%
GOTO:EOF

:preparePlugin
	set plugin=%~1
	set plugin_ver=%~2
	if exist SQLiteStudio\plugins\%plugin%.dll (
		echo Creating plugin update: %plugin%
		mkdir plugins\%plugin%\SQLiteStudio
		copy SQLiteStudio\plugins\%plugin%.dll plugins\%plugin%\SQLiteStudio > nul

		cd plugins\%plugin%
		%ZIP% a -r ..\%plugin%-%plugin_ver%.zip SQLiteStudio > nul
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
