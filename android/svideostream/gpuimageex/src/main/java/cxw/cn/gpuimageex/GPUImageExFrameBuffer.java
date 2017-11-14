package cxw.cn.gpuimageex;

import android.opengl.GLES20;

/**
 * Created by cxw on 2017/11/6.
 */

public class GPUImageExFrameBuffer {

    public static final int NO_FRAMEBUFFER = -1;
    private  int mWidth;
    private  int mHeight;
    private  int mFrameBufferId = NO_FRAMEBUFFER;
    private  int mTextureId = OpenGlUtils.NO_TEXTURE;

    public GPUImageExFrameBuffer(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return ;
        }
        mWidth = width;
        mHeight = height;
        int[]  framebuffer = new int[1];
        int[] texture = new int[1];
        //generate a frame buffer
        GLES20.glGenFramebuffers(1, framebuffer, 0);
        //generate a texture and set some params;
        GLES20.glGenTextures(1, texture, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, width, height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        //attach the texture to this framebuffer
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, framebuffer[0]);
        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, texture[0], 0);

        //restore status
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);

        mFrameBufferId = framebuffer[0];
        mTextureId = texture[0];
    }
    public int getTextureId()
    {
        return mTextureId;
    }
    public  void activeFrameBuffer()
    {
        if (mFrameBufferId != NO_FRAMEBUFFER)
        {
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBufferId);
            GLES20.glViewport(0, 0, mWidth, mHeight);
        }
    }
    public int getFrameBufferWidth()
    {
        return mWidth;
    }
    public int getFrameBufferHeight()
    {
        return mHeight;
    }
    public void release()
    {
        if (mFrameBufferId != NO_FRAMEBUFFER)
        {
            GLES20.glDeleteFramebuffers(1, new int[mFrameBufferId], 0);
            mFrameBufferId = NO_FRAMEBUFFER;
        }
        if (mTextureId != OpenGlUtils.NO_TEXTURE)
        {
            GLES20.glDeleteTextures(1, new int[mTextureId], 0);
            mTextureId = OpenGlUtils.NO_TEXTURE;
        }
    }
}
