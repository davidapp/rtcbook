//
//  MacOCCamera.h
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/20/2021 for Dream.
//  Copyright 2021. All rights reserved.
//
#import <AVFoundation/AVFoundation.h>


@interface MacOCCamera : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id)initWithHanlder:(void*)callback;

- (void)destroy;

// 当前有哪些采集设备
+ (NSArray<AVCaptureDevice*>*) allDevices;
// 选择默认的采集设备
- (BOOL)set_default_device;
// 用ID来指定采集设备
- (BOOL)set_camera_device_by_unique_id:(NSString*)unique_id;
// 获取当前选中的设备
- (AVCaptureDevice*) get_current_device;


// 当前采集设备，支持哪些采样配置
- (NSArray<AVCaptureDeviceFormat*>*) allSupportFormats;
// 选哪个分辨率
- (void)config_capture_width:(int)w height:(int)h;
// 获取当前选中的格式
- (AVCaptureDeviceFormat*) get_current_format;
// 当前设置的宽是多少
- (int) get_current_width;
// 当前设置的高是多少
- (int) get_current_height;


// 对应分辨率下支持的帧率有哪些
- (NSArray<AVFrameRateRange*>*) allSupportFrameRates: (AVCaptureDeviceFormat*)format;
// 设置输入设备的分辨率 帧率分辨是多少
- (void)config_capture_frate:(int)fr;
// 当前分辨率是多少
- (int) get_current_framerate;


// 当前采集设备，支持哪些输出像素格式
- (NSArray<NSNumber*>*) allSupportPixelFormats;
// 设置像素输出格式是什么
- (BOOL)set_output_format:(int)format;
// 当前输出像素格式是什么
- (int) get_current_pixel_format;


// 启动摄像头
- (BOOL)start;
// 停止摄像头
- (BOOL)stop;
// 是否正在采集
- (BOOL)isStarting;
// 暂停采集
- (BOOL)pause;
// 恢复采集
- (BOOL)resume;

// 性能统计
- (void)perf;
@end
