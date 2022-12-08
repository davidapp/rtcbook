//
//  MacOCCamera.m
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 04/20/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#import "MacOCCamera.h"
#include "MacCamera.h"
#include "Base/DTime.h"
#include "Base/DTimer.h"

#define DEFAULT_CAPTURE_WIDTH 1280
#define DEFAULT_CAPTURE_HEIGHT 720
#define DEFAULT_FRAME_RATE 25
#define DEFAULT_PIXEL_FORMAT kCVPixelFormatType_32BGRA


DTimer perfTimer;
std::atomic<BOOL> g_bFirst(TRUE);
std::atomic<BOOL> g_bFrame(TRUE);
#define MAC_CAPTURE_INIT 1
#define MAC_CAPTURE_START 2
#define MAC_CAPTURE_FRAME 3

@implementation MacOCCamera {
    //Mac采集的过程，主要靠AVCaptureSession
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
    std::atomic<BOOL> capture_is_changing_;
    
    //回调指针
    DVideoCaptureCallback* m_userCapCallback;
    
    //用户设置进来的属性值
    int m_userFormat;
    int m_userWidth;
    int m_userHeight;
    int m_userFPS;
    
    //真实的属性值
    int m_devWidth;
    int m_devHeight;
    int m_devFPS;
    
    //当前的属性
    AVCaptureDevice* current_device_;
    AVCaptureConnection* capture_connection_;
    std::atomic<BOOL> capture_started_;
}

//返回所有的视频采集设备
+ (NSArray<AVCaptureDevice*>*) allDevices {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
     return [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
#pragma clang diagnostic pop
}

//初始化
- (id)initWithHanlder:(void*)callback {
    perfTimer.Start(MAC_CAPTURE_INIT);
    if (self = [super init]) {
        // 采集会话对象
        capture_session_ = [[AVCaptureSession alloc] init];
        if (!capture_session_) {
            return nil;
        }
        
        // 采集线程队列
        capture_queue_ = dispatch_queue_create("VideoCaptureObjC", DISPATCH_QUEUE_SERIAL);
        if (!capture_queue_) {
            return nil;
        }

        // 保证方法同步执行完
        capture_change_condition_ = [[NSCondition alloc] init];
        if (!capture_change_condition_) {
            return nil;
        }
        capture_is_changing_ = NO;
        
        // 设置回调
        m_userCapCallback = (DVideoCaptureCallback*)callback;
        
        // 将默认设备作为输入设备
        [self set_default_device];
        
        // 设置默认的分辨率
        [self config_capture_width:DEFAULT_CAPTURE_WIDTH height:DEFAULT_CAPTURE_HEIGHT];
        
        // 设置默认的帧率
        [self config_capture_frate:DEFAULT_FRAME_RATE];

        // 设置默认的输出格式
        [self set_output_format:DEFAULT_PIXEL_FORMAT];
    }
    
    perfTimer.Stop(MAC_CAPTURE_INIT);
    return self;
}

- (void)destroy {
    m_userCapCallback = nullptr;
    
    current_device_ = nil;
    capture_connection_ = nil;
    
    capture_change_condition_ = nil;
    capture_queue_ = nil;
    capture_session_ = nil;
}

- (BOOL)set_default_device {
    [self wait_capture_change];
    
    // 移除当前现有的输入源
    NSArray* current_inputs = [capture_session_ inputs];
    if ([current_inputs count] > 0) {
        AVCaptureInput* input = (AVCaptureInput*)[current_inputs objectAtIndex:0];
        [capture_session_ removeInput:input];
    }

    // 获取默认设备
    AVCaptureDevice* target_device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];

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
    return YES;
}

// 设置输入设备，保存到 current_device_ 中
// [capture_session_ addInput] + AVCaptureDeviceInput + current_device_
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        for (AVCaptureDevice* device in [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]) {
            if ([unique_id isEqual:device.uniqueID]) {
                target_device = device;
            }
        }
#pragma clang diagnostic pop
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

// 获取当前选中的设备
- (AVCaptureDevice*) get_current_device {
    return current_device_;
}

// 所有支持的格式
- (NSArray<AVCaptureDeviceFormat*>*) allSupportFormats {
    if (current_device_ == nil) {
        return nil;
    }
    return [current_device_ formats];
}

// 设置输入设备的分辨率是多少 [current_device_ setActiveFormat]
- (void)config_capture_width:(int)w height:(int)h{
    //设置格式之前，必选先设置输入设备
    if (current_device_ == nil) {
        return;
    }
    
    // 分辨率有效范围
    if (w < 0 || h < 0 || w > 4000 || h > 4000) {
        return;
    }

    m_userWidth = w;
    m_userHeight = h;

    //设置分辨率，找一个最接近的
    AVCaptureDeviceFormat* best_format = [current_device_ formats][0];
    for (AVCaptureDeviceFormat* format in [current_device_ formats]) {
        CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(
            (CMVideoFormatDescriptionRef)[format formatDescription]);
        // 宽和高都大于等于指定的，就认为合适
        if ((dimensions.width >= w) && (dimensions.height >= h)) {
            best_format = format;
            m_devWidth = dimensions.width;
            m_devHeight = dimensions.height;
            break;
        }
    }

    if ([current_device_ lockForConfiguration:nil] != YES) {
        return;
    }
    
    //设置采样格式
    [current_device_ setActiveFormat:best_format];
    
    [current_device_ unlockForConfiguration];
}

// 获取当前选中的格式
- (AVCaptureDeviceFormat*) get_current_format {
    return current_device_.activeFormat;
}

- (int)get_current_width {
    return m_devWidth;
}

- (int)get_current_height {
    return m_devHeight;
}


// 对应分辨率下支持的帧率有哪些
- (NSArray<AVFrameRateRange*>*) allSupportFrameRates: (AVCaptureDeviceFormat*)format {
    return format.videoSupportedFrameRateRanges;
}

// 设置输入设备的帧率是多少 [current_device_ setActiveFormat]
- (void)config_capture_frate:(int)fr {
    //设置格式之前，必选先设置输入设备
    if (current_device_ == nil) {
        return;
    }

    // 帧率有效范围
    if (fr < 0 || fr > 60) {
        return;
    }

    m_userFPS = fr;
    
    // 找一个该分辨率下，接近最大的帧率
    BOOL is_fps_supported = NO;
    int32_t max_fps_supported = 0;
    for (AVFrameRateRange* range in current_device_.activeFormat.videoSupportedFrameRateRanges) {
        if (range.maxFrameRate > max_fps_supported) {
            max_fps_supported = range.maxFrameRate;
        }
        
        // 找到大于等于用户设置帧率的range
        if (range.maxFrameRate >= fr) {
            m_devFPS = fr;
            is_fps_supported = YES;
            break;
        }
    }

    // 如果对应分辨率下，不支持设置的最大帧率，就用该分辨率下的最大帧率
    if (!is_fps_supported) {
        m_devFPS = max_fps_supported;
    }
    
    if ([current_device_ lockForConfiguration:nil] != YES) {
        return;
    }

    //设置帧率靠这个，但是Mac下好像不生效
    current_device_.activeVideoMinFrameDuration = (CMTime){
        .value = 1,
        .timescale = m_devFPS,
        .flags = kCMTimeFlags_Valid,
        .epoch = 0,
    };
    
    current_device_.activeVideoMaxFrameDuration = (CMTime){
        .value = 1,
        .timescale = m_devFPS,
        .flags = kCMTimeFlags_Valid,
        .epoch = 0,
    };
    
    [current_device_ unlockForConfiguration];
}

- (int)get_current_framerate {
    return m_devFPS;
}

// 当前设备所支持的所有格式
- (NSArray<NSNumber*>*) allSupportPixelFormats {
    AVCaptureVideoDataOutput* output = (AVCaptureVideoDataOutput*)[capture_session_ outputs][0];
    return [output availableVideoCVPixelFormatTypes];
}

// 设置输出的像素格式  [capture_session_ addOutput] AVCaptureVideoDataOutput
- (BOOL)set_output_format:(int)format {
    if (m_userFormat == format) {
        return YES;
    }
    
    // 移除当前现有的输出格式
    NSArray* current_outputs = [capture_session_ outputs];
    if ([current_outputs count] > 0) {
        AVCaptureVideoDataOutput* output = (AVCaptureVideoDataOutput*)[current_outputs objectAtIndex:0];
        [capture_session_ removeOutput:output];
    }

    //需要采集的像素格式 通过 AVCaptureVideoDataOutput 指定
    AVCaptureVideoDataOutput* capture_output = [[AVCaptureVideoDataOutput alloc] init];
    NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey;
    NSNumber* value = [NSNumber numberWithUnsignedInt:format];
    NSDictionary* video_settings = [NSDictionary dictionaryWithObject:value forKey:key];
    capture_output.videoSettings = video_settings;

    //添加输出
    if ([capture_session_ canAddOutput:capture_output]) {
        [capture_session_ addOutput:capture_output];
        m_userFormat = format;
    }
    else {
        return NO;
    }
    
    return YES;
}

- (int) get_current_pixel_format {
    return m_userFormat;
}

// [capture_session_ startRunning]
- (BOOL)start {
    g_bFirst = TRUE;
    g_bFrame = TRUE;
    perfTimer.Start(MAC_CAPTURE_START);
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
   
    [capture_session_ commitConfiguration];
    
    [capture_session_ startRunning];
    [self signal_capture_change];
}

// [capture_session_ stopRunning]
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

// capture_started_
- (BOOL)isStarting {
    return capture_started_;
}

// capture_connection_.enabled = NO
- (BOOL)pause {
    if (capture_connection_) {
        capture_connection_.enabled = NO;
    }
    return YES;
}

// capture_connection_.enabled = YES
- (BOOL)resume {
    if (capture_connection_) {
        capture_connection_.enabled = YES;
    }
    return YES;
}

// 设置输入设备 启动设备 停止设备 时调用，保证操作的原子性
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
    
    //如果停止之后，就不应该回调
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
    pData->AddUInt64(4, DTime::GetUnixTimeStamp64());
    if (m_userCapCallback) {
        //回调之前，再次确认是没有停止的
        if (capture_started_) {
            m_userCapCallback->OnBufferCaptured(pData);
            if (g_bFirst) {
                perfTimer.Stop(MAC_CAPTURE_START);
                g_bFirst = FALSE;
                perfTimer.Start(MAC_CAPTURE_FRAME);
            }
            else {
                if (g_bFrame) {
                    perfTimer.Stop(MAC_CAPTURE_FRAME);
                    g_bFrame = FALSE;
                }
            }
        }
    }
}

- (void)perf {
    perfTimer.OutputAll(DTIME_IN_MS);
}
@end
