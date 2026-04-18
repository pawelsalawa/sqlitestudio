#ifndef ADIANTUM_VFS_H
#define ADIANTUM_VFS_H

#include <sqlite3.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

// Forward declarations
struct AdiantumCtx;

// Forward declaration for sqlite3_io_methods
struct sqlite3_io_methods;

// Adiantum file handle - extends sqlite3_file with encryption context.
// Lifetime: placement-new'd in xOpen, explicitly destroyed in xClose. The
// raw storage is provided by SQLite (sqlite3_vfs::szOsFile). Never use
// memset on this struct — the shared_ptr member has non-trivial semantics.
struct AdiantumFile
{
    sqlite3_file base;                  // Parent VFS file handle (C struct)
    sqlite3_file* pRealFile;            // Underlying file handle (heap-allocated char[])
    std::shared_ptr<AdiantumCtx> ctx;   // Encryption context (owning, kept alive even if keymap removed)
    std::string path;                   // Canonical path for keymap bookkeeping on close
    bool isEncrypted;                   // True iff ctx is non-null and this file is encrypted
};

class AdiantumVFS
{
    public:
        // Initialize the VFS (called from plugin init)
        static void initialize();

        // Register/unregister keys for a database path
        static void registerMainDbKey(const std::string& canonicalPath, const uint8_t rawKey[32]);
        static void unregisterMainDbKey(const std::string& canonicalPath);

        // Lookup context by filename (handles -wal/-journal/-shm suffixes)
        static std::shared_ptr<AdiantumCtx> lookupByOpenName(const char* zName);

        // Increment/decrement reference count for a path
        static void incrementRefCount(const std::string& path);
        static void decrementRefCount(const std::string& path);

        // Get the sqlite3_vfs pointer
        static sqlite3_vfs* getVfs();

    private:
        friend struct AdiantumFile;
        static std::mutex s_keyMutex;
        static std::unordered_map<std::string, std::shared_ptr<AdiantumCtx>> s_keyByPath;
        static std::unordered_map<std::string, int> s_refCount;
        static sqlite3_vfs s_vfs;
        static bool s_initialized;
};

#endif // ADIANTUM_VFS_H
