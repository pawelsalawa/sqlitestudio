add_executable(tst_sqlfileexecutortest
    tst_sqlfileexecutortest.cpp
)

qt_add_resources(_rcc testfiles.qrc)
target_sources(tst_sqlfileexecutortest PRIVATE ${_rcc})

letos_set_test_properties(tst_sqlfileexecutortest)

