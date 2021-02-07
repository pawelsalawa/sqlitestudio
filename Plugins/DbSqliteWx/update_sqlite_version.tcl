#!/usr/bin/env tclsh

# Download page:
# https://github.com/utelle/wxsqlite3/releases
# Link from download page will redicrect to the codeload.....
# So don't replace whole link, only change version part.

set THE_URL "https://codeload.github.com/utelle/wxsqlite3/zip/v4.6.4"
set SRC_DIR "src"
set FILES [list \
	sqlite3mc_amalgamation.c \
	sqlite3mc_amalgamation.h \
]

proc process {} {
	if {[catch {
		wget $::THE_URL sqlite.zip
		puts "Decompressing to 'sqlite' directory."
		exec 7z x -osqlite sqlite.zip
		
		set dir [lindex [glob -directory sqlite wxsqlite3-*] 0]
		
		foreach f $::FILES {
			copy $dir/$::SRC_DIR/$f
		}

		file rename -force sqlite3mc_amalgamation.c wxsqlite3.c
		file rename -force sqlite3mc_amalgamation.h wxsqlite3.h
		
	}]} {
		puts $::errorInfo
	}
	file delete -force sqlite.zip sqlite
}

proc copy {file} {
	set fd [open $file r]
	set data [read $fd]
	close $fd

	set data [string map [list sqlite3 wx_sqlite3] $data]
	set data [string map [list \
			wx_sqlite3mc_amalgamation. wxsqlite3. \
		] $data]
 
	set outFile [file tail $file]
	puts "Copying $outFile"
	set fd [open $outFile w+]
	puts $fd $data
	close $fd
}

proc wget {url {filename {}}} {
	puts "Downloading $url"

	package require http
	if {[catch {package require twapi_crypto}]} {
		package require tls 1.7
		http::register https 443 [list ::tls::socket -autoservername true]
	} else {
		http::register https 443 [list ::twapi::tls_socket]
	}
	
	if {$filename == ""} {
		set filename [file tail $url]
	}
	set r [http::geturl $url -binary 1]

	set fo [open $filename w]
	fconfigure $fo -translation binary
	puts -nonewline $fo [http::data $r]
	close $fo

	::http::cleanup $r
}

process