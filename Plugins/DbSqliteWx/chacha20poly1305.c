/*
** This file contains th implementation for
**   - the ChaCha20 cipher
**   - the Poly1305 message digest
**
** The code was taken from the public domain implementation
** of the sqleet project (https://github.com/resilar/sqleet)
*/

#include <stdint.h>

#define ROL32(x, c) (((x) << (c)) | ((x) >> (32-(c))))
#define ROR32(x, c) (((x) >> (c)) | ((x) << (32-(c))))

#define LOAD32_LE(p)            \
  ( ((uint32_t)((p)[0]) <<  0)  \
  | ((uint32_t)((p)[1]) <<  8)  \
  | ((uint32_t)((p)[2]) << 16)  \
  | ((uint32_t)((p)[3]) << 24)  \
  )
#define LOAD32_BE(p)            \
  ( ((uint32_t)((p)[3]) <<  0)  \
  | ((uint32_t)((p)[2]) <<  8)  \
  | ((uint32_t)((p)[1]) << 16)  \
  | ((uint32_t)((p)[0]) << 24)  \
  )

#define STORE32_LE(p, v)        \
  (p)[0] = ((v) >>  0) & 0xFF;  \
  (p)[1] = ((v) >>  8) & 0xFF;  \
  (p)[2] = ((v) >> 16) & 0xFF;  \
  (p)[3] = ((v) >> 24) & 0xFF;
#define STORE32_BE(p, v)        \
  (p)[3] = ((v) >>  0) & 0xFF;  \
  (p)[2] = ((v) >>  8) & 0xFF;  \
  (p)[1] = ((v) >> 16) & 0xFF;  \
  (p)[0] = ((v) >> 24) & 0xFF;
#define STORE64_BE(p, v)        \
  (p)[7] = ((v) >>  0) & 0xFF;  \
  (p)[6] = ((v) >>  8) & 0xFF;  \
  (p)[5] = ((v) >> 16) & 0xFF;  \
  (p)[4] = ((v) >> 24) & 0xFF;  \
  (p)[3] = ((v) >> 32) & 0xFF;  \
  (p)[2] = ((v) >> 40) & 0xFF;  \
  (p)[1] = ((v) >> 48) & 0xFF;  \
  (p)[0] = ((v) >> 56) & 0xFF;

/*
 * ChaCha20 stream cipher
 */
static void chacha20_block(unsigned char out[64], const uint32_t in[16])
{
  int i;
  uint32_t x[16];
  memcpy(x, in, sizeof(uint32_t) * 16);

  #define QR(x, a, b, c, d)                           \
  x[a] += x[b]; x[d] ^= x[a]; x[d] = ROL32(x[d], 16); \
  x[c] += x[d]; x[b] ^= x[c]; x[b] = ROL32(x[b], 12); \
  x[a] += x[b]; x[d] ^= x[a]; x[d] = ROL32(x[d],  8); \
  x[c] += x[d]; x[b] ^= x[c]; x[b] = ROL32(x[b],  7);
  for (i = 0; i < 10; i++)
  {
    /* Column round */
    QR(x, 0, 4, 8, 12)
    QR(x, 1, 5, 9, 13)
    QR(x, 2, 6, 10, 14)
    QR(x, 3, 7, 11, 15)
    /* Diagonal round */
    QR(x, 0, 5, 10, 15)
    QR(x, 1, 6, 11, 12)
    QR(x, 2, 7, 8, 13)
    QR(x, 3, 4, 9, 14)
  }
  #undef QR

  for (i = 0; i < 16; i++)
  {
    const uint32_t v = x[i] + in[i];
    STORE32_LE(out, v);
    out += 4;
  }
}

void chacha20_xor(unsigned char* data, size_t n, const unsigned char key[32],
                  const unsigned char nonce[12], uint32_t counter)
{
  size_t i;
  uint32_t state[16];
  unsigned char block[64];
  static const unsigned char sigma[16] = "expand 32-byte k";

  state[ 0] = LOAD32_LE(sigma +  0);
  state[ 1] = LOAD32_LE(sigma +  4);
  state[ 2] = LOAD32_LE(sigma +  8);
  state[ 3] = LOAD32_LE(sigma + 12);

  state[ 4] = LOAD32_LE(key +  0);
  state[ 5] = LOAD32_LE(key +  4);
  state[ 6] = LOAD32_LE(key +  8);
  state[ 7] = LOAD32_LE(key + 12);
  state[ 8] = LOAD32_LE(key + 16);
  state[ 9] = LOAD32_LE(key + 20);
  state[10] = LOAD32_LE(key + 24);
  state[11] = LOAD32_LE(key + 28);

  state[12] = counter;

  state[13] = LOAD32_LE(nonce + 0);
  state[14] = LOAD32_LE(nonce + 4);
  state[15] = LOAD32_LE(nonce + 8);

  while (n >= 64)
  {
    chacha20_block(block, state);
    for (i = 0; i < 64; i++)
    {
      data[i] ^= block[i];
    }
    state[12]++;
    data += 64;
    n -= 64;
  }

  if (n > 0)
  {
    chacha20_block(block, state);
    for (i = 0; i < n; i++)
    {
      data[i] ^= block[i];
    }
  }
}

/*
 * Poly1305 authentication tags
 */
void poly1305(const unsigned char* msg, size_t n, const unsigned char key[32],
              unsigned char tag[16])
{
  uint32_t c, m, w;
  uint32_t r0, r1, r2, r3, r4;
  uint32_t s1, s2, s3, s4;
  uint64_t f0, f1, f2, f3;
  uint32_t g0, g1, g2, g3, g4;
  uint32_t h0, h1, h2, h3, h4;
  unsigned char buf[16];
  size_t i;

  c = 1 << 24;
  r0 = (LOAD32_LE(key +  0) >> 0) & 0x03FFFFFF;
  r1 = (LOAD32_LE(key +  3) >> 2) & 0x03FFFF03;
  r2 = (LOAD32_LE(key +  6) >> 4) & 0x03FFC0FF;
  r3 = (LOAD32_LE(key +  9) >> 6) & 0x03F03FFF;
  r4 = (LOAD32_LE(key + 12) >> 8) & 0x000FFFFF;
  s1 = r1 * 5; s2 = r2 * 5; s3 = r3 * 5; s4 = r4 * 5;
  h0 = h1 = h2 = h3 = h4 = 0;
  while (n >= 16)
  {
    uint64_t d0, d1, d2, d3, d4;
process_block:
    h0 += (LOAD32_LE(msg +  0) >> 0) & 0x03FFFFFF;
    h1 += (LOAD32_LE(msg +  3) >> 2) & 0x03FFFFFF;
    h2 += (LOAD32_LE(msg +  6) >> 4) & 0x03FFFFFF;
    h3 += (LOAD32_LE(msg +  9) >> 6) & 0x03FFFFFF;
    h4 += (LOAD32_LE(msg + 12) >> 8) | c;

    #define MUL(a,b) ((uint64_t)(a) * (b))
    d0 = MUL(h0,r0) + MUL(h1,s4) + MUL(h2,s3) + MUL(h3,s2) + MUL(h4,s1);
    d1 = MUL(h0,r1) + MUL(h1,r0) + MUL(h2,s4) + MUL(h3,s3) + MUL(h4,s2);
    d2 = MUL(h0,r2) + MUL(h1,r1) + MUL(h2,r0) + MUL(h3,s4) + MUL(h4,s3);
    d3 = MUL(h0,r3) + MUL(h1,r2) + MUL(h2,r1) + MUL(h3,r0) + MUL(h4,s4);
    d4 = MUL(h0,r4) + MUL(h1,r3) + MUL(h2,r2) + MUL(h3,r1) + MUL(h4,r0);
    #undef MUL

    h0 = d0 & 0x03FFFFFF; d1 += (uint32_t)(d0 >> 26);
    h1 = d1 & 0x03FFFFFF; d2 += (uint32_t)(d1 >> 26);
    h2 = d2 & 0x03FFFFFF; d3 += (uint32_t)(d2 >> 26);
    h3 = d3 & 0x03FFFFFF; d4 += (uint32_t)(d3 >> 26);
    h4 = d4 & 0x03FFFFFF; h0 += (uint32_t)(d4 >> 26) * 5;
    h1 += (h0 >> 26); h0 = h0 & 0x03FFFFFF;

    msg += 16;
    n -= 16;
  }
  if (n)
  {
    for (i = 0; i < n; i++) buf[i] = msg[i];
    buf[i++] = 1;
    while (i < 16) buf[i++] = 0;
    msg = buf;
    n = 16;
    c = 0;
    goto process_block;
  }
  *(volatile uint32_t*) &r0 = 0;
  *(volatile uint32_t*) &r1 = 0; *(volatile uint32_t*) &s1 = 0;
  *(volatile uint32_t*) &r2 = 0; *(volatile uint32_t*) &s2 = 0;
  *(volatile uint32_t*) &r3 = 0; *(volatile uint32_t*) &s3 = 0;
  *(volatile uint32_t*) &r4 = 0; *(volatile uint32_t*) &s4 = 0;

  h2 += (h1 >> 26);     h1 &= 0x03FFFFFF;
  h3 += (h2 >> 26);     h2 &= 0x03FFFFFF;
  h4 += (h3 >> 26);     h3 &= 0x03FFFFFF;
  h0 += (h4 >> 26) * 5; h4 &= 0x03FFFFFF;
  h1 += (h0 >> 26);     h0 &= 0x03FFFFFF;

  g0 = h0 + 5;
  g1 = h1 + (g0 >> 26); g0 &= 0x03FFFFFF;
  g2 = h2 + (g1 >> 26); g1 &= 0x03FFFFFF;
  g3 = h3 + (g2 >> 26); g2 &= 0x03FFFFFF;
  g4 = h4 + (g3 >> 26) - (1 << 26); g3 &= 0x03FFFFFF;

  w = ~(m = (g4 >> 31) - 1);
  h0 = (h0 & w) | (g0 & m);
  h1 = (h1 & w) | (g1 & m);
  h2 = (h2 & w) | (g2 & m);
  h3 = (h3 & w) | (g3 & m);
  h4 = (h4 & w) | (g4 & m);

  f0 = ((h0 >>  0) | (h1 << 26)) + (uint64_t) LOAD32_LE(&key[16]);
  f1 = ((h1 >>  6) | (h2 << 20)) + (uint64_t) LOAD32_LE(&key[20]);
  f2 = ((h2 >> 12) | (h3 << 14)) + (uint64_t) LOAD32_LE(&key[24]);
  f3 = ((h3 >> 18) | (h4 <<  8)) + (uint64_t) LOAD32_LE(&key[28]);

  STORE32_LE(tag +  0, f0); f1 += (f0 >> 32);
  STORE32_LE(tag +  4, f1); f2 += (f1 >> 32);
  STORE32_LE(tag +  8, f2); f3 += (f2 >> 32);
  STORE32_LE(tag + 12, f3);
}

int poly1305_tagcmp(const unsigned char tag1[16], const unsigned char tag2[16])
{
  unsigned int d = 0;
  d |= tag1[ 0] ^ tag2[ 0];
  d |= tag1[ 1] ^ tag2[ 1];
  d |= tag1[ 2] ^ tag2[ 2];
  d |= tag1[ 3] ^ tag2[ 3];
  d |= tag1[ 4] ^ tag2[ 4];
  d |= tag1[ 5] ^ tag2[ 5];
  d |= tag1[ 6] ^ tag2[ 6];
  d |= tag1[ 7] ^ tag2[ 7];
  d |= tag1[ 8] ^ tag2[ 8];
  d |= tag1[ 9] ^ tag2[ 9];
  d |= tag1[10] ^ tag2[10];
  d |= tag1[11] ^ tag2[11];
  d |= tag1[12] ^ tag2[12];
  d |= tag1[13] ^ tag2[13];
  d |= tag1[14] ^ tag2[14];
  d |= tag1[15] ^ tag2[15];
  return d;
}

/*
 * Platform-specific entropy functions for seeding RNG
 */
#if defined(__unix__) || defined(__APPLE__)
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

#ifdef __linux__
#include <linux/random.h>
#endif

/* Returns the number of urandom bytes read (either 0 or n) */
static size_t read_urandom(void* buf, size_t n)
{
  size_t i;
  ssize_t ret;
  int fd, count;
  struct stat st;
  int errnold = errno;

  do
  {
    fd = open("/dev/urandom", O_RDONLY, 0);
  }
  while (fd == -1 && errno == EINTR);
  if (fd == -1)
    goto fail;
  fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);

  /* Check the sanity of the device node */
  if (fstat(fd, &st) == -1 || !S_ISCHR(st.st_mode)
                         #ifdef __linux__
                           || ioctl(fd, RNDGETENTCNT, &count) == -1
                         #endif
     )
  {
    close(fd);
    goto fail;
  }

  /* Read bytes */
  for (i = 0; i < n; i += ret)
  {
    while ((ret = read(fd, (char *)buf + i, n - i)) == -1)
    {
      if (errno != EAGAIN && errno != EINTR)
      {
        close(fd);
        goto fail;
      }
    }
  }
  close(fd);

  /* Verify that the random device returned non-zero data */
  for (i = 0; i < n; i++)
  {
    if (((unsigned char *)buf)[i] != 0)
    {
      errno = errnold;
      return n;
    }
  }

  /* Tiny n may unintentionally fall through! */
fail:
  fprintf(stderr, "bad /dev/urandom RNG)\n");
  abort(); /* PANIC! */
  return 0;
}

static size_t entropy(void* buf, size_t n)
{
#if defined(__linux__) && defined(SYS_getrandom)
  if (syscall(SYS_getrandom, buf, n, 0) == n)
    return n;
#elif defined(SYS_getentropy)
  if (syscall(SYS_getentropy, buf, n) == 0)
    return n;
#endif
  return read_urandom(buf, n);
}

#elif defined(_WIN32)

#include <windows.h>
#define RtlGenRandom SystemFunction036
BOOLEAN NTAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);
#pragma comment(lib, "advapi32.lib")

static size_t entropy(void* buf, size_t n)
{
  return RtlGenRandom(buf, n) ? n : 0;
}

#else
# error "Secure pseudorandom number generator unimplemented for this OS"
#endif

/*
 * ChaCha20 random number generator
 */
void chacha20_rng(void* out, size_t n)
{
  static size_t available = 0;
  static uint32_t counter = 0xFFFFFFFF;
  static unsigned char key[32], nonce[12], buffer[64];
  wx_sqlite3_mutex* mutex;
  size_t m;
    
  mutex = wx_sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_PRNG);
  wx_sqlite3_mutex_enter(mutex);
  while (n > 0)
  {
    if (available == 0)
    {
      if (counter == 0xFFFFFFFF)
      {
        if (entropy(key, sizeof(key)) != sizeof(key))
          abort();
        if (entropy(nonce, sizeof(nonce)) != sizeof(nonce))
          abort();
        counter = 0;
      }
      chacha20_xor(buffer, sizeof(buffer), key, nonce, ++counter);
      available = sizeof(buffer);
    }
    m = (available < n) ? available : n;
    memcpy(out, buffer + (sizeof(buffer) - available), m);
    out = (unsigned char *)out + m;
    available -= m;
    n -= m;
  }
  wx_sqlite3_mutex_leave(mutex);
}

