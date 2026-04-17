#include "adiantum_vfs.h"
#include "adiantum_cipher.h"
#include <sqlite3.h>
#include <cstring>
#include <algorithm>
#include <random>

// Forward declarations of VFS methods
static int adiantum_xOpen(sqlite3_vfs* pVfs, const char* zName, sqlite3_file* pFile, int flags, int* pOutFlags);
static int adiantum_xDelete(sqlite3_vfs* pVfs, const char* zName, int syncDir);
static int adiantum_xAccess(sqlite3_vfs* pVfs, const char* zName, int flags, int* pResOut);
static int adiantum_xFullPathname(sqlite3_vfs* pVfs, const char* zName, int nOut, char* zOut);
static void* adiantum_xDlOpen(sqlite3_vfs* pVfs, const char* zFilename);
static void adiantum_xDlError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg);
static void (*adiantum_xDlSym(sqlite3_vfs* pVfs, void* pHdle, const char* zSymbol))(void);
static void adiantum_xDlClose(sqlite3_vfs* pVfs, void* pHandle);
static int adiantum_xRandomness(sqlite3_vfs* pVfs, int nByte, char* zOut);
static int adiantum_xSleep(sqlite3_vfs* pVfs, int microseconds);
static int adiantum_xCurrentTime(sqlite3_vfs* pVfs, double* pOut);
static int adiantum_xGetLastError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg);

// Per-file I/O methods
static int adiantum_file_xClose(sqlite3_file* pFile);
static int adiantum_file_xRead(sqlite3_file* pFile, void* pBuf, int iAmt, sqlite3_int64 iOfst);
static int adiantum_file_xWrite(sqlite3_file* pFile, const void* pBuf, int iAmt, sqlite3_int64 iOfst);
static int adiantum_file_xTruncate(sqlite3_file* pFile, sqlite3_int64 size);
static int adiantum_file_xSync(sqlite3_file* pFile, int flags);
static int adiantum_file_xFileSize(sqlite3_file* pFile, sqlite3_int64* pSize);
static int adiantum_file_xLock(sqlite3_file* pFile, int level);
static int adiantum_file_xUnlock(sqlite3_file* pFile, int level);
static int adiantum_file_xCheckReservedLock(sqlite3_file* pFile, int* pResOut);
static int adiantum_file_xFileControl(sqlite3_file* pFile, int op, void* pArg);
static int adiantum_file_xSectorSize(sqlite3_file* pFile);
static int adiantum_file_xDeviceCharacteristics(sqlite3_file* pFile);
static int adiantum_file_xShmMap(sqlite3_file* pFile, int iPg, int pgsz, int flags, void volatile** pp);
static int adiantum_file_xShmLock(sqlite3_file* pFile, int offset, int n, int flags);
static void adiantum_file_xShmBarrier(sqlite3_file* pFile);
static int adiantum_file_xShmUnmap(sqlite3_file* pFile, int del);
static int adiantum_file_xFetch(sqlite3_file* pFile, int iAmt, sqlite3_int64 iOfst, void** pp);
static int adiantum_file_xUnfetch(sqlite3_file* pFile, int iAmt, char* pPg);

// VFS instance
sqlite3_vfs AdiantumVFS::s_vfs;
bool AdiantumVFS::s_initialized = false;
std::mutex AdiantumVFS::s_keyMutex;
std::unordered_map<std::string, std::shared_ptr<AdiantumCtx>> AdiantumVFS::s_keyByPath;
std::unordered_map<std::string, int> AdiantumVFS::s_refCount;

// Get the base VFS (default)
static sqlite3_vfs* getBaseVfs()
{
    return sqlite3_vfs_find(nullptr);
}

void AdiantumVFS::initialize()
{
    if (s_initialized) return;

    memset(&s_vfs, 0, sizeof(s_vfs));
    s_vfs.iVersion = 3;  // SQLite 3.7.0+
    s_vfs.szOsFile = sizeof(AdiantumFile);
    s_vfs.mxPathname = 1024;
    s_vfs.zName = "adiantum";
    s_vfs.pAppData = nullptr;
    s_vfs.xOpen = adiantum_xOpen;
    s_vfs.xDelete = adiantum_xDelete;
    s_vfs.xAccess = adiantum_xAccess;
    s_vfs.xFullPathname = adiantum_xFullPathname;
    s_vfs.xDlOpen = nullptr;
    s_vfs.xDlError = nullptr;
    s_vfs.xDlSym = nullptr;
    s_vfs.xDlClose = nullptr;
    s_vfs.xRandomness = nullptr;
    s_vfs.xSleep = nullptr;
    s_vfs.xCurrentTime = nullptr;
    s_vfs.xGetLastError = nullptr;

    sqlite3_vfs_register(&s_vfs, 1);
    s_initialized = true;
}

sqlite3_vfs* AdiantumVFS::getVfs()
{
    return &s_vfs;
}

void AdiantumVFS::registerMainDbKey(const std::string& canonicalPath, const uint8_t rawKey[32])
{
    std::lock_guard<std::mutex> lock(s_keyMutex);
    auto ctx = std::make_shared<AdiantumCtx>(rawKey);
    s_keyByPath[canonicalPath] = ctx;
    s_refCount[canonicalPath] = 0;
}

void AdiantumVFS::unregisterMainDbKey(const std::string& canonicalPath)
{
    std::lock_guard<std::mutex> lock(s_keyMutex);
    s_keyByPath.erase(canonicalPath);
    s_refCount.erase(canonicalPath);
}

void AdiantumVFS::incrementRefCount(const std::string& path)
{
    std::lock_guard<std::mutex> lock(s_keyMutex);
    s_refCount[path]++;
}

void AdiantumVFS::decrementRefCount(const std::string& path)
{
    std::lock_guard<std::mutex> lock(s_keyMutex);
    auto it = s_refCount.find(path);
    if (it != s_refCount.end()) {
        it->second--;
        if (it->second <= 0) {
            s_keyByPath.erase(path);
            s_refCount.erase(it);
        }
    }
}

std::shared_ptr<AdiantumCtx> AdiantumVFS::lookupByOpenName(const char* zName)
{
    std::lock_guard<std::mutex> lock(s_keyMutex);

    if (!zName) {
        return nullptr;
    }

    std::string path(zName);

    // Try direct lookup first
    auto it = s_keyByPath.find(path);
    if (it != s_keyByPath.end()) {
        return it->second;
    }

    // Try stripping suffixes (-wal, -journal, -shm)
    static const char* suffixes[] = { "-wal", "-journal", "-shm" };
    for (const char* suffix : suffixes) {
        if (path.length() > strlen(suffix)) {
            size_t pos = path.rfind(suffix);
            if (pos == path.length() - strlen(suffix)) {
                std::string base = path.substr(0, pos);
                it = s_keyByPath.find(base);
                if (it != s_keyByPath.end()) {
                    return it->second;
                }
            }
        }
    }

    return nullptr;
}

// Round down to nearest 4096
static inline sqlite3_int64 roundDown4096(sqlite3_int64 off)
{
    return off & ~(sqlite3_int64(4096) - 1);
}

// Round up to nearest 4096
static inline sqlite3_int64 roundUp4096(sqlite3_int64 off)
{
    return (off + 4095) & ~(sqlite3_int64(4096) - 1);
}

static int adiantum_xOpen(sqlite3_vfs* pVfs, const char* zName, sqlite3_file* pFile, int flags, int* pOutFlags)
{
    sqlite3_vfs* pBase = getBaseVfs();
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    // Initialize our part
    memset(p, 0, sizeof(AdiantumFile));
    p->pRealFile = nullptr;
    p->ctx = nullptr;
    p->isEncrypted = false;
    p->isInitialized = false;
    p->refCount = 0;

    // Allocate the underlying file structure with proper size for base VFS
    p->pRealFile = reinterpret_cast<sqlite3_file*>(new char[pBase->szOsFile]);
    memset(p->pRealFile, 0, pBase->szOsFile);

    // Delegate to base VFS
    int rc = pBase->xOpen(pBase, zName, p->pRealFile, flags, pOutFlags);
    if (rc != SQLITE_OK) {
        delete p->pRealFile;
        p->pRealFile = nullptr;
        return rc;
    }

    // If this is a main database file, lookup the key
    if (zName && (flags & SQLITE_OPEN_MAIN_DB)) {
        p->ctx = AdiantumVFS::lookupByOpenName(zName);
        if (p->ctx) {
            p->isEncrypted = true;
            AdiantumVFS::incrementRefCount(std::string(zName));
        }
    }

    return SQLITE_OK;
}

static int adiantum_xDelete(sqlite3_vfs* pVfs, const char* zName, int syncDir)
{
    sqlite3_vfs* pBase = getBaseVfs();
    return pBase->xDelete(pBase, zName, syncDir);
}

static int adiantum_xAccess(sqlite3_vfs* pVfs, const char* zName, int flags, int* pResOut)
{
    sqlite3_vfs* pBase = getBaseVfs();
    return pBase->xAccess(pBase, zName, flags, pResOut);
}

static int adiantum_xFullPathname(sqlite3_vfs* pVfs, const char* zName, int nOut, char* zOut)
{
    sqlite3_vfs* pBase = getBaseVfs();
    return pBase->xFullPathname(pBase, zName, nOut, zOut);
}

static void* adiantum_xDlOpen(sqlite3_vfs* pVfs, const char* zFilename)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xDlOpen) {
        return pBase->xDlOpen(pBase, zFilename);
    }
    return nullptr;
}

static void adiantum_xDlError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xDlError) {
        pBase->xDlError(pBase, nByte, zErrMsg);
    }
}

static void (*adiantum_xDlSym(sqlite3_vfs* pVfs, void* pHdle, const char* zSymbol))(void)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xDlSym) {
        return pBase->xDlSym(pBase, pHdle, zSymbol);
    }
    return nullptr;
}

static void adiantum_xDlClose(sqlite3_vfs* pVfs, void* pHandle)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xDlClose) {
        pBase->xDlClose(pBase, pHandle);
    }
}

static int adiantum_xRandomness(sqlite3_vfs* pVfs, int nByte, char* zOut)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xRandomness) {
        return pBase->xRandomness(pBase, nByte, zOut);
    }
    // Fallback: use std::random_device
    std::random_device rd;
    for (int i = 0; i < nByte; i++) {
        zOut[i] = static_cast<char>(rd() & 0xFF);
    }
    return nByte;
}

static int adiantum_xSleep(sqlite3_vfs* pVfs, int microseconds)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xSleep) {
        return pBase->xSleep(pBase, microseconds);
    }
    return microseconds;
}

static int adiantum_xCurrentTime(sqlite3_vfs* pVfs, double* pOut)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xCurrentTime) {
        return pBase->xCurrentTime(pBase, pOut);
    }
    *pOut = 0.0;
    return SQLITE_OK;
}

static int adiantum_xGetLastError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg)
{
    sqlite3_vfs* pBase = getBaseVfs();
    if (pBase && pBase->xGetLastError) {
        return pBase->xGetLastError(pBase, nByte, zErrMsg);
    }
    return SQLITE_OK;
}

// File methods
static int adiantum_file_xClose(sqlite3_file* pFile)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    if (p->pRealFile) {
        int rc = SQLITE_OK;
        if (p->pRealFile->pMethods) {
            rc = p->pRealFile->pMethods->xClose(p->pRealFile);
        }
        delete[] reinterpret_cast<char*>(p->pRealFile);
        p->pRealFile = nullptr;
        return rc;
    }
    return SQLITE_OK;
}

static int adiantum_file_xRead(sqlite3_file* pFile, void* pBuf, int iAmt, sqlite3_int64 iOfst)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    if (!p->isEncrypted || !p->ctx || !p->ctx->initialized) {
        // Pass through to underlying file
        return p->pRealFile->pMethods->xRead(p->pRealFile, pBuf, iAmt, iOfst);
    }

    // For encrypted files, we need to decrypt
    // If reading at offset 0 with length ~100, this is SQLite checking for header
    // Return short read to let SQLite know this isn't a plain database
    if (iOfst == 0 && iAmt == 100 && !p->isInitialized) {
        return SQLITE_IOERR_SHORT_READ;
    }

    // Calculate aligned block range
    sqlite3_int64 blockStart = roundDown4096(iOfst);
    sqlite3_int64 blockEnd = roundUp4096(iOfst + iAmt);
    sqlite3_int64 blockLen = blockEnd - blockStart;

    // Temporary buffer for aligned read
    uint8_t* tempBuf = new uint8_t[blockLen];

    // Read aligned from underlying file
    int rc = p->pRealFile->pMethods->xRead(p->pRealFile, tempBuf, blockLen, blockStart);
    if (rc != SQLITE_OK) {
        delete[] tempBuf;
        return rc;
    }

    // Decrypt block by block
    uint8_t* out = reinterpret_cast<uint8_t*>(pBuf);
    sqlite3_int64 remaining = iAmt;
    sqlite3_int64 offset = iOfst;
    sqlite3_int64 blockOffset = blockStart;

    while (remaining > 0) {
        // Decrypt this 4K block in-place
        uint8_t tweak[8];
        put_le64(tweak, static_cast<uint64_t>(blockOffset));

        hbsh_decrypt(p->ctx.get(), tempBuf + (blockOffset - blockStart), tweak);

        // Copy decrypted data to output
        sqlite3_int64 blockStartLocal = (offset - blockOffset);
        sqlite3_int64 toCopy = std::min(sqlite3_int64(4096) - blockStartLocal, remaining);

        memcpy(out, tempBuf + (blockOffset - blockStart) + blockStartLocal, toCopy);

        out += toCopy;
        offset += toCopy;
        remaining -= toCopy;
        blockOffset += 4096;
    }

    delete[] tempBuf;
    return SQLITE_OK;
}

static int adiantum_file_xWrite(sqlite3_file* pFile, const void* pBuf, int iAmt, sqlite3_int64 iOfst)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    if (!p->isEncrypted || !p->ctx || !p->ctx->initialized) {
        // Pass through to underlying file
        return p->pRealFile->pMethods->xWrite(p->pRealFile, pBuf, iAmt, iOfst);
    }

    // Calculate aligned block range
    sqlite3_int64 blockStart = roundDown4096(iOfst);
    sqlite3_int64 blockEnd = roundUp4096(iOfst + iAmt);
    sqlite3_int64 blockLen = blockEnd - blockStart;

    // Temporary buffer for aligned operation
    uint8_t* tempBuf = new uint8_t[blockLen];

    // Read existing data for partial block writes
    if (blockStart < iOfst || (iOfst + iAmt) < blockEnd) {
        // Partial block - need to read existing data first
        int rc = p->pRealFile->pMethods->xRead(p->pRealFile, tempBuf, blockLen, blockStart);
        if (rc != SQLITE_OK && rc != SQLITE_IOERR_SHORT_READ) {
            delete[] tempBuf;
            return rc;
        }
    } else {
        memset(tempBuf, 0, blockLen);
    }

    // Copy new data into temp buffer
    const uint8_t* in = reinterpret_cast<const uint8_t*>(pBuf);
    sqlite3_int64 remaining = iAmt;
    sqlite3_int64 offset = iOfst;

    while (remaining > 0) {
        sqlite3_int64 blockOffLocal = offset - blockStart;
        sqlite3_int64 toCopy = std::min(sqlite3_int64(4096) - blockOffLocal, remaining);

        memcpy(tempBuf + blockOffLocal, in, toCopy);

        in += toCopy;
        offset += toCopy;
        remaining -= toCopy;
    }

    // Encrypt block by block
    uint8_t* encBuf = tempBuf;
    sqlite3_int64 encOffset = blockStart;

    while (encOffset < blockEnd) {
        uint8_t tweak[8];
        put_le64(tweak, static_cast<uint64_t>(encOffset));

        hbsh_encrypt(p->ctx.get(), encBuf, tweak);

        encBuf += 4096;
        encOffset += 4096;
    }

    // Write encrypted data
    int rc = p->pRealFile->pMethods->xWrite(p->pRealFile, tempBuf, blockLen, blockStart);
    delete[] tempBuf;

    return rc;
}

static int adiantum_file_xTruncate(sqlite3_file* pFile, sqlite3_int64 size)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    // Round up to 4096
    sqlite3_int64 roundedSize = roundUp4096(size);

    if (!p->isEncrypted || !p->ctx || !p->ctx->initialized) {
        return p->pRealFile->pMethods->xTruncate(p->pRealFile, roundedSize);
    }

    // For encrypted files, we need to handle truncation
    if (size < roundedSize) {
        // Read the partial block first
        uint8_t block[4096];
        sqlite3_int64 blockStart = roundDown4096(size);

        int rc = p->pRealFile->pMethods->xRead(p->pRealFile, block, 4096, blockStart);
        if (rc != SQLITE_OK && rc != SQLITE_IOERR_SHORT_READ) {
            return rc;
        }

        // Zero out the portion after size
        memset(block + (size - blockStart), 0, roundedSize - size);

        // Encrypt and write back
        uint8_t tweak[8];
        put_le64(tweak, static_cast<uint64_t>(blockStart));
        hbsh_encrypt(p->ctx.get(), block, tweak);

        rc = p->pRealFile->pMethods->xWrite(p->pRealFile, block, 4096, blockStart);
        if (rc != SQLITE_OK) {
            return rc;
        }
    }

    return p->pRealFile->pMethods->xTruncate(p->pRealFile, roundedSize);
}

static int adiantum_file_xSync(sqlite3_file* pFile, int flags)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xSync(p->pRealFile, flags);
}

static int adiantum_file_xFileSize(sqlite3_file* pFile, sqlite3_int64* pSize)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xFileSize(p->pRealFile, pSize);
}

static int adiantum_file_xLock(sqlite3_file* pFile, int level)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xLock(p->pRealFile, level);
}

static int adiantum_file_xUnlock(sqlite3_file* pFile, int level)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xUnlock(p->pRealFile, level);
}

static int adiantum_file_xCheckReservedLock(sqlite3_file* pFile, int* pResOut)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xCheckReservedLock(p->pRealFile, pResOut);
}

static int adiantum_file_xFileControl(sqlite3_file* pFile, int op, void* pArg)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);

    // Handle FCNTL_PRAGMA for hexkey
    if (op == SQLITE_FCNTL_PRAGMA) {
        char*** azArg = static_cast<char***>(pArg);
        if (azArg && *azArg && (*azArg)[1]) {
            const char* pragma = (*azArg)[1];
            if (strcmp(pragma, "key") == 0 || strcmp(pragma, "hexkey") == 0 || strcmp(pragma, "textkey") == 0) {
                // This is handled by the DbAdiantumInstance
                return SQLITE_OK;
            }
        }
        return SQLITE_NOTFOUND;
    }

    // Handle FCNTL_MMAP_SIZE - disable mmap for encrypted files
    if (op == SQLITE_FCNTL_MMAP_SIZE) {
        sqlite3_int64* pSize = static_cast<sqlite3_int64*>(pArg);
        *pSize = 0;  // Disable mmap
        return SQLITE_OK;
    }

    return p->pRealFile->pMethods->xFileControl(p->pRealFile, op, pArg);
}

static int adiantum_file_xSectorSize(sqlite3_file* pFile)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    int baseSector = p->pRealFile->pMethods->xSectorSize(p->pRealFile);
    // Return LCM of base sector size and our block size
    return std::lcm(baseSector, 4096);
}

static int adiantum_file_xDeviceCharacteristics(sqlite3_file* pFile)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    int chars = p->pRealFile->pMethods->xDeviceCharacteristics(p->pRealFile);
    // Filter out unsafe characteristics
    chars &= (SQLITE_IOCAP_ATOMIC |
              SQLITE_IOCAP_ATOMIC512 |
              SQLITE_IOCAP_ATOMIC1K |
              SQLITE_IOCAP_ATOMIC2K |
              SQLITE_IOCAP_ATOMIC4K |
              SQLITE_IOCAP_SAFE_APPEND |
              SQLITE_IOCAP_SEQUENTIAL);
    return chars;
}

static int adiantum_file_xShmMap(sqlite3_file* pFile, int iPg, int pgsz, int flags, void volatile** pp)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xShmMap(p->pRealFile, iPg, pgsz, flags, pp);
}

static int adiantum_file_xShmLock(sqlite3_file* pFile, int offset, int n, int flags)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xShmLock(p->pRealFile, offset, n, flags);
}

static void adiantum_file_xShmBarrier(sqlite3_file* pFile)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    p->pRealFile->pMethods->xShmBarrier(p->pRealFile);
}

static int adiantum_file_xShmUnmap(sqlite3_file* pFile, int del)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xShmUnmap(p->pRealFile, del);
}

static int adiantum_file_xFetch(sqlite3_file* pFile, int iAmt, sqlite3_int64 iOfst, void** pp)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    // Disable fetch (mmap) for encrypted files
    if (p->isEncrypted && p->ctx && p->ctx->initialized) {
        return SQLITE_OK;
    }
    return p->pRealFile->pMethods->xFetch(p->pRealFile, iAmt, iOfst, pp);
}

static int adiantum_file_xUnfetch(sqlite3_file* pFile, int iAmt, char* pPg)
{
    AdiantumFile* p = reinterpret_cast<AdiantumFile*>(pFile);
    return p->pRealFile->pMethods->xUnfetch(p->pRealFile, iAmt, pPg);
}
