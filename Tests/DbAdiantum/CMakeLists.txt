cmake_minimum_required(VERSION 3.16)

project(TestAdiantumCrypto)

find_package(Qt6 REQUIRED COMPONENTS Test)

set(PROJECT_SOURCES
    TestAdiantumCrypto.cpp
    ../../Plugins/DbAdiantum/adiantum_cipher.cpp
)

add_executable(TestAdiantumCrypto ${PROJECT_SOURCES})
set_target_properties(TestAdiantumCrypto PROPERTIES AUTOMOC ON)

target_link_libraries(TestAdiantumCrypto PRIVATE
    Qt6::Core
    Qt6::Test
)

# Copy the plugin to the test executable directory
# Plugin is pre-built at ${DBADIANTUM_PLUGIN_PATH}


