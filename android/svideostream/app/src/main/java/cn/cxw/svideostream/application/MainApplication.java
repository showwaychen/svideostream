package cn.cxw.svideostream.application;

import android.app.Application;

/**
 * Created by user on 2017/11/3.
 */

public class MainApplication extends Application {
    private static MainApplication instance;
    Setting mSetting = null;
    @Override public void onCreate() {
        super.onCreate();
        instance = this;
        mSetting = new Setting(this);
    }
    public static MainApplication getInstance() {
        return instance;
    }
    public Setting getSetting()
    {
        return mSetting;
    }
}
