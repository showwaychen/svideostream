package cn.cxw.svideostream.utils;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

/**
 * Created by user on 2017/10/19.
 */

public class PermissionCheck {
    //已授权.
    public  static int AUTHPERMISSION = 0;
    public  static int USEREJECT_NOTIPS = 1;
    public  static int REQPERMISSION = 2;

    public static final int MY_PERMISSIONS_REQUEST_OK = 1;

    public static int Check(Activity context, String permission)
    {
        if (ContextCompat.checkSelfPermission(context,
                permission)
                != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(context,
                    permission)) {
                    return USEREJECT_NOTIPS;
            } else {
                ActivityCompat.requestPermissions(context,
                        new String[]{permission},
                        MY_PERMISSIONS_REQUEST_OK);
                return REQPERMISSION;
            }
        }
        return AUTHPERMISSION;
    }
}
