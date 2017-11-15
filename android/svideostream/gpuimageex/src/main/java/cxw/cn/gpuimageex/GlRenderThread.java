package cxw.cn.gpuimageex;

import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;
import android.view.Surface;

import java.util.concurrent.atomic.AtomicBoolean;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;

/**
 * Created by cxw on 2017/11/5.
 */

public class GlRenderThread extends Thread {
    static final  String TAG = "GlRenderThread";
    private AtomicBoolean mShouldRender;
    protected Surface mSurface;
    private GLRenderer mRenderer;
    private Object mSyncToken;
    boolean mRequestRender = false;
    boolean mRequestDestroy = false;
    private EGL10 mEgl;
    private EGLDisplay mEglDisplay = EGL10.EGL_NO_DISPLAY;
    private EGLContext mEglContext = EGL10.EGL_NO_CONTEXT;
    private EGLContext mEglSharedContext = EGL10.EGL_NO_CONTEXT;
    private EGLSurface mEglSurface = EGL10.EGL_NO_SURFACE;
    private GL mGL;
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private static final int EGL_OPENGL_ES2_BIT = 4;

    protected int mViewWidth = 0;
    protected int mViewHeight = 0;
    private boolean m_needResize = false;
    public interface GLRenderer {
        void onInit();
        void onResize(int width, int height);
        void onDrawFrame();
        void onDeinit();
    }

    protected  GlRenderThread()
    {
        mSyncToken = new Object();
    }
    protected void setSurface(Surface surface)
    {
        mSurface = surface;
    }
    public GlRenderThread(Surface surface, GLRenderer renderer) {
        mSurface = surface;
        mRenderer = renderer;
        mSyncToken = new Object();
        Log.d(TAG, "new GlRenderThread");
//        mShouldRender = shouldRender;
    }
    public  void setRender(GLRenderer render)
    {
        mRenderer = render;
    }
    public void setSharedContext(EGLContext sharedcontext)
    {
        mEglSharedContext = sharedcontext;
    }
    public EGLContext getEglContext()
    {
        return mEglContext;
    }
    private void initGL() {
        mEgl = (EGL10) EGLContext.getEGL();

        mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
            throw new RuntimeException("eglGetdisplay failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        int[] version = new int[2];
        if (!mEgl.eglInitialize(mEglDisplay, version)) {
            throw new RuntimeException("eglInitialize failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

//            int[] configAttribs = {
//                    EGL10.EGL_BUFFER_SIZE, 32,
//                    EGL10.EGL_ALPHA_SIZE, 8,
//                    EGL10.EGL_BLUE_SIZE, 8,
//                    EGL10.EGL_GREEN_SIZE, 8,
//                    EGL10.EGL_RED_SIZE, 8,
//                    EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
//                    EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
//                    EGL10.EGL_NONE
//            };
        int[] configAttribs = {
                EGL10.EGL_BUFFER_SIZE, 32,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 5,
                EGL10.EGL_GREEN_SIZE, 6,
                EGL10.EGL_RED_SIZE, 5,
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
                EGL10.EGL_NONE
        };

        int[] numConfigs = new int[1];
        EGLConfig[] configs = new EGLConfig[1];
        if (!mEgl.eglChooseConfig(mEglDisplay, configAttribs, configs, 1, numConfigs)) {
            throw new RuntimeException("eglChooseConfig failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        int[] contextAttribs = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL10.EGL_NONE
        };
        mEglContext = mEgl.eglCreateContext(mEglDisplay, configs[0], mEglSharedContext, contextAttribs);
        mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, configs[0], mSurface, null);
        if (mEglSurface == EGL10.EGL_NO_SURFACE || mEglContext == EGL10.EGL_NO_CONTEXT) {
            int error = mEgl.eglGetError();
            if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                throw new RuntimeException("eglCreateWindowSurface returned  EGL_BAD_NATIVE_WINDOW. ");
            }
            throw new RuntimeException("eglCreateWindowSurface failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
            throw new RuntimeException("eglMakeCurrent failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        mGL = mEglContext.getGL();
        GLES20.glFlush();
        Log.d(TAG, "initGL over");
    }

    public void stopRender()
    {
        Log.i(TAG, "requestDestroy");
        synchronized (mSyncToken) {
            mRequestDestroy = true;
            mSyncToken.notifyAll();
        }
    }
    private void destoryGL() {
        mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,
                EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
        mEgl.eglDestroyContext(mEglDisplay, mEglContext);
        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
        mEgl.eglTerminate(mEglDisplay);

        mEglContext = EGL10.EGL_NO_CONTEXT;
        mEglSurface = EGL10.EGL_NO_SURFACE;
    }
    public void requestRender() {
        synchronized (mSyncToken) {
            mRequestRender = true;
            mSyncToken.notifyAll();
        }
    }
    public void requestResize(int width, int height)
    {
        mViewHeight = height;
        mViewWidth = width;
        m_needResize = true;
        requestRender();
    }
    public void run() {
        setName("GlRenderThread " + getId());
        initGL();

        Log.d(TAG, "gl thread start run ");
        if (mRenderer != null) {
            mRenderer.onInit();;
        }
        while (true) {
            synchronized (mSyncToken) {
                if (mRequestDestroy) {
                    break;
                }
                if (m_needResize)
                {
                    if (mRenderer != null)
                    {
                        Log.d(TAG, "resize callback");
                        mRenderer.onResize(mViewWidth, mViewHeight);
                    }
                    m_needResize = false;
                }
                if (mRequestRender == false) {
                    try {
                        mSyncToken.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                mRequestRender = false;
                if (mRequestDestroy) {
                    break;
                }
            }
//                Log.i(TAG, "onDrawFrame ##");
            if (mRenderer != null)
                mRenderer.onDrawFrame();
            mEgl.eglSwapBuffers(mEglDisplay, mEglSurface);

        }
        if (mRenderer != null)
        {
            mRenderer.onDeinit();
        }
        mSurface = null;
        destoryGL();
    }
}
