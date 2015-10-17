package pl.com.salsoft.sqlitestudioremote.internal;

import android.util.Base64;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Created by SalSoft Pawel Salawa on 17.02.15.
 */
public class Utils {
    public static String LOG_TAG = "SQLiteStudioRemote";

    public static String toBlobString(byte[] blob) {
        StringBuilder builder = new StringBuilder();
        builder.append("X'").append(bytesToHex(blob)).append("'");
        return builder.toString();
    }

    final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static String createRegexFromGlob(String glob) {
        String out = "^";
        for (int i = 0; i < glob.length(); ++i) {
            final char c = glob.charAt(i);
            switch (c) {
                case '*':
                    out += ".*";
                    break;
                case '?':
                    out += '.';
                    break;
                case '.':
                    out += "\\.";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                default:
                    out += c;
            }
        }
        out += '$';
        return out;
    }

    public static byte[] md5(String s) {
        try {
            MessageDigest digest = java.security.MessageDigest.getInstance("MD5");
            digest.update(s.getBytes());
            return digest.digest();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return new byte[] {};
    }
}
