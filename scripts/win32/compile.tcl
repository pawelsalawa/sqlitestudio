set cpu_cores $env(NUMBER_OF_PROCESSORS)
if {$cpu_cores > 1} {
	incr cpu_cores -1 ;# if more than 1 available, leave 1 for OS to run smoothly
}
puts "Using $cpu_cores CPU cores."

if {![catch {exec where /q qmake}]} {
	set QMAKE [exec where qmake]
	puts "INFO: Qt's qmake found at $QMAKE"
} else {
	puts "ERROR: Cannot find Qt"
	exit 1
}
puts "Using qmake: $QMAKE"

if {![catch {exec where /q mingw32-make}]} {
	set MAKE [exec where mingw32-make]
	puts "INFO: MinGW32's make found in $MAKE"
} else {
	puts "ERROR: Cannot find MinGW32 \[mingw32-make.exe]"
	exit 1
}
puts "Using make: $MAKE"

set cdir [pwd]
cd ../..
set parent_dir [pwd]
cd $cdir
puts [pwd]

set output "$parent_dir/output"

file delete -force $output

cd $parent_dir
file mkdir output output/build output/build/Plugins

proc dt {} {
	return "\[[clock format [clock seconds] -format "%Y-%m-%d %H:%M:%S"]\]"
}

puts "[dt] Compiling core app."
cd output/build
exec $QMAKE ../../SQLiteStudio3
catch {exec $MAKE -j $cpu_cores} res
puts $res

puts "[dt] Compiling plugins."
cd Plugins
exec $QMAKE ../../../Plugins
catch {exec $MAKE -j $cpu_cores} res
puts $res

puts "[dt] Compilation finished."
cd $cdir