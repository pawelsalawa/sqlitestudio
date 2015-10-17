. config.sh

cp -f SQLiteStudioRemote.jar EmployeeDirectory/SQLiteStudioRemote && \
    cd EmployeeDirectory && ./gradlew assemble && cd .. && \
    $ADB -d install -r EmployeeDirectory/app/build/outputs/apk/app-debug.apk
