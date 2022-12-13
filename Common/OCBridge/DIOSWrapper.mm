#import "DIOSWrapper.h"
#include "DIOSCamera.h"

@implementation DIOSWrapper

+ (void)printAllVideoDevice {
    DIOSCamera::PrintAll();
}


@end
