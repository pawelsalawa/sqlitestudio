. config.sh

pkg=$($AAPT dump badging $APK|awk -F" " '/package/ {print $2}'|awk -F"'" '/name=/ {print $2}')
act=$($AAPT dump badging $APK|awk -F" " '/launchable-activity/ {print $2}'|awk -F"'" '/name=/ {print $2}')
$ADB shell am start -n $pkg/$act
