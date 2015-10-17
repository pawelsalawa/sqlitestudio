#!/usr/bin/env tclsh

package require json

#set IP "localhost"
set IP "192.168.0.17"
set PORT 12121

if {$argc != 1} {
    puts "$argv0 <json data>"
    exit
}

proc pdict {dict {pattern *}} {
   set longest 0
   set keys [dict keys $dict $pattern] 
   foreach key $keys {
      set l [string length $key]
      if {$l > $longest} {set longest $l}
   }
   foreach key $keys {
      puts [format "%-${longest}s = %s" $key [dict get $dict $key]]
   }
}

proc READ {} {
    upvar sock sock
    set sizeBytes [read $sock 4]
    binary scan $sizeBytes i size
    return [read $sock $size]
}

proc WRITE {data} {
    upvar sock sock
    set bytes [string bytelength $data]
    set binLen [binary format i $bytes]
    puts -nonewline $sock $binLen
    flush $sock

#    set sizeRes [READ]
#    if {$sizeRes == "{\"size\":\"ok\"}"} {
#        puts "size ok"
#    } else {
#        puts "size error"
#        exit 1
#    }
    
    puts -nonewline $sock $data
    flush $sock
}

lassign $argv query

set sock [socket $IP $PORT]
fconfigure $sock -translation binary

WRITE $query
pdict [json::json2dict [READ]]

close $sock
