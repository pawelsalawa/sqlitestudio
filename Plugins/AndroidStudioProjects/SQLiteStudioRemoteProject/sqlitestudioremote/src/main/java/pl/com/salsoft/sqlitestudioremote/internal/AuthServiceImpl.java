package pl.com.salsoft.sqlitestudioremote.internal;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

/**
 * Created by SalSoft Pawel Salawa on 11.04.15.
 */
public class AuthServiceImpl implements AuthService {

    private String password;
    private List<Pattern> ipBlackList;
    private List<Pattern> ipWhiteList;

    public AuthServiceImpl(String password, List<String> ipBlackList, List<String> ipWhiteList) {
        this.password = password;

        if (ipBlackList != null) {
            this.ipBlackList = new ArrayList<>();
            for (String ipBlack : ipBlackList) {
                this.ipBlackList.add(Pattern.compile(Utils.createRegexFromGlob(ipBlack)));
            }
        }

        if (ipWhiteList != null) {
            this.ipWhiteList = new ArrayList<>();
            for (String ipWhite : ipWhiteList) {
                this.ipWhiteList.add(Pattern.compile(Utils.createRegexFromGlob(ipWhite)));
            }
        }
    }

    @Override
    public boolean isAuthRequired() {
        return password != null && !password.isEmpty();
    }

    @Override
    public boolean authorize(String password) {
        return password != null && password.equals(this.password);
    }

    @Override
    public boolean isIpAllowed(String ip) {
        if (ipBlackList == null) {
            return true;
        }

        boolean allowed = true;
        for (Pattern blackIp : ipBlackList) {
            if (blackIp.matcher(ip).matches()) {
                allowed = false;
                break;
            }
        }

        if (ipWhiteList == null) {
            return allowed;
        }

        for (Pattern whiteIp : ipWhiteList) {
            if (whiteIp.matcher(ip).matches()) {
                allowed = true;
                break;
            }
        }

        return allowed;
    }
}
