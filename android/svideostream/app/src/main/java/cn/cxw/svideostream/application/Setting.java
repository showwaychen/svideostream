package cn.cxw.svideostream.application;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import cn.cxw.svideostream.R;
import cn.cxw.svideostreamlib.VideoStreamSetting;

/**
 * Created by cxw on 2017/10/29.
 */

public class Setting extends VideoStreamSetting {
    private Context mAppContext;
    private SharedPreferences mSharedPreference;

    public Setting(Context context)
    {
        mAppContext = context.getApplicationContext();
        mSharedPreference = PreferenceManager.getDefaultSharedPreferences(mAppContext);
    }
    @Override
    public int getVideoEncoderType()
    {
        String key = mAppContext.getString(R.string.pref_key_videoencodertype);
        String value = mSharedPreference.getString(key, "");
        try {
            return Integer.valueOf(value).intValue();
        }
        catch (NumberFormatException e)
        {
            return 0;
        }
    }
    @Override
    public boolean getAudioEnable()
    {
        String key = mAppContext.getString(R.string.pref_key_audioenable);
        return mSharedPreference.getBoolean(key, true);
    }

    @Override
    public int getVideoBitrate() {
        String key = mAppContext.getString(R.string.pref_key_videoencodebitrate);
        String value = mSharedPreference.getString(key, "300000");
        try {
            return Integer.valueOf(value).intValue();
        }
        catch (NumberFormatException e)
        {
            return super.getVideoBitrate();
        }
    }

    @Override
    public int getVideoFps() {
        String key = mAppContext.getString(R.string.pref_key_videoencodefps);
        String value = mSharedPreference.getString(key, "15");
        try {
            return Integer.valueOf(value).intValue();
        }
        catch (NumberFormatException e)
        {
            return super.getVideoFps();
        }
    }
}
