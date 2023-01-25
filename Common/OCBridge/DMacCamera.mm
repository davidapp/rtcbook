#import "DMacCamera.h"
#import <AVFoundation/AVFoundation.h>
#include <string>

std::string fourccToStr(FourCharCode code) {
    const char* pc = (const char*)&code;
    std::string ret;
    ret += pc[3];
    ret += pc[2];
    ret += pc[1];
    ret += pc[0];
    return ret;
}

DVoid DMacCamera::PrintAll()
{
    NSArray<AVCaptureDevice *> *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    DUInt32 nCount = (DUInt32)[devices count];
    for (DUInt32 i=0;i<nCount;i++) {
        AVCaptureDevice *dev = [devices objectAtIndex:i];
        printf("[localizedName]%s\n",dev.localizedName.UTF8String);
        printf("[uniqueID]%s\n",dev.uniqueID.UTF8String);
        printf("[position]%d\n",(DInt32)dev.position);
        
        for (AVCaptureDeviceFormat* format in [dev formats]) {
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(
                (CMVideoFormatDescriptionRef)[format formatDescription]);
            printf("[size](%d*%d)",dimensions.width,dimensions.height);

            OSType pixelFormat = CMFormatDescriptionGetMediaSubType(format.formatDescription);
            printf("[format](%s)",fourccToStr(pixelFormat).c_str());
            
            for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
                printf("[fps](%f-%f)\n", range.minFrameRate, range.maxFrameRate);
            }
        }
    }
}


