//
//  IOSCamera.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/28/2021 for Dream.
//  Copyright 2021. All rights reserved.
//
#include "Data/DArray.h"
#include "Data/DData.h"
#include "Base/DBuffer.h"

class DVideoCaptureCallback {
public:
    virtual void OnBufferCaptured(DData* videoFrame) = 0;
};

#define DEVICE_PLATFORM_KEY 0
#define DEVICE_PLATFORM_UNKNOWN 0
#define DEVICE_PLATFORM_WIN 1
#define DEVICE_PLATFORM_MAC 2
#define DEVICE_PLATFORM_IOS 3
#define DEVICE_PLATFORM_ANDROID 4
#define DEVICE_PLATFORM_LINUX 5

#define VIDEO_DEVICE_POSITION_KEY 1
#define VIDEO_DEVICE_POSITION_UNKNOWN 0
#define VIDEO_DEVICE_POSITION_FRONT 1
#define VIDEO_DEVICE_POSITION_BACK 2
#define VIDEO_DEVICE_POSITION_EXTERNAL 3

#define VIDEO_DEVICE_UID_KEY 2
#define VIDEO_DEVICE_MODELID_KEY 3
#define VIDEO_DEVICE_NAME_KEY 4
#define VIDEO_DEVICE_MANU_KEY 5
#define VIDEO_DEVICE_TTYPE_KEY 6

#define VIDEO_DEVICE_FORMATS_KEY 9



class IOSCamera {
public:
    static int start(DVideoCaptureCallback* callback);
    static void stop();
    static DArray* GetVideoDevices(DBool withMisc=false);
    static DArray* GetVideoDevicesAll();

private:
    
#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)
public:
	static DVoid Test();
#endif
};
