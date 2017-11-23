package cn.cxw.svideostream.application;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.Nullable;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;

import java.lang.ref.WeakReference;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.Locale;

import cn.cxw.svideostream.R;
import cn.cxw.svideostreamlib.SVideoStream;
import cn.cxw.svideostreamlib.VideoStreamConstants;
import cn.cxw.svideostreamlib.VideoStreamProxy;

/**
 * Created by cxw on 2017/11/12.
 */

public class StreamService  extends Service implements SVideoStream.IStreamEventObserver {
    static String TAG = StreamService.class.getCanonicalName();

    private static final int NOTIFICATION_ID = 1; // 如果id设置为0,会导致不能设置为前台service
    private static final int REQUEST_CODE = 100;
    // 防止多个应用的广播交叉。。
    private static final String UUID = java.util.UUID.randomUUID().toString();



    public static class StreamAction {
        public static final String RECORD_READY = "action_record_ready_" + UUID;

        public static final String CLEAR_NOTIFICATION = "action_clear_notification_" + UUID;

        public static final String LIVE_READY = "action_live_ready_" + UUID;

        public static final String RECORD_START = "action_record_start_" + UUID;

        public static final String RECORD_STOP = "action_record_stop_" + UUID;
        public static final String RECORD_PAUSE = "action_record_pause_" + UUID;
        public static final String RECORD_RESUME = "action_record_resume_" + UUID;
    }
    Notification notification = null;
   NotificationManager  mNotificationManager = null;
    protected String action;

    boolean isLive = false;

    Handler mHandler = null;

    RemoteViews remoteViews = null;

     void refershRemoteUi()
    {
        if (remoteViews == null)
        {
            return ;
        }
        if (isLive)
        {
            remoteViews.setTextViewText(R.id.tv_streamtype, "live");
        }
        else
        {
            remoteViews.setTextViewText(R.id.tv_streamtype, "record");
        }
        int state = GlobalVideoStream.getScreenCaptureOwn().getState();
        Log.d(TAG,"stream state is = " + state);
        if (state == VideoStreamConstants.StreamState_STARTED || state == VideoStreamConstants.StreamState_STARTING)
        {
            remoteViews.setTextViewText(R.id.btn_start, "stop");
            PendingIntent stopIntent = PendingIntent.getBroadcast(this, REQUEST_CODE,  new Intent(StreamAction.RECORD_STOP), PendingIntent.FLAG_UPDATE_CURRENT);
            remoteViews.setOnClickPendingIntent(R.id.btn_start,  stopIntent);
            remoteViews.setTextViewText(R.id.tv_streamtime, "");
        }
        else if (state == VideoStreamConstants.StreamState_NONE || state == VideoStreamConstants.StreamState_STOPED)
        {
            remoteViews.setTextViewText(R.id.btn_start, "start");
            PendingIntent startIntent = PendingIntent.getBroadcast(this, REQUEST_CODE,  new Intent(StreamAction.RECORD_START), PendingIntent.FLAG_UPDATE_CURRENT);
            remoteViews.setOnClickPendingIntent(R.id.btn_start,  startIntent);
            remoteViews.setTextViewText(R.id.tv_streamtime, "");
        }

        if (notification != null)
        {
            mNotificationManager.notify(NOTIFICATION_ID, notification);
        }
    }
    static String secToString(float second) {

        long millis = (long)(second * 1000);
        boolean negative = millis < 0;
        millis = java.lang.Math.abs(millis);

        millis /= 1000;
        int sec = (int) (millis % 60);
        millis /= 60;
        int min = (int) (millis % 60);
        millis /= 60;
        int hours = (int) millis;

        String time;
        DecimalFormat format = (DecimalFormat) NumberFormat.getInstance(Locale.US);
        format.applyPattern("00");

        if (millis > 0) {
            time = (negative ? "-" : "") + hours + ":" + format.format(min) + ":" + format.format(sec);
        } else {
            time = (negative ? "-" : "") + min + ":" + format.format(sec);
        }

        return time;
    }
    void updateRemoteUiTime()
    {
        if (remoteViews == null)
        {
            return ;
        }
        int state = GlobalVideoStream.getScreenCaptureOwn().getState();
        if (state == VideoStreamConstants.StreamState_STARTED)
        {
            long time = GlobalVideoStream.getScreenCaptureOwn().getDuration();
            float sectime = (float) (time / 1000.0);
            remoteViews.setTextViewText(R.id.tv_streamtime, secToString(sectime));

        }
        mNotificationManager.notify(NOTIFICATION_ID, notification);
    }
    static class MyHandler extends Handler
    {
        private WeakReference<StreamService> mService;
        public static final  int SHOWTIME = 0;
        public MyHandler(StreamService service)
        {
            super(Looper.getMainLooper());
            mService = new WeakReference<>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case SHOWTIME:
                    if (mService.get() != null)
                    {
                        mService.get().updateRemoteUiTime();
                        if (GlobalVideoStream.getScreenCaptureOwn().getState() == VideoStreamConstants.StreamState_STARTED)
                        {
                            sendEmptyMessageDelayed(SHOWTIME, 1000);
                        }
                    }
                    break;
            }
        }
    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        IntentFilter filter = new IntentFilter();
        filter.addAction(StreamAction.RECORD_READY);
        filter.addAction(StreamAction.LIVE_READY);
        filter.addAction(StreamAction.CLEAR_NOTIFICATION);
        filter.addAction(StreamAction.RECORD_START);
        filter.addAction(StreamAction.RECORD_STOP);
        filter.addAction(StreamAction.RECORD_RESUME);
        filter.addAction(StreamAction.RECORD_PAUSE);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        registerReceiver(mRecordReceiver, filter);
        mHandler = new MyHandler(this);

        //初始化通知管理者
        mNotificationManager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    }
    void startForeground()
    {
        if (remoteViews == null)
        {
            remoteViews = new RemoteViews(this.getPackageName(), R.layout.notification_screen);
        }

        //按钮点击事件：
        refershRemoteUi();
        Notification.Builder builder = new Notification.Builder(this.getApplicationContext()).setContent(remoteViews); //获取一个Notification构造器

        builder.setWhen(System.currentTimeMillis())
               .setSmallIcon(R.mipmap.ic_launcher_round);
        notification = builder.getNotification(); // 获取构建好的Notification
        notification.flags = Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;
        startForeground(NOTIFICATION_ID, notification);
    }

    void stopForground()
    {
        stopForeground(true);
    }
    void clearNotification()
    {
//        stopForeground(true);
    }
    @Override
    public void onEvent(final int eventid, final int error) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                refershRemoteUi();
                if (eventid == VideoStreamConstants.SE_StreamStarted)
                {
                   mHandler.sendEmptyMessageDelayed(MyHandler.SHOWTIME, 1000);
                }
                if (eventid == VideoStreamConstants.SE_StreamFailed)
                {
                    Toast.makeText(getApplicationContext(), VideoStreamConstants.getErrorDes(error), Toast.LENGTH_LONG).show();
                }
            }
        });
    }
    void startRecord(boolean isLive)
    {
        VideoStreamProxy mVideoStream = GlobalVideoStream.getScreenCaptureOwn();
        mVideoStream.setVideoStreamSetting(MainApplication.getInstance().getSetting());
        mVideoStream.setStreamEventObserver(this);
        if (isLive)
        {
            String liveurl = GlobalSetting.getLiveUrl();
            if (!liveurl.isEmpty())
            {
                mVideoStream.setPublishUrl(liveurl);
            }
            else
            {
                return ;
            }
        }
        else
        {
            mVideoStream.setRecordPath(GlobalSetting.getRecordPath() + "svideostream_screen.mp4");

        }
        mVideoStream.startStream();
    }
    void stopRecord()
    {
        GlobalVideoStream.getScreenCaptureOwn().stopStream();
        refershRemoteUi();
    }
    void pauseRecord()
    {

    }
    void resumeRecord()
    {

    }
    private BroadcastReceiver mRecordReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            action = intent.getAction();
            Log.i(TAG, "action = " + action);
            if (StreamAction.RECORD_READY.equals(action)) {
                isLive = false;
                startForeground();
            } else if (StreamAction.LIVE_READY.equals(action)) {
                isLive = true;
                startForeground();
            } else if (StreamAction.CLEAR_NOTIFICATION.equals(action)) {
                Log.d(TAG, "receiver clear notification action");
//                stopForeground();
            } else if (StreamAction.RECORD_START.equals(action)) {
                startRecord(isLive);
//                mHandler.sendEmptyMessage(STRAT);
            }  else if (StreamAction.RECORD_STOP.equals(action)) {
                stopRecord();
//                mHandler.sendEmptyMessage(STOP);
//                stopForeground();
            } else if (StreamAction.RECORD_PAUSE.equals(action)) {
                pauseRecord();
//                mHandler.sendEmptyMessage(PAUSE);
            } else if (StreamAction.RECORD_RESUME.equals(action)) {
                resumeRecord();
//                mHandler.sendEmptyMessage(RESUME);
            } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                pauseRecord();
//                mHandler.sendEmptyMessage(PAUSE);
            } else if (Intent.ACTION_SCREEN_ON.equals(action)) {
                resumeRecord();
//                mHandler.sendEmptyMessage(RESUME);
            }
        }
    };
    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();
        stopForground();
        unregisterReceiver(mRecordReceiver);
    }

    /*
    准备录制.
    */
    public static void readyToRecord(Context context)
    {
        context.sendBroadcast(new Intent(StreamService.StreamAction.RECORD_READY));
    }
    /*
   准备录制.
   */
    public static void readyToLive(Context context)
    {
        context.sendBroadcast(new Intent(StreamService.StreamAction.LIVE_READY));
    }
    /**
     * 启动录制
     * @param context
     */
    public static void startRecording(Context context){
        context.sendBroadcast(new Intent(StreamService.StreamAction.RECORD_START));
    }


    /**
     * 结束
     * @param context
     */
    public static void stopRecording(Context context) {
        context.sendBroadcast(new Intent(StreamService.StreamAction.RECORD_STOP));
    }

    /**
     * 暂停录制
     * @param context
     */
    public static void pauseRecording(Context context) {
        context.sendBroadcast(new Intent(StreamService.StreamAction.RECORD_PAUSE));
    }

    /**
     * 继续录制
     * @param context
     */
    public static void resumeRecording(Context context) {
        context.sendBroadcast(new Intent(StreamService.StreamAction.RECORD_RESUME));  //发送广播，目的是为了在前台显示录制通知栏
    }

    /**
     * 清除通知栏
     * @param context
     */
    public static void clearNotificatton(Context context) {
        context.sendBroadcast(new Intent(StreamService.StreamAction.CLEAR_NOTIFICATION));
    }
}
