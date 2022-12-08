#import "DIOSCamera.h"
#import <AVFoundation/AVFoundation.h>

DVoid DIOSCamera::PrintAll()
{
    NSArray<AVCaptureDevice *> *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    DUInt32 nCount = (DUInt32)[devices count];
    for (DUInt32 i=0;i<nCount;i++) {
        AVCaptureDevice *dev = [devices objectAtIndex:i];
        printf("[localizedName]%s\n",dev.localizedName.UTF8String);
        printf("[uniqueID]%s\n",dev.uniqueID.UTF8String);
        printf("[position]%d\n",(DInt32)dev.position);
    }
}
