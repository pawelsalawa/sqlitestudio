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

process sqlcipher.c
process sqlcipher.h
