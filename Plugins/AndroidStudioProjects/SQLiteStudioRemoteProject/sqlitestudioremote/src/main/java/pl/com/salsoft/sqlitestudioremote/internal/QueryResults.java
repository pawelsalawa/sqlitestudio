package pl.com.salsoft.sqlitestudioremote.internal;

import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.os.OperationCanceledException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

/**
 * Created by SalSoft Pawel Salawa on 09.02.15.
 */
public class QueryResults {
    private List<String> columnNames;
    private List<HashMap<String,Object>> data;
    private ErrorCode errorCode = ErrorCode.SQLITE_OK;
    private String errorMessage;

    public QueryResults() {
    }

    public void readResults(Cursor cursor) {
        columnNames = Arrays.asList(cursor.getColumnNames());
        data = new ArrayList<>();
        for (boolean ok = cursor.moveToFirst(); ok; ok = cursor.moveToNext()) {
            data.add(readRow(cursor));
        }
    }

    public QueryResults(OperationCanceledException e) {
        errorMessage = e.getMessage();
        errorCode = ErrorCode.SQLITE_INTERRUPT;
    }

    public QueryResults(SQLiteException e, ErrorCode code) {
        errorMessage = e.getMessage();
        errorCode = code;
    }

    public boolean isError() {
        return errorCode != ErrorCode.SQLITE_OK;
    }

    public List<String> getColumnNames() {
        return columnNames;
    }

    public List<HashMap<String, Object>> getData() {
        return data;
    }

    public ErrorCode getErrorCode() {
        return errorCode;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    private HashMap<String, Object> readRow(Cursor cursor) {
        HashMap<String, Object> map = new HashMap<>();
        for (int i = 0; i < cursor.getColumnCount(); i++) {
            Object val = getValue(i, cursor);
            map.put(columnNames.get(i), val);
        }
        return map;
    }

    private Object getValue(int i, Cursor cursor) {
        switch (cursor.getType(i)) {
            case Cursor.FIELD_TYPE_BLOB:
                return cursor.getBlob(i);
            case Cursor.FIELD_TYPE_FLOAT:
                return cursor.getDouble(i);
            case Cursor.FIELD_TYPE_INTEGER:
                return cursor.getLong(i);
            case Cursor.FIELD_TYPE_NULL:
                return null;
            case Cursor.FIELD_TYPE_STRING:
                return cursor.getString(i);
        }
        throw new SQLiteException("Unknown field type for column number: "+i);
    }
}
