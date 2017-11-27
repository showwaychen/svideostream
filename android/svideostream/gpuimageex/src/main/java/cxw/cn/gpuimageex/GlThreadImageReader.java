package cxw.cn.gpuimageex;

import android.graphics.PixelFormat;
import android.media.ImageReader;
import android.os.Build;
import android.util.Log;

/**
 * Created by cxw on 2017/11/14.
 */

public class GlThreadImageReader extends GlRenderThread{
    ImageReader mImageReader = null;
    int mWidth;
    int mHeight;
    public GlThreadImageReader(int width, int height, GlRenderThread.GLRenderer renderer)
    {
        mWidth = width;
        mHeight = height;
        mThreadName = "GlThreadImageReader";
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            mImageReader = ImageReader.newInstance(width, height, PixelFormat.RGBA_8888, 2);
            setSurface(mImageReader.getSurface());
            setRender(renderer);

        }
    }

    public void setImageAvailableListener(ImageReader.OnImageAvailableListener listener)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            if (mImageReader != null)
            {
                mImageReader.setOnImageAvailableListener(listener, null);
            }
        }
    }
    public void release()
    {
        Log.d(mThreadName, "gl thread stop");
        stopRender();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            if (mImageReader != null)
            {
                mImageReader.setOnImageAvailableListener(null, null);
                mImageReader.close();
                mImageReader = null;
            }
        }

    }

    @Override
    public synchronized void start() {
        Log.d(mThreadName, "gl thread start");
        super.start();
        requestResize(mWidth, mHeight);
    }
}
