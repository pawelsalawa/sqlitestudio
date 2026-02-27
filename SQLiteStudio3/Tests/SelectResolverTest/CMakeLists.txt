add_executable(tst_selectresolvertest
    tst_selectresolvertest.cpp
)

qt_add_resources(_rcc schemas.qrc)
target_sources(tst_selectresolvertest PRIVATE ${_rcc})

sqlitestudio_set_test_properties(tst_selectresolvertest)
