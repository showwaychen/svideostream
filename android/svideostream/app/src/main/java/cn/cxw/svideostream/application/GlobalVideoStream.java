package cn.cxw.svideostream.application;

import cn.cxw.svideostreamlib.VideoStreamProxy;

/**
 * Created by cxw on 2017/11/19.
 */

public class GlobalVideoStream {
    static VideoStreamProxy mGpuImageSourceUsing = null;
    static VideoStreamProxy mScreenCaptureUsing = null;
    public  static VideoStreamProxy getGPUImageSourceOwn()
    {
        if (mGpuImageSourceUsing == null)
        {
            mGpuImageSourceUsing = new VideoStreamProxy();
        }
        return mGpuImageSourceUsing;
    }
    public  static VideoStreamProxy getScreenCaptureOwn()
    {
        if (mScreenCaptureUsing == null)
        {
            mScreenCaptureUsing = new VideoStreamProxy();
        }
        return mScreenCaptureUsing;
    }
    public  static void destroyGlobalVideoStream()
    {
        if (mScreenCaptureUsing != null)
        {
            mScreenCaptureUsing.destroyStream();
        }
        if (mGpuImageSourceUsing != null)
        {
            mGpuImageSourceUsing.destroyStream();
        }
    }
}
