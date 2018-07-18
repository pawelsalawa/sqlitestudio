#!/usr/bin/env tclsh

proc process {f} {
  set fd [open $f r]
  set data [read $fd]  
  close $fd
  
  set data [string map [list sqlite3 wx_sqlite3] $data]
  set data [string map [list wx_sqlite3.c wxsqlite3.c] $data]
  set data [string map [list wx_sqlite3.h wxsqlite3.h] $data]
  
  set fd [open $f w]
  puts $fd $data
  close $fd
}

if {[file exists sqlite3.c]} {
	file rename -force sqlite3.c wxsqlite3.c
}

if {[file exists sqlite3.h]} {
	file rename -force sqlite3.h wxsqlite3.h
	file copy -force wxsqlite3.h wxsqlite3_unmodified.h
}

foreach f [concat [glob -nocomplain *.c] [glob -nocomplain *.h]] {
	if {[string match -nocase "dbsqlitewx*" $f]} continue
	process $f
}
