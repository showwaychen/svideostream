package cn.cxw.svideostream.activities;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import cn.cxw.svideostream.R;
import cn.cxw.svideostream.application.GlobalVideoStream;
import cn.cxw.svideostream.application.StreamService;
import cn.cxw.svideostream.utils.PermissionCheck;
import cn.cxw.svideostreamlib.CommonSetting;
import cn.cxw.svideostreamlib.ScreenVideoFrameSource;
import cn.cxw.svideostreamlib.VideoStreamConstants;
import cn.cxw.svideostreamlib.VideoStreamProxy;

/**
 * Created by cxw on 2017/11/12.
 */

public class ScreenCaptureActivity extends AppCompatActivity implements View.OnClickListener, ScreenVideoFrameSource.VideoFrameSourceObserver {
    static public  String TAG = ScreenCaptureActivity.class.getCanonicalName();
    ScreenVideoFrameSource mScreenVideoFrameSource = null;
    private MediaProjectionManager mProjectionManager;
    private MediaProjection mMediaProjection;
    VideoStreamProxy mVideoStream = GlobalVideoStream.getScreenCaptureOwn();
    private static final int PERMISSION_CODE = 100;//1;
    public  static void Show(Context context)
    {
        Intent intent = new Intent(context, ScreenCaptureActivity.class);
        context.startActivity(intent);
    }
    EditText metWidth;
    EditText metHeight;
    Button mbtnScreenRecord;
    Button mbtnScreenLive;

    boolean misLive = false;

    private static int nVersion;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_screen);
        PermissionCheck.Check(this, Manifest.permission.RECORD_AUDIO, PermissionCheck.MY_PERMISSIONS_REQUEST_OK);
        Intent startIntent = new Intent(this, StreamService.class);
        startService(startIntent);
        initView();


    }
    void initView()
    {
//        CommonSetting.nativeSetLogLevel(VideoStreamConstants.LS_WARNING);
        mScreenVideoFrameSource = new ScreenVideoFrameSource();
        mScreenVideoFrameSource.setObserver(this);
        mVideoStream.setVideoFrameSource(mScreenVideoFrameSource);
        metWidth = (EditText)findViewById(R.id.et_width);
        metHeight = (EditText)findViewById(R.id.et_height);
        mbtnScreenRecord = (Button) findViewById(R.id.btn_screenstart);
        mbtnScreenRecord.setOnClickListener(this);
        findViewById(R.id.btn_screenstop).setOnClickListener(this);
        mbtnScreenLive = (Button) findViewById(R.id.btn_screenstartlive);
        mbtnScreenLive.setOnClickListener(this);

        nVersion = Build.VERSION.SDK_INT;

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                mProjectionManager = (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
                startActivityForResult(mProjectionManager.createScreenCaptureIntent(), PERMISSION_CODE);
        }
    }
    @Override
    public void onDestroy() {
        super.onDestroy();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                if (mMediaProjection != null) {
                    mMediaProjection.stop();
                    mMediaProjection = null;
                }
        }
        Intent stopIntent = new Intent(this, StreamService.class);
        stopService(stopIntent);
    }
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != PERMISSION_CODE) {
            Log.e(TAG, "Unknown request code: " + requestCode);
            return;
        }
        if (resultCode != RESULT_OK) {
            Toast.makeText(this, "User denied screen sharing permission", Toast.LENGTH_SHORT).show();
            return;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mMediaProjection = mProjectionManager.getMediaProjection(resultCode, data);
        }
    }
    boolean startCapture(int width, int height)
    {
        if (width == 0 || height == 0)
        {
            return false;
        }
        mScreenVideoFrameSource.setSize(width, height);
        mScreenVideoFrameSource.setupMediaProjection(mMediaProjection);
        return mScreenVideoFrameSource.startScreenCapture();
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.btn_screenstart)
        {
            if (startCapture(Integer.valueOf("" + metWidth.getText()), Integer.valueOf("" + metHeight.getText())))
            {
                mbtnScreenLive.setEnabled(false);
                mbtnScreenRecord.setEnabled(false);
                misLive = false;
                Toast.makeText(this,  "" + metWidth.getText() + "   " + metHeight.getText(), Toast.LENGTH_LONG).show();
            }
            else
            {
                Toast.makeText(this, "failed to start Capture ", Toast.LENGTH_LONG).show();
            }



        }
        else if (view.getId() == R.id.btn_screenstartlive)
        {
            if (startCapture(Integer.valueOf("" + metWidth.getText()), Integer.valueOf("" + metHeight.getText())))
            {
                mbtnScreenLive.setEnabled(false);
                mbtnScreenRecord.setEnabled(false);
                misLive = true;
                Toast.makeText(this,  "" + metWidth.getText() + "   " + metHeight.getText(), Toast.LENGTH_LONG).show();
            }
            else
            {
                Toast.makeText(this, "failed to start Capture ", Toast.LENGTH_LONG).show();
            }
        }
        else if (view.getId() == R.id.btn_screenstop)
        {

            if (mScreenVideoFrameSource != null)
            {
                mScreenVideoFrameSource.stop();
                mScreenVideoFrameSource = null;
            }
            StreamService.stopRecording(this);
            mbtnScreenLive.setEnabled(true);
            mbtnScreenRecord.setEnabled(true);
        }
    }

    @Override
    public void onStarted() {
//        startStream(misLive);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (misLive)
                {
                    StreamService.readyToLive(ScreenCaptureActivity.this);
                }
                else
                {
                    StreamService.readyToRecord(ScreenCaptureActivity.this);
                }
                Intent MyIntent = new Intent(Intent.ACTION_MAIN);
                MyIntent.addCategory(Intent.CATEGORY_HOME);
                startActivity(MyIntent);
            }
        });

    }
}
