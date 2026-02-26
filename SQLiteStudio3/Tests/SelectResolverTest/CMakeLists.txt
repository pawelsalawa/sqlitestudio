add_executable(tst_selectresolvertest
    tst_selectresolvertest.cpp
)
sqlitestudio_set_test_properties(tst_selectresolvertest)

# Required for Qt 6.2 (no default QT_RESOURCE_PREFIX)
set_target_properties(tst_selectresolvertest PROPERTIES
    QT_RESOURCE_PREFIX "/"
)

qt_add_resources(tst_selectresolvertest
    schemas.qrc
)
