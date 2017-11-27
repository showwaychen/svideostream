package cn.cxw.svideostream.widget;

import android.content.Context;
import android.support.annotation.NonNull;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import java.util.HashSet;
import java.util.Set;

import cxw.cn.gpuimageex.IPreviewView;

/**
 * Created by cxw on 2017/11/7.
 */

public class SurfaceViewPreview extends SurfaceView implements IPreviewView {
    private SurfaceCallback mSurfaceHolder;
    private boolean mIsFormatChanged;
    private int mFormat;
    private int mWidth;
    private int mHeight;
    public  SurfaceViewPreview(Context context)
    {
        super(context);
        initView();
    }

    public SurfaceViewPreview(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }
    public SurfaceViewPreview(Context context, AttributeSet attrs, int defStyleAttr)
    {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private static final class InternalSurfaceHolder implements ISurfaceHolder {
        private SurfaceHolder mSurfaceHolder;
        public InternalSurfaceHolder(SurfaceHolder surfaceHolder)
        {
            mSurfaceHolder = surfaceHolder;
        }
        @NonNull
        @Override
        public Surface getSurface() {
            return mSurfaceHolder.getSurface();
        }
    }
    private static final class SurfaceCallback implements SurfaceHolder.Callback {
        private SurfaceHolder mSurfaceHolder;
        private boolean mIsFormatChanged;
        private int mFormat;
        private int mWidth;
        private int mHeight;

        private Set<IPreviewCallback> mRenderCallback = new HashSet<IPreviewCallback>();


        public void addRenderCallback(@NonNull IPreviewCallback callback) {
            mRenderCallback.add(callback);

            ISurfaceHolder surfaceHolder = null;
            if (mSurfaceHolder != null) {
                if (surfaceHolder == null)
                    surfaceHolder = new InternalSurfaceHolder(mSurfaceHolder);
                callback.onSurfaceCreated(surfaceHolder, mWidth, mHeight);
            }

            if (mIsFormatChanged) {
                if (surfaceHolder == null)
                    surfaceHolder = new InternalSurfaceHolder(mSurfaceHolder);
                callback.onSurfaceChanged(surfaceHolder, mWidth, mHeight);
            }
        }

        public void removeRenderCallback(@NonNull IPreviewCallback callback) {
            mRenderCallback.remove(callback);
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            mSurfaceHolder = holder;
            mIsFormatChanged = false;
            mFormat = 0;
            mWidth = 0;
            mHeight = 0;

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder( mSurfaceHolder);
            Set<IPreviewCallback> tmpCallback = new HashSet<>(mRenderCallback);
            for (IPreviewCallback renderCallback : tmpCallback) {
                renderCallback.onSurfaceCreated(surfaceHolder, 0, 0);
            }
        }

        @Override
        public void surfaceDestroyed(SurfaceHolder holder) {
            mSurfaceHolder = null;
            mIsFormatChanged = false;
            mFormat = 0;
            mWidth = 0;
            mHeight = 0;

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder( mSurfaceHolder);
            Set<IPreviewCallback> tmpCallback = new HashSet<>(mRenderCallback);
            for (IPreviewCallback renderCallback : tmpCallback) {
                renderCallback.onSurfaceDestroyed(surfaceHolder);
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format,
                                   int width, int height) {
            mSurfaceHolder = holder;
            mIsFormatChanged = true;
            mFormat = format;
            mWidth = width;
            mHeight = height;

            // mMeasureHelper.setVideoSize(width, height);

            ISurfaceHolder surfaceHolder = new InternalSurfaceHolder(mSurfaceHolder);
            Set<IPreviewCallback> tmpCallback = new HashSet<>(mRenderCallback);
            for (IPreviewCallback renderCallback : tmpCallback) {
                renderCallback.onSurfaceChanged(surfaceHolder, width, height);
            }
        }
    }
    private void initView()
    {
        mSurfaceHolder = new SurfaceCallback();
        getHolder().addCallback(mSurfaceHolder);
        //noinspection deprecation
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_NORMAL);
    }


    @Override
    public View getView() {
        return this;
    }

    @Override
    public void addRenderCallback(@NonNull IPreviewCallback callback) {
        mSurfaceHolder.addRenderCallback(callback);
    }

    @Override
    public void removeRenderCallback(@NonNull IPreviewCallback callback) {
        mSurfaceHolder.removeRenderCallback(callback);
    }
}
