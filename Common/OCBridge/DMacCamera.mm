//
//  MacCamera.mm
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/20/2021 for Dream.
//  Copyright 2021. All rights reserved.
//


#import "MacCamera.h"
#import "MacOCCamera.h"
#include "Base/DString.h"
#include "Base/DBuffer.h"

MacOCCamera *g_camera;

// MacCamera
int MacCamera::start(DVideoCaptureCallback* callback) {
    g_camera = [[MacOCCamera alloc] initWithHanlder:callback];
    [g_camera start];
    return 0;
}

void MacCamera::stop() {

}

DData* AVCaptureDeviceFormatToDData(AVCaptureDeviceFormat* format) {
    DData* pItem = DData::CreateDData(20);
    
    DStringA mtName([format.mediaType UTF8String]);
    pItem->AddStringA(0, mtName);
    
    CMFormatDescriptionRef desc = format.formatDescription;
    CMVideoDimensions dimension = CMVideoFormatDescriptionGetDimensions(desc);
    pItem->AddUInt32(1, dimension.width);
    pItem->AddUInt32(2, dimension.height);
    
    DArray* pRange = DArray::CreateDArray();
    NSArray<AVFrameRateRange *>* frRanges = format.videoSupportedFrameRateRanges;
    for (AVFrameRateRange* range in frRanges) {
        DData *pCaps = DData::CreateDData(5);
        pCaps->AddDouble(1, range.minFrameRate);
        pCaps->AddDouble(2, range.maxFrameRate);
        pCaps->AddDouble(3, range.minFrameDuration.value);
        pCaps->AddDouble(4, range.maxFrameDuration.value);
        pRange->AddData(pCaps);
        pCaps->Release();
    }
    //pItem->AddArray(3, pRange);
    pRange->Release();
    
    //skip 14 ios properties
    
    if (@available(macOS 10.15, *)) {
        pItem->AddUInt32(4, (DUInt32)format.autoFocusSystem);   //AVCaptureAutoFocusSystemNone=0
        DArray* pCS = DArray::CreateDArray();
        NSArray<NSNumber *> *scs = format.supportedColorSpaces;
        for (NSNumber* cs in scs) {
            pCS->AddInt32(cs.intValue);
        }
        pItem->AddArray(5, pCS);
        pCS->Release();
    }
    
    //skip 7 ios properties
    return pItem;
}

DArray* MacCamera::GetVideoDevices() {
    NSArray<AVCaptureDevice*>* all_devices = [MacOCCamera allDevices];
    DULong device_count = [all_devices count];
    DArray *pRet = DArray::CreateDArray();
    for (int i = 0; i < device_count; i++) {
        DData* pItem = DData::CreateDData(30);
        pItem->AddInt8(DEVICE_PLATFORM_KEY, DEVICE_PLATFORM_MAC);
        
        AVCaptureDevice* device = [all_devices objectAtIndex:i];
        if (i == 0) {
            pItem->AddInt8(VIDEO_DEVICE_POSITION_KEY, VIDEO_DEVICE_POSITION_FRONT);
        } else {
            pItem->AddInt8(VIDEO_DEVICE_POSITION_KEY, VIDEO_DEVICE_POSITION_EXTERNAL);
        }
        
        DStringA did([device.uniqueID UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_UID_KEY, did);
                
        DStringA mid([device.modelID UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_MODELID_KEY, mid);
        
        DStringA dname([device.localizedName UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_NAME_KEY, dname);

        DStringA manu([device.manufacturer UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_MANU_KEY, manu);

        pItem->AddBool(7, device.connected);
        pItem->AddBool(8, device.inUseByAnotherApplication);
        pItem->AddBool(9, device.suspended);
        
        DData* pActiveFormatData = AVCaptureDeviceFormatToDData(device.activeFormat);
        pItem->AddData(10, pActiveFormatData);
        pActiveFormatData->Release();

        pRet->AddData(pItem);
        pItem->Release();
    }
    return pRet;
}

DArray* MacCamera::GetVideoDevicesAll() {
    NSArray<AVCaptureDevice*>* all_devices = [MacOCCamera allDevices];;
    DULong device_count = [all_devices count];
    DArray *pRet = DArray::CreateDArray();
    for (int i = 0; i < device_count; i++) {
        DData* pItem = DData::CreateDData(30);
        pItem->AddInt8(DEVICE_PLATFORM_KEY, DEVICE_PLATFORM_MAC);
        
        AVCaptureDevice* device = [all_devices objectAtIndex:i];
        if (i == 0) {
            pItem->AddInt8(VIDEO_DEVICE_POSITION_KEY, VIDEO_DEVICE_POSITION_FRONT);
        } else {
            pItem->AddInt8(VIDEO_DEVICE_POSITION_KEY, VIDEO_DEVICE_POSITION_EXTERNAL);
        }
        
        DStringA did([device.uniqueID UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_UID_KEY, did);
                
        DStringA mid([device.modelID UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_MODELID_KEY, mid);
        
        DStringA dname([device.localizedName UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_NAME_KEY, dname);

        DStringA manu([device.manufacturer UTF8String]);
        pItem->AddStringA(VIDEO_DEVICE_MANU_KEY, manu);
        
        pItem->AddInt32(VIDEO_DEVICE_TTYPE_KEY, device.transportType);
        pItem->AddBool(7, device.connected);
        pItem->AddBool(8, device.inUseByAnotherApplication);
        pItem->AddBool(9, device.suspended);
        
        DArray *pLDevices = DArray::CreateDArray();
        for (AVCaptureDevice* dev in device.linkedDevices) {
            DStringA devname([dev.localizedName UTF8String]);
            pLDevices->AddStringA(devname);
        }
        pItem->AddArray(10, pLDevices);
        pLDevices->Release();
       
        DArray *pFormats = DArray::CreateDArray();
        for (AVCaptureDeviceFormat* format in device.formats) {
            DData *pFormat = AVCaptureDeviceFormatToDData(format);
            pFormats->AddData(pFormat);
            pFormat->Release();
        }
        pItem->AddArray(11, pFormats);
        pFormats->Release();
        
        DData* pActiveFormatData = AVCaptureDeviceFormatToDData(device.activeFormat);
        pItem->AddData(12, pActiveFormatData);
        pActiveFormatData->Release();
        
        pItem->AddUInt32(13, (DUInt32)device.activeVideoMinFrameDuration.value);
        pItem->AddUInt32(14, (DUInt32)device.activeVideoMaxFrameDuration.value);
        pItem->AddUInt32(15, (DUInt32)device.position);
        
        pItem->AddBool(16, device.hasFlash);
        pItem->AddBool(17, device.hasTorch);
        pItem->AddBool(18, device.adjustingFocus);
        pItem->AddBool(19, device.exposurePointOfInterestSupported);
        pItem->AddBool(20, device.adjustingExposure);
        pItem->AddBool(21, device.transportControlsSupported);

        pItem->AddUInt32(22, (DUInt32)device.torchMode);
        pItem->AddUInt32(23, (DUInt32)device.focusMode);
        pItem->AddUInt32(24, (DUInt32)device.focusPointOfInterestSupported);
    
        pItem->AddUInt32(25, (DUInt32)device.exposureMode);
        pItem->AddUInt32(26, (DUInt32)device.transportControlsPlaybackMode);
        pItem->AddDouble(27, (DUInt32)device.transportControlsSpeed);
        
        pItem->AddDouble(28, device.focusPointOfInterest.x);
        pItem->AddDouble(29, device.focusPointOfInterest.y);
        pItem->AddDouble(30, device.exposurePointOfInterest.x);
        pItem->AddDouble(31, device.exposurePointOfInterest.y);
        
        //NSArray<AVCaptureDeviceInputSource *> *inputSources
        //AVCaptureDeviceInputSource *activeInputSource
        
        //10.14 AVAuthorizationStatus
                
        if (@available(macOS 10.15, *)) {
            DStringA devType([device.deviceType UTF8String]);
            pItem->AddStringA(32, devType);
            pItem->AddBool(33, device.flashAvailable);
            pItem->AddBool(34, device.torchAvailable);
            pItem->AddBool(35, device.torchActive);
            pItem->AddDouble(36, device.torchLevel);
            pItem->AddUInt32(37, (DUInt32)device.activeColorSpace);
        }
        
        pRet->AddData(pItem);
        pItem->Release();
    }
    return pRet;
}

#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)
DVoid MacCamera::Test() {
    DArray* pDevices = MacCamera::GetVideoDevices();
    pDevices->Dump();
    pDevices->Release();
}
#endif
