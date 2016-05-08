package pl.com.salsoft.sqlitestudioremote.internal;

import android.content.Context;
import android.util.Base64;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;

/**
 * Created by SalSoft Pawel Salawa on 17.02.15.
 */
public class ClientHandler implements Runnable {

    private static enum State
    {
        READING_SIZE,
        READING_DATA
    }

    private static enum Error
    {
        INVALID_FORMAT,
        NO_COMMAND_SPECIFIED,
        UNKNOWN_COMMAND,
        NO_DATABASE_SPECIFIED,
        ERROR_READING_FROM_CLIENT
    }

    private static enum Command
    {
        LIST,
        QUERY,
        DELETE_DB
    }

    private static final int MAX_SIZE = 10 * 1024 * 1024;
    private static final String COMMAND_KEY = "cmd";
    private static final String AUTHORIZE_KEY = "auth";
    private static final String QUERY_KEY = "query";
    private static final String DBNAME_KEY = "db";
    private static final String GENERIC_ERROR_KEY = "generic_error";
    private static final String ERROR_CODE_KEY = "error_code";
    private static final String ERROR_MESSAGE_KEY = "error_message";
    private static final String COLUMNS_KEY = "columns";
    private static final String DATA_KEY = "data";
    private static final String DBLIST_KEY = "list";
    private static final String SIZE_KEY = "size";
    private static final String RESULT_KEY = "result";
    private static final String CONFIRM_VALUE = "ok";
    private static final String FAILURE_VALUE = "error";
    private static final String PONG_VALUE = "pong";
    private static final String tokenTpl = "06fn43%d3ig7ws%d53";

    private Socket clientSocket;
    private SocketChannel channel;
    private ClientJobContainer jobContainer;
    private boolean running = true;
    private InputStream inputStream;
    private OutputStream outputStream;
    private byte[] sizeBuffer = new byte[4];
    private byte[] dataBuffer = new byte[0];
    private DataInputStream dataInputStream;
    private State currentState = State.READING_SIZE;
    private Context context;
    private SQLiteStudioDbService dbService;
    private AuthService authService;
    private boolean authorized = false;
    private boolean denyAccess = false;

    public ClientHandler(Socket clientSocket, Context context, ClientJobContainer jobContainer,
                         AuthService authService) {
        this.clientSocket = clientSocket;
        this.jobContainer = jobContainer;
        this.context = context;
        this.authService = authService;
        dbService = new SQLiteStudioDbService(context);
        authorized = !authService.isAuthRequired();
    }

    public synchronized void close() {
        running = false;
        try {
            clientSocket.close();
        } catch (IOException e) {
            // Irrelevant
        }
    }

    private synchronized boolean isRunning() {
        return running;
    }

    @Override
    public void run() {
        String ip = clientSocket.getInetAddress().getHostAddress();
        Log.d(Utils.LOG_TAG, "New client from " + ip);

        if (!authService.isIpAllowed(ip)) {
            Log.e(Utils.LOG_TAG, "Client's IP address not allowed: " + ip + ", disconnecting.");
            cleanUp();
            return;
        }

        if (!init()) {
            Log.e(Utils.LOG_TAG, "Could not initialize handler for the client.");
            cleanUp();
            return;
        }

        while (isRunning() && !denyAccess) {
            readClientChannel();
        }

        cleanUp();
        Log.d(Utils.LOG_TAG, "Disconnected client "+ip);
    }

    private void readClientChannel() {
        if (!clientSocket.isConnected()) {
            close();
            return;
        }

        try {
            switch (currentState) {
                case READING_SIZE:
                    dataInputStream.readFully(sizeBuffer);
                    break;
                case READING_DATA:
                    dataInputStream.readFully(dataBuffer);
                    break;
            }
        } catch (EOFException e) {
            close();
            return;
        } catch (IOException e) {
            Log.e(Utils.LOG_TAG, "Error while reading input from client: "+e.getMessage(), e);
            sendError(Error.ERROR_READING_FROM_CLIENT);
            return;
        }

        // Everything went okay, process data
        switch (currentState) {
            case READING_SIZE: {
                int size = ByteBuffer.wrap(sizeBuffer).order(ByteOrder.LITTLE_ENDIAN).getInt();
                if (size > MAX_SIZE) {
                    Log.e(Utils.LOG_TAG, "Error while reading input from client: maximum size exceeded: "+size);
                    sendError(Error.ERROR_READING_FROM_CLIENT);
                    return;
                }
                currentState = State.READING_DATA;
                dataBuffer = new byte[size];
                break;
            }
            case READING_DATA: {
                String str = null;
                try {
                    str = new String(dataBuffer, "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    Log.e(Utils.LOG_TAG, "Error while reading data from client: "+e.getMessage(), e);
                    sendError(Error.ERROR_READING_FROM_CLIENT);
                    return;
                }
                handleRequest(str);
                currentState = State.READING_SIZE;
                break;
            }
        }
    }

    private boolean init() {
        try {
            inputStream = clientSocket.getInputStream();
            outputStream = clientSocket.getOutputStream();
        } catch (IOException e) {
            return false;
        }
        dataInputStream = new DataInputStream(inputStream);
        return true;
    }

    private void handleRequest(String data) {
        JSONObject json;
        try {
            json = new JSONObject(data);
        } catch (JSONException e) {
            sendError(Error.INVALID_FORMAT);
            return;
        }

        HashMap<String,Object> map = (HashMap<String,Object>)JsonConverter.fromJsonValue(json);

        if (!authorized) {
            authorize(map);
            return;
        }

        if (!map.containsKey(COMMAND_KEY)) {
            sendError(Error.NO_COMMAND_SPECIFIED);
            return;
        }

        Command cmd;
        try {
            cmd = Command.valueOf("" + map.get(COMMAND_KEY));
        } catch (IllegalArgumentException e) {
            sendError(Error.UNKNOWN_COMMAND);
            return;
        }

        switch (cmd) {
            case LIST:
                send(DBLIST_KEY, dbService.getDbList());
                break;
            case QUERY:
                execAndRespond(map.get(DBNAME_KEY), ""+map.get(QUERY_KEY));
                break;
            case DELETE_DB:
                deleteDbAndRespond(map.get(DBNAME_KEY));
                break;
        }
    }

    private void authorize(HashMap<String, Object> map) {
        if (!map.containsKey(AUTHORIZE_KEY)) {
            Log.w(Utils.LOG_TAG, "Client authorization failed - no 'auth' key in first request.");
            denyAccess = true;
            return;
        }

        String pass = "" + map.get(AUTHORIZE_KEY);
        if (!authService.authorize(pass)) {
            Log.w(Utils.LOG_TAG, "Client authorization failed - invalid password: " + pass);
            denyAccess = true;
            return;
        }

        authorized = true;
        Log.w(Utils.LOG_TAG, "Client authorization successful.");
        sendResult(CONFIRM_VALUE);
    }

    private void deleteDbAndRespond(Object dbName) {
        if (dbName == null || dbName.toString().isEmpty()) {
            sendError(Error.NO_DATABASE_SPECIFIED);
            return;
        }

        String dbNameStr = dbName.toString();
        sendResult(dbService.deleteDb(dbNameStr) ? CONFIRM_VALUE : FAILURE_VALUE);
    }

    private void execAndRespond(Object dbName, String data) {
        if (dbName == null || dbName.toString().isEmpty()) {
            sendError(Error.NO_DATABASE_SPECIFIED);
            return;
        }

        String dbNameStr = dbName.toString();
        HashMap<String,Object> map = new HashMap<>();
        QueryResults results = dbService.exec(dbNameStr, data);
        if (results.isError()) {
            map.put(ERROR_CODE_KEY, results.getErrorCode());
            map.put(ERROR_MESSAGE_KEY, results.getErrorMessage());
        } else {
            map.put(COLUMNS_KEY, results.getColumnNames());
            map.put(DATA_KEY, results.getData());
        }
        send(map);
    }

    private void sendError(Error error) {
        send(GENERIC_ERROR_KEY, error.ordinal());
    }

    private void sendResult(String resultValue) {
        send(RESULT_KEY, resultValue);
    }

    private void send(String key, Object value) {
        HashMap<String,Object> map = new HashMap<>();
        map.put(key, value);
        send(map);
    }

    private void send(HashMap<String,Object> map) {
        send(JsonConverter.toJsonValue(map).toString());
    }

    private void send(String response) {
        try {
            byte[] bytes = response.getBytes("UTF-8");

            ByteBuffer sizeBuffer = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN);
            sizeBuffer.putInt(bytes.length);

            outputStream.write(sizeBuffer.array());
            outputStream.write(bytes);
        } catch (UnsupportedEncodingException e) {
            Log.e(Utils.LOG_TAG, "Could not convert response to UTF-8: " + e.getMessage(), e);
        } catch (IOException e) {
            Log.e(Utils.LOG_TAG, "Could not send response to client: " + e.getMessage(), e);
        }
    }

    private void cleanUp() {
        if (dbService != null) {
            dbService.releaseAll();
        }

        if (inputStream != null) {
            try {
                inputStream.close();
            } catch (IOException e) {
                // Irrelevant
            }
        }

        if (outputStream != null) {
            try {
                outputStream.close();
            } catch (IOException e) {
                // Irrelevant
            }
        }

        if (dataInputStream != null) {
            try {
                dataInputStream.close();
            } catch (IOException e) {
                // Irrelevant
            }
        }

        if (clientSocket != null) {
            try {
                clientSocket.close();
            } catch (IOException e) {
                // Irrelevant
            }
        }
        jobContainer.removeJob(this);
    }
}
