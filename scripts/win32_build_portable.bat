set QMAKE="C:\Qt\5.4\mingw491_32\bin\qmake.exe"
set MAKE="C:\Qt\Tools\mingw491_32\bin\mingw32-make.exe"
set cpu_cores=3

set cdir=%CD%

cd ..

rmdir /s /q output
mkdir output
mkdir output\build
mkdir output\build\Plugins

cd output/build
%QMAKE% CONFIG+=portable ..\..\SQLiteStudio3
%MAKE% -j %cpu_cores%

cd Plugins
%QMAKE% CONFIG+=portable ..\..\..\Plugins
%MAKE% -j %cpu_cores%

cd %cdir%
