package pl.com.salsoft.sqlitestudioremote;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import pl.com.salsoft.sqlitestudioremote.internal.SQLiteStudioListener;
import pl.com.salsoft.sqlitestudioremote.internal.Utils;

/**
 * Created by SalSoft Pawel Salawa on 10.02.15.
 */
public class SQLiteStudioService extends Service {
    public static final int DEFAULT_PORT = 12121;
    private static SQLiteStudioService staticInstance;

    private SQLiteStudioListener listener;
    private Thread listenerThread;

    private boolean running = false;
    private int port = DEFAULT_PORT;
    private String password;
    private List<String> ipBlackList = new ArrayList<>();
    private List<String> ipWhiteList = new ArrayList<>();

    public void start(Context context) {
        if (running) {
            return;
        }

        listener = new SQLiteStudioListener(context);
        listener.setPort(port);
        listener.setPassword(password);
        listener.setIpBlackList(ipBlackList);
        listener.setIpWhiteList(ipWhiteList);

        listenerThread = new Thread(listener);
        listenerThread.start();
        running = true;
        Log.d(Utils.LOG_TAG, "Started instance on port " + port);
    }

    public void stop() {
        if (!running) {
            return;
        }

        Log.d(Utils.LOG_TAG, "Shutting down SQLiteStudioService instance.");
        listener.close();
        try {
            listenerThread.join();
        } catch (InterruptedException e) {
        }
        running = false;
    }

    public static SQLiteStudioService instance() {
        if (staticInstance == null) {
            staticInstance = new SQLiteStudioService();
        }
        return staticInstance;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public void addIpToBlackList(String ip) {
        ipBlackList.add(ip);
    }

    public void addIpToWhiteList(String ip) {
        ipWhiteList.add(ip);
    }

    public void setIpBlackList(String... ip) {
        ipBlackList.clear();
        for (String singleIp : ip) {
            ipBlackList.add(singleIp);
        }
    }

    public void setIpWhiteList(String... ip) {
        ipBlackList.clear();
        for (String singleIp : ip) {
            ipWhiteList.add(singleIp);
        }
    }

    public void setPort(int port) {
        this.port = port;
    }

    public boolean isRunning() {
        return running;
    }
}
