package cxw.cn.gpuimageex;

import android.media.Image;
import android.media.ImageReader;
import android.opengl.GLES20;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.LinkedList;
import java.util.Queue;

import cxw.cn.gpuimageex.util.TextureRotationUtil;

/**
 * Created by cxw on 2017/11/5.
 */

public class GPUImageEx implements GlRenderThread.GLRenderer, GPUImageExCamera.CameraEventObserver, IPreviewView.IPreviewCallback {
    static String TAG = GPUImageEx.class.getCanonicalName();
    static final float CUBE[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
    };

    //preview callback to startpreview or stoppreview
    @Override
    public void onSurfaceCreated(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {
            startPreView();
    }

    @Override
    public void onSurfaceChanged(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {

    }

    @Override
    public void onSurfaceDestroyed(@NonNull IPreviewView.ISurfaceHolder holder) {
        stopPreview();
    }


    public interface GPUImageExObserver
    {
        void OnProcessingFrame(ByteBuffer framedata, int stride, int height);
    }
    //gl thread
    GlRenderThread mGlRenderThread = null;
    GlThreadImageReader mGlProcessThread = null;

    boolean mbUseImageReaderThread = false;
    int mSharedTextId = OpenGlUtils.NO_TEXTURE;

    GPUImageExObserver mObserver = null;

    //filter
    GPUImageExCamera mGPUImageCamera = null;
    private GPUImageFilter mFilter = new GPUImageFilter();
    GPUImageFilter mDisplayFilter = new GPUImageFilter();


    GPUImageExFrameBuffer mFrameBuffer = null;


    private final Queue<Runnable> mRunOnDraw;


    FloatBuffer mCubeBuffer;
    FloatBuffer mDisplayTextureBuffer;
    FloatBuffer mOffScreenTextureBuffer;


    private int mOutputWidth;
    private int mOutputHeight;

    ByteBuffer mCaptureBuffer = null;
    static GPUImageEx sGPUImageEx = new GPUImageEx();
    IPreviewView mPreviewView = null;
    public static GPUImageEx getInst()
    {
        return sGPUImageEx;
    }
    protected GPUImageEx()
    {
        mRunOnDraw = new LinkedList<Runnable>();
        mGPUImageCamera = new GPUImageExCamera(this);
        mCubeBuffer = ByteBuffer.allocateDirect(CUBE.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mCubeBuffer.put(CUBE).position(0);
        mDisplayTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.ONSCREEN_TEXTURE_NO_ROTATION.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mDisplayTextureBuffer.put(TextureRotationUtil.ONSCREEN_TEXTURE_NO_ROTATION).position(0);

        mOffScreenTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mOffScreenTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            mbUseImageReaderThread = true;
        }

    }
    public void setPreviewView(IPreviewView pview)
    {
        mPreviewView = pview;
        if (pview != null)
        {
            mGlRenderThread = new GlThreadPreview(pview);
            pview.addRenderCallback(this);
        }
    }
    public void setObserver(GPUImageExObserver observer)
    {
        mObserver = observer;
    }

    public void setFilter(final GPUImageFilter filter)
    {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                if (mFilter != null) {
                    mFilter.destroy();
                }
                mFilter = filter;
                if (mFilter != null) {
                    mFilter.init();
                    mFilter.onOutputSizeChanged(mGPUImageCamera.getCameraFrameWidth(), mGPUImageCamera.getCameraFrameHeight());
                }
            }
        });
    }
    protected void runOnDraw(final Runnable runnable) {
        synchronized (mRunOnDraw) {
            mRunOnDraw.add(runnable);
        }
    }
    private void runAll(Queue<Runnable> queue) {
        synchronized (queue) {
            while (!queue.isEmpty()) {
                queue.poll().run();
            }
        }
    }
    public  void requestRender()
    {
        if (mbUseImageReaderThread && mGlProcessThread != null)
        {
            mGlProcessThread.requestRender();
        }
        else
        {
            if (mGlRenderThread != null)
            {
                mGlRenderThread.requestRender();
            }
        }

    }
    class PreviewRender implements GlRenderThread.GLRenderer
    {
        GPUImageFilter mPreviewFilter = new GPUImageFilter();
        int mWidth = 0;
        int mHeight = 0;
        @Override
        public void onInit() {
            mPreviewFilter.init();
//            startPreView();
        }

        @Override
        public void onResize(int width, int height) {
            mPreviewFilter.onOutputSizeChanged(width, height);
            mWidth = width;
            mHeight = height;
        }

        @Override
        public void onDrawFrame() {
            GLES20.glViewport(0, 0, mWidth, mHeight);
            GLES20.glClearColor(255, 255, 255, 1);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
            mPreviewFilter.onDraw(mSharedTextId, mCubeBuffer, mDisplayTextureBuffer);
        }

        @Override
        public void onDeinit() {
            mPreviewFilter.destroy();
//            stopPreview();
        }
    }
    void startPreviewGlThread()
    {
        if (mGlRenderThread == null)
        {
            mGlRenderThread = new GlThreadPreview(mPreviewView);
        }
        if (mbUseImageReaderThread && mGlProcessThread != null)
        {
            mGlRenderThread.setSharedContext(mGlProcessThread.getEglContext());
            mGlRenderThread.setRender(new PreviewRender());

        }
        else
        {
            mGlRenderThread.setRender(this);
        }
        mGlRenderThread.start();
    }

    //camera about
    @RequiresApi(api = Build.VERSION_CODES.KITKAT)
    class ImageAvailable implements ImageReader.OnImageAvailableListener
    {
            @RequiresApi(api = Build.VERSION_CODES.KITKAT)
            @Override
            public void onImageAvailable(ImageReader reader) {
                Image image = null;
                image = reader.acquireNextImage();
                if (image != null) {
                    try {
                        Image.Plane[] planes = image.getPlanes();
                        ByteBuffer buffer = planes[0].getBuffer();
                        int mSrcStride = 0;
                        mSrcStride = planes[0].getRowStride();
                        if (mObserver != null)
                        {
                            mObserver.OnProcessingFrame(buffer, mSrcStride, image.getHeight());
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    finally {
                        if (image!=null) {
                            image.close();
                        }
                    }
                }
        }
    }
    public void startPreView()
    {
        if (isPreview())
        {
            Log.w(TAG, "has started preview");
            return ;
        }
        if (mbUseImageReaderThread) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                mGlProcessThread = new GlThreadImageReader(mGPUImageCamera.getCameraFrameWidth(), mGPUImageCamera.getCameraFrameHeight(), this);
                mGlProcessThread.setImageAvailableListener(new ImageAvailable());
                mGlProcessThread.start();
            }
            return ;
        }
        startPreviewGlThread();
    }
    public void stopPreview()
    {
        if (mbUseImageReaderThread || mGlProcessThread != null)
        {
            mGlProcessThread.stopRender();
            mGlProcessThread.release();
            mGlProcessThread = null;
        }
        mGlRenderThread.stopRender();
        mGlRenderThread = null;
    }
    public boolean isFrontCamera() {
        return mGPUImageCamera.isFrontCamera();
    }

    public boolean isBackCamera() {
        return mGPUImageCamera.isBackCamera();
    }

    public boolean hadFrontCamera() {
        return mGPUImageCamera.hasFrontCamera();
    }

    public boolean hadBackCamera() {
        return mGPUImageCamera.hasBackCamera();
    }

    public boolean isPreview() {
        return mGPUImageCamera.isPreview();
    }
    public void setCameraSize(int width , int height)
    {
        mGPUImageCamera.setCameraSize(width, height);
    }
//    public void setCameraRotation(int rotation)
//    {
//        mGPUImageCamera.setCameraRotation(rotation);
//    }
    public int getProcessedFrameWidth()
    {
        return mGPUImageCamera.getCameraFrameWidth();
    }
    public int getProcessedFrameHeight()
    {
        return mGPUImageCamera.getCameraFrameHeight();
    }
    //camera event
    @Override
    public void onStartedPreview() {

    }

    @Override
    public void onStopPreview() {

    }

    @Override
    public void onFrameFrameAvailable() {
        requestRender();
    }
    //open gl es thread
    @Override
    public void onDrawFrame() {
        runAll(mRunOnDraw);
        if (mFrameBuffer == null)
        {
            mFrameBuffer = new GPUImageExFrameBuffer(mGPUImageCamera.getCameraFrameWidth(), mGPUImageCamera.getCameraFrameHeight());
            if (mFilter != null)
            {
                mFilter.onOutputSizeChanged(mGPUImageCamera.getCameraFrameWidth(), mGPUImageCamera.getCameraFrameHeight());
            }

            mCaptureBuffer = ByteBuffer.allocateDirect(mFrameBuffer.getFrameBufferHeight() * mFrameBuffer.getFrameBufferWidth() * 4);

        }
        int displaytextureid = -1;
//            mFrameBuffer.activeFrameBuffer();
//            GLES20.glClearColor(0, 0, 0, 1);
//            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

            GLES20.glViewport(0, 0, mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight());
            //get camera image
            mGPUImageCamera.onDraw();
            displaytextureid = mGPUImageCamera.getTextureId();

            mFrameBuffer.activeFrameBuffer();
            GLES20.glClearColor(0, 0, 0, 1);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
            if (mFilter != null)
            {
                mFilter.onDraw(mGPUImageCamera.getTextureId(), mCubeBuffer, mOffScreenTextureBuffer);
                displaytextureid = mFrameBuffer.getTextureId();
            }

        if (!mbUseImageReaderThread && mObserver != null)
        {
            long ptime = System.currentTimeMillis();
            //???????
            GLES20.glReadPixels(0, 0,
                    mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight(),
                    GLES20.GL_RGBA,
                    GLES20.GL_UNSIGNED_BYTE,
                    mCaptureBuffer);
//            Log.d(TAG, "glReadPixels time = " + (System.currentTimeMillis() - ptime));
            mObserver.OnProcessingFrame(mCaptureBuffer, mFrameBuffer.getFrameBufferWidth() * 4, mFrameBuffer.getFrameBufferHeight());
        }
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        onDrawDisplay(displaytextureid);
    }

    void onDrawDisplay(int textureid)
    {
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glViewport(0, 0, mOutputWidth, mOutputHeight);
        //display framebuffer
        mDisplayFilter.onDraw(textureid, mCubeBuffer, mDisplayTextureBuffer);
        mSharedTextId = mFrameBuffer.getTextureId();
        if (mbUseImageReaderThread && mGlRenderThread != null)
        {
            mGlRenderThread.requestRender();
        }

    }
    @Override
    public void onInit() {
        mGPUImageCamera.init();
        if (mFilter != null)
        {
            mFilter.init();
        }
        mDisplayFilter.init();
        if (mbUseImageReaderThread)
        {
            startPreviewGlThread();
        }

    }
    @Override
    public void onResize(int width, int height) {
        Log.d(TAG, "onResize width = " + width + "height = " + height);
        mOutputWidth = width;
        mOutputHeight = height;
//        mGPUImageCamera.onOutputSizeChanged(mOutputWidth, mOutputHeight);
//        mDisplayFilter.onOutputSizeChanged(mOutputWidth, mOutputHeight);
//        if (mFilter != null)
//        {
//            mFilter.onOutputSizeChanged(mOutputWidth, mOutputHeight);
//        }
//        GLES20.glViewport(0, 0, width, height);

    }
    @Override
    public void onDeinit() {
        if (mFrameBuffer != null)
        {
            mFrameBuffer.release();
            mFrameBuffer = null;
        }
        if (mFilter != null)
        {
            mFilter.destroy();
        }
        mDisplayFilter.destroy();
        mGPUImageCamera.stopPreview();
    }
}
