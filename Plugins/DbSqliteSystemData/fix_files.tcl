#!/usr/bin/env tclsh

proc process {f} {
  set shortName [string map [list systemdata_ ""] $f]
  if {[file exists $shortName]} {
    file rename -force $shortName $f
  }

  set fd [open $f r]
  set data [read $fd]  
  close $fd
  
  #set data [string map [list systemdata_systemdata_ systemdata_] $data]
  set data [string map [list \
	sqlite3 systemdata_sqlite3 \
	../core/ "" \
	interop.h systemdata_interop.h \
	../win/crypt.c systemdata_crypt.c \
	../contrib/extension-functions.c systemdata_extension-functions.c \
  ] $data]
  
  set fd [open $f w]
  puts $fd $data
  close $fd
}

foreach f {
	systemdata_sqlite3.c
	systemdata_sqlite3.h
	systemdata_sqlite3ext.h
	systemdata_interop.c
	systemdata_interop.h
	systemdata_extension-functions.c
	systemdata_crypt.c
} {
	process $f
}
