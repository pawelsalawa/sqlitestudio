@echo off

set QT_DIR=c:\Qt\5.3\mingw482_32\bin
set QMAKE=%QT_DIR%\qmake.exe
set OLDDIR=%CD%

rem Find Qt
if exist %QMAKE% (
	echo Qt found at %QT_DIR%
) else (
	echo Cannot find Qt
	GOTO:EOF
)

cd %OLDDIR%

@echo on

rem Clean up
cd ..\output

rem Create a copy
rmdir /s /q portable
mkdir portable\SQLiteStudio
xcopy SQLiteStudio portable\SQLiteStudio /s /e

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
	copy "%%i.dll" %PORTABLE%
)

mkdir %PORTABLE%\iconengines %PORTABLE%\imageformats %PORTABLE%\platforms %PORTABLE%\printsupport
cd %QT_DIR%\..\plugins

copy iconengines\qsvgicon.dll %PORTABLE%\iconengines
copy platforms\qwindows.dll %PORTABLE%\platforms
copy printsupport\windowsprintersupport.dll %PORTABLE%\printsupport
for %%i in (qdds qgif qicns qico qjpeg qsvg qtga qtiff qwbmp) do (
	copy imageformats\%%i.dll %PORTABLE%\imageformats
)

rem Copy app-specific deps
cd %OLDDIR%\..\..\lib
copy *.dll %PORTABLE%

@echo off
cd %OLDDIR%
