package cn.cxw.svideostream.application;

import android.app.Application;

import com.squareup.leakcanary.LeakCanary;
import com.squareup.leakcanary.RefWatcher;

/**
 * Created by user on 2017/11/3.
 */

public class MainApplication extends Application {
    private static MainApplication instance;
    private RefWatcher mRefWatcher;
    Setting mSetting = null;
    @Override public void onCreate() {
        super.onCreate();
        instance = this;
        mSetting = new Setting(this);
                if (LeakCanary.isInAnalyzerProcess(this)) {
            // This process is dedicated to LeakCanary for heap analysis.
            // You should not init your app in this process.
            return;
        }
        mRefWatcher = LeakCanary.install(this);
    }
    public static MainApplication getInstance() {
        return instance;
    }
    public Setting getSetting()
    {
        return mSetting;
    }
    public static RefWatcher getRefWatcher() {
        return getInstance().mRefWatcher;
    }
}
