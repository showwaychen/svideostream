package cn.cxw.magiccameralive;

import java.nio.ByteBuffer;

import cn.cxw.magiccameralib.MagicCamera;
import cn.cxw.magiccameralib.imagefilter.GPUImageBeautifyFilter;
import cn.cxw.magiccameralib.imagefilter.GPUImageFilter;
import cn.cxw.openglesutils.glthread.IPreviewView;
import cn.cxw.svideostreamlib.VideoFrameSource;
import cn.cxw.svideostreamlib.VideoStreamConstants;
/**
 * Created by user on 2018/3/2.
 */

public class MagicCameraFrameSource extends VideoFrameSource implements MagicCamera.MagicCameraFrameObserver {
    MagicCamera mMagicCamera = MagicCamera.getInst();
    {
        mMagicCamera.setFrameObserver(this);
        mSrcFormate = VideoStreamConstants.IMAGE_FORMAT_ABGR;
    }
    public void setPreviewView(IPreviewView view)
    {
        mMagicCamera.setPreviewView(view);
    }
    public void enableBeatyFilter(boolean isenable)
    {
        mMagicCamera.setOpenglDrawer(isenable?new GPUImageBeautifyFilter():new GPUImageFilter());
    }
    public void switchCamera()
    {
        mMagicCamera.switchCamera();
    }
    public void setCameraSize(int width, int height)
    {
        mMagicCamera.setCameraSize(width, height);
    }
    public MagicCamera getMagicCamera()
    {
        return mMagicCamera;
    }

    @Override
    public int getSrcWidth() {
        return mMagicCamera.getFrameWidth();
    }

    @Override
    public int getSrcHeight() {
        return mMagicCamera.getFrameHeight();
    }

    @Override
    public boolean isStarted() {
        return mMagicCamera.isPreview();
    }

    @Override
    public void OnProcessingFrame(ByteBuffer framedata, int stride, int width, int height) {
        if (stride != mSrcStride)
        {
            mSrcStride = stride;
            mState = State.kStarted;

            NotifyObserver();
        }
        if (mFrameCallback != null)
        {
            mFrameCallback.onVideoFrameComing(framedata, stride, height);
        }
    }
}
