package cxw.cn.gpuimageex;

import android.opengl.GLES20;
import android.util.Log;

import java.nio.ByteBuffer;

/**
 * Created by user on 2017/12/5.
 */

public class ReadPixelCapture extends OpenglCapture{
    public  static String TAG = ReadPixelCapture.class.getCanonicalName();
    @Override
    public boolean initCapture(int width, int height) {
        mWidth = width;
        mHeight = height;
        if (mWidth == 0 || mHeight == 0)
        {
            mbInitOk = false;
            return false;
        }
        mCaptureBuffer = ByteBuffer.allocateDirect(width * height * 4);
        mbInitOk = true;
        return true;
    }

    @Override
    public void onCapture() {
        if (!mbInitOk)
        {
            Log.w(TAG, "mbinitOk = false");
            return ;
        }
        if (mCallback != null)
        {
            long ptime = System.currentTimeMillis();
            //???????
            GLES20.glReadPixels(0, 0,
                    mWidth, mHeight,
                    GLES20.GL_RGBA,
                    GLES20.GL_UNSIGNED_BYTE,
                    mCaptureBuffer);
//            Log.d(TAG, "glReadPixels time = " + (System.currentTimeMillis() - ptime));
            mCallback.onPreviewFrame(mCaptureBuffer, mWidth * 4, mHeight, ptime);
        }

    }

    @Override
    public void destroy() {
        mbInitOk = false;
        mCaptureBuffer = null;
    }
}
