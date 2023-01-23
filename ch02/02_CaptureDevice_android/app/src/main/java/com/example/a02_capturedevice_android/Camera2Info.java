package com.example.a02_capturedevice_android;

import android.content.Context;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.util.AndroidException;
import android.util.Log;

public class Camera2Info {
    static public void printDevice(Context context) {
        CameraManager cm = (CameraManager)context.getSystemService(Context.CAMERA_SERVICE);
        try {
            String[] ids = cm.getCameraIdList();
            Log.i("Camera2", "Count: " +new Integer(ids.length).toString());
            for (String id_name : ids)
            {
                Log.i("Camera2", "name:" + id_name);
                CameraCharacteristics ch = cm.getCameraCharacteristics(id_name);
                if (ch.get(CameraCharacteristics.LENS_FACING) == CameraMetadata.LENS_FACING_BACK) {
                    Log.i("Camera2", "facing: FACING_BACK");
                }
                else if (ch.get(CameraCharacteristics.LENS_FACING) == CameraMetadata.LENS_FACING_FRONT) {
                    Log.i("Camera2", "facing: FACING_FRONT");
                }
                Log.i("Camera2", "SENSOR_ORIENTATION:" + new Integer(ch.get(CameraCharacteristics.SENSOR_ORIENTATION)).toString());
            }
        }
        catch (AndroidException e) {

        }
    }
}
