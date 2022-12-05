//
//  IOSOCCamera.mm
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/28/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#import "IOSOCCamera.h"
#include "IOSCamera.h"

@implementation IOSOCCamera {
    //iOS采集的过程，主要靠AVCaptureSession
    //canAddInput AddInput inputs AVCaptureDeviceInput
    //canAddOutput addOutput outputs AVCaptureVideoDataOutput
    //beginConfiguration commitConfiguration
    //startRunning stopRunning
    AVCaptureSession* capture_session_;
    
    //背后执行与回调采集数据的线程
    dispatch_queue_t capture_queue_;
    
    //保证操作的同步性
    //lock unlock wait signal
    NSCondition* capture_change_condition_;
    BOOL capture_is_changing_;
    
    AVCaptureDevice* current_device_;
    AVCaptureConnection* capture_connection_;
    std::atomic<BOOL> capture_started_;

    int m_format;
    int m_width;
    int m_height;
    int m_fps;
    
    DVideoCaptureCallback* m_CapCallback;
}

- (id)initWithHanlder:(void*)callback {
    if (self = [super init]) {
        capture_session_ = [[AVCaptureSession alloc] init];
        capture_queue_ = dispatch_queue_create("VideoCaptureObjC", DISPATCH_QUEUE_SERIAL);

        capture_change_condition_ = [[NSCondition alloc] init];
        capture_is_changing_ = NO;
 
        if (!capture_session_ || !capture_change_condition_) {
            return nil;
        }
        
        m_CapCallback = (DVideoCaptureCallback*)callback;
        
        //m_format = kCVPixelFormatType_32RGBA;
        m_format = kCVPixelFormatType_32BGRA;
        [self set_output_format:m_format];
    }
    return self;
}

// 设置输入设备，保存到 current_device_ 中
- (BOOL)set_camera_device_by_unique_id:(NSString*)unique_id {
    [self wait_capture_change];

    // 移除当前现有的输入源
    NSArray* current_inputs = [capture_session_ inputs];
    if ([current_inputs count] > 0) {
        AVCaptureInput* input = (AVCaptureInput*)[current_inputs objectAtIndex:0];
        [capture_session_ removeInput:input];
    }

    // 按 DeviceID 去找到对应的 AVCaptureDevice 10.15用AVCaptureDeviceDiscoverySession
    AVCaptureDevice* target_device = nil;
    if (@available(macOS 10.15, *)) {
        AVCaptureDeviceDiscoverySession *session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera] mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];
        for (AVCaptureDevice* device in session.devices) {
            if ([unique_id isEqual:device.uniqueID]) {
                target_device = device;
            }
        }
    }
    else {
        for (AVCaptureDevice* device in [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]) {
            if ([unique_id isEqual:device.uniqueID]) {
                target_device = device;
            }
        }
    }

    // 如果没有找到就返回
    if (!target_device) {
        return NO;
    }

    // 找到后，就赋值给 current_device_
    current_device_ = target_device;

    // 构造一个 AVCaptureDeviceInput
    NSError* device_error = nil;
    AVCaptureDeviceInput* new_capture_input =
        [AVCaptureDeviceInput deviceInputWithDevice:target_device error:&device_error];

    if (!new_capture_input) {
        return NO;
    }

    // 将Input添加给 capture_session_
    [capture_session_ beginConfiguration];

    BOOL addedCaptureInput = NO;
    if ([capture_session_ canAddInput:new_capture_input]) {
        [capture_session_ addInput:new_capture_input];
        addedCaptureInput = YES;
    } else {
        addedCaptureInput = NO;
    }

    [capture_session_ commitConfiguration];

    [self signal_capture_change];

    return addedCaptureInput;
}

//设置输入设备的分辨率 帧率分辨是多少 [current_device_ setActiveFormat]
- (void)config_capture_width:(int)w height:(int)h frate:(int)fr {
    if (current_device_ == nil) {
        return;
    }
    
    // 分辨率有效范围
    if (w < 0 || h < 0 || w > 4000 || h > 4000) {
        return;
    }

    // 帧率有效范围
    if (fr < 0 || fr > 60) {
        return;
    }

    if ([current_device_ lockForConfiguration:nil] != YES) {
        return;
    }
    
    //设置分辨率，找一个最接近的
    AVCaptureDeviceFormat* best_format = [current_device_ formats][0];
    for (AVCaptureDeviceFormat* format in [current_device_ formats]) {
        CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(
            (CMVideoFormatDescriptionRef)[format formatDescription]);
        if ((dimensions.width >= w) && (dimensions.height >= h)) {
            best_format = format;
            m_width = dimensions.width;
            m_height = dimensions.height;
            break;
        }
    }

    [current_device_ setActiveFormat:best_format];

    // 找一个该分辨率下，接近最大的帧率
    BOOL is_fps_supported = NO;
    AVCaptureDeviceFormat* current_format = [current_device_ activeFormat];
    AVFrameRateRange* range;
    int32_t max_fps_supported = 0;
    for (range in current_format.videoSupportedFrameRateRanges) {
        if (range.maxFrameRate > max_fps_supported) {
            max_fps_supported = range.maxFrameRate;
            m_fps = range.maxFrameRate;
        }
        
        if (range.maxFrameRate >= fr) {
            is_fps_supported = YES;
            break;
        }
    }

    // 如果对应分辨率下，不支持设置的最大帧率，就用该分辨率下的最大帧率
    if (!is_fps_supported) {
        m_fps = max_fps_supported;
    }
    
    //设置帧率靠这个
    //CMTime time = CMTimeMake(1, 5);
    //[current_device_ setActiveVideoMinFrameDuration:time];
    //[current_device_ setActiveVideoMaxFrameDuration:time];

    current_device_.activeVideoMinFrameDuration = (CMTime){
        .value = 1,
        .timescale = fr,
        .flags = kCMTimeFlags_Valid,
        .epoch = 0,
    };
    
    current_device_.activeVideoMaxFrameDuration = (CMTime){
        .value = 1,
        .timescale = fr,
        .flags = kCMTimeFlags_Valid,
        .epoch = 0,
    };
    
    [current_device_ unlockForConfiguration];
}

// 设置输出的像素格式
- (BOOL)set_output_format:(int)format {
    if (!capture_session_ || !capture_change_condition_) {
        return NO;
    }
    
    //需要采集的像素格式 通过 AVCaptureVideoDataOutput 指定
    AVCaptureVideoDataOutput* capture_output = [[AVCaptureVideoDataOutput alloc] init];
    NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey;
    NSNumber* value = [NSNumber numberWithUnsignedInt:format];
    NSDictionary* video_settings = [NSDictionary dictionaryWithObject:value forKey:key];
    capture_output.videoSettings = video_settings;

    if ([capture_session_ canAddOutput:capture_output]) {
        [capture_session_ addOutput:capture_output];
        m_format = format;
    }
    else {
        return NO;
    }
    
    return YES;
}

// 要启动摄像头，就要设置好输入和输出
- (BOOL)start {
    [self wait_capture_change];

    if (!capture_session_) {
        return NO;
    }

    // 注册回调通知的对象与线程
    AVCaptureVideoDataOutput* current_output = [[capture_session_ outputs] firstObject];
    if (!current_output) {
        return NO;
    }
    [current_output setSampleBufferDelegate:self queue:capture_queue_];
    
    capture_started_ = YES;
    
    // 到采集线程去启动采集任务
    dispatch_async(capture_queue_, ^{
        [self start_capture_with_output_in_background:current_output];
    });

    return YES;
}

- (void)start_capture_with_output_in_background:(AVCaptureVideoDataOutput*)current_output {
    [capture_session_ beginConfiguration];
    
    capture_connection_ = [current_output connectionWithMediaType:AVMediaTypeVideo];
    [self config_capture_width:1280 height:720 frate:1];
    
    [capture_session_ commitConfiguration];
    
    [capture_session_ startRunning];
    [self signal_capture_change];
}

- (BOOL)stop {
    [self wait_capture_change];

    [[[capture_session_ outputs] firstObject] setSampleBufferDelegate:nil queue:nil];

    if (!capture_connection_) {
        return NO;
    }

    capture_started_ = NO;
    
    dispatch_async(capture_queue_, ^(void) {
        [self stop_capture_in_background];
    });
    return YES;
}

- (void)stop_capture_in_background {
    [capture_session_ stopRunning];
    [self signal_capture_change];
}

- (BOOL)pause {
    capture_connection_.enabled = NO;
    return YES;
}

- (BOOL)resume {
    capture_connection_.enabled = YES;
    return YES;
}

// start_camera 之前调用，防止方法重入
- (void)wait_capture_change {
    [capture_change_condition_ lock];

    while (capture_is_changing_) {
        [capture_change_condition_ wait];
    }
    capture_is_changing_ = YES;

    [capture_change_condition_ unlock];
}

- (void)signal_capture_change {
    [capture_change_condition_ lock];

    capture_is_changing_ = NO;

    [capture_change_condition_ signal];
    [capture_change_condition_ unlock];
}

// 回调函数
- (void)captureOutput:(AVCaptureOutput*)capture_output
    didOutputSampleBuffer:(CMSampleBufferRef)sample_buffer
           fromConnection:(AVCaptureConnection*)connection {
    if (!capture_started_) {
        return;
    }
    
    //从 CMSampleBufferRef 拿到一个 CVImageBufferRef
    CVImageBufferRef video_frame = CMSampleBufferGetImageBuffer(sample_buffer);
    
    size_t width = CVPixelBufferGetWidth(video_frame);
    size_t height = CVPixelBufferGetHeight(video_frame);
    OSType cvtype = CVPixelBufferGetPixelFormatType(video_frame); //BGRA
    CVPixelBufferLockBaseAddress(video_frame, kCVPixelBufferLock_ReadOnly);
    void *pBuf = CVPixelBufferGetBaseAddress(video_frame);
    CVPixelBufferUnlockBaseAddress(video_frame, kCVPixelBufferLock_ReadOnly);
    size_t datasize = CVPixelBufferGetDataSize(video_frame);
       
    DData *pData = DData::CreateDData(4);
    pData->AddUInt32(0, cvtype);
    pData->AddUInt32(1, (DUInt32)width);
    pData->AddUInt32(2, (DUInt32)height);
    DBuffer bufImage(pBuf, (DUInt32)datasize);
    pData->AddBuffer(3, bufImage);
    if (m_CapCallback) {
        m_CapCallback->OnBufferCaptured(pData);
    }
}

@end
