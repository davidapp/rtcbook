#import "DMacCamera.h"
#import <AVFoundation/AVFoundation.h>

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
            printf("[size](%d*%d)\n",dimensions.width,dimensions.height);
            NSArray<AVFrameRateRange*>* fpsArr = format.videoSupportedFrameRateRanges;
            NSLog(@"%@", fpsArr);
        }
    }
}


