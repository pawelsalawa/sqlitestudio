#!/usr/bin/env tclsh

# tclsh assemble.tcl c:/tmp/installer

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
		}
		set qtPkgFiles {
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
			}
			DbSqlite2 {
				sqlite.dll
			}
		}
	}
	default {
		set mainPkgFiles {
		}
		set qtPkgFiles {
		}
		array set pluginDeps {
		}
	}
}

proc defineGlobalVars {} {
	lassign $::argv ::targetDir
	set ::portableDir [file normalize ../../output/portable]
	set ::releaseDate [clock format [clock seconds] -format "%Y-%m-%d"]

	switch $::OS {
		"macosx" {
			set ::finalExecutable "SQLiteStudio.app"
			set ::wizardStyle "Mac"
			set ::initialTargetDir "/Applications/SQLiteStudio.app"
			set ::libExt "dylib"
		}
		"win32" {
			set ::finalExecutable "SQLiteStudio.exe"
			set ::wizardStyle "Modern"
			set ::initialTargetDir "C:\\Program Files\\SQLiteStudio"
			set ::libExt "dll"
			set qtCoreFile "$::portableDir/SQLiteStudio/Qt5Core.dll"
		}
		default {
			set ::finalExecutable "sqlitestudio"
			set ::wizardStyle "Modern"
			set ::initialTargetDir "/opt/SQLiteStudio"
			set ::libExt "so"
		}
	}
	
	# Version
	set verFile "../../SQLiteStudio3/coreSQLiteStudio/sqlitestudio.cpp"
	set fd [open $verFile r]
	set ver [lindex [regexp -inline {int\s+sqlitestudioVersion\s*\=\s*(\d+)} [read $fd]] 1]
	set ::sqliteStudioVersion [toVersion $ver]
	set data [read $fd]
	close $fd
	
	# Qt version
	set fd [open $qtCoreFile r]
	chan configure $fd -translation binary -encoding binary
	set data [read $fd]
	close $fd
	set ::qtVer [lindex [regexp -inline -- {Qt\s+(\d\.\d{1,2}\.\d{1,2})} $data] 1]

	# Output binary
	set ::output [file normalize $::targetDir/../InstallSQLiteStudio-${::sqliteStudioVersion}.exe]
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
	file delete -force $::output
	file delete -force $targetDir
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

proc copyMainPkg {targetDir} {
	set pkgDir $targetDir/packages/pl.com.salsoft.sqlitestudio
	
	puts "Copying core app files."
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data
	foreach f $::mainPkgFiles {
		file copy -force $::portableDir/SQLiteStudio/$f $pkgDir/data/
	}
	file copy ../../SQLiteStudio3/sqlitestudio/package.xml $pkgDir/meta/

	mapFile $pkgDir/meta/package.xml [list %VERSION% $::sqliteStudioVersion %DATE% $::releaseDate]
	
	puts "Compressing core app files."
	archiveContentsOf $pkgDir/data sqlitestudio.7z
}

proc copyQtPkg {targetDir} {
	set pkgDir $targetDir/packages/io.qt
	
	puts "Copying Qt."
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data
	foreach f $::qtPkgFiles {
		file copy -force $::portableDir/SQLiteStudio/$f $pkgDir/data/
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

	mapFile $targetDir/config/config.xml [list %SQLITESTUDIO_VERSION% $::sqliteStudioVersion %FINAL_EXECUTABLE% $::finalExecutable %WIZARD_STYLE% $::wizardStyle %TARGET_DIR% $::initialTargetDir]
}

proc createOutputBinary {targetDir} {
	puts "Creating installer binary: $::output"
	exec binarycreator -f -p $targetDir/packages -c $targetDir/config/config.xml $::output
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

	set files [glob -tails -directory ../../output/portable/SQLiteStudio/plugins $mask]
	set ::plugins [list]
	foreach f $files {
		set plugin [string range $f 0 end-$toRemove]
		set ver [readPluginVersion ../../Plugins/$plugin/${plugin}.json]
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
	
	puts "Copying plugin: $name ($ver)"
	file mkdir $pkgDir $pkgDir/meta $pkgDir/data $pkgDir/data/plugins
	file copy -force $::portableDir/SQLiteStudio/plugins/${name}.$::libExt $pkgDir/data/plugins/
	if {[info exists ::pluginDeps($name)]} {
		foreach f $::pluginDeps($name) {
			file copy -force $::portableDir/SQLiteStudio/$f $pkgDir/data/
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

if {$argc < 1} {
	puts stderr "Specify target dir."
	exit 1
}

if {![file exists ../../output/portable]} {
	puts stderr "[file normalize ../../output/portable] does not exist"
	exit 1
}

if {[catch {exec archivegen --help} res] && [string first "Usage: archivegen" $res] == -1} {
	puts stderr "archivegen (QtIFW) missing in PATH."
	exit 1
}

defineGlobalVars
collectPlugins
initDirs $targetDir
copyConfig $targetDir
copyMainPkg $targetDir
copyQtPkg $targetDir
copyPluginPkgs $targetDir
createOutputBinary $targetDir