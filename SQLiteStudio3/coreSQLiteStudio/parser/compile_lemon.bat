@echo off
set LEMON=C:\utils\lemon.exe
set GCC=C:\Qt\Tools\mingw810_64\bin\gcc.exe

%GCC% -o %LEMON% lemon.c

