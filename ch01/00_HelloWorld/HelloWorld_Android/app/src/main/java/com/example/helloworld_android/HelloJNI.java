package com.example.helloworld_android;

public class HelloJNI {
    public native void HelloNative();
    public void Hello() {
        HelloNative();
    }
}
