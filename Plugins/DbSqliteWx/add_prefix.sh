#!/usr/bin/env tclsh

proc process {f} {
  set fd [open $f r]
  set data [read $fd]  
  close $fd
  
  set data [string map [list sqlite3 wx_sqlite3] $data]
  
  set fd [open $f w]
  puts $fd $data
  close $fd
}

foreach f {
	wxsqlite3.c
	wxsqlite3.h
	sqlite3ext.h
	sqlite3secure.c
	sqlite3userauth.h
	userauth.c
	extensionfunctions.c
	codec.c
	codec.h
	codecext.c
	csv.c
	rijndael.c
	rijndael.h
	sha2.h
	sha2.c
} {
	process $f
}
