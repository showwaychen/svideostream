package cxw.cn.gpuimageex;

import java.nio.ByteBuffer;

/**
 * Created by user on 2017/12/5.
 */

public abstract class OpenglCapture {

    static public final int CAPTURETYPE_READPIXEL = 0;
    static public final int CAPTURETYPE_GRAPHICBUFFER = 1;
    public static OpenglCapture createCapturer(int type)
    {
        switch (type)
        {
            case CAPTURETYPE_READPIXEL:
                return new ReadPixelCapture();
            case CAPTURETYPE_GRAPHICBUFFER:
                return new GraphicBufferCapture();
                default:
                    return null;
        }
    }
    public interface IFrameCaptured {
        void onPreviewFrame(ByteBuffer directFrameBuffer, int stride, int height, long ptsMS);
    }
    ByteBuffer mCaptureBuffer = null;
    boolean mbInitOk = false;
    int mTextureId = OpenGlUtils.NO_TEXTURE;

    int mWidth = 0;
    int mHeight = 0;
    IFrameCaptured mCallback = null;
    public void setOnCapture(IFrameCaptured cb)
    {
        mCallback = cb;
    }
    public void setTextureId(int textureId)
    {
        mTextureId = textureId;
    }
    abstract public boolean initCapture(int width, int height);

    abstract public void onCapture();

    abstract public void destroy();

}
