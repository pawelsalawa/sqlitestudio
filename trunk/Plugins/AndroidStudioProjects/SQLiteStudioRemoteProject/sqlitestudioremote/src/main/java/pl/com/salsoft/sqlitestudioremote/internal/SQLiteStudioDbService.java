package pl.com.salsoft.sqlitestudioremote.internal;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteAbortException;
import android.database.sqlite.SQLiteAccessPermException;
import android.database.sqlite.SQLiteBindOrColumnIndexOutOfRangeException;
import android.database.sqlite.SQLiteBlobTooBigException;
import android.database.sqlite.SQLiteCantOpenDatabaseException;
import android.database.sqlite.SQLiteConstraintException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabaseCorruptException;
import android.database.sqlite.SQLiteDatabaseLockedException;
import android.database.sqlite.SQLiteDatatypeMismatchException;
import android.database.sqlite.SQLiteDiskIOException;
import android.database.sqlite.SQLiteDoneException;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteFullException;
import android.database.sqlite.SQLiteMisuseException;
import android.database.sqlite.SQLiteOutOfMemoryException;
import android.database.sqlite.SQLiteReadOnlyDatabaseException;
import android.database.sqlite.SQLiteTableLockedException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Created by SalSoft Pawel Salawa on 04.02.15.
 */
public class SQLiteStudioDbService {

    private HashMap<String,SQLiteDatabase> managedDatabases = new HashMap<>();
    private Context context;

    public SQLiteStudioDbService(Context context) {
        this.context = context.getApplicationContext();
    }

    public List<String> getDbList() {
        List<String> filteredList = new ArrayList<>();
        for (String dbFile : context.databaseList()) {
            if (dbFile.endsWith("-journal"))
                continue;

            filteredList.add(dbFile);
        }
        return filteredList;
    }

    public boolean deleteDb(String dbName) {
        return context.deleteDatabase(dbName);
    }

    public void releaseAll() {
        for (SQLiteDatabase db : managedDatabases.values()) {
            db.close();
        }
        managedDatabases.clear();
    }

    public QueryResults exec(String dbName, String query) {
        SQLiteDatabase db = getDb(dbName);
        QueryResults results;
        try {
            Cursor cursor = db.rawQuery(query, null);
            results = new QueryResults();
            results.readResults(cursor);
        } catch (SQLiteAbortException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_ABORT);
        } catch (SQLiteAccessPermException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_PERM);
        } catch (SQLiteBindOrColumnIndexOutOfRangeException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_RANGE);
        } catch (SQLiteBlobTooBigException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_TOOBIG);
        } catch (SQLiteCantOpenDatabaseException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_CANTOPEN);
        } catch (SQLiteConstraintException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_CONSTRAINT);
        } catch (SQLiteDatabaseCorruptException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_CORRUPT);
        } catch (SQLiteDatabaseLockedException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_BUSY);
        } catch (SQLiteDatatypeMismatchException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_MISMATCH);
        } catch (SQLiteDiskIOException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_IOERR);
        } catch (SQLiteDoneException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_DONE);
        } catch (SQLiteFullException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_FULL);
        } catch (SQLiteMisuseException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_MISUSE);
        } catch (SQLiteOutOfMemoryException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_NOMEM);
        } catch (SQLiteReadOnlyDatabaseException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_READONLY);
        } catch (SQLiteTableLockedException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_LOCKED);
        } catch (SQLiteException e) {
            results = new QueryResults(e, ErrorCode.SQLITE_ERROR);
        }

        return results;
    }

    private SQLiteDatabase getDb(String name) {
        if (managedDatabases.containsKey(name)) {
            return managedDatabases.get(name);
        }

        SQLiteStudioDbOpenHelper helper = new SQLiteStudioDbOpenHelper(context, name);
        SQLiteDatabase db = helper.getWritableDatabase();
        managedDatabases.put(name, db);
        return db;
    }
}
