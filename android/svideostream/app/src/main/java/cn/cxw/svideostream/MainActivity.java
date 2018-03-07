package cn.cxw.svideostream;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import cn.cxw.androidcomutils.PermissionCheck;
import cn.cxw.svideostream.activities.GPUImageExCameraActivity;
import cn.cxw.svideostream.activities.ScreenCaptureActivity;
import cn.cxw.svideostream.activities.SettingActivity;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        InitView();
        int writePermission = PermissionCheck.Check(this, Manifest.permission.WRITE_EXTERNAL_STORAGE, PermissionCheck.MY_PERMISSIONS_REQUEST_OK);
        Log.d("test", "" + writePermission);
        if(PermissionCheck.USEREJECT_NOTIPS == writePermission)
        {
            Toast.makeText(this, "未获取写文件权限。", Toast.LENGTH_SHORT);
        }
    }
    void InitView()
    {
        findViewById(R.id.tv_camera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (PermissionCheck.Check(MainActivity.this, Manifest.permission.CAMERA, PermissionCheck.MY_PERMISSIONS_REQUEST_OK) == PermissionCheck.AUTHPERMISSION)
                {
                    GPUImageExCameraActivity.Show(MainActivity.this);

//                    startActivity(v.getId());
                }
                else
                {
                    Toast.makeText(MainActivity.this, "no camera permission", Toast.LENGTH_SHORT).show();
                }
//                CameraActivity.Show(MainActivity.this);
//                                SettingActivity.showActivity(MainActivity.this);

            }
        });
        findViewById(R.id.tv_setting).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                 Toast.makeText(MainActivity.this, "" + (new Setting(MainActivity.this.getApplicationContext())).getAudioEnable(), Toast.LENGTH_LONG).show();
                SettingActivity.showActivity(MainActivity.this);
            }
        });
        findViewById(R.id.tv_screen).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                ScreenCaptureActivity.Show(MainActivity.this);
            }
        });
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if(permissions[0].compareTo(Manifest.permission.CAMERA) == 0)
        {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED)
            {
                GPUImageExCameraActivity.Show(MainActivity.this);
//                startActivity(requestCode);
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
    private long mPreClickTime;
    public static void AppExit(Context context) {
        try {
            //退出程序
            android.os.Process.killProcess(android.os.Process.myPid());
            System.exit(1);
        } catch (Exception e) {
        }
    }
    void startActivity(int viewid)
    {
        switch (viewid)
        {
            case R.id.tv_camera:
                GPUImageExCameraActivity.Show(this);
                break;
        }
    }
    @Override
    public void onBackPressed() {
        // 拿着当前的时间戳--记录的时间搓 如何间隔2s以上,我就弹出toast提示
        if (System.currentTimeMillis() - mPreClickTime > 2000) {// 两次点击的时间间隔>2s
            Toast.makeText(getApplicationContext(), "再按一次,退出应用", Toast.LENGTH_SHORT).show();
            mPreClickTime = System.currentTimeMillis();
            return;
        } else {
            // 两次点击的时间间隔<2s
            // 完成退出
           AppExit(getBaseContext());
        }
    }
}
