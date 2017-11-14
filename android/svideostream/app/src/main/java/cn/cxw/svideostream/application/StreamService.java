package cn.cxw.svideostream.application;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.support.annotation.Nullable;

import cn.cxw.svideostreamlib.VideoStreamProxy;

/**
 * Created by cxw on 2017/11/12.
 */

public class StreamService  extends Service {
    VideoStreamProxy mVideoStream = null;
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
         mVideoStream = new VideoStreamProxy();
        super.onCreate();
    }
    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        mVideoStream.stopStream();
        mVideoStream = null;
        super.onDestroy();
    }
}
