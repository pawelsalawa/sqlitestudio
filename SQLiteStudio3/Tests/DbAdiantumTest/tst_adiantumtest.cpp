#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QByteArray>
#include <QDebug>

// Cipher header resolved via target include dir (Plugins/DbAdiantum).
#include "adiantum_cipher.h"


class TestAdiantumCrypto : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // NH Hash tests
    void test_nh_hash_basic();
    void test_nh_hash_empty_message();
    void test_nh_hash_1024_bytes();
    void test_nh_hash_1025_bytes();
    void test_nh_hash_round_trip();

    // GF(2^128) tests
    void test_gf128_add();
    void test_gf128_sub();
    void test_gf128_add_sub_identity();

    // AES-256 ECB tests
    void test_aes256_ecb_encrypt_decrypt();

    // XChaCha12 tests
    void test_xchacha12_stream_basic();

    // HBSH tests
    void test_hbsh_encrypt_decrypt_basic();
    void test_hbsh_encrypt_decrypt_4096();
    void test_hbsh_decrypt_rejects_wrong_key();

    // Subkey derivation tests
    void test_make_adiantum_subkeys();

private:
    // Helper to compare byte arrays
    bool bytesEqual(const uint8_t* a, const uint8_t* b, size_t len);
};

void TestAdiantumCrypto::initTestCase()
{
    qDebug() << "Testing Adiantum crypto primitives";
}

void TestAdiantumCrypto::cleanupTestCase()
{
}

bool TestAdiantumCrypto::bytesEqual(const uint8_t* a, const uint8_t* b, size_t len)
{
    return memcmp(a, b, len) == 0;
}

void TestAdiantumCrypto::test_nh_hash_basic()
{
    uint8_t key[1072] = {0};
    memset(key, 0x42, 1072);  // Non-zero key
    uint8_t msg[64] = {0};
    memset(msg, 0x17, 64);    // Non-zero message

    uint8_t out[32];
    nh_sum(out, msg, 64, key);

    bool allZeros = true;
    for (int i = 0; i < 32; i++) {
        if (out[i] != 0) {
            allZeros = false;
            break;
        }
    }
    QVERIFY2(!allZeros, "NH hash output should not be all zeros for non-zero input");

    uint8_t out2[32];
    nh_sum(out2, msg, 64, key);
    QVERIFY2(bytesEqual(out, out2, 32), "NH hash should be deterministic");
}

void TestAdiantumCrypto::test_nh_hash_empty_message()
{
    uint8_t key[1072] = {0};
    uint8_t msg[1] = {0};

    uint8_t out[32];
    nh_sum(out, msg, 0, key);

    bool allZeros = true;
    for (int i = 0; i < 32; i++) {
        if (out[i] != 0) {
            allZeros = false;
            break;
        }
    }
    QVERIFY2(allZeros, "NH hash of empty message with zero key should be zero");
}

void TestAdiantumCrypto::test_nh_hash_1024_bytes()
{
    uint8_t key[1072] = {0};
    memset(key, 0xAB, 1072);

    uint8_t msg[1024];
    memset(msg, 0xCD, 1024);

    uint8_t out[32];
    nh_sum(out, msg, 1024, key);

    uint8_t out2[32];
    nh_sum(out2, msg, 1024, key);
    QVERIFY2(bytesEqual(out, out2, 32), "NH hash should be deterministic for 1024 bytes");

    uint8_t key2[1072] = {0};
    memset(key2, 0xEF, 1072);
    uint8_t out3[32];
    nh_sum(out3, msg, 1024, key2);
    QVERIFY2(!bytesEqual(out, out3, 32), "Different keys should produce different NH hash");
}

void TestAdiantumCrypto::test_nh_hash_1025_bytes()
{
    uint8_t key[1072] = {0};
    memset(key, 0x12, 1072);

    uint8_t msg[1025];
    memset(msg, 0x34, 1025);

    uint8_t out[32];
    nh_sum(out, msg, 1025, key);

    QVERIFY2(true, "NH hash should handle 1025 bytes without crashing");

    uint8_t out2[32];
    nh_sum(out2, msg, 1025, key);
    QVERIFY2(bytesEqual(out, out2, 32), "NH hash should be deterministic for 1025 bytes");
}

void TestAdiantumCrypto::test_nh_hash_round_trip()
{
    uint8_t key[1072];
    for (int i = 0; i < 1072; i++) key[i] = i & 0xFF;

    uint8_t msg[256];
    for (int i = 0; i < 256; i++) msg[i] = (i * 2) & 0xFF;

    uint8_t out1[32];
    uint8_t out2[32];

    nh_sum(out1, msg, 256, key);
    nh_sum(out2, msg, 256, key);

    QVERIFY2(bytesEqual(out1, out2, 32), "NH hash should be reproducible");
}

void TestAdiantumCrypto::test_gf128_add()
{
    uint8_t x[16] = {0};
    uint8_t y[16] = {0};
    uint8_t z[16] = {0};

    gf128_add(z, x, y);
    QVERIFY2(bytesEqual(z, x, 16), "0 + 0 should equal 0");

    x[0] = 1;
    y[0] = 1;
    gf128_add(z, x, y);
    QVERIFY2(z[0] == 2 && z[1] == 0, "1 + 1 should equal 2 in little-endian");

    memset(x, 0xFF, 16);
    memset(y, 0, 16);
    y[0] = 1;
    gf128_add(z, x, y);
    // 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF + 0x01 overflows to 0 in 128-bit
    QVERIFY2(z[0] == 0 && z[1] == 0 && z[2] == 0, "Carry should propagate correctly (overflow to 0)");
}

void TestAdiantumCrypto::test_gf128_sub()
{
    uint8_t x[16] = {0};
    uint8_t y[16] = {0};
    uint8_t z[16] = {0};

    gf128_sub(z, x, y);
    QVERIFY2(bytesEqual(z, x, 16), "0 - 0 should equal 0");

    x[0] = 2;
    y[0] = 1;
    gf128_sub(z, x, y);
    QVERIFY2(z[0] == 1 && z[1] == 0, "2 - 1 should equal 1");

    memset(x, 0, 16);
    memset(y, 0xFF, 16);
    gf128_sub(z, x, y);
    // In standard integer 128-bit: 0 - 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF = 1
    // (because it's 1 - max, with borrow from high bits)
    QVERIFY2(z[0] == 1 && z[1] == 0, "Borrow should give 1 as high byte");
}

void TestAdiantumCrypto::test_gf128_add_sub_identity()
{
    uint8_t a[16];
    uint8_t b[16];
    uint8_t c[16];

    for (int i = 0; i < 16; i++) a[i] = (i * 0x11) & 0xFF;

    gf128_add(b, a, a);
    gf128_sub(c, b, a);

    QVERIFY2(bytesEqual(a, c, 16), "a + a - a should equal a");
}

void TestAdiantumCrypto::test_aes256_ecb_encrypt_decrypt()
{
    uint8_t key[32];
    uint8_t plaintext[16] = "Hello, World!";
    uint8_t ciphertext[16];
    uint8_t decrypted[16];

    for (int i = 0; i < 32; i++) key[i] = i;

    aes256_ecb_encrypt(ciphertext, plaintext, key);

    QVERIFY2(!bytesEqual(ciphertext, plaintext, 16), "Encrypted text should differ from plaintext");

    aes256_ecb_decrypt(decrypted, ciphertext, key);

    QVERIFY2(bytesEqual(decrypted, plaintext, 16), "Decrypted text should match original plaintext");

    uint8_t ciphertext2[16];
    aes256_ecb_encrypt(ciphertext2, plaintext, key);
    QVERIFY2(bytesEqual(ciphertext, ciphertext2, 16), "AES encryption should be deterministic");
}

void TestAdiantumCrypto::test_xchacha12_stream_basic()
{
    uint8_t key[32];
    uint8_t nonce[24];
    uint8_t plaintext[64];
    uint8_t ciphertext[64];

    for (int i = 0; i < 32; i++) key[i] = i;
    for (int i = 0; i < 24; i++) nonce[i] = i;
    for (int i = 0; i < 64; i++) plaintext[i] = 0;

    xchacha12_stream(ciphertext, plaintext, 64, key, nonce);

    QVERIFY2(!bytesEqual(ciphertext, plaintext, 64), "Stream ciphertext should differ from zero plaintext");

    uint8_t decrypted[64];
    xchacha12_stream(decrypted, ciphertext, 64, key, nonce);
    QVERIFY2(bytesEqual(decrypted, plaintext, 64), "Stream cipher should decrypt correctly");

    uint8_t nonce2[24];
    for (int i = 0; i < 24; i++) nonce2[i] = ~nonce[i] & 0xFF;
    uint8_t ciphertext2[64];
    xchacha12_stream(ciphertext2, plaintext, 64, key, nonce2);
    QVERIFY2(!bytesEqual(ciphertext, ciphertext2, 64), "Different nonces should produce different ciphertext");
}

void TestAdiantumCrypto::test_hbsh_encrypt_decrypt_basic()
{
    AdiantumCtx ctx;
    uint8_t key[32] = {0};
    uint8_t plaintext[4096];
    uint8_t ciphertext[4096];
    uint8_t decrypted[4096];

    for (int i = 0; i < 32; i++) key[i] = i;
    for (int i = 0; i < 4096; i++) plaintext[i] = i & 0xFF;

    ctx.deriveSubKeys(key);

    memcpy(ciphertext, plaintext, 4096);

    uint8_t tweak[8] = {0};
    hbsh_encrypt(&ctx, ciphertext, tweak);

    QVERIFY2(!bytesEqual(ciphertext, plaintext, 4096), "Encrypted block should differ from plaintext");

    memcpy(decrypted, ciphertext, 4096);
    hbsh_decrypt(&ctx, decrypted, tweak);

    QVERIFY2(bytesEqual(decrypted, plaintext, 4096), "Decrypted block should match original plaintext");
}

void TestAdiantumCrypto::test_hbsh_encrypt_decrypt_4096()
{
    AdiantumCtx ctx;
    uint8_t key[32];
    uint8_t block[4096];
    uint8_t blockCopy[4096];

    for (int i = 0; i < 32; i++) key[i] = (i * 7) & 0xFF;
    for (int i = 0; i < 4096; i++) block[i] = (i * 13) & 0xFF;

    ctx.deriveSubKeys(key);
    memcpy(blockCopy, block, 4096);

    uint8_t tweak[8] = {0};
    hbsh_encrypt(&ctx, block, tweak);

    hbsh_decrypt(&ctx, block, tweak);

    QVERIFY2(bytesEqual(block, blockCopy, 4096), "Encrypt/decrypt cycle should restore original data");
}

void TestAdiantumCrypto::test_hbsh_decrypt_rejects_wrong_key()
{
    AdiantumCtx ctx1, ctx2;
    uint8_t key1[32], key2[32];
    uint8_t plaintext[4096];
    uint8_t ciphertext[4096];

    for (int i = 0; i < 32; i++) key1[i] = i;
    for (int i = 0; i < 32; i++) key2[i] = ~i & 0xFF;
    for (int i = 0; i < 4096; i++) plaintext[i] = i & 0xFF;

    ctx1.deriveSubKeys(key1);
    ctx2.deriveSubKeys(key2);

    memcpy(ciphertext, plaintext, 4096);
    uint8_t tweak[8] = {0};
    hbsh_encrypt(&ctx1, ciphertext, tweak);

    hbsh_decrypt(&ctx2, ciphertext, tweak);

    bool isOriginal = bytesEqual(ciphertext, plaintext, 4096);
    QVERIFY2(!isOriginal, "Decryption with wrong key should not recover original plaintext");
}

void TestAdiantumCrypto::test_make_adiantum_subkeys()
{
    AdiantumCtx ctx;
    uint8_t key[32];
    for (int i = 0; i < 32; i++) key[i] = i;

    ctx.deriveSubKeys(key);

    QVERIFY2(ctx.initialized, "Context should be initialized after deriveSubKeys");

    bool allZeroAes = true;
    for (int i = 0; i < 32; i++) {
        if (ctx.aesKey[i] != 0) { allZeroAes = false; break; }
    }
    QVERIFY2(!allZeroAes, "AES key should not be all zeros");

    AdiantumCtx ctx2;
    ctx2.deriveSubKeys(key);
    QVERIFY2(bytesEqual(ctx.aesKey, ctx2.aesKey, 32), "Same key should produce same AES subkey");
    QVERIFY2(bytesEqual(ctx.polyKeyT, ctx2.polyKeyT, 16), "Same key should produce same PolyKeyT");
    QVERIFY2(bytesEqual(ctx.polyKeyM, ctx2.polyKeyM, 16), "Same key should produce same PolyKeyM");
}

QTEST_MAIN(TestAdiantumCrypto)
#include "tst_adiantumtest.moc"
