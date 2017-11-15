package cxw.cn.gpuimageex;

<<<<<<< HEAD
import android.media.Image;
import android.media.ImageReader;
import android.opengl.GLES20;
import android.os.Build;
import android.support.annotation.RequiresApi;
=======
import android.opengl.GLES20;
import android.support.annotation.NonNull;
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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

<<<<<<< HEAD
public class GPUImageEx implements GlRenderThread.GLRenderer, GPUImageExCamera.CameraEventObserver {
=======
public class GPUImageEx implements IPreviewView.IPreviewCallback,GlRenderThread.GLRenderer {
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    static String TAG = GPUImageEx.class.getCanonicalName();
    static final float CUBE[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
    };


<<<<<<< HEAD


    public interface GPUImageExObserver
    {
        void OnProcessingFrame(byte[] framedata, int stride, int height);
    }
    //gl thread
    GlRenderThread mGlRenderThread = null;
    GlThreadImageReader mGlProcessThread = null;

    boolean mbUseImageReaderThread = false;
    int mSharedTextId = OpenGlUtils.NO_TEXTURE;

=======
    public interface GPUImageExObserver
    {
        void OnProcessingFrame(byte[] framedata, int width, int height);
    }
    GlRenderThread mGlRenderThread = null;
    IPreviewView mPreviewView = null;
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    GPUImageExObserver mObserver = null;

    //filter
    GPUImageExCamera mGPUImageCamera = null;
    private GPUImageFilter mFilter = new GPUImageFilter();
    GPUImageFilter mDisplayFilter = new GPUImageFilter();


    GPUImageExFrameBuffer mFrameBuffer = null;
<<<<<<< HEAD


    private final Queue<Runnable> mRunOnDraw;


=======
    private final Queue<Runnable> mRunOnDraw;
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    FloatBuffer mCubeBuffer;
    FloatBuffer mDisplayTextureBuffer;
    FloatBuffer mOffScreenTextureBuffer;


    private int mOutputWidth;
    private int mOutputHeight;

    ByteBuffer mCaptureBuffer = null;
    static GPUImageEx sGPUImageEx = new GPUImageEx();
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
<<<<<<< HEAD
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mbUseImageReaderThread = true;
        }
=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568

    }
    public void setPreviewView(IPreviewView pview)
    {
<<<<<<< HEAD
        mGlRenderThread = new GlThreadPreview(pview);

=======
        mPreviewView = pview;
        pview.addRenderCallback(this);
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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
<<<<<<< HEAD
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
            byte[] bytebuffer = null;
            @RequiresApi(api = Build.VERSION_CODES.KITKAT)
            @Override
            public void onImageAvailable(ImageReader reader) {
                Image image = null;
                image = reader.acquireLatestImage();
                if (image != null) {
                    try {
                        Image.Plane[] planes = image.getPlanes();
                        ByteBuffer buffer = planes[0].getBuffer();
                        int mSrcStride = 0;
                        mSrcStride = planes[0].getRowStride();
                        if (mObserver != null)
                        {
                            if (bytebuffer == null)
                            {
                                bytebuffer = new byte[buffer.remaining()];
                            }
                            buffer.get(bytebuffer, 0, bytebuffer.length);
                            mObserver.OnProcessingFrame(bytebuffer, mSrcStride, image.getHeight());
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
        if (mbUseImageReaderThread) {
            mGlProcessThread = new GlThreadImageReader(mGPUImageCamera.getCameraFrameWidth(), mGPUImageCamera.getCameraFrameHeight(), this);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                mGlProcessThread.setImageAvailableListener(new ImageAvailable());
            }
            mGlProcessThread.start();
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

        }
        mGlRenderThread.stopRender();
=======
        if (mGlRenderThread != null)
        {
            mGlRenderThread.requestRender();
        }
    }

    //camera about
    public void startPreView()
    {

//        mGPUImageCamera.startPreview();
    }
    public void stopPreview()
    {
//        mGPUImageCamera.stopPreview();
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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
<<<<<<< HEAD
//    public void setCameraRotation(int rotation)
//    {
//        mGPUImageCamera.setCameraRotation(rotation);
//    }
=======
    public void setCameraRotation(int rotation)
    {
        mGPUImageCamera.setCameraRotation(rotation);
    }
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    public int getProcessedFrameWidth()
    {
        return mGPUImageCamera.getCameraFrameWidth();
    }
    public int getProcessedFrameHeight()
    {
        return mGPUImageCamera.getCameraFrameHeight();
    }
<<<<<<< HEAD
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
=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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
<<<<<<< HEAD

=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
            mCaptureBuffer = ByteBuffer.allocate(mFrameBuffer.getFrameBufferHeight() * mFrameBuffer.getFrameBufferWidth() * 4);

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
        if (mObserver != null)
        {
<<<<<<< HEAD
//            long ptime = System.currentTimeMillis();
//            //???????
//            GLES20.glReadPixels(0, 0,
//                    mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight(),
//                    GLES20.GL_RGBA,
//                    GLES20.GL_UNSIGNED_BYTE,
//                    mCaptureBuffer);
////            Log.d(TAG, "glReadPixels time = " + (System.currentTimeMillis() - ptime));
//            mObserver.OnProcessingFrame(mCaptureBuffer.array(), mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight());
=======
            long ptime = System.currentTimeMillis();
            //???????
            GLES20.glReadPixels(0, 0,
                    mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight(),
                    GLES20.GL_RGBA,
                    GLES20.GL_UNSIGNED_BYTE,
                    mCaptureBuffer);
//            Log.d(TAG, "glReadPixels time = " + (System.currentTimeMillis() - ptime));
            mObserver.OnProcessingFrame(mCaptureBuffer.array(), mFrameBuffer.getFrameBufferWidth(), mFrameBuffer.getFrameBufferHeight());
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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
<<<<<<< HEAD
        mSharedTextId = mFrameBuffer.getTextureId();
        if (mbUseImageReaderThread && mGlRenderThread != null)
        {
            mGlRenderThread.requestRender();
        }

=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
    }
    @Override
    public void onInit() {
        mGPUImageCamera.init();
        if (mFilter != null)
        {
            mFilter.init();
        }
        mDisplayFilter.init();
<<<<<<< HEAD
        if (mbUseImageReaderThread)
        {
            startPreviewGlThread();
        }
=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568

    }
    @Override
    public void onResize(int width, int height) {
<<<<<<< HEAD
        Log.d(TAG, "onResize width = " + width + "height = " + height);
=======
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
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
<<<<<<< HEAD
=======
    @Override
    public void onSurfaceCreated(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {
        mGlRenderThread = new GlRenderThread(holder.getSurface(), this);
        mGlRenderThread.start();
    }

    @Override
    public void onSurfaceChanged(@NonNull IPreviewView.ISurfaceHolder holder, int width, int height) {
        if (mGlRenderThread != null)
        {
            mGlRenderThread.requestResize(width, height);
        }
    }

    @Override
    public void onSurfaceDestroyed(@NonNull IPreviewView.ISurfaceHolder holder) {
        if (mGlRenderThread != null)
        {
            mGlRenderThread.stopRender();
            mGlRenderThread = null;
        }
    }
>>>>>>> 6b62b3a06fc12fc6edb6813b4f5254d5c85f0568
}
