package cn.cxw.svideostreamlib;

/**
 * Created by cxw on 2017/11/12.
 */

public abstract class VideoFrameSource {
    public interface OnVideoFrameComing
    {
        void onVideoFrameComing(byte[] framedata, int stride, int height);
    }
    protected int mSrcWidth = 0;
    protected int mSrcStride = 0;
    protected int mSrcHeight = 0;
    protected int mSrcFormate = 0;
    protected int mRotation = 0;

    protected OnVideoFrameComing mFrameCallback = null;
<<<<<<< HEAD
    public interface VideoFrameSourceObserver
    {
        void onStarted();
    }
    VideoFrameSourceObserver mObserver = null;
    public void setObserver(VideoFrameSourceObserver cb)
    {
        mObserver = cb;
    }
=======

>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    public int getSrcWidth()
    {
        return mSrcWidth;
    }
    public  int getSrcStride()
    {
        return mSrcStride;
    }
    public  int getSrcHeight()
    {
        return mSrcHeight;
    }
    public  int getSrcFormate()
    {
        return mSrcFormate;
    }
    public  int getRotiation()
    {
        return mRotation;
    }
    public void setFrameCallback(OnVideoFrameComing  cb)
    {
        mFrameCallback = cb;
    }

    public abstract boolean isStarted();
}
