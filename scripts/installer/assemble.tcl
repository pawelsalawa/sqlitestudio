#!/usr/bin/env tclsh

# tclsh assemble.tcl c:/tmp/installer [--repo]

package require platform
lassign [split [platform::generic] -] OS ARCH

set cfgFiles {
	config.xml
	bg.png
	logo.png
	sqlitestudio.icns
	sqlitestudio.ico
	watermark.png
	controller.qs
}

switch $::OS {
	"macosx" {
		set mainPkgFiles {
			SQLiteStudio.app/Contents/Frameworks/libcoreSQLiteStudio.1.0.0.dylib
			SQLiteStudio.app/Contents/Frameworks/libguiSQLiteStudio.1.0.0.dylib
			SQLiteStudio.app/Contents/Frameworks/libsqlite3.0.dylib
			SQLiteStudio.app/Contents/MacOS/SQLiteStudio
			SQLiteStudio.app/Contents/MacOS/sqlitestudiocli
			SQLiteStudio.app/Contents/Resources
			SQLiteStudio.app/Contents/Info.plist
			SQLiteStudio.app/Contents/PkgInfo
		}
		set qtPkgFiles {
			SQLiteStudio.app/Contents/Frameworks/QtCore.framework
			SQLiteStudio.app/Contents/Frameworks/QtGui.framework
			SQLiteStudio.app/Contents/Frameworks/QtNetwork.framework
			SQLiteStudio.app/Contents/Frameworks/QtPrintSupport.framework
			SQLiteStudio.app/Contents/Frameworks/QtScript.framework
			SQLiteStudio.app/Contents/Frameworks/QtSvg.framework
			SQLiteStudio.app/Contents/Frameworks/QtWidgets.framework
			SQLiteStudio.app/Contents/Frameworks/QtXml.framework
			SQLiteStudio.app/Contents/PlugIns/bearer
			SQLiteStudio.app/Contents/PlugIns/iconengines
			SQLiteStudio.app/Contents/PlugIns/imageformats
			SQLiteStudio.app/Contents/PlugIns/platforms
			SQLiteStudio.app/Contents/PlugIns/printsupport
			SQLiteStudio.app/Contents/PlugIns/styles
		}
		array set pluginDeps {
		}
	}
	"win32" {
		set mainPkgFiles {
			coreSQLiteStudio.dll
			guiSQLiteStudio.dll
			sqlite3.dll
			SQLiteStudio.exe
			sqlitestudiocli.exe
		}
		set qtPkgFiles {
			iconengines
			imageformats
			platforms
			printsupport
			styles
			libeay32.dll
			libgcc_s_dw2-1.dll
			libstdc++-6.dll
			libwinpthread-1.dll
			qt.conf
			Qt5Core.dll
			Qt5Gui.dll
			Qt5Network.dll
			Qt5PrintSupport.dll
			Qt5Script.dll
			Qt5Svg.dll
			Qt5Widgets.dll
			Qt5Xml.dll
		}
		array set pluginDeps {
			ScriptingTcl {
				tcl86.dll
				zlib1.dll
			}
			DbSqlite2 {
				sqlite.dll
			}
		}
	}
	default {
		set mainPkgFiles {
			lib/libcoreSQLiteStudio.so
			lib/libguiSQLiteStudio.so
			lib/libsqlite3.so
			sqlitestudio
			sqlitestudiocli
		}
		set qtPkgFiles {
			iconengines/libqsvgicon.so
			imageformats/libqgif.so
			imageformats/libqicns.so
			imageformats/libqico.so
			imageformats/ibqjpeg.so
			imageformats/libqsvg.so
			imageformats/libqtga.so
			imageformats/libqtiff.so
			platforms/libqxcb.so
			printsupport/libcupsprintersupport.so
			lib/libQt5Concurrent.so
			lib/libQt5Core.so
			lib/libQt5DBus.so
			lib/libQt5Gui.so
			lib/libQt5Network.so
			lib/libQt5PrintSupport.so
			lib/libQt5Script.so
			lib/libQt5Svg.so
			lib/libQt5Widgets.so
			lib/libQt5XcbQpa.so
			lib/libQt5Xml.so
			lib/libicudata.so
			lib/libicui18n.so
			lib/libicuuc.so.56
		}
		array set pluginDeps {
			DbSqlite2 {
				lib/libsqlite.so
			}
		}
	}
}

proc defineGlobalVars {} {
	lassign $::argv ::targetDir
	set ::releaseDate [clock format [clock seconds] -format "%Y-%m-%d"]
	
	# Version
	set verFile "../../SQLiteStudio3/coreSQLiteStudio/sqlitestudio.cpp"
	set fd [open $verFile r]
	set ver [lindex [regexp -inline {int\s+sqlitestudioVersion\s*\=\s*(\d+)} [read $fd]] 1]
	######################################################################################### VER +1
	#incr ver
	set ::sqliteStudioVersion [toVersion $ver]
	set data [read $fd]
	close $fd

	switch $::OS {
		"macosx" {
			set ::portableDir [file normalize ../../output]
			set ::finalExecutable "Contents/MacOS/SQLiteStudio"
			set ::wizardStyle "Mac"
			set ::initialTargetDir "/Applications/SQLiteStudio.app"
			set ::libExt "dylib"
			set ::updateArch "macosx"
			set ::pluginsDir "SQLiteStudio.app/Contents/PlugIns"
			set ::libPref "lib"
			set ::dirsToSkipInPathBeginning 1
			set qtCoreFile "$::portableDir/SQLiteStudio/SQLiteStudio.app/Contents/Frameworks/QtCore.framework/QtCore"
			set ::output [file normalize $::targetDir/InstallSQLiteStudio-${::sqliteStudioVersion}]
		}
		"win32" {
			set ::portableDir [file normalize ../../output/portable]
			set ::finalExecutable "SQLiteStudio.exe"
			set ::wizardStyle "Modern"
			set ::initialTargetDir "C:\\Program Files\\SQLiteStudio"
			set ::libExt "dll"
			set ::updateArch "windows"
			set ::pluginsDir "plugins"
			set ::libPref ""
			set ::dirsToSkipInPathBeginning 0
			set qtCoreFile "$::portableDir/SQLiteStudio/Qt5Core.dll"
			set ::output [file normalize $::targetDir/InstallSQLiteStudio-${::sqliteStudioVersion}.exe]
		}
		default {
			set ::portableDir [file normalize ../../output/portable]
			set ::finalExecutable "sqlitestudio"
			set ::wizardStyle "Modern"
			set ::initialTargetDir "/opt/SQLiteStudio"
			set ::updateArch "linux"
			set ::pluginsDir "plugins"
			set ::libExt "so"
			set ::libPref "lib"
			set ::dirsToSkipInPathBeginning 0
			set qtCoreFile "$::portableDir/SQLiteStudio/lib/libQt5Core.so"
			set ::output [file normalize $::targetDir/InstallSQLiteStudio-${::sqliteStudioVersion}]
		}
	}
	
	# Qt version
	set fd [open $qtCoreFile r]
	chan configure $fd -translation binary -encoding binary
	set data [read $fd]
	close $fd
	set ::qtVer [lindex [regexp -inline -- {Qt\s+(\d\.\d{1,2}\.\d{1,2})} $data] 1]
	######################################################################################### VER +1
	#set ::qtVer 6.0.0

	set ::startApp $::finalExecutable

	# Repository
	set ::repoDir [file normalize $::targetDir/REPO]
}

proc toVersion {ver} {
	return [expr {$ver / 10000}].[expr {$ver / 100 % 100}].[expr {$ver % 100}]
}

proc mapFile {f props} {
	set fd [open $f r]
	set data [read $fd]
	close $fd

	set data [string map $props $data]
	
	set fd [open $f w+]
	puts $fd $data
	close $fd
}

proc initDirs {targetDir} {
	file delete -force $::output $::repoDir $targetDir
	file mkdir $targetDir $targetDir/config $targetDir/packages
}

proc archiveContentsOf {dir intoFile} {
	set cdir [pwd]
	cd $dir
	set files [glob *]
	exec archivegen $intoFile {*}$files
	file delete -force {*}$files
	cd $cdir
}

proc copyFileWithLinks {fromDir toDir file} {
	set targetFile [file join {*}[lrange [file split $file] $::dirsToSkipInPathBeginning end]]
	file mkdir [file dirname $toDir/$targetFile]
	switch -- $::OS {
		"macosx" {
			file copy -force $fromDir/$file $toDir/$targetFile
			set fd [file dirname $fromDir/$file]
			set td [file dirname $toDir/$targetFile]
			set fileOnly [lindex [file split $file] end]
			set filePrefix [lindex [split $fileOnly .] 0]
			foreach f [glob -nocomplain -tails -directory $fd "${filePrefix}.*.$::libExt"] {
				file copy -force $fd/$f $td/$f
			}
		}
		"win32" {
			file copy -force $fromDir/$file $toDir/$targetFile
		}
		default {
			set fd [file dirname $fromDir/$file]
			set td [file dirname $toDir/$targetFile]
			set fileOnly [lindex [file split $file] end]
			foreach f [glob -nocomplain -tails -directory $fd "${fileOnly}*"] {
				file copy -force $fd/$f $td/$f
			}
		}
	}
}

proc copyMainPkg {targetDir} {
	set pkgDir $targetDir/packages/pl.com.salsoft.sqlitestudio
	
	puts "Copying core app files."
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data
	foreach f $::mainPkgFiles {
		copyFileWithLinks $::portableDir/SQLiteStudio $pkgDir/data $f
	}
	file copy ../../SQLiteStudio3/sqlitestudio/package.xml ../../SQLiteStudio3/sqlitestudio/register_file_types.ui ../../SQLiteStudio3/sqlitestudio/installscript.qs $pkgDir/meta/
	
	file mkdir $pkgDir/data/app_icon
	file copy ../../SQLiteStudio3/guiSQLiteStudio/img/sqlitestudio.ico $pkgDir/data/app_icon
	file copy ../../SQLiteStudio3/guiSQLiteStudio/img/sqlitestudio.icns $pkgDir/data/app_icon
	file copy ../../SQLiteStudio3/guiSQLiteStudio/img/sqlitestudio.svg $pkgDir/data/app_icon
	file copy ../../SQLiteStudio3/guiSQLiteStudio/img/sqlitestudio_logo.png $pkgDir/data/app_icon/sqlitestudio.png

	mapFile $pkgDir/meta/package.xml [list %VERSION% $::sqliteStudioVersion %DATE% $::releaseDate]
	
	puts "Compressing core app files."
	archiveContentsOf $pkgDir/data sqlitestudio.7z
}

proc copyQtPkg {targetDir} {
	set pkgDir $targetDir/packages/io.qt
	
	puts "Copying Qt."
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data
	foreach f $::qtPkgFiles {
		copyFileWithLinks $::portableDir/SQLiteStudio $pkgDir/data $f
	}
	file copy ../../SQLiteStudio3/qt_package.xml $pkgDir/meta/package.xml

	mapFile $pkgDir/meta/package.xml [list %VERSION% $::qtVer %DATE% $::releaseDate]
	
	puts "Compressing Qt."
	archiveContentsOf $pkgDir/data qt.7z
}

proc copyConfig {targetDir} {
	foreach f $::cfgFiles {
		file copy config/$f $targetDir/config/
	}

	mapFile $targetDir/config/config.xml [list %SQLITESTUDIO_VERSION% $::sqliteStudioVersion %FINAL_EXECUTABLE% $::finalExecutable \
		%WIZARD_STYLE% $::wizardStyle %TARGET_DIR% $::initialTargetDir %UPDATE_ARCH% $::updateArch]
}

proc createOutputBinary {targetDir} {
	if {$::OS == "macosx"} {
	    puts "Creating installer binary: $::output.dmg"
	    exec binarycreator -f -p $targetDir/packages -c $targetDir/config/config.xml $::output.dmg
	    puts "Creating installer binary: $::output"
	    exec binarycreator -f -p $targetDir/packages -c $targetDir/config/config.xml $::output
	} else {
	    puts "Creating installer binary: $::output"
	    exec binarycreator -f -p $targetDir/packages -c $targetDir/config/config.xml $::output
	}
}

proc createOutputRepo {targetDir} {
	puts "Creating update repository: $::repoDir"
	exec repogen -p $targetDir/packages $::repoDir
}

proc readPluginVersion {path} {
	set fd [open $path r]
	set ver [lindex [regexp -inline -- {\"version\"\s*\:\s*(\d+)} [read $fd]] 1]
	close $fd
	return [toVersion $ver]
}

proc collectPlugins {} {
	set mask "*.$::libExt"
	set toRemove [expr {[string length $mask] - 1}]
	set prefToRemove [string length $::libPref]

	set files [glob -tails -directory $::portableDir/SQLiteStudio/$::pluginsDir $mask]
	set ::plugins [list]
	foreach f $files {
		set plugin [string range $f $prefToRemove end-$toRemove]
		set ver [readPluginVersion ../../Plugins/$plugin/[string tolower $plugin].json]
		######################################################################################### VER +1
		#set ver 9.9.9
		lappend ::plugins [dict create name $plugin version $ver]
	}
}

proc readPluginPkgName {path} {
	set fd [open $path r]
	set pkg [lindex [regexp -inline -- {\<Name\>(.*)\<\/Name\>} [read $fd]] 1]
	close $fd
	return $pkg
}

proc copyPluginPkg {pluginDict targetDir} {
	set name [dict get $pluginDict name]
	set ver [dict get $pluginDict version]
	set packageXml ../../Plugins/$name/package.xml

	if {![file exists $packageXml]} {
		puts "Skipping plugin $name, because package.xml for this plugin does not exist."
		return
	}
	
	set pkgName [readPluginPkgName $packageXml]
	set pkgDir $targetDir/packages/$pkgName

	set targetPluginsDir [file join {*}[lrange [file split $::pluginsDir] $::dirsToSkipInPathBeginning end]]

	puts "Copying plugin: $name ($ver)"
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data $pkgDir/data/$targetPluginsDir
	file copy -force $::portableDir/SQLiteStudio/$::pluginsDir/$::libPref${name}.$::libExt $pkgDir/data/$targetPluginsDir/
	if {[info exists ::pluginDeps($name)]} {
		foreach f $::pluginDeps($name) {
			copyFileWithLinks $::portableDir/SQLiteStudio $pkgDir/data $f
		}
	}
	file copy $packageXml $pkgDir/meta/

	mapFile $pkgDir/meta/package.xml [list %VERSION% $ver %DATE% $::releaseDate]
	
	puts "Compressing $name."
	archiveContentsOf $pkgDir/data $name.7z
}

proc copyPluginPkgs {targetDir} {
	set packageXml ../../Plugins/package.xml
	set pluginsPkg $targetDir/packages/pl.com.salsoft.sqlitestudio.plugins
	file mkdir $pluginsPkg $pluginsPkg/data $pluginsPkg/meta
	file copy $packageXml $pluginsPkg/meta/
	mapFile $pluginsPkg/meta/package.xml [list %DATE% $::releaseDate]

	foreach p $::plugins {
		copyPluginPkg $p $targetDir
	}
}

if {$argc < 1 || $argc > 2} {
	puts stderr "Usage: $argv0 <installer output dir> \[--repo]"
	exit 1
}

if {[catch {exec archivegen --help} res] && [string first "Usage: archivegen" $res] == -1} {
    puts stderr "archivegen (QtIFW) missing in PATH."
    exit 1
}

defineGlobalVars

if {![file exists $::portableDir]} {
	puts stderr "[file normalize $::portableDir] does not exist"
	exit 1
}

collectPlugins
initDirs $targetDir
copyConfig $targetDir
copyMainPkg $targetDir
copyQtPkg $targetDir
copyPluginPkgs $targetDir
createOutputBinary $targetDir

if {$argc >= 2 && [lindex $argv 1] == "--repo"} {
	createOutputRepo $targetDir
}
