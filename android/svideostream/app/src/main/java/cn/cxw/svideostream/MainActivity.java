package cn.cxw.svideostream;

import android.Manifest;
import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import cn.cxw.svideostream.activities.CameraActivity;
import cn.cxw.svideostream.activities.SettingActivity;
import cn.cxw.svideostream.application.Setting;
import cn.cxw.svideostream.utils.PermissionCheck;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        InitView();
        PermissionCheck.Check(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);
    }
    void InitView()
    {
        findViewById(R.id.tv_camera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                CameraActivity.Show(MainActivity.this);
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
