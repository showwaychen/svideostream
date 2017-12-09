package cn.cxw.svideostreamlib;

import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;

/**
 * Created by cxw on 2017/11/12.
 */

public abstract class VideoFrameSource {
    public  enum State
    {
        kNONE,
        kStartting,
        kStarted,
        kStopped
    }

    public interface OnVideoFrameComing
    {
        void onVideoFrameComing(ByteBuffer framedata, int stride, int height);
    }
    protected int mSrcWidth = 0;
    protected int mSrcStride = 0;
    protected int mSrcHeight = 0;
    protected int mSrcFormate = 0;
    protected int mRotation = 0;

    protected  State mState = State.kNONE;
    protected OnVideoFrameComing mFrameCallback = null;
    public interface VideoFrameSourceObserver
    {
        void onStarted();
    }
    WeakReference<VideoFrameSourceObserver> mObserver = null;
    public void setObserver(VideoFrameSourceObserver cb)
    {
        mObserver = new WeakReference<VideoFrameSourceObserver>(cb);
    }
    void NotifyObserver()
    {
        if (mObserver != null)
        {
            VideoFrameSourceObserver tmpobserver = mObserver.get();
            if (tmpobserver != null)
            {
                tmpobserver.onStarted();
            }
        }
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

    public State getState()
    {
        return mState;
    }
}
