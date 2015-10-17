package pl.com.salsoft.sqlitestudioremote.internal;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * Created by SalSoft Pawel Salawa on 08.02.15.
 */
public class SQLiteStudioDbOpenHelper extends SQLiteOpenHelper {
    public SQLiteStudioDbOpenHelper(Context context, String dbPath) {
        super(context, dbPath, null, 1);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
    }
}
