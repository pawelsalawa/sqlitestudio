package pl.com.salsoft.sqlitestudioremote.internal;

import android.content.Context;
import android.util.Log;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;
import java.util.concurrent.BlockingDeque;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import pl.com.salsoft.sqlitestudioremote.SQLiteStudioService;

/**
 * Created by SalSoft Pawel Salawa on 10.02.15.
 */
public class SQLiteStudioListener implements Runnable, ClientJobContainer {

    private final static int interval = 1000;

    private ServerSocket serverSocket;
    private int port = SQLiteStudioService.DEFAULT_PORT;
    private boolean running = true;
    private ThreadPoolExecutor threadPool;
    private BlockingDeque<Runnable> jobsQueue;
    private List<ClientHandler> clientJobs;
    private Context context;
    private String password;
    private List<String> ipWhiteList;
    private List<String> ipBlackList;
    private AuthService authService;

    public SQLiteStudioListener(Context context) {
        this.context = context;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public synchronized void close() {
        running = false;

        if (serverSocket != null) {
            try {
                serverSocket.close();
            } catch (IOException e) {
            }
            serverSocket = null;
        }

        if (threadPool != null && clientJobs != null) {
            threadPool.shutdown();
            for (ClientHandler handler : clientJobs) {
                handler.close();
            }

            try {
                threadPool.awaitTermination(10, TimeUnit.SECONDS);
            } catch (InterruptedException e) {
            }
        }
    }

    private synchronized boolean isRunning() {
        return running;
    }

    @Override
    public void run() {
        if (!init()) {
            return;
        }

        Log.d(Utils.LOG_TAG, "Listening for clients...");

        while (isRunning()) {
            try {
                Socket clientSocket = serverSocket.accept();
                ClientHandler clientHandler = new ClientHandler(clientSocket, context, this, authService);
                clientJobs.add(clientHandler);
                threadPool.execute(clientHandler);
            } catch (IOException e) {
                continue;
            }
        }

        Log.d(Utils.LOG_TAG, "Listener thread finished.");
    }

    private boolean init() {
        try {
            serverSocket = new ServerSocket(port, 5);
            serverSocket.setSoTimeout(interval);
        } catch (IOException e) {
            Log.e(Utils.LOG_TAG, "Error while opening listening socket: "+e.getMessage(), e);
            return false;
        }

        jobsQueue = new LinkedBlockingDeque<>(1);
        clientJobs = new CopyOnWriteArrayList<>();
        threadPool = new ThreadPoolExecutor(20, 20, 10, TimeUnit.SECONDS, jobsQueue);
        authService = new AuthServiceImpl(password, ipBlackList, ipWhiteList);
        return true;
    }

    @Override
    public void removeJob(ClientHandler handler) {
        clientJobs.remove(handler);
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public void setIpWhiteList(List<String> ipWhiteList) {
        this.ipWhiteList = ipWhiteList;
    }

    public void setIpBlackList(List<String> ipBlackList) {
        this.ipBlackList = ipBlackList;
    }
}
