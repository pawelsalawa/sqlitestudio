add_executable(tst_selectresolvertest
    tst_selectresolvertest.cpp
)
sqlitestudio_set_test_properties(tst_selectresolvertest)

qt_add_resources(tst_selectresolvertest
    schemas.qrc
)
