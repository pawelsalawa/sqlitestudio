@echo off
set LEMON=C:\utils\lemon.exe

rem %LEMON% -l -q -s sqlite3_parse.y
rem %LEMON% -l -p sqlite3_parse.y
%LEMON% -l -q sqlite3_parse.y
move sqlite3_parse.c sqlite3_parse.cpp
