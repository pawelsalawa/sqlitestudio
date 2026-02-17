find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PROJECT_HEADERS
    fusiondarkplugin.h
)

set(PROJECT_SOURCES
    fusiondarkplugin.cpp
)

add_library(FusionDarkStyle SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_style_properties(FusionDarkStyle)
target_compile_definitions(FusionDarkStyle PRIVATE
    QT_DEPRECATED_WARNINGS
)
target_link_libraries(FusionDarkStyle PRIVATE
    Qt::Core
    Qt::Widgets
)
