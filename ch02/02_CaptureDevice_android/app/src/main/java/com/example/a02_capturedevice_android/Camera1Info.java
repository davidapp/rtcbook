package com.example.a02_capturedevice_android;

import android.graphics.ImageFormat;
import android.util.Log;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;

import java.util.List;

public class Camera1Info {
    public static void printDevice() {
        int nCameraCount = Camera.getNumberOfCameras();
        Log.i("Camera1", "Count: " +new Integer(nCameraCount).toString());
        CameraInfo info = new CameraInfo();
        for (int i = 0; i < nCameraCount; i++) {
            Camera.getCameraInfo(i, info);
            Log.i("Camera1", "facing: " + new Integer(info.facing).toString());
            Log.i("Camera1", "orientation:" + new Integer(info.orientation).toString());

            android.hardware.Camera camera = android.hardware.Camera.open(i);
            android.hardware.Camera.Parameters paras = camera.getParameters();
            for (android.hardware.Camera.Size size : paras.getSupportedPictureSizes()) {
                //size.width, size.height; 宽度和高度
                String logStr = String.format("size: %d*%d ", size.width, size.height);
                Log.i("Camera1", logStr);
            }
            List<int[]> listFpsRange = paras.getSupportedPreviewFpsRange(); // 帧率*/
            for (int j = 0; j < listFpsRange.size(); j++)
            {
                int[] fps = listFpsRange.get(j);
                Log.i("Camera1", String.format("fps: min=%d,max=%d",fps[0],fps[1]));
            }
            //List<Integer> formats = paras.getSupportedPictureFormats();
            List<Integer> formats = paras.getSupportedPreviewFormats();
            for (int k = 0; k < formats.size(); k++)
            {
                int format = formats.get(k);
                Log.i("Camera1", String.format("format[%d]=%d: ", k, format) + GetFormatStr(format));
            }

            camera.release();
        }
    }
    public static String GetFormatStr(int format) {
        if (format == ImageFormat.UNKNOWN) return "UNKNOWN";
        else if (format == ImageFormat.RGB_565) return "RGB_565";
        else if (format == ImageFormat.YV12) return "YV12";
        else if (format == ImageFormat.Y8) return "Y8";
        //else if (format == ImageFormat.Y16) return "Y16";
        else if (format == ImageFormat.NV16) return "NV16";
        else if (format == ImageFormat.NV21) return "NV21";
        else if (format == ImageFormat.YUY2) return "YUY2";
        else if (format == ImageFormat.JPEG) return "JPEG";
        else if (format == ImageFormat.DEPTH_JPEG) return "DEPTH_JPEG";
        else if (format == ImageFormat.YUV_420_888) return "YUV_420_888";
        else if (format == ImageFormat.YUV_422_888) return "YUV_422_888";
        else if (format == ImageFormat.YUV_444_888) return "YUV_444_888";
        else if (format == ImageFormat.FLEX_RGB_888) return "FLEX_RGB_888";
        else if (format == ImageFormat.FLEX_RGBA_8888) return "FLEX_RGBA_8888";
        else if (format == ImageFormat.RAW_SENSOR) return "RAW_SENSOR";
        else if (format == ImageFormat.RAW_PRIVATE) return "RAW_PRIVATE";
        else if (format == ImageFormat.RAW10) return "RAW10";
        else if (format == ImageFormat.RAW12) return "RAW12";
        else if (format == ImageFormat.DEPTH16) return "DEPTH16";
        else if (format == ImageFormat.DEPTH_POINT_CLOUD) return "DEPTH_POINT_CLOUD";
        //else if (format == ImageFormat.RAW_DEPTH) return "RAW_DEPTH";
        //else if (format == ImageFormat.RAW_DEPTH10) return "RAW_DEPTH10";
        else if (format == ImageFormat.PRIVATE) return "PRIVATE";
        else if (format == ImageFormat.HEIC) return "HEIC";
        return "UNKNOWN";
    }
}
