package pl.com.salsoft.sqlitestudioremote.internal;

/**
* Created by SalSoft Pawel Salawa on 09.02.15.
*/
enum ErrorCode {
    SQLITE_ABORT(4),
    SQLITE_AUTH(23),
    SQLITE_BUSY(5),
    SQLITE_CANTOPEN(14),
    SQLITE_CONSTRAINT(19),
    SQLITE_CORRUPT(11),
    SQLITE_DONE(101),
    SQLITE_EMPTY(16),
    SQLITE_ERROR(1),
    SQLITE_FORMAT(24),
    SQLITE_FULL(13),
    SQLITE_INTERNAL(2),
    SQLITE_INTERRUPT(9),
    SQLITE_IOERR(10),
    SQLITE_LOCKED(6),
    SQLITE_MISMATCH(20),
    SQLITE_MISUSE(21),
    SQLITE_NOLFS(22),
    SQLITE_NOMEM(7),
    SQLITE_NOTADB(26),
    SQLITE_NOTFOUND(12),
    SQLITE_NOTICE(27),
    SQLITE_OK(0),
    SQLITE_PERM(3),
    SQLITE_PROTOCOL(15),
    SQLITE_RANGE(25),
    SQLITE_READONLY(8),
    SQLITE_ROW(100),
    SQLITE_SCHEMA(17),
    SQLITE_TOOBIG(18),
    SQLITE_WARNING(28),
    ;

    private int code;

    ErrorCode(int code) {
        this.code = code;
    }

    public int getCode() {
        return code;
    }
}
