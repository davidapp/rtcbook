package com.example.a04_camara1capture;

import android.util.Log;

import java.util.concurrent.TimeUnit;

public class JavaTimer {
    public static void Start(int index) {
        time_record[index] = System.nanoTime();
    }

    public static long Stop(int index) {
        long nowTime = System.nanoTime();
        long lastTime = time_record[index];
        long durationInMs = TimeUnit.NANOSECONDS.toMillis((nowTime - lastTime));
        time_record[index] = durationInMs;
        return durationInMs;
    }

    public static void Print(int index) {
        String logStr = String.format("slot[%d] = %d us", index, time_record[index]);
        Log.i("JavaTimer", logStr);
    }

    public static long [] time_record = new long[100];
}
