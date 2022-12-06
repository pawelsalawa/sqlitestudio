#!/usr/bin/env tclsh

proc usage {} {
    puts "$::argv0 add_lang <lang_name>"
    puts "$::argv0 add_plugin <plugin>"
    puts "$::argv0 status"
}

lassign $argv op lang

if {$::tcl_platform(platform) == "windows"} {
    set ERR_NULL "2>NUL"
} else {
    set ERR_NULL "2>/dev/null"
}

proc find {dir mask} {
	set results [list]
	foreach f [glob -nocomplain -directory $dir *] {
		if {[file isdirectory $f]} {
			lappend results {*}[find $f $mask]
			continue;
		}
		
		if {[string match $mask [lindex [file split $f] end]]} {
			lappend results $f
		}
	}
	return $results
}

proc countstrings {data search} {
    set l [string length $search]
    set count 0
    while {[set i [string first $search $data]]>=0} {
        incr count
        incr i $l
        set data [string range $data $i end]
    }
    set count
}

proc findLangs {} {
    set langs [list]
    foreach f [find coreSQLiteStudio "*.ts"] {
		set lang [lindex [regexp -inline {[^_]*_(\w+(\w+)?).ts$} $f] 1]
		if {$lang == ""} continue
        lappend langs $lang
	}
	return $langs
}

proc scanLangs {} {
    set langs [dict create]
    foreach f [find .. "*.ts"] {
        set lang [lindex [regexp -inline {[^_]*_(\w+(\w+)?).ts$} $f] 1]
        if {[dict exists $langs $lang]} {
            set langDict [dict get $langs $lang]
        } else {
            set langDict [dict create translated 0 untranslated 0]
        }
    
        set fd [open $f r]
        set data [read $fd]
        close $fd
        
        set c1 [countstrings $data "<translation>"]
        set c2 [countstrings $data "<translation type=\"unfinished\">"]
        dict incr langDict translated $c1
        dict incr langDict untranslated $c2
        dict set langs $lang $langDict
    }

    return $langs
}

proc finalTsFix {f} {
	set fd [open $f r]
	set data [read $fd]
	close $fd
	
	set parts [lsort -unique [regexp -all -inline -- {>[^<]*\"[^<]*<} $data]]
	foreach part $parts {
		set fixedPart [string map [list \" "&quot;"] $part]
		set data [string trim [string map [list $part $fixedPart] $data]]
	}
	
	set data [string trim [string map [list ' "&apos;" " " "&#xa0;"] $data]]
	
	set fd [open $f w+]
	puts $fd $data
	close $fd
}

switch -- $op {
    "update" {
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
				puts "updating $f"
				exec lupdate $f
			} res
			# foreach line [split $res \n] {
				# if {[string first Q_OBJECT $line] > -1} {
					# puts $line
				# }
				# if {[regexp -- {^.*\w+\.ts.*$} $line]} {
					# puts -nonewline [lindex [regexp -inline -- {^.*"([\w\/\\\.]+\.ts)".*$} $line] 1]
					# puts -nonewline ": "
				# }
				# if {[regexp -- {^.*\d+[^\d]+\(\d+[^\d]+\d+.*\).*$} $line]} {
					# puts -nonewline [lindex [regexp -inline -- {\S+.*} $line] 0]
					# set new [lindex [regexp -inline -- {^.*\d+[^\d]+(\d+)[^\d]+\d+.*$} $line] 1]
					# if {$new > 0} {
						# puts -nonewline " <- !!!!!!!!!!!"
					# }
					# puts ""
				# }
			# }
		}

		set tsFiles [list]
		foreach d [list coreSQLiteStudio guiSQLiteStudio sqlitestudio sqlitestudiocli] {
			lappend tsFiles {*}[glob -directory $d/translations -nocomplain *.ts]
		}

		foreach d [glob -directory ../Plugins -tails -nocomplain *] {
			if {![file isdirectory ../Plugins/$d/translations]} continue
			lappend tsFiles {*}[glob -directory ../Plugins/$d/translations -nocomplain *.ts]
		}

		foreach f $tsFiles {
			if {[catch {
				puts "formatting $f"
				exec xmllint --format -o $f $f
				finalTsFix $f
			} res]} {
				puts $res
			}
		}
    }
    "status" {
		set langs [scanLangs]
		foreach k [dict keys $langs] {
			set lang [dict get $langs $k]
			set tr [dict get $lang translated]
			set untr [dict get $lang untranslated]
			set all [expr {$tr + $untr}]
			if {$all == 0} continue

			set perc [expr {round(double($tr)/$all * 1000)/10.0}]
			
			set lang [string tolower $lang]
			puts "$lang - ${perc}% ($tr / $all)"
		}
    }
    "add_plugin" {
		if {$argc != 2} {
			usage
			exit 1
		}
		
		set plug [lindex $argv 1]
		set plugPro ../Plugins/$plug/$plug.pro
		if {![file exists $plugPro]} {
			puts "$plugPro does not exist."
			exit 1
		}
		
		if {![file exists ../Plugins/$plug/translations]} {
			file mkdir ../Plugins/$plug/translations
		}
		
		set plugTs ../Plugins/$plug/translations/$plug.ts
		if {![file exists $plugTs]} {
			catch {
				exec lupdate-pro $plugPro -ts $plugTs
			}
			if {![file exists $plugTs]} {
				puts "Failed to create $plugTs."
				exit 1
			}
		}
		
		foreach lang [findLangs] {
			set plugTs ../Plugins/$plug/translations/${plug}_${lang}.ts
			if {![file exists $plugTs]} {
				catch {exec lupdate-pro $plugPro -ts $plugTs}
			}
		}
    }
    "add_lang" {
		if {$argc != 2} {
			usage
			exit 1
		}

		foreach d [list coreSQLiteStudio guiSQLiteStudio sqlitestudio sqlitestudiocli] {
			set pro $d/$d.pro
			set ts "$d/translations/${d}_$lang.ts"
			if {![file exists $ts]} {
				catch {exec lupdate-pro $pro -ts $ts}
			}
		}

		foreach d [glob -directory ../Plugins -tails -nocomplain *] {
			if {![file isdirectory ../Plugins/$d]} continue
			set pro ../Plugins/$d/$d.pro
			set ts "../Plugins/$d/translations/${d}_$lang.ts"
			if {![file exists $ts]} {
				catch {exec lupdate-pro $pro -ts $ts}
			}
		}
	}
	"validate" {
		set ok 0
		set fail 0
		foreach f [find .. "*.ts"] {
			catch {exec xmllint --noout --schema ts.xsd $f} out
			set parts [split $out " "]
			if {[lindex $parts 1] != "validates"} {
				puts "$f: $out"
				incr fail
			} else {
				incr ok
			}
		}
		puts "Ok: $ok, Failed: $fail"
	}
    default {
        usage
    }
}

