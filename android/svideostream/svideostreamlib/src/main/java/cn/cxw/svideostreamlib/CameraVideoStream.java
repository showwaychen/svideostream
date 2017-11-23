package cn.cxw.svideostreamlib;

import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;

import cn.cxw.svideostreamlib.Camera.CameraInfo;

/**
 * Created by cxw on 2017/10/17.
 */

public class CameraVideoStream implements Camera.PreviewCallback{
    public static String TAG = CameraVideoStream.class.getCanonicalName();
    public static class  CameraSize {
        public int width;
        public int height;

        public CameraSize() {
            width = 0;
            height = 0;
        }

        public CameraSize(int width, int height) {
            this.width = width;
            this.height = height;
        }
    }
    private CameraSize cameraSize;
    private SurfaceHolder holder;
    private int cameraType;
    private int cameraOrientation;
    private boolean isPreview;
    private boolean isPause;
    private Camera camera;
    private byte [] cameraBuffer1;
    private byte [] cameraBuffer2;

    private int mCameraFacing;
    private int mCameraId;
    SVideoStream mSVideoStream = null;
    VideoStreamSetting mSetting = new VideoStreamSetting();
    private static final int NO_CAMERA = -1;
    public CameraVideoStream()
    {
        mSVideoStream = new SVideoStream();
    }
    public int getCameraId(int cameraFacing) {
        int cameraId = NO_CAMERA;
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; ++i) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == cameraFacing) {
                cameraId = i;
                Log.i(TAG, "find camera of facing = " + cameraFacing + " with id = " + cameraId);
                break;
            }
        }
        if (cameraId == NO_CAMERA) {
            Log.e(TAG, "cannovld3_u8 t find camera of facing = " + cameraFacing);
        }
        return cameraId;
    }
    private boolean hadCamera(int cameraFacing) {
        return getCameraId(cameraFacing) != NO_CAMERA;
    }
    //camera
    public static boolean checkAuthorization() {
        try {
            Camera camera = Camera.open(0);
            if (camera == null) {
                return false;
            }
            camera.release();
            camera = null;
        } catch (Exception e) {
            Log.e(TAG, "checkAuthorization err :" + e.getMessage());
            return false;
        }
        return true;
    }
    @Override
    public void onPreviewFrame(byte[] bytes, Camera camera) {
//            if (mSVideoStream != null)
//            {
//                mSVideoStream.InpputVideoData(bytes);
//            }
        camera.addCallbackBuffer(bytes);
    }
    public boolean isPreview() {
        return isPreview;
    }


    //videostream
    private boolean startPreview() {
        if (isPreview())
        {
            return true;
        }
        Log.i(TAG, "VideoRecorder startPreview.");
        CameraInfo info = CameraInfo.getInstance();
        if (!info.supportCamera(cameraType)) {
            Log.e(TAG, "Not support cameraType = " + cameraType);
            return false;
        }
        try {
            camera = Camera.open(info.getCameraID(cameraType));
            Camera.Parameters params = camera.getParameters();
            params.setPreviewSize(cameraSize.width, cameraSize.height);
            params.setPreviewFormat(ImageFormat.NV21); // default
            if (params.getSupportedFocusModes().contains(
                    Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            }
            camera.setParameters(params);
            if (cameraOrientation == CameraInfo.CAMERA_PORTRAIT) {
                camera.setDisplayOrientation(90);
            }
            camera.setPreviewCallbackWithBuffer(this);
            cameraBuffer1 = new byte[cameraSize.width * cameraSize.height * 3 / 2];
            cameraBuffer2 = new byte[cameraSize.width * cameraSize.height * 3 / 2];
            camera.addCallbackBuffer(cameraBuffer1);
            camera.addCallbackBuffer(cameraBuffer2);
            camera.setPreviewDisplay(holder);
            camera.startPreview();
            camera.cancelAutoFocus();
            isPreview = true;
        } catch (Exception e) {
            Log.e(TAG, "VideoRecorder start err :" + e.getMessage());
            isPreview = false;
            return false;
        }
        return true;
    }

    public void stopPreview() {
        Log.i(TAG, "VideoRecorder stopPreview.");
        if (camera != null) {
            camera.stopPreview();
            camera.setPreviewCallback(null);
            camera.release();
            camera = null;
            cameraBuffer1 = null;
            cameraBuffer2 = null;
        }
        isPreview = false;
    }

    private boolean restartPreview() {
        stopPreview();
        return startPreview();
    }

    public boolean startPreview(SurfaceHolder holder, int cameraType, int cameraOrientation, CameraSize cameraSize) {
        this.cameraType = cameraType;
        this.cameraOrientation = cameraOrientation;
        this.cameraSize = cameraSize;
        this.holder = holder;
        return restartPreview();
    }

    public void stop() {
        Log.i(TAG, "VideoRecorder stop.");
//        isRecord = false;
        isPause = false;
        stopStream();
    }

    public void pause() {
        Log.i(TAG, "VideoRecorder pause.");
        isPause = true;
//        if (pauseTime == 0) {
//            pauseTime = System.currentTimeMillis();
//        }
    }

    public void resume() {
        Log.i(TAG, "VideoRecorder resume.");
//        if (pauseTime != 0) {
//            pauseSumTime += System.currentTimeMillis() - pauseTime;
//            pauseTime = 0;
//        }
        isPause = false;
    }

    public boolean isFrontCamera()
    {
        return cameraType == Camera.CameraInfo.CAMERA_FACING_FRONT;
    }

    // 当前工作的是否是后置摄像头
    public boolean isBackCamera() {
        return cameraType == Camera.CameraInfo.CAMERA_FACING_BACK;
    }

    // 该设备是否拥有前置摄像头
    public boolean hadFrontCamera() {
        return hadCamera(Camera.CameraInfo.CAMERA_FACING_FRONT);
    }
    public boolean hadBackCamera() {
        return hadCamera(Camera.CameraInfo.CAMERA_FACING_BACK);
    }
    public void switchCamera(int type) {
        Log.i(TAG, "switchCamera");
        if (type != Camera.CameraInfo.CAMERA_FACING_FRONT || type !=  Camera.CameraInfo.CAMERA_FACING_BACK )
        {
            return ;
        }
        if (cameraType != type)
        {
            cameraType =  type;
            restartPreview();
        }
    }
    //stream
    int mStreamWidth = 0;
    int mStreamHeight = 0;
    public void setStreamSize(int width, int height)
    {
        mStreamHeight = height;
        mStreamWidth = width;
        mSVideoStream.SetDstSize(mStreamWidth, mStreamHeight);
    }
    public void setRecordPath(String path)
    {
        mSVideoStream.SetStreamType(VideoStreamConstants.ST_RECORD);
        mSVideoStream.SetRecordFilename(path);
    }
    public void setPublishUrl(String url)
    {
        mSVideoStream.SetStreamType(VideoStreamConstants.ST_LIVE);
        mSVideoStream.SetPublishUrl(url);
    }
    public void setVideoStreamSetting(VideoStreamSetting setting)
    {
        mSetting = setting;
    }
    public int startStream()
    {
        if (!isPreview())
        {
            Log.d(TAG, "camera is not in previewing");
            return -1;
        }
        int ret = 0;
        mSVideoStream.SetSrcImageParams(VideoStreamConstants.IMAGE_FORMAT_NV21, cameraSize.width, cameraSize.width, cameraSize.height);
        mSVideoStream.SetRotation((cameraOrientation == CameraInfo.CAMERA_PORTRAIT)?VideoStreamConstants.kRotate90:VideoStreamConstants.kRotate0);
        mSVideoStream.SetAudioEnable(mSetting.getAudioEnable());
        mSVideoStream.setVideoEncoderType(mSetting.getVideoEncoderType());

        mSVideoStream.SetVideoEncodeParams(mSetting.getVideoBitrate(), mSetting.getVideoFps());
        ret = mSVideoStream.StartStream();
        return ret;
    }
    public int stopStream()
    {
        return mSVideoStream.StopStream();
    }
    }
