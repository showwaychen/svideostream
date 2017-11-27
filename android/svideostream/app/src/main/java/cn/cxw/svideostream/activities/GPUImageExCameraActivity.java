package cn.cxw.svideostream.activities;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.FrameLayout;
import android.widget.TableLayout;
import android.widget.Toast;

import cn.cxw.svideostream.R;
import cn.cxw.svideostream.application.GlobalSetting;
import cn.cxw.svideostream.application.GlobalVideoStream;
import cn.cxw.svideostream.application.MainApplication;
import cn.cxw.svideostream.widget.InfoHudViewHolder;
import cn.cxw.svideostream.widget.SurfaceViewPreview;
import cn.cxw.svideostreamlib.CommonSetting;
import cn.cxw.svideostreamlib.GPUImageExFrameSource;
import cn.cxw.svideostreamlib.SVideoStream;
import cn.cxw.svideostreamlib.VideoFrameSource;
import cn.cxw.svideostreamlib.VideoStreamConstants;
import cn.cxw.svideostreamlib.VideoStreamProxy;

/**
 * Created by cxw on 2017/11/9.
 */

public class GPUImageExCameraActivity extends AppCompatActivity implements View.OnClickListener, SVideoStream.IStreamEventObserver, VideoFrameSource.VideoFrameSourceObserver {

    static String TAG = GPUImageExCameraActivity.class.getCanonicalName();
    public  static void Show(Context context)
    {
        Intent intent = new Intent(context, GPUImageExCameraActivity.class);
        context.startActivity(intent);
    }

    private PowerManager.WakeLock mWakeLock;
    CheckBox m_cbRecord = null;
    CheckBox mcbLive = null;
    CheckBox m_cbCameraSwitch = null;
    CheckBox m_cbLight = null;
    SurfaceViewPreview m_svDisplay = null;
    FrameLayout mflCamera = null;
    Button mbtnFilter = null;
    TableLayout mHudView = null;
    InfoHudViewHolder mHudViewHolder = null;

    GPUImageExFrameSource mGPUVideoSource = null;
    VideoStreamProxy mVideoStream = GlobalVideoStream.getGPUImageSourceOwn();
    Handler mHandlerTimer = null;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);
        InitView();
        initCamera();
        acquireWakeLock();

    }
    void InitView()
    {
        mflCamera = (FrameLayout)findViewById(R.id.fl_camera);
        m_cbCameraSwitch = (CheckBox)findViewById(R.id.cb_cameraswitch);
        m_cbLight = (CheckBox)findViewById(R.id.cb_light);
        m_cbRecord = (CheckBox)findViewById(R.id.cb_record);
        m_cbRecord.setEnabled(false);
        m_svDisplay = new SurfaceViewPreview(this);
        mflCamera.addView(m_svDisplay);
        mbtnFilter = (Button)findViewById(R.id.btn_filter);
        mbtnFilter.setVisibility(View.VISIBLE);
        mcbLive = (CheckBox)findViewById(R.id.cb_live);
        mcbLive.setEnabled(false);
        m_cbCameraSwitch.setOnClickListener(this);
        m_cbLight.setOnClickListener(this);
        m_cbRecord.setOnClickListener(this);
        mcbLive.setOnClickListener(this);
        mbtnFilter.setOnClickListener(this);
        mHudView = (TableLayout)findViewById(R.id.tl_info);
        mHudViewHolder = new InfoHudViewHolder(this, mHudView);
    }
    void initCamera()
    {
        mHandlerTimer = new Handler()
        {
            public void handleMessage(Message msg) {
                switch (msg.what)
                {
                    case 0:
                        if (mHudViewHolder != null)
                        {
                            mHudViewHolder.updateInfo(mVideoStream.getStatsReport());
                        }
                        if (GlobalVideoStream.getGPUImageSourceOwn().getState() == VideoStreamConstants.StreamState_STARTED)
                        {
                            sendEmptyMessageDelayed(0, 1000);
                        }
                        else
                        {
                            mHudViewHolder.updateInfo(null);
                        }
                        break;
                }
            };
        };
        CommonSetting.nativeSetLogLevel(VideoStreamConstants.LS_INFO);
        mGPUVideoSource = new GPUImageExFrameSource();
        mGPUVideoSource.setObserver(this);
        mVideoStream.setVideoFrameSource(mGPUVideoSource);
        mVideoStream.setStreamEventObserver(this);
//        Log.d(TAG, MainApplication.getInstance().getSetting().getH264EncoderConfigs().toString());
        mVideoStream.setVideoStreamSetting(MainApplication.getInstance().getSetting());
        mGPUVideoSource.setCameraSize(640, 480);
        mGPUVideoSource.setPreviewView(m_svDisplay);
    }
    int  StartStream(boolean islive)
    {
        if (islive)
        {
            String liveurl = GlobalSetting.getLiveUrl();
            if (!liveurl.isEmpty())
            {
                mVideoStream.setPublishUrl(liveurl);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            mVideoStream.setRecordPath(GlobalSetting.getRecordPath() + "svideostream_camera.mp4");

        }
        return mVideoStream.startStream();
    }
    void StopStream()
    {
        mVideoStream.stopStream();
    }
    @Override
    protected void onResume() {
        super.onResume();
    }
    @Override
    protected void onPause() {
        super.onPause();
    }
    @Override
    protected void onDestroy() {
        releaseWakeLock();
        super.onDestroy();
        mVideoStream.stopStream();
    }
    // 获取电源锁
    protected void acquireWakeLock() {
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            mWakeLock = null;
        }

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, getClass().getName());
        mWakeLock.setReferenceCounted(false);
        mWakeLock.acquire();
    }
    // 释放电源锁
    protected void releaseWakeLock() {
        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            mWakeLock = null;
        }
    }

    @Override
    public void onClick(View view) {
        int vid = view.getId();
        switch (vid)
        {
            case R.id.btn_filter:
                GPUImageFilterTools.showDialog(this, new GPUImageFilterTools.OnBeautyEnableListener() {
                    @Override
                    public void onBeautyEnable(boolean isenable) {
                        if (mGPUVideoSource != null)
                        {
                            mGPUVideoSource.enableBeatyFilter(isenable);
                        }
                    }
                });
            break;
            case R.id.cb_record:
                if (m_cbRecord.isChecked())
                {
                    if (0 != StartStream(false))
                    {
                        m_cbCameraSwitch.setChecked(false);
                        m_cbRecord.setText(R.string.start_record);

                        return ;
                    }
                    m_cbRecord.setText(R.string.stop_record);
                    mcbLive.setEnabled(false);
                }
                else
                {
                    StopStream();
                    m_cbRecord.setText(R.string.start_record);
                    mcbLive.setEnabled(true);

                }
                break;
            case R.id.cb_live:
                if (mcbLive.isChecked())
                {
                    if (0 != StartStream(true))
                    {
                        mcbLive.setChecked(false);
                        mcbLive.setText(R.string.start_live);
                        return ;
                    }
                    m_cbRecord.setEnabled(false);
                    mcbLive.setText(R.string.stop_live);
                }
                else
                {
                    StopStream();
                    mcbLive.setText(R.string.start_live);
                    m_cbRecord.setEnabled(true);
                }
                break;
        }

    }

    @Override
    public void onEvent(final int eventid, final  int error) {
        if (eventid == VideoStreamConstants.SE_StreamStarted)
        {
            mHandlerTimer.sendEmptyMessageDelayed(0, 1000);
        }

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (eventid)
                {
                    case VideoStreamConstants.SE_StreamStarted:

                        Toast.makeText(GPUImageExCameraActivity.this, "stream started", Toast.LENGTH_LONG).show();
                        break;
                    case VideoStreamConstants.SE_LiveConnected:
                        Toast.makeText(GPUImageExCameraActivity.this, "live connected ok", Toast.LENGTH_LONG).show();
                        break;
                    case VideoStreamConstants.SE_StreamFailed:
                        Toast.makeText(getApplicationContext(), VideoStreamConstants.getErrorDes(error), Toast.LENGTH_LONG).show();
                        break;
                }
            }
        });
    }

    @Override
    public void onStarted() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                m_cbRecord.setEnabled(true);
                mcbLive.setEnabled(true);
            }
        });

    }

    static class GPUImageFilterTools {

        interface OnBeautyEnableListener
        {
            void onBeautyEnable(boolean isenable);
        }
        public static void showDialog(final Context context,
                                      final OnBeautyEnableListener listener) {
//            final FilterList filters = new FilterList();

            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder.setTitle("Choose a filter");
            String[] filter = new String[2];
            filter[0] = "Normal";
            filter[1] = "Beauty";
            builder.setItems(filter,
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(final DialogInterface dialog, final int item) {
                            listener.onBeautyEnable((item == 0)?false:true);
                        }
                    });
            builder.create().show();
        }
    }
}
