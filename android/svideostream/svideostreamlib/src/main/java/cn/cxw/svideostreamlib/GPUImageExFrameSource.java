package cn.cxw.svideostreamlib;

<<<<<<< HEAD
import android.util.Log;

=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
import cxw.cn.gpuimageex.GPUImageBeautifyFilter;
import cxw.cn.gpuimageex.GPUImageEx;
import cxw.cn.gpuimageex.GPUImageFilter;
import cxw.cn.gpuimageex.IPreviewView;

/**
 * Created by cxw on 2017/11/12.
 */

public class GPUImageExFrameSource extends VideoFrameSource implements GPUImageEx.GPUImageExObserver{

    public  static String TAG = GPUImageExFrameSource.class.getCanonicalName();
    GPUImageEx mGpuImageEx = GPUImageEx.getInst();

    public GPUImageExFrameSource()
    {
        mGpuImageEx.setObserver(this);
        mSrcFormate = VideoStreamConstants.IMAGE_FORMAT_ABGR;
    }
<<<<<<< HEAD
    public void startPreview()
    {
        if(mGpuImageEx.isPreview())
        {
            Log.d(TAG," has already previewed");
            return ;
        }
        mSrcStride = 0;
        mGpuImageEx.startPreView();
    }
    public void stopPreview()
    {
        mGpuImageEx.stopPreview();
    }
=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    public void setCameraSize(int width, int height)
    {
        mGpuImageEx.setCameraSize(width, height);
    }
    public void enableBeatyFilter(boolean isenable)
    {
        mGpuImageEx.setFilter(isenable?new GPUImageBeautifyFilter():new GPUImageFilter());
    }
    public void setPreviewView(IPreviewView view)
    {
        mGpuImageEx.setPreviewView(view);
    }
    @Override
<<<<<<< HEAD
    public void OnProcessingFrame(byte[] framedata, int stride, int height) {
        if (stride != mSrcStride)
        {
            mSrcStride = stride;
            if (mObserver != null)
            {
                mObserver.onStarted();
            }
        }

        if (mFrameCallback != null)
        {
            mFrameCallback.onVideoFrameComing(framedata, stride, height);
=======
    public void OnProcessingFrame(byte[] framedata, int width, int height) {
        if (mFrameCallback != null)
        {
            mFrameCallback.onVideoFrameComing(framedata, mSrcStride, height);
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
        }
    }

    @Override
    public int getSrcWidth() {
        return mGpuImageEx.getProcessedFrameWidth();
    }

<<<<<<< HEAD
=======
    @Override
    public int getSrcStride() {
        return getSrcWidth() * 4;
    }
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568

    @Override
    public int getSrcHeight() {
        return mGpuImageEx.getProcessedFrameHeight();
    }

    @Override
    public boolean isStarted() {
        return mGpuImageEx.isPreview();
    }
}
