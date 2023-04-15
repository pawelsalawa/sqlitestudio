#!/usr/bin/env tclsh

# This one has to be run under Unix, because of autotools

# Download page:
# https://github.com/sqlcipher/sqlcipher/releases

set THE_URL "https://codeload.github.com/sqlcipher/sqlcipher/zip/v4.5.3"

proc process {} {
	if {[catch {
		wget $::THE_URL sqlite.zip
		puts "Decompressing to 'sqlite' directory."
		exec 7z x -osqlite sqlite.zip
		
		set dir [lindex [glob -directory sqlite sqlcipher-*] 0]
		
		puts "Running ./configure and make to generate sqlite3.c"
		cd $dir
		catch {exec ./configure --enable-fts5 --enable-json1 --enable-update-limit --enable-geopoly --enable-rtree --enable-session --enable-gcov --disable-tcl}
		catch {exec make}
		cd ../..
		
		copy $dir/sqlite3.c
		copy $dir/sqlite3.h

		file rename -force sqlite3.c sqlcipher.c
		file rename -force sqlite3.h sqlcipher.h
	}]} {
		puts $::errorInfo
	}
	file delete -force sqlite.zip sqlite
}

proc copy {file} {
	set fd [open $file r]
	set data [read $fd]
	close $fd

	set data [string map [list sqlite3 sqlcipher_sqlite3] $data]
 
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
		package require tls
		http::register https 443 [list ::tls::socket]
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