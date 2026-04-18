# Unit tests for DbAdiantum crypto primitives (NH, HashNHPoly1305, HBSH, XChaCha12).
# The plugin CMake entry already finds sodium + OpenSSL as REQUIRED at configure
# time; we re-find them here so the test executable can link independently of
# whether the plugin target is built.
find_package(sodium REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(tst_adiantumtest
    tst_adiantumtest.cpp
    ${CMAKE_SOURCE_DIR}/Plugins/DbAdiantum/adiantum_cipher.cpp
)

target_include_directories(tst_adiantumtest PRIVATE
    ${CMAKE_SOURCE_DIR}/Plugins/DbAdiantum
    ${sodium_INCLUDE_DIR}
)

target_compile_definitions(tst_adiantumtest PRIVATE
    USE_LIBSODIUM
    USE_OPENSSL
)

target_link_libraries(tst_adiantumtest PRIVATE
    sodium::sodium
    OpenSSL::Crypto
)

sqlitestudio_set_test_properties(tst_adiantumtest)
