#include "db/sqlerrorcodes.h"

// This is defined in sqlite3.h, but we don't want to make a dependency
// from coreSQLiteStudio to sqlite3.h just for this one constraint,
// while the coreSQLiteStudio doesn't depend on sqlite lib at all.
// This should not change anyway. It's a documented value.
#define SQLITE_INTERRUPT    9

bool SqlErrorCode::isInterrupted(int errorCode)
{
    // Luckily SQLITE_INTERRUPT has the same value defined in both SQLite versions.
    return (errorCode == SqlErrorCode::INTERRUPTED || errorCode == SQLITE_INTERRUPT);
}
