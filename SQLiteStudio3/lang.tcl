#!/usr/bin/env tclsh

proc usage {} {
    puts "$argv0 (add|remove) <lang_name>"
}

if {$argc != 2} {
    usage
    exit 1
}

lassign $argv op lang

if {$op ni [list "add" "remove"]} {
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

    set ts "${p}_$lang.ts"
    if {$op == "add"} {
	set data [string map [list "TRANSLATIONS += " "TRANSLATIONS += $ts \\\n\t\t"] $data]
    } else {
	regsub -- "$ts\\s*(\\\\)?\n\\s*" $data "" data
    }
    
    set fd [open ../Plugins/$p/$p.pro w+]
    puts $fd $data
    close $fd
    
    puts "Updated $d.pro"
}
