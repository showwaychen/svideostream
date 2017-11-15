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
    public interface VideoFrameSourceObserver
    {
        void onStarted();
    }
    VideoFrameSourceObserver mObserver = null;
    public void setObserver(VideoFrameSourceObserver cb)
    {
        mObserver = cb;
    }
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
