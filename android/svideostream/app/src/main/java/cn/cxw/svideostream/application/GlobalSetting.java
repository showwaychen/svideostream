package cn.cxw.svideostream.application;

import android.os.Environment;
import android.util.Log;

import java.io.File;

/**
 * Created by user on 2017/11/23.
 */

public class GlobalSetting {
    public static String getRecordPath()
    {
        String recordpath = Environment.getExternalStorageDirectory() + "/svideostream/";
        File file = new File(recordpath);
        if (!file.exists())
        {
            if (!file.mkdir())
            {
                Log.e("getRecordPath", " mkdirs failed");
            }
        }
        return recordpath;
    }
    public static String getLiveUrl()
    {
        return "rtmp://940239";
    }
}
