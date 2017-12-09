package cn.cxw.svideostream.activities;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.FrameLayout;
import android.widget.Toast;

import cn.cxw.svideostream.R;
import cn.cxw.svideostream.application.MainApplication;
import cn.cxw.svideostream.utils.PermissionCheck;
import cn.cxw.svideostreamlib.Camera.CameraInfo;
import cn.cxw.svideostreamlib.CameraVideoStream;

/**
 * Created by cxw on 2017/10/18.
 */

public class CameraActivity  extends AppCompatActivity implements View.OnClickListener, SurfaceHolder.Callback {

    CameraVideoStream m_CameraVideoStream = null;

    CheckBox m_cbRecord = null;
    CheckBox mcbLive = null;
    Button m_cbCameraSwitch = null;
    CheckBox m_cbLight = null;
    SurfaceView m_svDisplay = null;
    FrameLayout mflCamera = null;

    public  static void Show(Context context)
    {
        Intent intent = new Intent(context, CameraActivity.class);
        context.startActivity(intent);
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);
        InitView();
        InitCamera();
        acquireWakeLock();

    }

    @Override
    protected void onDestroy() {
        releaseWakeLock();
        super.onDestroy();
    }

    void InitView()
    {
        mflCamera = (FrameLayout)findViewById(R.id.fl_camera);
        m_cbCameraSwitch = (Button)findViewById(R.id.cb_cameraswitch);
        m_cbLight = (CheckBox)findViewById(R.id.cb_light);
        m_cbRecord = (CheckBox)findViewById(R.id.cb_record);
        m_svDisplay = new SurfaceView(this);
        mflCamera.addView(m_svDisplay);

        mcbLive = (CheckBox)findViewById(R.id.cb_live);
        m_cbCameraSwitch.setOnClickListener(this);
        m_cbLight.setOnClickListener(this);
        m_cbRecord.setOnClickListener(this);
        mcbLive.setOnClickListener(this);


    }
    private PowerManager.WakeLock mWakeLock;

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
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if(permissions[0].compareTo(Manifest.permission.CAMERA) == 0)
        {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED)
            {
                SurfaceHolder holder = m_svDisplay.getHolder();
                holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
                holder.addCallback(this);
                startPreview(holder);
            }
            else
            {
                Toast.makeText(this, "需要摄像头权限", Toast.LENGTH_LONG).show();
            }
            PermissionCheck.Check(this, Manifest.permission.RECORD_AUDIO, PermissionCheck.MY_PERMISSIONS_REQUEST_OK);
        }
        if (requestCode != PermissionCheck.MY_PERMISSIONS_REQUEST_OK)
        {
             super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }
    void InitCamera()
    {


        m_CameraVideoStream = new CameraVideoStream();
        m_CameraVideoStream.setVideoStreamSetting(MainApplication.getInstance().getSetting());
        int percheck = PermissionCheck.Check(this, Manifest.permission.CAMERA, PermissionCheck.MY_PERMISSIONS_REQUEST_OK);
       if(percheck == PermissionCheck.AUTHPERMISSION)
       {
           SurfaceHolder holder = m_svDisplay.getHolder();
           holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
           holder.addCallback(this);
           startPreview(holder);
       }
        else if (percheck == PermissionCheck.REQPERMISSION)
       {
           Toast.makeText(this, "请求权限中...", Toast.LENGTH_LONG).show();
       }
        else if (percheck == PermissionCheck.USEREJECT_NOTIPS)
       {
           Toast.makeText(this, "用户已禁止授权，需要用户手动开户权限。", Toast.LENGTH_LONG).show();

       }
//      似乎不能连接调用。
//        PermissionCheck.Check(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
//        PermissionCheck.Check(this, Manifest.permission.READ_EXTERNAL_STORAGE);
//        if()
//        {
//
//        }
//        else
//        {
//            Toast.makeText(this, "未获取到录音权限", Toast.LENGTH_LONG).show();
//        }

    }

    int  StartStream(boolean islive)
    {
        if( PermissionCheck.Check(this, Manifest.permission.READ_EXTERNAL_STORAGE, PermissionCheck.MY_PERMISSIONS_REQUEST_OK) == PermissionCheck.AUTHPERMISSION && PermissionCheck.Check(this, Manifest.permission.WRITE_EXTERNAL_STORAGE, PermissionCheck.MY_PERMISSIONS_REQUEST_OK) == PermissionCheck.AUTHPERMISSION)
        {
            m_CameraVideoStream.setStreamSize(480, 640);
            if (islive)
            {
                String liveurl = "";
                if (!liveurl.isEmpty())
                {
                    m_CameraVideoStream.setPublishUrl(liveurl);
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                m_CameraVideoStream.setRecordPath(Environment.getExternalStorageDirectory() + "/svideostream.mp4");

            }
            return m_CameraVideoStream.startStream();
        }
        else
        {
            Toast.makeText(this, "无写文件权限",Toast.LENGTH_LONG).show();
        }
        return -1;
    }
    void startPreview(final  SurfaceHolder lholder)
    {
        new Thread(new Runnable() {
            @Override
            public void run() {
                int cameratype = 0;
                if (m_CameraVideoStream.hadFrontCamera())
                {
                    cameratype = Camera.CameraInfo.CAMERA_FACING_FRONT;
                }
                else if (m_CameraVideoStream.hadBackCamera())
                {
                    cameratype = Camera.CameraInfo.CAMERA_FACING_BACK;
                }
                else
                {
                    return ;
                }
                m_CameraVideoStream.startPreview(lholder, cameratype, CameraInfo.CAMERA_PORTRAIT, new CameraVideoStream.CameraSize(640, 480));
            }
        }).start();
    }
    void StopStream()
    {
        m_CameraVideoStream.stopStream();
    }
    @Override
    public void onClick(View v) {
            int id = v.getId();
        switch (id)
        {
            case R.id.cb_cameraswitch:

//                m_CameraVideoStream.switchCamera(m_cbCameraSwitch.isChecked()? Camera.CameraInfo.CAMERA_FACING_BACK: Camera.CameraInfo.CAMERA_FACING_FRONT);
//                m_cbCameraSwitch.setText(m_cbCameraSwitch.isChecked()?"后":"前");
                break;
            case R.id.cb_light:
                break;
            case R.id.cb_record:
                if (m_cbRecord.isChecked())
                {
                    if (0 != StartStream(false))
                    {
//                        m_cbCameraSwitch.setChecked(false);
                        m_cbRecord.setText("开始录制");
                    }
                    m_cbRecord.setText("停止录制");
                }
                else
                {
                    StopStream();
                    m_cbRecord.setText("开始录制");
                }
                break;
            case R.id.cb_live:
                if (mcbLive.isChecked())
                {
                    if (0 != StartStream(true))
                    {
                        mcbLive.setChecked(false);
                        mcbLive.setText("开始直播");
                    }
                    mcbLive.setText("停止直播");
                }
                else
                {
                    StopStream();
                    mcbLive.setText("开始直播");
                }
                break;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        final SurfaceHolder lholder = holder;


    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        m_CameraVideoStream.stopPreview();
    }
}
