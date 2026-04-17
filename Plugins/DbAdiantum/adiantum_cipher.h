#ifndef ADIANTUM_CIPHER_H
#define ADIANTUM_CIPHER_H

#include <cstdint>
#include <cstdlib>
#include <cstring>

// Constants
constexpr size_t ADIANTUM_KEY_SIZE = 32;      // 256-bit key
constexpr size_t ADIANTUM_BLOCK_SIZE = 4096;  // SQLite default page size
constexpr size_t ADIANTUM_TWEAK_SIZE = 8;     // 64-bit tweak
constexpr size_t ADIANTUM_NONCE_SIZE = 24;    // XChaCha nonce
constexpr size_t ADIANTUM_KEY_NH_SIZE = 1072; // 1024 + 48 for NH hash

// Adiantum context (per-file)
struct AdiantumCtx
{
    uint8_t  masterKey[32];            // Original 256-bit master key
    uint8_t  aesKey[32];           // AES-256 key
    uint8_t  polyKeyT[16];         // Poly1305 key for tweak hash
    uint8_t  polyKeyM[16];         // Poly1305 key for message hash
    uint8_t  nhKey[ADIANTUM_KEY_NH_SIZE]; // NH hash key (1024 + 48)
    bool     initialized;
    uint8_t  blockBuf[ADIANTUM_BLOCK_SIZE];
    uint8_t  tweakBuf[ADIANTUM_TWEAK_SIZE];

    AdiantumCtx();
    explicit AdiantumCtx(const uint8_t key[ADIANTUM_KEY_SIZE]);
    void deriveSubKeys(const uint8_t* masterKey);
};

// Initialize context with empty keys
void AdiantumCtx_init(AdiantumCtx* ctx);

// Derive sub-keys from 32-byte master key
void AdiantumCtx_deriveSubKeys(AdiantumCtx* ctx, const uint8_t* masterKey);

// HBSH encryption - encrypts a 4096-byte block in-place
// block: 4096-byte buffer (input plaintext, output ciphertext)
// tweak: 8-byte tweak (typically block offset)
void hbsh_encrypt(AdiantumCtx* ctx, uint8_t* block, const uint8_t* tweak);

// HBSH decryption - decrypts a 4096-byte block in-place
void hbsh_decrypt(AdiantumCtx* ctx, uint8_t* block, const uint8_t* tweak);

// NH hash - computes 32-byte hash
// msg: message to hash (must be multiple of 16 bytes)
// msgLen: length of message in bytes
// key: NH key (must be at least msgLen + 48 bytes)
void nh_sum(uint8_t out[32], const uint8_t* msg, size_t msgLen, const uint8_t* key);

// Poly1305 MAC - computes 16-byte authentication tag
// msg: message to authenticate
// msgLen: length of message in bytes
// key: 32-byte Poly1305 key
void poly1305_auth(uint8_t out[16], const uint8_t* msg, size_t msgLen, const uint8_t* key);

// XChaCha12 subkey derivation using HChaCha
// subkey: output 32-byte subkey
// key: 32-byte key
// nonce: 16-byte nonce (first 16 bytes of 24-byte nonce)
void hchacha12(uint8_t subkey[32], const uint8_t* key, const uint8_t* nonce);

// XChaCha12 IETF stream cipher
// dst: output buffer (can equal src for in-place)
// src: input buffer
// nbytes: number of bytes
// key: 32-byte key
// nonce: 24-byte nonce (XChaCha12 uses 24-byte nonce)
void xchacha12_stream(uint8_t* dst, const uint8_t* src, size_t nbytes,
                      const uint8_t* key, const uint8_t* nonce);

// AES-256 ECB single-block encryption (16 bytes in, 16 bytes out)
// dst: output buffer (must have 16 bytes space)
// src: input buffer (must have 16 bytes)
// key: 32-byte AES key
void aes256_ecb_encrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key);

// AES-256 ECB single-block decryption
void aes256_ecb_decrypt(uint8_t* dst, const uint8_t* src, const uint8_t* key);

// Utility functions
inline uint64_t le64(const uint8_t* b) {
    return uint64_t(b[0]) | (uint64_t(b[1]) << 8) |
           (uint64_t(b[2]) << 16) | (uint64_t(b[3]) << 24) |
           (uint64_t(b[4]) << 32) | (uint64_t(b[5]) << 40) |
           (uint64_t(b[6]) << 48) | (uint64_t(b[7]) << 56);
}

inline void put_le64(uint8_t* b, uint64_t v) {
    b[0] = uint8_t(v);
    b[1] = uint8_t(v >> 8);
    b[2] = uint8_t(v >> 16);
    b[3] = uint8_t(v >> 24);
    b[4] = uint8_t(v >> 32);
    b[5] = uint8_t(v >> 40);
    b[6] = uint8_t(v >> 48);
    b[7] = uint8_t(v >> 56);
}

inline void put_le32(uint8_t* b, uint32_t v) {
    b[0] = uint8_t(v);
    b[1] = uint8_t(v >> 8);
    b[2] = uint8_t(v >> 16);
    b[3] = uint8_t(v >> 24);
}

// GF(2^128) addition - out = x + y (little-endian uint128)
inline void gf128_add(uint8_t* out, const uint8_t* x, const uint8_t* y) {
    uint64_t x0 = le64(x);
    uint64_t x1 = le64(x + 8);
    uint64_t y0 = le64(y);
    uint64_t y1 = le64(y + 8);
    uint64_t r0, r1, c;
    unsigned char carry;

    // Add low 64 bits
    r0 = x0 + y0;
    c = (r0 < x0) ? 1 : 0;
    // Add high 64 bits with carry
    r1 = x1 + y1 + c;
    carry = (r1 < x1) ? 1 : 0;

    put_le64(out, r0);
    put_le64(out + 8, r1);
}

// GF(2^128) subtraction - out = x - y (little-endian uint128)
inline void gf128_sub(uint8_t* out, const uint8_t* x, const uint8_t* y) {
    uint64_t x0 = le64(x);
    uint64_t x1 = le64(x + 8);
    uint64_t y0 = le64(y);
    uint64_t y1 = le64(y + 8);
    uint64_t r0, r1, c;

    // Subtract low 64 bits
    r0 = x0 - y0;
    c = (r0 > x0) ? 1 : 0;
    // Subtract high 64 bits with borrow
    r1 = x1 - y1 - c;
    // borrow = (r1 > x1) ? 1 : 0; // Not needed for this use

    put_le64(out, r0);
    put_le64(out + 8, r1);
}

#endif // ADIANTUM_CIPHER_H
