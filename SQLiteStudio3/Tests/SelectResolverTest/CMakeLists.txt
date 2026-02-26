add_executable(tst_selectresolvertest
    tst_selectresolvertest.cpp
)
sqlitestudio_set_test_properties(tst_selectresolvertest)

qt_add_resources(tst_selectresolvertest
    PREFIX "/"   # required for Qt 6.2 (no default QT_RESOURCE_PREFIX)
    schemas.qrc
)
