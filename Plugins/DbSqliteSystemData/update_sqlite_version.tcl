#!/usr/bin/env tclsh

# Download page:
# http://system.data.sqlite.org/index.html/doc/trunk/www/downloads.wiki

set THE_URL "http://system.data.sqlite.org/blobs/1.0.112.0/sqlite-netFx-source-1.0.112.0.zip"
set SRC_DIR "SQLite.Interop/src"

proc process {} {
	if {[catch {
		wget $::THE_URL sqlite.zip
		puts "Decompressing to 'sqlite' directory."
		exec 7z x -osqlite sqlite.zip
		
		puts "Scanning source code"
		set maps [list \
				sqlite3 systemdata_sqlite3 \
				interop.h systemdata_interop.h \
			]

		foreach dir [glob -tails -directory sqlite/$::SRC_DIR -nocomplain *] {
			puts "Transforming ../$dir/ to systemdata_"
			lappend maps ../$dir/ systemdata_
		}
		
		foreach dir [glob -directory sqlite/$::SRC_DIR -nocomplain *] {
			foreach c [glob -directory $dir -nocomplain *.c] {
				copy $c $maps
			}
			foreach h [glob -directory $dir -nocomplain *.h] {
				copy $h $maps
			}
		}
		
		updatePro
		
	}]} {
		puts $::errorInfo
	}
	file delete -force sqlite.zip sqlite
}

proc updatePro {} {
	set fd [open DbSqliteSystemData.pro r]
	set data [read $fd]
	close $fd
	
	set res [lindex [regexp -inline {SOURCES\s*\+\=\s*((\S+\.cp{0,2})[\s\n\\]*)+} $data] 0]
	set sources [regexp -inline -all {\S+\.cp{0,2}} $res]
	
	set res [lindex [regexp -inline {HEADERS\s*\+\=\s*((\S+\.hp{0,2})[\s\n\\]*)+} $data] 0]
	set headers [regexp -inline -all {\S+\.hp{0,2}} $res]
	
	set newSources [glob systemdata_*.c*]
	set newHeaders [glob systemdata_*.h*]
	
	set mergedSources [list]
	foreach src $sources {
		if {[string match "systemdata_*" $src]} continue
		lappend mergedSources $src
	}
	lappend mergedSources {*}$newSources
	set newSourceContents "SOURCES += "
	append newSourceContents [join $mergedSources " \\\n    "]
	append newSourceContents "\n\n"
	
	set mergedHeaders [list]
	foreach hdr $headers {
		if {[string match "systemdata_*" $hdr]} continue
		lappend mergedHeaders $hdr
	}
	lappend mergedHeaders {*}$newHeaders
	set newHeaderContents "HEADERS += "
	append newHeaderContents [join $mergedHeaders " \\\n    "]
	append newHeaderContents "\n\n"

	regexp -indices {SOURCES\s*\+\=\s*((\S+\.cp{0,2})[\s\n\\]*)+} $data srcIdxPair
	set data [string replace $data {*}$srcIdxPair $newSourceContents]

	regexp -indices {HEADERS\s*\+\=\s*((\S+\.hp{0,2})[\s\n\\]*)+} $data hdrIdxPair
	set data [string replace $data {*}$hdrIdxPair $newHeaderContents]
	
	set fd [open DbSqliteSystemData.pro w+]
	puts $fd $data
	close $fd
}

proc copy {file maps} {
	set fd [open $file r]
	set data [read $fd]
	close $fd

	set data [string map $maps $data]
	set data [string map [list systemdata_systemdata_ systemdata_] $data]
 
	set outFile systemdata_[file tail $file]
	puts "Copying $file to $outFile"
	set fd [open $outFile w+]
	puts $fd $data
	close $fd
}

proc wget {url {filename {}}} {
	puts "Downloading $url"

	package require http

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