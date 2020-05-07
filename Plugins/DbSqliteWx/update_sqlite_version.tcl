#!/usr/bin/env tclsh

# Download page:
# https://github.com/utelle/wxsqlite3/releases

set THE_URL "https://codeload.github.com/utelle/wxsqlite3/zip/v4.5.1"
set SRC_DIR "sqlite3secure/src"
set FILES [list \
	chacha20poly1305.c \
    fastpbkdf2.c \
    fileio.c \
    md5.c \
    rekeyvacuum.c \
    sha1.c \
    shathree.c \
    test_windirent.c \
	carray.c \
	codec.c \
	codecext.c \
	csv.c \
	extensionfunctions.c \
	rijndael.c \
	sha2.c \
	sqlite3.c \
	sqlite3secure.c \
	userauth.c \
	series.c \
	\
	codec.h \
    rijndael.h \
    sha2.h \
	sqlite3.h \
    sqlite3ext.h \
    sqlite3userauth.h \
    fastpbkdf2.h \
    sha1.h \
    sqlite3secure.h \
    test_windirent.h \
	mystdint.h \
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

		file rename -force sqlite3.c wxsqlite3.c
		file rename -force sqlite3.h wxsqlite3.h
		
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
			wx_sqlite3secure. sqlite3secure. \
			wx_sqlite3ext.h sqlite3ext.h \
			wx_sqlite3userauth.h sqlite3userauth.h \
			wx_sqlite3.c wxsqlite3.c \
			wx_sqlite3.h wxsqlite3.h \
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