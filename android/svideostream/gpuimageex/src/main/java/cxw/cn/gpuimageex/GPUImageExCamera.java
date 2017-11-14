package cxw.cn.gpuimageex;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import cxw.cn.gpuimageex.util.TextureRotationUtil;


/**
 * Created by user on 2017/11/6.
 */

public class GPUImageExCamera extends GPUImageFilter{
    private static final String TAG = GPUImageExCamera.class.getCanonicalName();
    private static final int NO_CAMERA = -1;

    public static final String VERTEX_SHADER = "" +
            "attribute vec4 position;\n" +
            "attribute vec4 inputTextureCoordinate;\n" +
            " \n" +
            "varying vec2 textureCoordinate;\n" +
            " \n" +
            "void main()\n" +
            "{\n" +
            "    gl_Position = position;\n" +
            "    textureCoordinate = inputTextureCoordinate.xy;\n" +
            "}";
    public static final String FRAGMENT_SHADER = "" +
            "#extension GL_OES_EGL_image_external : require\n" +
            "varying highp vec2 textureCoordinate;\n" +
            " \n" +
            "uniform samplerExternalOES inputImageTexture;\n" +
            " \n" +
            "void main()\n" +
            "{\n" +
            "     gl_FragColor = texture2D(inputImageTexture, textureCoordinate);\n" +
            "}";
    GPUImageEx mGPUImageEx;

    boolean mIsPreview = false;

    SurfaceTexture mSurfaceTexture = null;
    int mCameraId = NO_CAMERA;
    int mCameraFace = -1;
    int mCameraWidth = 640;
    int mCameraHeight = 480;
    Camera mCamera = null;
    int mTextureid = OpenGlUtils.NO_TEXTURE;
    FloatBuffer mVertexBuffer;
    float mDefaultVertex[] = {-1.0f, -1.0f,
                              1.0f, -1.0f,
                              -1.0f, 1.0f,
                              1.0f, 1.0f};
    FloatBuffer mTextureCoorBuffer;

    private Rotation mRotation = Rotation.NORMAL;
    GPUImageExFrameBuffer mFrameBuffer = null;
    public GPUImageExCamera(GPUImageEx gpuimagecontext)
    {
        super(VERTEX_SHADER, FRAGMENT_SHADER);
        mGPUImageEx = gpuimagecontext;
        if (hasFrontCamera())
        {
            mCameraFace = Camera.CameraInfo.CAMERA_FACING_FRONT;
        }else if (hasBackCamera())
        {
            mCameraFace = Camera.CameraInfo.CAMERA_FACING_BACK;
        }
        mVertexBuffer = ByteBuffer.allocateDirect(mDefaultVertex.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mVertexBuffer.put(mDefaultVertex).position(0);
        mTextureCoorBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mTextureCoorBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
    }

    public void setCameraSize(int width , int height)
    {
        if (width <= 0 || height <= 0)
        {
            return ;
        }
        mCameraWidth = width;
        mCameraHeight = height;
    }
    public void setCameraRotation(int rotation)
    {
        switch (rotation)
        {
            case 0:
                mRotation = Rotation.NORMAL;
                break;
            case 90:
                mRotation = Rotation.ROTATION_90;
                break;
            case 180:
                mRotation = Rotation.ROTATION_180;
                break;
            case 270:
                mRotation = Rotation.ROTATION_270;
                break;
        }
    }
    public  int getCameraFrameWidth()
    {
        return (mRotation == Rotation.ROTATION_90  || mRotation == Rotation.ROTATION_270)?mCameraHeight:mCameraWidth;
    }
    public int getCameraFrameHeight()
    {
        return (mRotation == Rotation.ROTATION_90  || mRotation == Rotation.ROTATION_270)?mCameraWidth:mCameraHeight;

    }
    private void adjustImageScaling() {
        float outputWidth = mOutputWidth;
        float outputHeight = mOutputHeight;
        if (mRotation == Rotation.ROTATION_270 || mRotation == Rotation.ROTATION_90) {
            outputWidth = mOutputHeight;
            outputHeight = mOutputWidth;
        }
        boolean flipHorizontal = mCameraFace == Camera.CameraInfo.CAMERA_FACING_FRONT;
        float[] textureCords = TextureRotationUtil.getRotation(mRotation, flipHorizontal, false);
        mTextureCoorBuffer.clear();
        mTextureCoorBuffer.put(textureCords).position(0);
    }
    public  int getTextureId()
    {
       return  mFrameBuffer.getTextureId();
    }
    public  void onDraw()
    {
        mSurfaceTexture.updateTexImage();
        if (mFrameBuffer == null)
        {
            mFrameBuffer = new GPUImageExFrameBuffer(getCameraFrameWidth(), getCameraFrameHeight());
        }
        GLES20.glUseProgram(mGLProgId);
        if (!isInitialized())
        {
            return ;
        }
        mFrameBuffer.activeFrameBuffer();
        GLES20.glClearColor(0, 0, 0, 1);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        mVertexBuffer.position(0);
        mTextureCoorBuffer.position(0);
        GLES20.glVertexAttribPointer(mGLAttribPosition, 2, GLES20.GL_FLOAT, false, 0, mVertexBuffer);
        GLES20.glEnableVertexAttribArray(mGLAttribPosition);

        GLES20.glVertexAttribPointer(mGLAttribTextureCoordinate, 2, GLES20.GL_FLOAT, false, 0, mTextureCoorBuffer);
        GLES20.glEnableVertexAttribArray(mGLAttribTextureCoordinate);
        if (mTextureid != OpenGlUtils.NO_TEXTURE)
        {
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureid);
            // Set the sampler to texture unit 0
            GLES20.glUniform1i(mGLUniformTexture, 0);
        }
        onDrawArraysPre();
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        //restore status
        GLES20.glDisableVertexAttribArray(mGLAttribPosition);
        GLES20.glDisableVertexAttribArray(mGLAttribTextureCoordinate);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
    }
    public boolean startPreview()
    {
        if (mIsPreview)
        {
            return true;
        }
        if (mSurfaceTexture == null)
        {
            Log.d(TAG, "no surfacetexture");
            return false;
        }
        mCameraId = getCameraId(mCameraFace);
        if (mCameraId == NO_CAMERA)
        {
            return false;
        }
        try {
            mCamera = Camera.open(mCameraId);
            Camera.Parameters cparams = mCamera.getParameters();
            if (cparams.getSupportedFocusModes().contains(
                    Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                cparams.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            }
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(mCameraId, info);
            setCameraRotation(info.orientation);
            cparams.setPreviewSize(mCameraWidth, mCameraHeight);
            mCamera.setParameters(cparams);

            mCamera.setPreviewTexture(mSurfaceTexture);
            mCamera.startPreview();
            adjustImageScaling();
            if (mFrameBuffer != null)
            {
                mFrameBuffer.release();
                mFrameBuffer = null;
            }
            mFrameBuffer = new GPUImageExFrameBuffer(getCameraFrameWidth(), getCameraFrameHeight());
            Log.d(TAG, "startPreview success");
            mIsPreview = true;
        }catch (Exception e)
        {
            Log.e(TAG, "startPreview failed = " + e.getMessage());
            mCamera = null;
            mCameraId = NO_CAMERA;
            return false;
        }
        return false;
    }
    public void stopPreview()
    {
        if (isPreview())
        {
            mCamera.stopPreview();
            mCamera.release();
            mCamera = null;
            mCameraId = NO_CAMERA;
            if (mFrameBuffer != null)
            {
                mFrameBuffer.release();
                mFrameBuffer = null;
            }
            mIsPreview = false;
        }
    }
    public boolean isPreview()
    {
        return mIsPreview;
    }
    public boolean isFrontCamera()
    {
        return mCameraFace == Camera.CameraInfo.CAMERA_FACING_FRONT;
    }
    public  boolean isBackCamera()
    {
        return mCameraFace == Camera.CameraInfo.CAMERA_FACING_BACK;
    }
    public int getCameraId(int cameraface)
    {
        int cameraid = NO_CAMERA;
        int numberCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberCameras; i++)
        {
            Camera.CameraInfo  info = new Camera.CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == cameraface)
            {
                cameraid = i;
                break;
            }
        }
        return cameraid;
    }
    public  boolean hasCamera(int cameraface)
    {
        return getCameraId(cameraface) != NO_CAMERA;
    }
    public boolean hasFrontCamera()
    {
        return hasCamera(Camera.CameraInfo.CAMERA_FACING_FRONT);
    }
    public boolean hasBackCamera()
    {
        return hasCamera(Camera.CameraInfo.CAMERA_FACING_BACK);
    }


    void createSurfaceTexture()
    {
        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        mSurfaceTexture = new SurfaceTexture(textures[0]);
        mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                mGPUImageEx.requestRender();
            }
        });
    }
    void destroySurfaceTexture()
    {
        if (mTextureid != OpenGlUtils.NO_TEXTURE)
        {
            GLES20.glDeleteTextures(1, new int[]{mTextureid}, 0);
            mTextureid = OpenGlUtils.NO_TEXTURE;
        }
    }
    @Override
    public void onInit() {
        super.onInit();
        createSurfaceTexture();
        startPreview();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        destroySurfaceTexture();
    }
}
