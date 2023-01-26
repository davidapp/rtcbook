package com.example.a04_camara1capture;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.List;
import java.util.Objects;

public class Camera1Capture {
    android.hardware.Camera camera;
    android.hardware.Camera.CameraInfo info;
    Handler cameraThreadHandler;
    SurfaceHolder m_holder;
    HandlerThread cameraThread;
    boolean bRunning = false;
    int nCurDevID = 0;

    public void create(int cameraid) {
        camera = android.hardware.Camera.open(cameraid);
        info = new android.hardware.Camera.CameraInfo();
        android.hardware.Camera.getCameraInfo(cameraid, info);
        final android.hardware.Camera.Parameters parameters = camera.getParameters();
        parameters.setPreviewFpsRange(15*1000, 15*1000);
        parameters.setPreviewSize(640, 480);
        parameters.setPreviewFormat(ImageFormat.NV21); // default
        camera.setParameters(parameters);
        nCurDevID = cameraid;
    }

    public void start() {
        bRunning = true;
        camera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                JavaTimer.Stop(0);
                JavaTimer.Print(0);
                cameraThreadHandler.post(() -> {
                    // size = width * height * 3 / 2;
                    Log.i("camera", "a frame receiverd");
                });
            }
        });
        cameraThread = new HandlerThread("captureThread");
        cameraThread.start();
        cameraThreadHandler = new Handler(cameraThread.getLooper());
        camera.startPreview();
    }

    public void stop() {
        bRunning = false;
        camera.setPreviewCallback(null);
        camera.stopPreview();
        cameraThread.quit();
    }

    public void setPreviewDisplay(SurfaceHolder holder) {
        try {
            camera.setPreviewDisplay(holder);
            m_holder = holder;
        }
        catch (Exception e) {

        }
    }

    public void switchDev(int cameraid) {
        if (nCurDevID == cameraid) return;
        if (bRunning) {
            stop();
            camera.release();
            create(cameraid);
            setPreviewDisplay(m_holder);
            start();
        }
        else {
            nCurDevID = cameraid;
        }
    }

    public void setCameraDisplayOrientation(Activity activity) {
        android.hardware.Camera.CameraInfo info =
                new android.hardware.Camera.CameraInfo();
        android.hardware.Camera.getCameraInfo(nCurDevID, info);
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0: degrees = 0; break;
            case Surface.ROTATION_90: degrees = 90; break;
            case Surface.ROTATION_180: degrees = 180; break;
            case Surface.ROTATION_270: degrees = 270; break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        camera.setDisplayOrientation(result);
    }
}
