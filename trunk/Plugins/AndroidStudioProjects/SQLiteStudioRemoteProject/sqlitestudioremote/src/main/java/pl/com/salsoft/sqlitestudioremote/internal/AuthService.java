package pl.com.salsoft.sqlitestudioremote.internal;

/**
 * Created by SalSoft Pawel Salawa on 11.04.15.
 */
public interface AuthService {
    boolean isAuthRequired();
    boolean authorize(String password);
    boolean isIpAllowed(String ip);
}
