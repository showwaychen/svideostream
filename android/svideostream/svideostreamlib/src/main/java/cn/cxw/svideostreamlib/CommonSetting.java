package cn.cxw.svideostreamlib;

/**
 * Created by user on 2017/10/18.
 */

public class CommonSetting {
    static
    {
        System.loadLibrary("svideostream");
    }
    public  static native void nativeSetLogLevel(int level);
}
