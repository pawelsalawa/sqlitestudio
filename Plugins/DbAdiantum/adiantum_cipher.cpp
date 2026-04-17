#include "adiantum_cipher.h"
#include <cstring>
#include <algorithm>

// Try to include libsodium and OpenSSL
#ifdef USE_LIBSODIUM
#include <sodium.h>
#endif

#ifdef USE_OPENSSL
#include <openssl/evp.h>
#endif

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
    uint32_t k[16];
    uint64_t sums[4] = {0, 0, 0, 0};

    // Initialize key window - first 4 elements are loaded from key
    // k[0..3] will be loaded from key[0..15]
    // k[4..15] will be loaded from key[i*4...(i+1)*4-1]
    for (int i = 4; i < 16; i++) {
        k[i] = 0;
    }

    // Process 16-byte blocks
    while (msgLen >= 16) {
        // Slide the key window - shift k values
        k[0] = k[4];
        k[1] = k[5];
        k[2] = k[6];
        k[3] = k[7];
        k[4] = k[8];
        k[5] = k[9];
        k[6] = k[10];
        k[7] = k[11];
        k[8] = k[12];
        k[9] = k[13];
        k[10] = k[14];
        k[11] = k[15];

        // Load next 48 bytes of key
        k[12] = le64(key + 0);
        k[13] = le64(key + 4);
        k[14] = le64(key + 8);
        k[15] = le64(key + 12);
        key += 16;

        // Load message block (4 x uint32)
        uint32_t m0 = le64(msg + 0);
        uint32_t m1 = le64(msg + 4);
        uint32_t m2 = le64(msg + 8);
        uint32_t m3 = le64(msg + 12);

        // Accumulate products: sums[i] += (m0+k[4i]) * (m2+k[4i+2])
        // and sums[i] += (m1+k[4i+1]) * (m3+k[4i+3])
        sums[0] += uint64_t(m0 + k[0]) * uint64_t(m2 + k[2]);
        sums[1] += uint64_t(m0 + k[4]) * uint64_t(m2 + k[6]);
        sums[2] += uint64_t(m0 + k[8]) * uint64_t(m2 + k[10]);
        sums[3] += uint64_t(m0 + k[12]) * uint64_t(m2 + k[14]);
        sums[0] += uint64_t(m1 + k[1]) * uint64_t(m3 + k[3]);
        sums[1] += uint64_t(m1 + k[5]) * uint64_t(m3 + k[7]);
        sums[2] += uint64_t(m1 + k[9]) * uint64_t(m3 + k[11]);
        sums[3] += uint64_t(m1 + k[13]) * uint64_t(m3 + k[15]);

        msg += 16;
        msgLen -= 16;
    }

    // Write output (4 x uint64 in little-endian)
    put_le64(out, sums[0]);
    put_le64(out + 8, sums[1]);
    put_le64(out + 16, sums[2]);
    put_le64(out + 24, sums[3]);
}

//=============================================================================
// GF(2^128) operations
//=============================================================================

static inline uint64_t add64_with_carry(uint64_t a, uint64_t b, uint64_t* carry) {
    uint64_t r = a + b;
    *carry = (r < a) ? 1 : 0;
    return r;
}

static inline uint64_t sub64_with_borrow(uint64_t a, uint64_t b, uint64_t* borrow) {
    uint64_t r = a - b;
    *borrow = (r > a) ? 1 : 0;
    return r;
}


//=============================================================================
// Poly1305
//=============================================================================

void poly1305_auth(uint8_t out[16], const uint8_t* msg, size_t msgLen, const uint8_t* key)
{
#ifdef USE_LIBSODIUM
    crypto_onetimeauth_poly1305(out, msg, msgLen, key);
#else
    // Fallback implementation for testing - NOT cryptographically secure
    // Use libsodium for production
    memset(out, 0, 16);
    // Simple collision-resistant-like mixing for testing
    uint64_t state[2] = {1, 0};
    for (size_t i = 0; i < msgLen; i++) {
        state[0] ^= uint64_t(msg[i]);
        state[0] = (state[0] * 0x9e3779b97f4a7c15ULL) + state[1];
    }
    memcpy(out, state, 16);
#endif
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

static void chacha20_block_generic(uint32_t st[16], const uint32_t key[8], const uint32_t nonce[4], int rounds)
{
    // Save initial state for final accumulation
    uint32_t initial[16];
    initial[0] = 0x61707865;  // "expa"
    initial[1] = 0x3320646e;  // "nd 3"
    initial[2] = 0x79622d32;  // "2-by"
    initial[3] = 0x6b206574;  // "te k"
    initial[4] = key[0];
    initial[5] = key[1];
    initial[6] = key[2];
    initial[7] = key[3];
    initial[8] = key[4];
    initial[9] = key[5];
    initial[10] = key[6];
    initial[11] = key[7];
    initial[12] = nonce[0];
    initial[13] = nonce[1];
    initial[14] = nonce[2];
    initial[15] = nonce[3];

    // Set initial state
    st[0] = initial[0];
    st[1] = initial[1];
    st[2] = initial[2];
    st[3] = initial[3];
    st[4] = initial[4];
    st[5] = initial[5];
    st[6] = initial[6];
    st[7] = initial[7];
    st[8] = initial[8];
    st[9] = initial[9];
    st[10] = initial[10];
    st[11] = initial[11];
    st[12] = initial[12];
    st[13] = initial[13];
    st[14] = initial[14];
    st[15] = initial[15];

    for (int i = 0; i < rounds; i += 2) {
        // Column rounds
        CHACHA_QUARTER_ROUND(st[0], st[4], st[8], st[12]);
        CHACHA_QUARTER_ROUND(st[1], st[5], st[9], st[13]);
        CHACHA_QUARTER_ROUND(st[2], st[6], st[10], st[14]);
        CHACHA_QUARTER_ROUND(st[3], st[7], st[11], st[15]);
        // Diagonal rounds
        CHACHA_QUARTER_ROUND(st[0], st[5], st[10], st[15]);
        CHACHA_QUARTER_ROUND(st[1], st[6], st[11], st[12]);
        CHACHA_QUARTER_ROUND(st[2], st[7], st[8], st[13]);
        CHACHA_QUARTER_ROUND(st[3], st[4], st[9], st[14]);
    }

    // Add initial state to get final keystream output
    for (int i = 0; i < 16; i++) {
        st[i] += initial[i];
    }
}

// HChaCha - the core of XChaCha
// Produces 256-bit output from 512-bit input (key + nonce)
void hchacha12(uint8_t subkey[32], const uint8_t* key, const uint8_t* nonce)
{
    uint32_t st[16];
    uint32_t k[8];
    uint32_t n[4];

    // Load key (little-endian)
    for (int i = 0; i < 8; i++) {
        k[i] = le64(key + i * 4);
    }

    // Load nonce (little-endian, only first 16 bytes used for HChaCha)
    for (int i = 0; i < 4; i++) {
        n[i] = le64(nonce + i * 4);
    }

    chacha20_block_generic(st, k, n, 12);

    // Write output - certain state elements form the output
    // These are uint32_t values (4 bytes each), use put_le32
    put_le32(subkey + 0, st[0]);
    put_le32(subkey + 4, st[1]);
    put_le32(subkey + 8, st[2]);
    put_le32(subkey + 12, st[3]);
    put_le32(subkey + 16, st[12]);
    put_le32(subkey + 20, st[13]);
    put_le32(subkey + 24, st[14]);
    put_le32(subkey + 28, st[15]);
}

// XChaCha12 IETF stream cipher
void xchacha12_stream(uint8_t* dst, const uint8_t* src, size_t nbytes,
                      const uint8_t* key, const uint8_t* nonce)
{
    // XChaCha12: HChaCha to derive subkey, then ChaCha12 with expanded nonce
    uint8_t subkey[32];
    uint8_t expandedNonce[16];

    // HChaCha12 with first 16 bytes of nonce
    hchacha12(subkey, key, nonce);

    // Expanded nonce = nonce[16..23] || LE(1) || zeros
    // For XChaCha, the nonce format is:
    // - First 16 bytes: used in HChaCha
    // - Next 8 bytes: part of counter/nonce for ChaCha
    // - Last 8 bytes: zeros (for IETF variant, counter is in first 4 bytes of this)
    memcpy(expandedNonce, nonce + 16, 8);
    // LE representation of block counter (starts at 0)
    put_le64(expandedNonce + 8, 1);  // Initial counter value

    // Now use ChaCha12 with the subkey
    uint32_t st[16];
    uint32_t k[8];
    for (int i = 0; i < 8; i++) {
        k[i] = le64(subkey + i * 4);
    }

    size_t offset = 0;
    uint64_t counter = 0;

    while (offset < nbytes) {
        uint8_t block[64];
        uint8_t blockNonce[16];
        memcpy(blockNonce, expandedNonce, 16);
        put_le64(blockNonce, counter);

        chacha20_block_generic(st, k, (uint32_t*)blockNonce, 12);

        // Serialize state to keystream - st[i] is uint32_t (4 bytes)
        uint8_t keystream[64];
        for (int i = 0; i < 16; i++) {
            put_le32(keystream + i * 4, st[i]);
        }

        // XOR with input
        size_t toXor = std::min(size_t(64), nbytes - offset);
        for (size_t i = 0; i < toXor; i++) {
            dst[offset + i] = src[offset + i] ^ keystream[i];
        }

        offset += 64;
        counter++;
    }
}

//=============================================================================
// AES-256 ECB
//=============================================================================

void aes256_ecb_encrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key)
{
#ifdef USE_OPENSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), nullptr, key, nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    int outLen;
    EVP_EncryptUpdate(ctx, dst, &outLen, src, 16);
    EVP_EncryptFinal_ex(ctx, dst + outLen, &outLen);
    EVP_CIPHER_CTX_free(ctx);
#else
    // Fallback - NOT cryptographically secure
    // Use OpenSSL for production
    for (int i = 0; i < 16; i++) {
        dst[i] = src[i] ^ key[i] ^ key[16 + (i ^ 7)];
    }
#endif
}

void aes256_ecb_decrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key)
{
#ifdef USE_OPENSSL
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(ctx);
    EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), nullptr, key, nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    int outLen;
    EVP_DecryptUpdate(ctx, dst, &outLen, src, 16);
    EVP_DecryptFinal_ex(ctx, dst + outLen, &outLen);
    EVP_CIPHER_CTX_free(ctx);
#else
    // Fallback - NOT cryptographically secure
    for (int i = 0; i < 16; i++) {
        dst[i] = src[i] ^ key[i] ^ key[16 + (i ^ 7)];
    }
#endif
}

//=============================================================================
// HBSH Hash: NH + Poly1305
// Reference: lukechampine.com/adiantum/adiantum.go - hashNHPoly1305.Sum
//=============================================================================

static void hash_nh_poly1305(uint8_t out[16],
                             const uint8_t* msg, size_t msgLen,
                             const uint8_t* tweak,
                             const uint8_t* keyT,
                             const uint8_t* keyM,
                             const uint8_t* keyNH)
{
    // Part 1: Poly1305 hash of (8 * len(msg) || tweak) with keyT
    uint8_t tweakBuf[24];  // 16 + 8
    put_le64(tweakBuf, uint64_t(8 * msgLen));
    memcpy(tweakBuf + 8, tweak, 8);

    uint8_t outT[16];
    poly1305_auth(outT, tweakBuf, sizeof(tweakBuf), keyT);

    // Part 2: NH hash in 1024-byte chunks, then Poly1305 with keyM
    uint8_t outM[16];
    uint8_t mac[32];  // Poly1305 produces 16 bytes, but NH outputs 32
    uint8_t nhOut[32];

    // Process message in 1024-byte chunks
    size_t remaining = msgLen;
    while (remaining >= 1024) {
        nh_sum(nhOut, msg, 1024, keyNH);
        // XOR into mac (this is what mac.Write does in Go)
        for (int i = 0; i < 32; i++) {
            mac[i] = nhOut[i];
        }
        msg += 1024;
        remaining -= 1024;
    }

    // Handle final (incomplete) chunk - pad to 16-byte multiple
    if (remaining > 0) {
        // Pad to 16-byte boundary
        uint8_t padded[1024] = {0};  // Max chunk size
        memcpy(padded, msg, remaining);
        nh_sum(nhOut, padded, (remaining + 15) & ~15, keyNH);
        for (int i = 0; i < 32; i++) {
            mac[i] ^= nhOut[i];
        }
    }

    // Final Poly1305 with keyM
    poly1305_auth(outM, mac, 32, keyM);

    // Part 3: GF(2^128) addition: out = outT + outM
    gf128_add(out, outT, outM);
}

//=============================================================================
// HBSH Encrypt/Decrypt
// Reference: lukechampine.com/adiantum/hbsh/hbsh.go
//=============================================================================

// XOR msg with keystream (in-place)
static void stream_xor(uint8_t* msg, size_t len, const uint8_t* key, const uint8_t* nonce)
{
    uint8_t stream[64];
    size_t offset = 0;

    while (offset < len) {
        uint8_t nonceBlock[16];
        memcpy(nonceBlock, nonce, 16);
        // XOR counter into last 8 bytes
        uint64_t counter = offset / 64;
        put_le64(nonceBlock + 8, counter);

        // Generate 64-byte keystream block
        uint32_t st[16];
        uint32_t k[8];
        for (int i = 0; i < 8; i++) {
            k[i] = le64(key + i * 4);
        }
        chacha20_block_generic(st, k, (uint32_t*)nonceBlock, 12);

        // Serialize - st[i] is uint32_t (4 bytes)
        for (int i = 0; i < 16; i++) {
            put_le32(stream + i * 4, st[i]);
        }

        // XOR
        size_t toXor = std::min(size_t(64), len - offset);
        for (size_t i = 0; i < toXor; i++) {
            msg[offset + i] ^= stream[i];
        }

        offset += 64;
    }
}

// HBSH stream XOR - similar to stream_xor but with proper XChaCha12 nonce handling
// Uses the full 24-byte nonce for HChaCha key derivation
static void hbsh_stream_xor(uint8_t* msg, size_t msgLen,
                             const uint8_t* key,
                             const uint8_t* nonce24,
                             uint64_t counter)
{
    // XChaCha12: derive subkey using HChaCha with first 16 bytes of nonce
    uint8_t subkey[32];
    hchacha12(subkey, key, nonce24);

    // ChaCha12 block with expanded nonce
    // Format: counter at bytes 0-3 (goes to st[12]), nonce bytes at bytes 8-15 (goes to st[13:15])
    uint8_t blockNonce[16];
    memset(blockNonce, 0, 16);

    // Generate keystream and XOR in 64-byte blocks
    uint32_t st[16];
    uint8_t keystream[64];
    size_t offset = 0;

    while (offset < msgLen) {
        // counter at bytes 0-3 (goes to st[12])
        uint32_t ctr32 = static_cast<uint32_t>(counter);
        memcpy(blockNonce, &ctr32, 4);
        // nonce bytes at bytes 8-15 (goes to st[13:15])
        memcpy(blockNonce + 8, nonce24 + 16, 8);

        // Generate 64-byte keystream using ChaCha12
        uint32_t k[8];
        for (int i = 0; i < 8; i++) {
            k[i] = le64(subkey + i * 4);
        }

        chacha20_block_generic(st, k, (uint32_t*)blockNonce, 12);

        // Serialize state to keystream - st[i] is uint32_t (4 bytes)
        for (int i = 0; i < 16; i++) {
            put_le32(keystream + i * 4, st[i]);
        }

        // XOR with msg (up to 64 bytes per block)
        size_t blockSize = std::min(msgLen - offset, (size_t)64);
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

    // Step 1: PM = PR ⊕ NH_hash(tweak, PL)  (first 16 bytes of 32-byte NH hash)
    // For HBSH, we use raw NH hash, not Poly1305(NH hash)
    uint8_t nhHash[32];
    nh_sum(nhHash, pl, plLen, ctx->nhKey);

    uint8_t pm[16];
    for (int i = 0; i < 16; i++) {
        pm[i] = pr[i] ^ nhHash[i];
    }

    // Step 2: CM = AES256_ECB_Encrypt(PM)
    uint8_t cm[16];
    aes256_ecb_encrypt(cm, pm, ctx->aesKey);

    // Step 3: Build nonce for XChaCha12: nonce24 = [cm[8:16] || tweak || 0x01 || zeros]
    // This is 17 bytes of actual data, padded to 24
    uint8_t nonce24[24];
    memset(nonce24, 0, sizeof(nonce24));
    memcpy(nonce24, cm + 8, 8);      // bytes 0-7: cm[8:16] (second half of AES output)
    memcpy(nonce24 + 8, tweak, 8);  // bytes 8-15: tweak
    nonce24[16] = 0x01;             // byte 16: constant 0x01
    // bytes 17-23 remain zero

    // Step 4: CL = PL ⊕ XChaCha12_Stream(PL)
    // XOR plaintext left with keystream
    // Use master key for HChaCha12 key derivation (not the 16-byte CM)
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

    // Step 2: Build same nonce as encrypt: nonce24 = [cm[8:16] || tweak || 0x01 || zeros]
    uint8_t nonce24[24];
    memset(nonce24, 0, sizeof(nonce24));
    memcpy(nonce24, cm + 8, 8);
    memcpy(nonce24 + 8, tweak, 8);
    nonce24[16] = 0x01;

    // Step 3: PL = CL ⊕ XChaCha12_Stream(CL)
    // XOR ciphertext left with same keystream to recover plaintext
    // Use master key for HChaCha12 key derivation
    hbsh_stream_xor(pl, plLen, ctx->masterKey, nonce24, 0);

    // Step 4: PM = AES256_ECB_Decrypt(CM)
    uint8_t pm[16];
    aes256_ecb_decrypt(pm, cm, ctx->aesKey);

    // Step 5: PR = PM ⊕ NH_hash(tweak, PL)  (first 16 bytes of 32-byte NH hash)
    uint8_t nhHash[32];
    nh_sum(nhHash, pl, plLen, ctx->nhKey);

    // PR = PM XOR NH_hash (reverses encrypt's XOR since XOR is its own inverse)
    for (int i = 0; i < 16; i++) {
        pr[i] = pm[i] ^ nhHash[i];
    }
}
