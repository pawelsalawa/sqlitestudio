#include "adiantum_cipher.h"
#include <cstring>
#include <algorithm>

// Mandatory crypto backends — fake fallbacks are cryptographically insecure.
#include <sodium.h>
#include <openssl/evp.h>

//=============================================================================
// AdiantumCtx
//=============================================================================

AdiantumCtx::AdiantumCtx()
    : initialized(false)
{
    memset(masterKey, 0, sizeof(masterKey));
    memset(aesKey, 0, sizeof(aesKey));
    memset(polyKeyT, 0, sizeof(polyKeyT));
    memset(polyKeyM, 0, sizeof(polyKeyM));
    memset(nhKey, 0, sizeof(nhKey));
    memset(blockBuf, 0, sizeof(blockBuf));
    memset(tweakBuf, 0, sizeof(tweakBuf));
}

AdiantumCtx::AdiantumCtx(const uint8_t key[ADIANTUM_KEY_SIZE])
    : initialized(false)
{
    deriveSubKeys(key);
}

void AdiantumCtx::deriveSubKeys(const uint8_t* masterKey)
{
    // Save master key for HBSH operations
    memcpy(this->masterKey, masterKey, 32);

    // makeAdiantum: use XChaCha12 with all-zero nonce to generate 1136 bytes
    uint8_t keyStream[32 + 16 + 16 + ADIANTUM_KEY_NH_SIZE];  // 1136 bytes
    memset(keyStream, 0, sizeof(keyStream));  // All-zero nonce for initial keystream

    // Generate keystream using XChaCha12
    xchacha12_stream(keyStream, keyStream, sizeof(keyStream), masterKey, keyStream);

    // Split the keystream into sub-keys
    memcpy(aesKey, keyStream, 32);
    memcpy(polyKeyT, keyStream + 32, 16);
    memcpy(polyKeyM, keyStream + 48, 16);
    memcpy(nhKey, keyStream + 64, ADIANTUM_KEY_NH_SIZE);

    initialized = true;
}

void AdiantumCtx_init(AdiantumCtx* ctx)
{
    if (ctx) {
        *ctx = AdiantumCtx();
    }
}

void AdiantumCtx_deriveSubKeys(AdiantumCtx* ctx, const uint8_t* masterKey)
{
    if (ctx) {
        ctx->deriveSubKeys(masterKey);
    }
}

//=============================================================================
// NH Hash
// Reference: lukechampine.com/adiantum/nh/nh_generic.go
//=============================================================================

void nh_sum(uint8_t out[32], const uint8_t* msg, size_t msgLen, const uint8_t* key)
{
    // NH hash (UMAC-style), 4 parallel hashes over sliding key window.
    // Each 16-byte message block consumes 16 bytes of key advance; each of
    // the 4 parallel hashes reads 16 bytes of key at offsets 0/16/32/48
    // from the current sliding position. Key buffer must hold msgLen + 48.
    uint64_t sums[4] = {0, 0, 0, 0};

    while (msgLen >= 16) {
        uint32_t m0 = le32(msg + 0);
        uint32_t m1 = le32(msg + 4);
        uint32_t m2 = le32(msg + 8);
        uint32_t m3 = le32(msg + 12);

        for (int j = 0; j < 4; j++) {
            const uint8_t* kj = key + 16 * j;
            uint32_t k0 = le32(kj + 0);
            uint32_t k1 = le32(kj + 4);
            uint32_t k2 = le32(kj + 8);
            uint32_t k3 = le32(kj + 12);
            sums[j] += uint64_t(m0 + k0) * uint64_t(m2 + k2);
            sums[j] += uint64_t(m1 + k1) * uint64_t(m3 + k3);
        }

        msg += 16;
        key += 16;
        msgLen -= 16;
    }

    put_le64(out, sums[0]);
    put_le64(out + 8, sums[1]);
    put_le64(out + 16, sums[2]);
    put_le64(out + 24, sums[3]);
}

//=============================================================================
// Poly1305
//=============================================================================

void poly1305_auth(uint8_t out[16], const uint8_t* msg, size_t msgLen, const uint8_t* key)
{
    crypto_onetimeauth_poly1305(out, msg, msgLen, key);
}

//=============================================================================
// ChaCha20 / XChaCha12
//=============================================================================

// ChaCha20 quarter round
#define CHACHA_QUARTER_ROUND(a, b, c, d) \
    a += b; d ^= a; d = (d << 16) | (d >> 16); \
    c += d; b ^= c; b = (b << 12) | (b >> 20); \
    a += b; d ^= a; d = (d << 8) | (d >> 24); \
    c += d; b ^= c; b = (b << 7) | (b >> 25);

// Run ChaCha rounds-only (no final +initial accumulation). Used by HChaCha.
static void chacha_rounds_only(uint32_t st[16], const uint32_t key[8], const uint32_t nonce[4], int rounds)
{
    st[0] = 0x61707865;  // "expa"
    st[1] = 0x3320646e;  // "nd 3"
    st[2] = 0x79622d32;  // "2-by"
    st[3] = 0x6b206574;  // "te k"
    st[4] = key[0];   st[5] = key[1];   st[6] = key[2];   st[7] = key[3];
    st[8] = key[4];   st[9] = key[5];   st[10] = key[6];  st[11] = key[7];
    st[12] = nonce[0]; st[13] = nonce[1]; st[14] = nonce[2]; st[15] = nonce[3];

    for (int i = 0; i < rounds; i += 2) {
        CHACHA_QUARTER_ROUND(st[0], st[4], st[8], st[12]);
        CHACHA_QUARTER_ROUND(st[1], st[5], st[9], st[13]);
        CHACHA_QUARTER_ROUND(st[2], st[6], st[10], st[14]);
        CHACHA_QUARTER_ROUND(st[3], st[7], st[11], st[15]);
        CHACHA_QUARTER_ROUND(st[0], st[5], st[10], st[15]);
        CHACHA_QUARTER_ROUND(st[1], st[6], st[11], st[12]);
        CHACHA_QUARTER_ROUND(st[2], st[7], st[8], st[13]);
        CHACHA_QUARTER_ROUND(st[3], st[4], st[9], st[14]);
    }
}

// Full ChaCha block: rounds + (+ initial state) for keystream generation.
static void chacha20_block_generic(uint32_t st[16], const uint32_t key[8], const uint32_t nonce[4], int rounds)
{
    uint32_t initial[16];
    initial[0] = 0x61707865; initial[1] = 0x3320646e;
    initial[2] = 0x79622d32; initial[3] = 0x6b206574;
    initial[4] = key[0];   initial[5] = key[1];   initial[6] = key[2];   initial[7] = key[3];
    initial[8] = key[4];   initial[9] = key[5];   initial[10] = key[6];  initial[11] = key[7];
    initial[12] = nonce[0]; initial[13] = nonce[1]; initial[14] = nonce[2]; initial[15] = nonce[3];

    chacha_rounds_only(st, key, nonce, rounds);

    for (int i = 0; i < 16; i++) {
        st[i] += initial[i];
    }
}

// HChaCha12 - the core of XChaCha12
// Produces 256-bit subkey from (key, nonce[0..15]) via rounds-only (no initial-state add).
void hchacha12(uint8_t subkey[32], const uint8_t* key, const uint8_t* nonce)
{
    uint32_t st[16];
    uint32_t k[8];
    uint32_t n[4];

    for (int i = 0; i < 8; i++) {
        k[i] = le32(key + i * 4);
    }
    for (int i = 0; i < 4; i++) {
        n[i] = le32(nonce + i * 4);
    }

    chacha_rounds_only(st, k, n, 12);

    put_le32(subkey + 0, st[0]);
    put_le32(subkey + 4, st[1]);
    put_le32(subkey + 8, st[2]);
    put_le32(subkey + 12, st[3]);
    put_le32(subkey + 16, st[12]);
    put_le32(subkey + 20, st[13]);
    put_le32(subkey + 24, st[14]);
    put_le32(subkey + 28, st[15]);
}

// Forward declaration for hbsh_stream_xor (defined later).
static void hbsh_stream_xor(uint8_t* msg, size_t msgLen,
                             const uint8_t* key,
                             const uint8_t* nonce24,
                             uint32_t counter);

// XChaCha12 IETF stream cipher (public helper; used for subkey derivation).
//   subkey   = HChaCha12(key, nonce[0..15])
//   state:     st[12]=counter32, st[13]=0, st[14..15]=le32(nonce[16..19]),le32(nonce[20..23])
// Counter starts at 0.
void xchacha12_stream(uint8_t* dst, const uint8_t* src, size_t nbytes,
                      const uint8_t* key, const uint8_t* nonce)
{
    // For dst != src, pre-copy then XOR in place (keystream path is XOR-only).
    if (dst != src) {
        memmove(dst, src, nbytes);
    }
    hbsh_stream_xor(dst, nbytes, key, nonce, 0);
}

//=============================================================================
// AES-256 ECB
//=============================================================================

void aes256_ecb_encrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), nullptr, key, nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    int outLen = 0;
    EVP_EncryptUpdate(ctx, dst, &outLen, src, 16);
    int finalLen = 0;
    EVP_EncryptFinal_ex(ctx, dst + outLen, &finalLen);
    EVP_CIPHER_CTX_free(ctx);
}

void aes256_ecb_decrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), nullptr, key, nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    int outLen = 0;
    EVP_DecryptUpdate(ctx, dst, &outLen, src, 16);
    int finalLen = 0;
    EVP_DecryptFinal_ex(ctx, dst + outLen, &finalLen);
    EVP_CIPHER_CTX_free(ctx);
}

//=============================================================================
// HBSH Hash: HashNHPoly1305(tweak, msg)
// Reference: lukechampine.com/adiantum/adiantum.go - hashNHPoly1305.Sum
//
//   hT = Poly1305(keyT, LE128(8*msgLen) || tweak)          // 24-byte input
//        where LE128 = LE64(8*msgLen) || zeros(8)
//        Adiantum spec: H_T = Poly1305_{K_T}(LE128(|M|·8) || T)
//   hM = Poly1305(keyM, NH(msg[0..1023]) || NH(msg[1024..2047]) || ...)
//   out = hT +_{GF(2^128)} hM
//
// Each NH chunk produces 32 bytes that are streamed into Poly1305 via the
// incremental _update/_final API (equivalent to Go's mac.Write in a loop).
//=============================================================================

void hash_nh_poly1305(uint8_t out[16], const AdiantumCtx* ctx,
                      const uint8_t* tweak, size_t tweakLen,
                      const uint8_t* msg, size_t msgLen)
{
    // Part 1: Poly1305(keyT, LE128(8*msgLen) || tweak).
    // Per Adiantum spec, the length prefix is LE128 (16 bytes), not LE64.
    // For msgLen up to 2^61 bytes the high 8 bytes are zero — but they MUST
    // be present, otherwise the tweak shifts left by 8 bytes and Poly1305
    // produces a different hT, breaking interop with reference implementations
    // (Linux kernel crypto/adiantum.c, lukechampine.com/adiantum, etc.).
    size_t tLen = (tweakLen > 8) ? 8 : tweakLen;
    uint8_t tweakBuf[24];
    memset(tweakBuf, 0, sizeof(tweakBuf));
    put_le64(tweakBuf, uint64_t(msgLen) * 8u);   // bytes 0..7  : LE64(bits)
                                                  // bytes 8..15 : zero padding (LE128 high)
    memcpy(tweakBuf + 16, tweak, tLen);          // bytes 16..  : tweak

    uint8_t outT[16];
    poly1305_auth(outT, tweakBuf, 16 + tLen, ctx->polyKeyT);

    // Part 2: incremental Poly1305(keyM) over concatenated NH-chunk outputs.
    crypto_onetimeauth_poly1305_state macState;
    crypto_onetimeauth_poly1305_init(&macState, ctx->polyKeyM);

    uint8_t nhOut[32];
    size_t remaining = msgLen;
    while (remaining >= 1024) {
        nh_sum(nhOut, msg, 1024, ctx->nhKey);
        crypto_onetimeauth_poly1305_update(&macState, nhOut, sizeof(nhOut));
        msg += 1024;
        remaining -= 1024;
    }
    if (remaining > 0) {
        uint8_t padded[1024];
        memset(padded, 0, sizeof(padded));
        memcpy(padded, msg, remaining);
        size_t paddedLen = (remaining + 15u) & ~size_t(15);
        nh_sum(nhOut, padded, paddedLen, ctx->nhKey);
        crypto_onetimeauth_poly1305_update(&macState, nhOut, sizeof(nhOut));
    }

    uint8_t outM[16];
    crypto_onetimeauth_poly1305_final(&macState, outM);

    // Part 3: GF(2^128) addition — out = hT + hM.
    gf128_add(out, outT, outM);
}

//=============================================================================
// HBSH Encrypt/Decrypt
// Reference: lukechampine.com/adiantum/hbsh/hbsh.go
//=============================================================================

// XChaCha12 keystream XOR (IETF layout).
//   subkey   = HChaCha12(key, nonce24[0..15])
//   block state: st[12]=counter32, st[13]=0, st[14..15]=le32(nonce24[16..19]),le32(nonce24[20..23])
//
// counter argument is the starting 32-bit block counter. msgLen may be any
// length; trailing partial block is handled correctly.
static void hbsh_stream_xor(uint8_t* msg, size_t msgLen,
                             const uint8_t* key,
                             const uint8_t* nonce24,
                             uint32_t counter)
{
    uint8_t subkey[32];
    hchacha12(subkey, key, nonce24);

    uint32_t k[8];
    for (int i = 0; i < 8; i++) {
        k[i] = le32(subkey + i * 4);
    }

    uint32_t st[16];
    uint8_t keystream[64];
    size_t offset = 0;

    while (offset < msgLen) {
        uint32_t n[4];
        n[0] = counter;
        n[1] = 0;
        n[2] = le32(nonce24 + 16);
        n[3] = le32(nonce24 + 20);

        chacha20_block_generic(st, k, n, 12);

        for (int i = 0; i < 16; i++) {
            put_le32(keystream + i * 4, st[i]);
        }

        size_t blockSize = std::min(msgLen - offset, size_t(64));
        for (size_t i = 0; i < blockSize; i++) {
            msg[offset + i] ^= keystream[i];
        }

        offset += 64;
        counter++;
    }
}

// HBSH Encrypt: encrypts a 4096-byte block in-place
// block: 4096-byte buffer (input plaintext, output ciphertext)
// tweak: 8-byte tweak (typically block offset)
void hbsh_encrypt(AdiantumCtx* ctx, uint8_t* block, const uint8_t* tweak)
{
    constexpr size_t blockLen = ADIANTUM_BLOCK_SIZE;
    constexpr size_t plLen = blockLen - 16;

    uint8_t* pl = block;
    uint8_t* pr = block + blockLen - 16;

    // Step 1: PM = PR ⊕ HashNHPoly1305(tweak, PL)
    uint8_t hash[16];
    hash_nh_poly1305(hash, ctx, tweak, ADIANTUM_TWEAK_SIZE, pl, plLen);

    uint8_t pm[16];
    for (int i = 0; i < 16; i++) {
        pm[i] = pr[i] ^ hash[i];
    }

    // Step 2: CM = AES256_ECB_Encrypt(PM)
    uint8_t cm[16];
    aes256_ecb_encrypt(cm, pm, ctx->aesKey);

    // Step 3: Build XChaCha12 nonce = CM(16) || 0x01 || zeros(7).
    // The tweak is NOT in the stream nonce; it is consumed by HashNHPoly1305.
    uint8_t nonce24[24];
    memset(nonce24, 0, sizeof(nonce24));
    memcpy(nonce24, cm, 16);
    nonce24[16] = 0x01;

    // Step 4: CL = PL ⊕ XChaCha12_Stream_masterKey(PL, nonce24)
    hbsh_stream_xor(pl, plLen, ctx->masterKey, nonce24, 0);

    // Step 5: Reconstruct ciphertext: block = CL || CM
    // CL is already in place (XORed with plaintext), now copy CM to PR position
    memcpy(pr, cm, 16);
}

void hbsh_decrypt(AdiantumCtx* ctx, uint8_t* block, const uint8_t* tweak)
{
    constexpr size_t blockLen = ADIANTUM_BLOCK_SIZE;
    constexpr size_t plLen = blockLen - 16;

    uint8_t* pl = block;
    uint8_t* pr = block + blockLen - 16;

    // Step 1: Extract CM from CR (last 16 bytes of block)
    uint8_t cm[16];
    memcpy(cm, pr, 16);

    // Step 2: Rebuild same XChaCha12 nonce = CM(16) || 0x01 || zeros(7).
    uint8_t nonce24[24];
    memset(nonce24, 0, sizeof(nonce24));
    memcpy(nonce24, cm, 16);
    nonce24[16] = 0x01;

    // Step 3: PL = CL ⊕ XChaCha12_Stream_masterKey(CL, nonce24)
    hbsh_stream_xor(pl, plLen, ctx->masterKey, nonce24, 0);

    // Step 4: PM = AES256_ECB_Decrypt(CM)
    uint8_t pm[16];
    aes256_ecb_decrypt(pm, cm, ctx->aesKey);

    // Step 5: PR = PM ⊕ HashNHPoly1305(tweak, PL)
    uint8_t hash[16];
    hash_nh_poly1305(hash, ctx, tweak, ADIANTUM_TWEAK_SIZE, pl, plLen);

    for (int i = 0; i < 16; i++) {
        pr[i] = pm[i] ^ hash[i];
    }
}
