package com.example.a04_camara1capture;

import android.util.Log;

import java.util.concurrent.TimeUnit;

public class JavaTimer {
    public static void Start(int index) {
        time_record[index] = System.nanoTime();
    }

    public static void Stop(int index) {
        if (time_record[index] == 0) return;
        long nowTime = System.nanoTime();
        long durationInUs = TimeUnit.NANOSECONDS.toMicros((nowTime - time_record[index]));
        time_record[index] = durationInUs;
    }

    public static void Print(int index) {
        if (time_record[index] == 0) return;
        String logStr = String.format("slot[%d] = %d us", index, time_record[index]);
        Log.i("JavaTimer", logStr);
        time_record[index] = 0;
    }

    public static long [] time_record = new long[100];
}
