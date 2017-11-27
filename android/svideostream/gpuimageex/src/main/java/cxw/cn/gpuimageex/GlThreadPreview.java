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
        mThreadName = "GlThreadPreview";
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
        super.stopRender();
        mNeedStartWhenCreate = false;
        mWidth = mHeight = -1;
        mSurface = null;
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
        Log.e(mThreadName, "thread start  check state is " + getState().name());
        super.start();
        if (mWidth >= 0 && mHeight >= 0)
        {
            requestResize(mWidth, mHeight);
        }
    }

    @Override
    public void stopRender() {
        mPreviewView.removeRenderCallback(this);
        mPreviewView = null;
        super.stopRender();
    }
}
