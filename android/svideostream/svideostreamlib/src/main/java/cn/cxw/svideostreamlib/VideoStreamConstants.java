package cn.cxw.svideostreamlib;

/**
 * Created by user on 2017/10/13.
 */

public class VideoStreamConstants {
    public static final  String VersionCode = "1.4";
    //stream type
    public  static int ST_LIVE = 1;
    public static int ST_RECORD = 2;

    //source data type
    public  static int SDT_IMAGEDATA = 0;
    public static int SDT_H264DATA = 1;

    //rotation scala
    public static int kRotate0 = 0;
    public static int kRotate90 = 90;
    public static int kRotate180 = 180;
    public static int kRotate270 = 270;


    //image format
    public static int IMAGE_FORMAT_NONE = 0;
    public static int IMAGE_FORMAT_NV12 = 1;
    public static int IMAGE_FORMAT_NV21 = 2;
    public static int IMAGE_FORMAT_ARGB = 3;
    public static int IMAGE_FORMAT_RGBA = 4;
    public static int IMAGE_FORMAT_ABGR = 5;
    public static int IMAGE_FORMAT_BGRA = 6;
    public static int IMAGE_FORMAT_I420 = 7;


    //video encode type
    public static int H264ENCODER_X264 = 0;
    public static int H264ENCODER_MEDIACODEC = 1;

    //log level
    public  static final int LS_SENSITIVE = 0;
    public  static final int LS_VERBOSE = 1;
    public  static final int LS_INFO = 2;
    public  static final int LS_WARNING = 3;
    public  static final int LS_ERROR = 4;
    public  static final int LS_NONE = 5;


    //stream event
    public  static final int SE_LiveConnected =0;
    public  static final int SE_LiveDisconnected = 1;
    public  static final int SE_RecordStartedSuccess = 2;
    public  static final int SE_StreamWarning = 3;
    public  static final int SE_StreamFailed = 4;
    public  static final int SE_StreamStarted = 5;
    //stream event error
    public  static final int kSE_NoneError = 0;
    public  static final int kSE_UnknowError = 1;
    public  static final int kSE_VideoEncoderOpenedFailed = 2;
    public  static final int kSE_AudioeEncoderOpenedFailed = 3;
    public  static final int kSE_LiveConnectFailed = 4;
    public  static final int kSE_RecordOpenFileFailed = 5;
}
