cmake_minimum_required(VERSION 3.16)

option(WITH_DbAdiantum "Build DbAdiantum plugin (Adiantum VFS encryption)" ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
# Crypto libraries
find_package(sodium QUIET)  # ChaCha20/Poly1305
find_package(OpenSSL QUIET)  # AES-256 ECB

set(PROJECT_HEADERS
    dbadiantum_global.h
    dbadiantum.h
    dbadiantuminstance.h
    adiantum_vfs.h
    adiantum_cipher.h
)

set(PROJECT_SOURCES
    dbadiantum.cpp
    dbadiantuminstance.cpp
    adiantum_vfs.cpp
    adiantum_cipher.cpp
)

add_library(DbAdiantum SHARED ${PROJECT_HEADERS} ${PROJECT_SOURCES})
sqlitestudio_set_plugin_properties(DbAdiantum)

target_compile_definitions(DbAdiantum PRIVATE DBADIANTUM_LIBRARY)

# Link dependencies
# Note: On Linux, coreSQLiteStudio is not linked - plugins are loaded at runtime
target_link_libraries(DbAdiantum PRIVATE
    Qt::Core
)
if(WIN32 OR APPLE)
    target_link_libraries(DbAdiantum PRIVATE coreSQLiteStudio)
endif()

# Link crypto libraries if found
if(sodium_FOUND)
    target_link_libraries(DbAdiantum PRIVATE sodium::sodium)
    target_include_directories(DbAdiantum PRIVATE ${sodium_INCLUDE_DIR})
    target_compile_definitions(DbAdiantum PRIVATE USE_LIBSODIUM)
endif()

if(OpenSSL_FOUND)
    target_link_libraries(DbAdiantum PRIVATE OpenSSL::Crypto)
    target_compile_definitions(DbAdiantum PRIVATE USE_OPENSSL)
endif()

if(NOT sodium_FOUND AND NOT OpenSSL_FOUND)
    message(WARNING "DbAdiantum: Neither sodium nor OpenSSL found. Using insecure fallback implementations.")
endif()

# Compile options
target_compile_options(DbAdiantum PRIVATE
    -Wall
    -Wno-unused-parameter
    -Wno-sign-compare
)
