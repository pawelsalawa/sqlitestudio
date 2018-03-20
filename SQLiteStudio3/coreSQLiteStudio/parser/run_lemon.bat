set LEMON=C:\utils\lemon.exe

rem %LEMON% -l -q -s sqlite3_parse.y
%LEMON% -l -q sqlite3_parse.y
move sqlite3_parse.c sqlite3_parse.cpp

%LEMON% -l -q sqlite2_parse.y
move sqlite2_parse.c sqlite2_parse.cpp
