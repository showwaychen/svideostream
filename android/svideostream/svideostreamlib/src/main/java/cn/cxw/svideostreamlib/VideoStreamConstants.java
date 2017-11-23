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

    //stream state
    public static final  int StreamState_NONE = 0;
    public static final  int StreamState_STARTING =1;
    public static final  int StreamState_STARTED  = 2;
    public static final  int StreamState_STOPPING = 3;
    public static final  int StreamState_STOPED = 4;

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

    public static String getErrorDes(int errornum)
    {
        switch (errornum)
        {
            case kSE_AudioeEncoderOpenedFailed:
                return "failed to open audio encoder ";
            case kSE_LiveConnectFailed:
                return "failed to connect living server failed";
            case kSE_RecordOpenFileFailed:
                return "failed to open file for recording";
            case kSE_VideoEncoderOpenedFailed:
                return "failed to open video encoder";
            case kSE_UnknowError:
                return "unkonw error";
        }
        return "no error";
    }

    //set key-value
    public static final int SKV_H264ENCODERCONFIG = 0;



    //h264 encoder config key
    public static final  String KEY_PROFILE = "profile";
           public static final String VALUE_BASELINE = "baseline";

    public static final  String KEY_PRESET = "preset";
        public static final String VALUE_superfast = "superfast";

    public static final String KEY_TUNE = "tune";
         public static final String VALUE_zerolatency = "zerolatency";

    public static final String KEY_RC_METHOD = "rc_method";
        public  static final String VALUE_rc_abr = "RC_ABR";
        public  static final String VALUE_rc_cqp = "RC_CQP";
        public  static final String VALUE_rc_crf = "RC_CRF";

}
