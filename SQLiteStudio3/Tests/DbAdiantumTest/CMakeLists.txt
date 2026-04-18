# Unit tests for DbAdiantum crypto primitives (NH, HashNHPoly1305, HBSH, XChaCha12).
# Use the same sodium discovery scheme as Plugins/DbAdiantum/CMakeLists.txt:
# CMake config first (vcpkg), pkg-config fallback (MSYS2 / Homebrew / most distros).
find_package(sodium QUIET)
if(TARGET sodium::sodium)
    set(SODIUM_TARGET sodium::sodium)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
    set(SODIUM_TARGET PkgConfig::SODIUM)
endif()
find_package(OpenSSL REQUIRED)

add_executable(tst_adiantumtest
    tst_adiantumtest.cpp
    ${CMAKE_SOURCE_DIR}/Plugins/DbAdiantum/adiantum_cipher.cpp
)

target_include_directories(tst_adiantumtest PRIVATE
    ${CMAKE_SOURCE_DIR}/Plugins/DbAdiantum
)

target_compile_definitions(tst_adiantumtest PRIVATE
    USE_LIBSODIUM
    USE_OPENSSL
)

target_link_libraries(tst_adiantumtest PRIVATE
    ${SODIUM_TARGET}
    OpenSSL::Crypto
)

sqlitestudio_set_test_properties(tst_adiantumtest)
