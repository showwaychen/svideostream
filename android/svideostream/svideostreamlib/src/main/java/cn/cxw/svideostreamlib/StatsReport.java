package cn.cxw.svideostreamlib;

/**
 * Created by cxw on 2017/11/17.
 */

public class StatsReport {
    public final String mType;
    public final StatsReport.Value[] mValues;
    public StatsReport(String type, StatsReport.Value[] values) {
        this.mType = type;
        this.mValues = values;
    }
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append("type: ").append(this.mType).append(", values: ");

        for(int i = 0; i < this.mValues.length; ++i) {
            builder.append(this.mValues[i].toString()).append(", ");
        }

        return builder.toString();
    }
    public static class Value {
        public final String name;
        public final String value;

        public Value(String name, String value) {
            this.name = name;
            this.value = value;
        }

        public String toString() {
            StringBuilder builder = new StringBuilder();
            builder.append("[").append(this.name).append(": ").append(this.value).append("]");
            return builder.toString();
        }
    }
}
