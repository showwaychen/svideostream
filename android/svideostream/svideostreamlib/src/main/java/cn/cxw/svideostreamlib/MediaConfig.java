package cn.cxw.svideostreamlib;

import java.util.HashMap;
import java.util.Iterator;

/**
 * Created by user on 2017/11/16.
 */

public class MediaConfig {
        private final HashMap<String, String> configs;

    public MediaConfig() {
        this.configs = new HashMap<>();
    }
    public void putConfig(String key, String value)
    {
        configs.put(key, value);
    }
    private static String stringifyKeyValuePairList(HashMap<String, String> list) {
        StringBuilder builder = new StringBuilder("[");
        for (HashMap.Entry pair : list.entrySet()) {
            if (builder.length() > 1) {
                builder.append(", ");
            }
            builder.append(pair.getKey() + ": " + pair.getValue());
        }
        return builder.append("]").toString();
    }
    private Iterator getKeySetIte()
    {

        return configs.entrySet().iterator();
    }
    public String toString()
    {
        return stringifyKeyValuePairList(configs);
    }
}
