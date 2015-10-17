package pl.com.salsoft.sqlitestudioremote.internal;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Created by SalSoft Pawel Salawa on 08.02.15.
 */
public class JsonConverter {
    public static Object fromJsonValue(Object input) {
        if (input instanceof JSONObject) {
            return fromJson((JSONObject) input);
        }

        if (input instanceof JSONArray) {
            return fromJson((JSONArray) input);
        }

        return input;
    }

    public static Object toJsonValue(Object o) {
        if (o == null) {
            return JSONObject.NULL;
        }
        if (o instanceof JSONArray || o instanceof JSONObject) {
            return o;
        }
        if (o.equals(JSONObject.NULL)) {
            return o;
        }
        try {
            if (o instanceof Collection) {
                return collectionToArray((Collection) o);
            }
            if (o instanceof byte[]) {
                JSONArray array = new JSONArray();
                array.put(Utils.toBlobString((byte[])o));
                return array;
            }
            if (o.getClass().isArray()) {
                return arrayToArray(o);
            }
            if (o instanceof Map) {
                return mapToObject((Map) o);
            }
            if (o instanceof Boolean ||
                    o instanceof Character ||
                    o instanceof Double ||
                    o instanceof Float ||
                    o instanceof Integer ||
                    o instanceof Long ||
                    o instanceof Short ||
                    o instanceof Byte ||
                    o instanceof String) {
                return o;
            }
            if (o.getClass().getPackage().getName().startsWith("java.")) {
                return o.toString();
            }
        } catch (Exception ignored) {
        }
        return null;
    }

    private static Object arrayToArray(Object o) {
        JSONArray array = new JSONArray();
        if (o.getClass().getComponentType().isPrimitive()) {
            int length = Array.getLength(o);
            for (int i = 0; i < length; i++) {
                Object obj = Array.get(o, i);
                array.put(obj);
            }
        } else {
            Object[] objects = (Object[]) o;
            for (Object obj : objects) {
                array.put(toJsonValue(obj));
            }
        }
        return array;
    }

    private static Object collectionToArray(Collection o) {
        JSONArray array = new JSONArray();
        for (Object obj : o) {
            array.put(toJsonValue(obj));
        }
        return array;
    }

    private static Object mapToObject(Map o) {
        JSONObject jsonObj = new JSONObject();
        for (Object key : o.keySet()) {
            try {
                jsonObj.put(""+key, toJsonValue(o.get(key)));
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return jsonObj;
    }

    private static HashMap<String,Object> fromJson(JSONObject object) {
        if (object == null) {
            return null;
        }

        HashMap<String,Object> map = new HashMap<>();
        Iterator<String> keys = object.keys();
        while (keys.hasNext()) {
            String key = keys.next();
            map.put(key, fromJsonValue(object.opt(key)));
        }
        return map;
    }

    private static List<Object> fromJson(JSONArray array) {
        if (array == null) {
            return null;
        }

        List<Object> list = new ArrayList<>();
        for (int i = 0; i < array.length(); i++) {
            list.add(fromJsonValue(array.opt(i)));
        }
        return list;
    }
}
