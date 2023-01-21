//
//  ViewController.m
//  02_AudioDevice_ios
//
//  Created by bytedance on 2023/1/16.
//

#import "ViewController.h"
#import <AVFoundation/AVFoundation.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
for (AVCaptureDevice *device in devices) {
    printf("%s\n", [[device localizedName] UTF8String]);
    }
}


@end
