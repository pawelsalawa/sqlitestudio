#!/usr/bin/env tclsh

proc process {f} {
  set fd [open $f r]
  set data [read $fd]  
  close $fd
  
  set data [string map [list sqlite3 sqlcipher_sqlite3] $data]
  
  set fd [open $f w]
  puts $fd $data
  close $fd
}

if {[file exists sqlite3.c]} {
    file rename -force sqlite3.c sqlcipher.c
}

if {[file exists sqlite3.h]} {
    file rename -force sqlite3.h sqlcipher.h
}

process sqlcipher.c
process sqlcipher.h
