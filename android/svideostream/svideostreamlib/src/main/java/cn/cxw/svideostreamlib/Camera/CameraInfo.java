package cn.cxw.svideostreamlib.Camera;

import android.hardware.Camera;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import cn.cxw.svideostreamlib.CameraVideoStream;

/**
 * Created by cxw on 2017/10/17.
 */

public class CameraInfo {
    protected static final String TAG = "CameraInfo";

    public final static int CAMERA_LANDSCAPE = 1;
    public final static int CAMERA_PORTRAIT = 2;

    private int frontCameraID;
    private int backCameraID;

    private int enjoyHeightArray[] = {270, 360, 450, 540};
    // always width > height
    private List<Camera.Size> orgFrontCameraSupportSizeList;
    private List<Camera.Size> orgBackCameraSupportSizeList;
    private CameraVideoStream.CameraSize frontCameraSize;
    private CameraVideoStream.CameraSize backCameraSize;
    private ArrayList<CameraVideoStream.CameraSize> enjoyFrontCameraSupportSizeList;
    private ArrayList<CameraVideoStream.CameraSize> enjoyBackCameraSupportSizeList;

    private static CameraInfo instance;
    public static CameraInfo getInstance() {
        if (instance == null) {
            instance = new CameraInfo();
        }
        return instance;
    }

    private CameraInfo() {
        // Init
        frontCameraID = getCameraID_(Camera.CameraInfo.CAMERA_FACING_FRONT);
        backCameraID = getCameraID_(Camera.CameraInfo.CAMERA_FACING_BACK);
        orgFrontCameraSupportSizeList = getSupportedPreviewSizes(frontCameraID);
        orgBackCameraSupportSizeList = getSupportedPreviewSizes(backCameraID);
        frontCameraSize = selectCameraSize(orgFrontCameraSupportSizeList);
        backCameraSize = selectCameraSize(orgBackCameraSupportSizeList);
        if (frontCameraSize != null) {
            Log.i(TAG, "front : " + frontCameraSize.width + "x" + frontCameraSize.height);
        }
        if (backCameraSize != null) {
            Log.i(TAG, "back : " + backCameraSize.width + "x" + backCameraSize.height);
        }

        enjoyFrontCameraSupportSizeList = selectEnjoySizeList(frontCameraSize);
        enjoyBackCameraSupportSizeList = selectEnjoySizeList(backCameraSize);
    }

    public boolean supportFrontCamera() {
        return frontCameraID != -1;
    }

    public boolean supportBackCamera() {
        return backCameraID != -1;
    }

    public boolean supportCamera(int cameraType) {
        if (cameraType == Camera.CameraInfo.CAMERA_FACING_BACK) {
            return supportBackCamera();
        } else if (cameraType == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            return supportFrontCamera();
        }
        return false;
    }

    public int getFrontCameraID() {
        return frontCameraID;
    }

    public int getBackCameraID() {
        return backCameraID;
    }

    public int getCameraID(int cameraType) {
        if (cameraType == Camera.CameraInfo.CAMERA_FACING_BACK) {
            return getBackCameraID();
        } else if (cameraType == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            return getFrontCameraID();
        }
        return -1;
    }

    public CameraVideoStream.CameraSize getFrontCameraSize() {
        return frontCameraSize;
    }

    public CameraVideoStream.CameraSize getBackCameraSize() {
        return backCameraSize;
    }

    public CameraVideoStream.CameraSize getCameraSize(int cameraType) {
        if (cameraType == Camera.CameraInfo.CAMERA_FACING_BACK) {
            return getBackCameraSize();
        } else {
            return getFrontCameraSize();
        }
    }

    public ArrayList<CameraVideoStream.CameraSize> getFrontCameraSupportSizeList() {
        return enjoyFrontCameraSupportSizeList;
    }

    public ArrayList<CameraVideoStream.CameraSize> getBackCameraSupportSizeList() {
        return enjoyBackCameraSupportSizeList;
    }

    public ArrayList<CameraVideoStream.CameraSize> getCameraSupportSizeList(int cameraType) {
        if (cameraType == Camera.CameraInfo.CAMERA_FACING_BACK) {
            return getBackCameraSupportSizeList();
        } else {
            return getFrontCameraSupportSizeList();
        }
    }

    private int getCameraID_(int cameraFacing) {
        int cameraID = -1;
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; ++i) {
            Camera.CameraInfo info = new Camera.CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == cameraFacing) {
                cameraID = i;
                Log.i(TAG, "find camera of facing = " + cameraFacing + " with id = " + cameraID);
                break;
            }
        }
        if (cameraFacing == -1) {
            Log.e(TAG, "cannot find camera of facing = " + cameraFacing);
        }
        return cameraID;
    }

    private List<Camera.Size> getSupportedPreviewSizes(int cameraID) {
        Camera camera = null;
        try {
            camera = Camera.open(cameraID);
        } catch (Exception e) {
            Log.e(TAG, "open camera" + cameraID + " failed.");
            return null;
        }

        if (camera == null) {
            return null;
        }
        Camera.Parameters params = camera.getParameters();
        List<Camera.Size> sizeList = new ArrayList<Camera.Size>(params.getSupportedPreviewSizes());
        for (Camera.Size size: sizeList
                ) {
            Log.i(TAG, size.width + "   " + size.height);
        }
        camera.release();
        camera = null;
        return sizeList;
    }

    private CameraVideoStream.CameraSize selectCameraSize(List<Camera.Size> orgSizeList) {
        if (orgSizeList == null) {
            return null;
        }
        CameraVideoStream.CameraSize cameraSize = new CameraVideoStream.CameraSize();
        Comparator<Camera.Size> comparator= new Comparator<Camera.Size>() {

            @Override
            public int compare(Camera.Size a, Camera.Size b) {
                // Enjoy 16:9
                int aDiff = Math.abs((a.width / 16) * 9 - a.height);
                int bDiff = Math.abs((b.width / 16) * 9 - b.height);
//				Log.i(TAG, "a = " + a.width + "x" + a.height + "diff=" + aDiff);
//				Log.i(TAG, "b = " + b.width + "x" + b.height + "diff=" + bDiff);
                if (aDiff < bDiff) {
                    return -1;
                } else if (aDiff > bDiff) {
                    return 1;
                } else if (a.height < b.height && a.height >= 720) {
                    return -1;
                }
                return 0;
            }

        };
        Collections.sort(orgSizeList, comparator);
        cameraSize.width = orgSizeList.get(0).width;
        cameraSize.height = orgSizeList.get(0).height;
        return cameraSize;
    }

    private ArrayList<CameraVideoStream.CameraSize> selectEnjoySizeList(CameraVideoStream.CameraSize cameraSize) {
        if (cameraSize == null) {
            return null;
        }
        ArrayList<CameraVideoStream.CameraSize> retSizeList = new ArrayList<CameraVideoStream.CameraSize>();

        int width = 0, hegiht = 0;
        for (int i = 0; i < enjoyHeightArray.length; i++) {
            if (cameraSize.height < enjoyHeightArray[i]) {
                break;
            }
            hegiht = enjoyHeightArray[i];
            width = cameraSize.width * hegiht / cameraSize.height;
            if (width % 2 != 0) {
                width ++;
            }
            retSizeList.add(new CameraVideoStream.CameraSize(width, hegiht));
        }

        return retSizeList;
    }
}
