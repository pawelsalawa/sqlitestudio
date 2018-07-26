set TARGET_DIR c:/temp/sqls-installer
set TMP_DIR c:/temp/SQLiteStudio

set OLDDIR [pwd]

file delete -force ../../output
puts "Compiling app."
exec tclsh ./compile.tcl
puts "Creating portable distro."
puts [exec tclsh ./create_dist_pkg.tcl]

cd ../../output/portable/SQLiteStudio
set VER [lindex [exec sqlitestudiocli --version] 1]
cd $OLDDIR

puts "Generating installator file."
cd ../installer
file delete -force $TARGET_DIR
puts [exec tclsh assemble.tcl $TARGET_DIR --repo]

puts "Installing in temporary location"
file delete -force $TMP_DIR
exec $TARGET_DIR/InstallSQLiteStudio-$VER.exe TargetDir=$TMP_DIR

file rename -force $TMP_DIR $TARGET_DIR/
cd $TARGET_DIR

puts "REMEMBER to zip $TARGET_DIR/SQLiteStudio into sqlitestudio-$VER.zip and uninstall that installation."

cd $OLDDIR
