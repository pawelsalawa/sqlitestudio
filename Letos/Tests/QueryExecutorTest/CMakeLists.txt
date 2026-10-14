add_executable(tst_queryexecutortest
    tst_queryexecutortest.cpp
    # These production classes are not exported from the core DLL.
    ../../core/impl/dbattacherimpl.cpp
    ../../core/plugins/dbpluginsqlite3.cpp
    ../../core/plugins/dbpluginsqlite3.h
    ../TestUtils/configmock.cpp
    ../TestUtils/extensionmanagermock.cpp
)

letos_set_test_properties(tst_queryexecutortest)

if(WIN32)
    # Load the core DLL built for this test, even if an older app build is on PATH.
    string(REPLACE ";" "\\;" queryexecutor_test_path "$ENV{PATH}")
    set_tests_properties(tst_queryexecutortest PROPERTIES
        ENVIRONMENT "PATH=$<TARGET_FILE_DIR:coreLetos>\;$<TARGET_FILE_DIR:TestUtils>\;${queryexecutor_test_path}"
    )
endif()
