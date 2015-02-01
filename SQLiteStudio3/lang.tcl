#!/usr/bin/env tclsh

proc usage {} {
    puts "$::argv0 (add|remove) <lang_name>"
    puts "$::argv0 (update|release)"
}

lassign $argv op lang

if {$::tcl_platform(platform) == "windows"} {
    set ERR_NULL "2>NUL"
} else {
    set ERR_NULL "2>/dev/null"
}

switch -- $op {
    "update" - "release" {
	if {$argc != 1} {
	    usage
	    exit 1
	}

	set files [list]
	foreach p [list coreSQLiteStudio guiSQLiteStudio sqlitestudio sqlitestudiocli] {
	    lappend files $p/$p.pro
	}

	foreach d [glob -directory ../Plugins -tails -nocomplain *] {
	    if {![file isdirectory ../Plugins/$d]} continue
	    lappend files ../Plugins/$d/$d.pro
	}
	
	foreach f $files {
	    catch {
		if {$op == "update"} {
		    exec lupdate $f
		} else {
		    exec lrelease $f $::ERR_NULL
		}
	    } res
	    if {$op == "release"} {
		puts $res
	    } else {
		foreach line [split $res \n] {
		    if {[string first Q_OBJECT $line] > -1} {
			puts $line
		    }
		    if {[regexp -- {^.*\w+\.ts.*$} $line]} {
			puts -nonewline [lindex [regexp -inline -- {^.*"([\w\/\\\.]+\.ts)".*$} $line] 1]
			puts -nonewline ": "
		    }
		    if {[regexp -- {^.*\d+[^\d]+\(\d+[^\d]+\d+.*\).*$} $line]} {
			puts -nonewline [lindex [regexp -inline -- {\S+.*} $line] 0]
			set new [lindex [regexp -inline -- {^.*\d+[^\d]+(\d+)[^\d]+\d+.*$} $line] 1]
			if {$new > 0} {
			    puts -nonewline " <- !!!!!!!!!!!"
			}
			puts ""
		    }
		}
	    }
	}
    }
    "add" - "remove" {
	if {$argc != 2} {
	    usage
	    exit 1
	}

	foreach p [list coreSQLiteStudio guiSQLiteStudio sqlitestudio sqlitestudiocli] {
	    set fd [open $p/$p.pro r]
	    set data [read $fd]
	    close $fd
	
	    set ts "translations/${p}_$lang.ts"
	    if {$op == "add"} {
		set data [string map [list "TRANSLATIONS += " "TRANSLATIONS += $ts \\\n\t\t"] $data]
	    } else {
		regsub -- "$ts\\s*(\\\\)?\n\\s*" $data "" data
	    }
	    
	    set fd [open $p/$p.pro w+]
	    puts $fd $data
	    close $fd
	    
	    puts "Updated $p.pro"
	}
	
	foreach d [glob -directory ../Plugins -tails -nocomplain *] {
	    if {![file isdirectory ../Plugins/$d]} continue
	
	    set fd [open ../Plugins/$d/$d.pro r]
	    set data [read $fd]
	    close $fd
	
	    set ts "${d}_$lang.ts"
	    if {$op == "add"} {
		set data [string map [list "TRANSLATIONS += " "TRANSLATIONS += $ts \\\n\t\t"] $data]
	    } else {
		regsub -- "$ts\\s*(\\\\)?\n\\s*" $data "" data
	    }
	    
	    set fd [open ../Plugins/$d/$d.pro w+]
	    puts $fd $data
	    close $fd
	    
	    puts "Updated $d.pro"
	}
    }
    default {
        usage
    }
}

