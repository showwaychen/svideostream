package cxw.cn.gpuimageex;

import android.support.annotation.NonNull;
import android.util.Log;

/**
 * Created by user on 2017/11/15.
 */

public class GlThreadPreview extends GlRenderThread implements IPreviewView.IPreviewCallback {

    IPreviewView mPreviewView = null;
    int mWidth = -1;
    int mHeight = -1;

    boolean mNeedStartWhenCreate = false;

    public GlThreadPreview(IPreviewView pview)
    {
        mPreviewView = pview;
        mPreviewView.addRenderCallback(this);
    }

    @Override
    public void onSurfaceCreated(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {
        setSurface(holder.getSurface());
        if (mNeedStartWhenCreate)
        {
            start();
        }
    }

    @Override
    public void onSurfaceChanged(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {
        mWidth = width;
        mHeight = height;
        requestResize(mWidth, mHeight);
    }

    @Override
    public void onSurfaceDestroyed(@NonNull IPreviewView.ISurfaceHolder holder) {
        stopRender();
        mWidth = mHeight = -1;
    }

    @Override
    public synchronized void start() {
        if (mSurface == null)
        {
            Log.w(TAG,"surface hasn't created");
            mNeedStartWhenCreate = true;
            return ;
        }
        mNeedStartWhenCreate = false;
        super.start();
        if (mWidth >= 0 && mHeight >= 0)
        {
            requestResize(mWidth, mHeight);
        }
    }
}