//
//  IOSOCCamera.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/28/2021 for Dream.
//  Copyright 2021. All rights reserved.
//
#import <AVFoundation/AVFoundation.h>


@interface IOSOCCamera : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithHanlder:(void*)callback;

//设置输入设备是哪个 [capture_session_ addInput] + AVCaptureDeviceInput + current_device_
- (BOOL)set_camera_device_by_unique_id:(NSString*)unique_id;

//设置输入设备的分辨率 帧率分辨是多少 [current_device_ setActiveFormat]
- (void)config_capture_width:(int)w height:(int)h frate:(int)fr;

//设置输出格式是什么 [capture_session_ addOutput] AVCaptureVideoDataOutput
- (BOOL)set_output_format:(int)format;

// [capture_session_ startRunning]
- (BOOL)start;

// [capture_session_ stopRunning]
- (BOOL)stop;

//capture_connection_.enabled = NO
- (BOOL)pause;

//capture_connection_.enabled = YES
- (BOOL)resume;


@end
