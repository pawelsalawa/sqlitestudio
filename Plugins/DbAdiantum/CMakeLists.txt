cmake_minimum_required(VERSION 3.16)

option(WITH_DbAdiantum "Build DbAdiantum plugin (Adiantum VFS encryption)" ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
# Crypto libraries - mandatory. Fake fallbacks are not cryptographically secure.
# Try CMake config first (vcpkg / some distros); fall back to pkg-config
# (MSYS2 / Homebrew / most Linux distros only ship libsodium.pc, no CMake config).
find_package(sodium QUIET)
if(TARGET sodium::sodium)
    set(SODIUM_TARGET sodium::sodium)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
    set(SODIUM_TARGET PkgConfig::SODIUM)
endif()
find_package(OpenSSL REQUIRED)  # AES-256 ECB

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
    target_link_libraries(DbAdiantum PRIVATE coreSQLiteStudio SQLite::SQLite3)
endif()

# Link crypto libraries (mandatory)
# SODIUM_TARGET (sodium::sodium or PkgConfig::SODIUM) carries
# INTERFACE_INCLUDE_DIRECTORIES, so no explicit target_include_directories needed.
target_link_libraries(DbAdiantum PRIVATE ${SODIUM_TARGET})
target_compile_definitions(DbAdiantum PRIVATE USE_LIBSODIUM)

target_link_libraries(DbAdiantum PRIVATE OpenSSL::Crypto)
target_compile_definitions(DbAdiantum PRIVATE USE_OPENSSL)

# Compile options
target_compile_options(DbAdiantum PRIVATE
    -Wall
    -Wno-unused-parameter
    -Wno-sign-compare
)
