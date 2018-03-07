package cn.cxw.screenlive;

import android.graphics.PixelFormat;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.Image;
import android.media.ImageReader;
import android.media.projection.MediaProjection;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.RequiresApi;
import android.util.Log;

import java.nio.ByteBuffer;

import cn.cxw.svideostreamlib.VideoFrameSource;
import cn.cxw.svideostreamlib.VideoStreamConstants;

/**
 * Created by cxw on 2017/11/12.
 */

public class ScreenVideoFrameSource extends VideoFrameSource {
    public static String TAG = ScreenVideoFrameSource.class.getCanonicalName();
    private MediaProjection mMediaProjection = null;
    ImageReader mImageReader = null;
    private VirtualDisplay mVirtualDisplay = null;
    private Handler mHandler;
    private static final int VIRTUAL_DISPLAY_FLAGS = DisplayManager.VIRTUAL_DISPLAY_FLAG_OWN_CONTENT_ONLY | DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC;


    public ScreenVideoFrameSource()
    {
        mHandler = new Handler();
        mSrcFormate = VideoStreamConstants.IMAGE_FORMAT_ABGR;
        mRotation = VideoStreamConstants.kRotate0;
    }
    public void setSize(int width, int height)
    {
        mSrcWidth = width;
        mSrcHeight = height;
    }
    public void setupMediaProjection(MediaProjection mediaProjection)
    {
        mMediaProjection = mediaProjection;
    }
    boolean createVirtualDisplay()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                mImageReader = ImageReader.newInstance(mSrcWidth, mSrcHeight, PixelFormat.RGBA_8888, 2);
            mVirtualDisplay = mMediaProjection.createVirtualDisplay("SVideoStream", mSrcWidth, mSrcHeight, 1, VIRTUAL_DISPLAY_FLAGS, mImageReader.getSurface(), null, mHandler);
            mImageReader.setOnImageAvailableListener(new ImageAvailableListener(), mHandler);
            return true;
        }
        Log.d(TAG, "cann't newInstance for Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT");
        return false;
    }
    @Override
    public boolean isStarted() {
       return mSrcStride > 0;
    }

    public boolean startScreenCapture()
    {
        if (mState == State.kStarted || mState == State.kStartting)
        {
            Log.d(TAG, "startScreenCapture has started");
            return true;
        }
        mSrcStride = 0;
        if (mMediaProjection == null || mSrcHeight == 0 || mSrcWidth == 0)
        {
            return false;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
        {
            boolean ret = createVirtualDisplay();
            mState = ret?State.kStartting:State.kNONE;
            return ret;
        }
        return false;
    }
    public void stop()
    {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            if(mVirtualDisplay != null)
            {
                mVirtualDisplay.release();
            }
            if (mImageReader != null)
            {
                mImageReader.setOnImageAvailableListener(null, null);
                mImageReader.close();
            }

        }
        mState = State.kStopped;

    }
    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    private class ImageAvailableListener implements ImageReader.OnImageAvailableListener {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = null;

            try {
                image = mImageReader.acquireLatestImage();
                if (image != null) {
                    Image.Plane[] planes = image.getPlanes();
                    ByteBuffer buffer = planes[0].getBuffer();

                    //we can not get widthStride before get frame, so here we set it
                    if (mSrcStride == 0) {
                        mSrcStride = planes[0].getRowStride();

                            mState = State.kStarted;
                            NotifyObserver();
                        Log.d(TAG, "widthStride"+mSrcStride+"width:"+mSrcWidth+"heigth:"+mSrcHeight);
                    }
//                    Log.d(TAG, "Stride = " + mSrcStride + " width = " + mSrcWidth + "height = " + mSrcHeight + " bufferisize" + buffer.remaining());
                    if (mFrameCallback != null)
                    {
                        mFrameCallback.onVideoFrameComing(buffer, mSrcStride, mSrcHeight);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (image!=null) {
                    image.close();
                }
            }
        }
    }
}
