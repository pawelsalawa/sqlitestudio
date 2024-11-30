#!/usr/bin/env tclsh

# Download page:
# https://github.com/utelle/wxsqlite3/releases
# Link from download page will redicrect to the codeload.....

set THE_URL "https://github.com/utelle/SQLite3MultipleCiphers/releases/download/v1.9.1/sqlite3mc-1.9.1-sqlite-3.47.1-amalgamation.zip"

set SRC_DIR "src"
set FILES [list \
	sqlite3mc_amalgamation.c \
	sqlite3mc_amalgamation.h \
]

package require http

proc process {} {
	if {[catch {
		wget $::THE_URL sqlite.zip
		puts "Decompressing to 'sqlite' directory."
		exec 7z x -osqlite sqlite.zip
		
		#set dir [lindex [glob -directory sqlite wxsqlite3-*] 0]
		set dir sqlite
		
		foreach f $::FILES {
			copy $dir/$f
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
	upvar #0 $r state
	while {[lindex [http::code $r] 1] == "302"} {
		foreach {name value} $state(meta) {
			if {[regexp -nocase ^location$ $name]} {
				puts "Redirection to $value"
				set r [http::geturl $value -binary 1]
			}
		}
	}

	set fo [open $filename w]
	fconfigure $fo -translation binary
	puts -nonewline $fo [http::data $r]
	close $fo

	::http::cleanup $r
}

process