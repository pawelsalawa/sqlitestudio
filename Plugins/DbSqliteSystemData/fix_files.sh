#!/usr/bin/env tclsh

proc process {f} {
  set fd [open $f r]
  set data [read $fd]  
  close $fd
  
  #set data [string map [list systemdata_systemdata_ systemdata_] $data]
  set data [string map [list sqlite3 systemdata_sqlite3 ../core/ ""] $data]
  
  set fd [open $f w]
  puts $fd $data
  close $fd
}

foreach f {
	systemdata_sqlite3.c
	systemdata_sqlite3.h
	systemdata_sqlite3ext.h
	systemdata_interop.c
	systemdata_extension-functions.c
	systemdata_crypt.c
} {
	process $f
}

# Concat
set targetFile systemdata_sqlite3.c
set srcFiles {
	systemdata_interop.c
	systemdata_crypt.c
	systemdata_extension-functions.c
}

set trgFd [open $targetFile a+]
foreach f $srcFiles {
	set fd [open $f r]
	puts $trgFd [read $fd]
	close $fd
}
close $trgFd
