package cxw.cn.gpuimageex;

import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Created by cxw on 2017/12/3.
 */

public class GraphicBufferCapture extends OpenglCapture{
    static boolean  sbLoadLib = false;
    public  static String TAG = GraphicBufferCapture.class.getCanonicalName();
    long mNativeInstance = 0;
    native long native_newInstance();
    native boolean native_initCapture(int width, int height);
    native void native_deinitCapture();
    native void native_captureFrame(int textureid);
    native void native_setDirectBuffer(ByteBuffer bufferdata);
    native void native_destroy();

    ByteBuffer mByteBufferData = null;
    public GraphicBufferCapture()
    {
        if (!sbLoadLib)
        {
            System.loadLibrary("openglcapture");
            sbLoadLib = true;
        }
    }

    void initDirectBuffer(int size)
    {
        Log.d(TAG, "init direct buffer size = " + size);
        mByteBufferData = ByteBuffer.allocateDirect(size);
        native_setDirectBuffer(mByteBufferData);

    }
    @Override
    public boolean initCapture(int width, int height) {
        mWidth = width;
        mHeight = height;
        mNativeInstance = native_newInstance();
        mbInitOk = native_initCapture(width, height);
       return mbInitOk;
    }

    @Override
    public void onCapture() {
        if (!mbInitOk)
        {
            Log.w(TAG, "hasn't been inited");
            return ;
        }
        synchronized (this)
        {
            native_captureFrame(mTextureId);
        }
    }

    @Override
    public void destroy() {
        synchronized (this)
        {
            native_deinitCapture();
            native_destroy();
            mNativeInstance = 0;
            mbInitOk = false;
        }

    }

    void nativeCallback(int stride, long pts)
    {
//        Log.d(TAG, "stride = " + stride);
        if (mCallback != null)
        {
            mCallback.onPreviewFrame(mByteBufferData, stride, mHeight, pts);
        }
    }
}
