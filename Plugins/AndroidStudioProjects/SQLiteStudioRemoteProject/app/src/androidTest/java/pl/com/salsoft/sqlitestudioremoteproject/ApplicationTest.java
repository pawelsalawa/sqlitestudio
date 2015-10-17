package pl.com.salsoft.sqlitestudioremoteproject;

import android.app.Application;
import android.content.ContentValues;
import android.database.sqlite.SQLiteDatabase;
import android.test.ApplicationTestCase;

import java.util.List;

import pl.com.salsoft.sqlitestudioremote.internal.QueryResults;
import pl.com.salsoft.sqlitestudioremote.internal.SQLiteStudioDbService;

/**
 * <a href="http://d.android.com/tools/testing/testing_android.html">Testing Fundamentals</a>
 */
public class ApplicationTest extends ApplicationTestCase<Application> {
    private static final String DBNAME = "test_database";
    private static final String TABLENAME = "test123";

    private SQLiteDatabase db;
    private SQLiteStudioDbService service;

    public ApplicationTest() {
        super(Application.class);
    }

    public void testDbList() {
        List<String> dbList = service.getDbList();
        assertEquals(1, dbList.size());
        assertEquals(DBNAME, dbList.get(0));
    }

    public void testSelect1() {
        QueryResults res = service.exec(db.getPath(), "SELECT * FROM " + TABLENAME);
        assertEquals(2, res.getData().size());
    }

    public void testSelect2() {
        QueryResults res = service.exec(db.getPath(), "SELECT * FROM "+TABLENAME+" WHERE id > 1");
        assertEquals(1, res.getData().size());
        assertTrue(res.getData().get(0).get("id") instanceof Long);
        assertTrue(res.getData().get(0).get("val") instanceof String);
        assertTrue(res.getData().get(0).get("val2") instanceof String);
        assertTrue(res.getData().get(0).get("val3") instanceof Double);
        assertEquals(2L, res.getData().get(0).get("id"));
    }

    public void testSelect3() {
        QueryResults res = service.exec(db.getPath(), "SELECT * FROM "+TABLENAME+" WHERE val2 = X'1E1F'");
        assertEquals(1, res.getData().size());
        assertEquals(2L, res.getData().get(0).get("id"));
    }

    public void testSelect4() {
        QueryResults res = service.exec(db.getPath(), "SELECT * FROM "+TABLENAME+" WHERE val3 > 1.23456788");
        assertEquals(1, res.getData().size());
        assertEquals(2L, res.getData().get(0).get("id"));
    }

    public void testInsert() {
        QueryResults res = service.exec(db.getPath(), "INSERT INTO "+TABLENAME+" VALUES (4, 'qwe', X'13', 5.44)");
        assertFalse(res.isError());

        res = service.exec(db.getPath(), "SELECT * FROM "+TABLENAME);
        assertEquals(3, res.getData().size());
    }

    public void testPragma() {
        QueryResults res = service.exec(db.getPath(), "PRAGMA table_info("+TABLENAME+")");
        assertFalse(res.isError());
        assertEquals(4, res.getData().size());
    }


    public void testError() {
        QueryResults res = service.exec(db.getPath(), "SELECT X X X X");
        assertTrue(res.isError());
        assertNotNull(res.getErrorMessage());
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();

        service = new SQLiteStudioDbService(getContext());

        db = (new DatabaseHelper(DBNAME, getContext())).getWritableDatabase();
        db.execSQL("drop table if exists "+TABLENAME);
        db.execSQL("create table "+TABLENAME+" (id INTEGER, val TEXT, val2 BLOB, val3 NUMERIC)");

        ContentValues vals = new ContentValues();
        vals.put("id", 1);
        vals.put("val", "value");
        vals.put("val2", new byte[] {34, 35});
        vals.put("val3", 0.55);
        db.insert(TABLENAME, null, vals);

        vals.put("id", 2);
        vals.put("val", "abc");
        vals.put("val2", new byte[] {30, 31});
        vals.put("val3", 1.23456789);
        db.insert(TABLENAME, null, vals);
    }
}