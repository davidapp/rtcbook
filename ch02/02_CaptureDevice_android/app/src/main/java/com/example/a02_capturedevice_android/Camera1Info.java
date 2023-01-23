package com.example.a02_capturedevice_android;

import android.util.Log;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;

public class Camera1Info {
    public static void printDevice() {
        int nCameraCount = Camera.getNumberOfCameras();
        Log.i("Camera1", "Count: " +new Integer(nCameraCount).toString());
        CameraInfo info = new CameraInfo();
        for (int i = 0; i < nCameraCount; i++) {
            Camera.getCameraInfo(i, info);
            Log.i("Camera1", "facing: " + new Integer(info.facing).toString());
            Log.i("Camera1", "orientation:" + new Integer(info.orientation).toString());
        }
    }
}
