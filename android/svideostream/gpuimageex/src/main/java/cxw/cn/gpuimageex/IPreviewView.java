package cxw.cn.gpuimageex;

import android.support.annotation.NonNull;
import android.view.Surface;
import android.view.View;

/**
 * Created by cxw on 2017/11/5.
 */

public interface IPreviewView {
    View getView();

    void addRenderCallback(@NonNull IPreviewCallback callback);

    void removeRenderCallback(@NonNull IPreviewCallback callback);
    interface ISurfaceHolder {

        @NonNull
        Surface getSurface();
    }
    interface IPreviewCallback {

        void onSurfaceCreated(@NonNull ISurfaceHolder holder, int width, int height);


        void onSurfaceChanged(@NonNull ISurfaceHolder holder,int width, int height);

        void onSurfaceDestroyed(@NonNull ISurfaceHolder holder);
    }
}
