#import "MacOCCamera.h"
#import <CoreVideo/CoreVideo.h>
#import <atomic>
#include "Video/DVideoFrame.h"
#include "Video/DVideoI420.h"

#define DEFAULT_CAPTURE_WIDTH 1280
#define DEFAULT_CAPTURE_HEIGHT 720
#define DEFAULT_FRAME_RATE 25
//#define DEFAULT_PIXEL_FORMAT kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
#define DEFAULT_PIXEL_FORMAT kCVPixelFormatType_420YpCbCr8Planar

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
             
        // 将默认设备作为输入设备
        [self set_default_device];
        
        // 设置默认的分辨率
        [self config_capture_width:DEFAULT_CAPTURE_WIDTH height:DEFAULT_CAPTURE_HEIGHT];
        
        // 设置默认的帧率
        [self config_capture_frate:DEFAULT_FRAME_RATE];

        // 设置默认的输出格式
        [self set_output_format:DEFAULT_PIXEL_FORMAT];
    }
    
    return self;
}

- (void)destroy {
    [self removeNotify];
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

- (BOOL)start {
    [self wait_capture_change];

    if (!capture_session_) {
        return NO;
    }
    
    [self addNotify];

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

- (BOOL)isStarting {
    return capture_started_;
}

- (BOOL)pause {
    if (capture_connection_) {
        capture_connection_.enabled = NO;
    }
    return YES;
}

- (BOOL)resume {
    if (capture_connection_) {
        capture_connection_.enabled = YES;
    }
    return YES;
}

- (void)addNotify
{
    NSNotificationCenter* notify = [NSNotificationCenter defaultCenter];
    // 运行错误通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureSessionRuntimeErrorNotification object:capture_session_];
    // 开始采集通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureSessionDidStartRunningNotification object:capture_session_];
    // 停止采集通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureSessionDidStopRunningNotification object:capture_session_];
    // 设备连接通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureDeviceWasConnectedNotification object:capture_session_];
    // 设备断开通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureDeviceWasDisconnectedNotification object:current_device_];
    // 设备中断通知
    [notify addObserver:self selector:@selector(onNotification:)
               name:AVCaptureSessionWasInterruptedNotification object:current_device_];
}

-(void)onNotification:(NSNotification *)notification{
    NSLog(@"%@", notification);
}

- (void)removeNotify
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
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
           fromConnection:(AVCaptureConnection*)connection
{
    //如果停止之后，就不应该回调
    if (!capture_started_) {
        return;
    }
    
    CVPixelBufferRef video_frame = CMSampleBufferGetImageBuffer(sample_buffer);
    
    size_t width = CVPixelBufferGetWidth(video_frame);
    size_t height = CVPixelBufferGetHeight(video_frame);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(video_frame);
    OSType cvtype = CVPixelBufferGetPixelFormatType(video_frame);
    // '420v' or 'y420'
    CVPixelBufferLockBaseAddress(video_frame, kCVPixelBufferLock_ReadOnly);
    void *pBuf = CVPixelBufferGetBaseAddress(video_frame);
    size_t datasize = CVPixelBufferGetDataSize(video_frame);
    
    DVideoFrame frame(width, height, DPixelFmt::I420);
    if (cvtype == kCVPixelFormatType_420YpCbCr8PlanarFullRange || cvtype == kCVPixelFormatType_420YpCbCr8Planar)
    {
        const uint8_t* src_y = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(video_frame, 0));
        const int src_ystride = CVPixelBufferGetBytesPerRowOfPlane(video_frame, 0);
        const uint8_t* src_u = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(video_frame, 1));
        const int src_ustride = CVPixelBufferGetBytesPerRowOfPlane(video_frame, 1);
        const uint8_t* src_v = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(video_frame, 2));
        const int src_vstride = CVPixelBufferGetBytesPerRowOfPlane(video_frame, 2);

        DInt32 dst_lineSize = frame.GetLineSize();
        DByte* dst_y = frame.GetBuf();
        DInt32 dst_stride_y = dst_lineSize;
        DByte* dst_u = dst_y + height * dst_lineSize;
        DInt32 dst_stride_u = dst_lineSize / 2;
        DByte* dst_v = dst_u + height * dst_lineSize / 4;
        DInt32 dst_stride_v = dst_lineSize / 2;

        DVideoI420::I420Copy(src_y, src_ystride, src_u, src_ustride, src_v, src_vstride, dst_y, dst_stride_y, dst_u, dst_stride_u, dst_v, dst_stride_v, width, height);
    }
    CVPixelBufferUnlockBaseAddress(video_frame, kCVPixelBufferLock_ReadOnly);
    
    NSLog(@"%zu*%zu, type:%d, pBuf:%p size:%zu", width, height, cvtype, pBuf, datasize);

    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:@"onFrame" object:(NSInteger)(void *)frame.GetBuf() userInfo: nil];
    });
}

+ (void)getAuthorizationStatus:(void(^)(void))authorizedBlock
{
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo]){
        case AVAuthorizationStatusNotDetermined:{
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted){
                 if (granted){
                     dispatch_async(dispatch_get_main_queue(), ^{
                         if (authorizedBlock) {
                             authorizedBlock();
                         }
                     });
                 }
                 NSLog(@"granted --- %d currentThread : %@",granted,NSThread.currentThread);
             }];
            NSLog(@"用户尚未授予或拒绝该权限:AVAuthorizationStatusNotDetermined");
        }
            break;
        case AVAuthorizationStatusRestricted:
            NSLog(@"不允许用户访问媒体捕获设备:AVAuthorizationStatusRestricted");
            break;
        case AVAuthorizationStatusDenied:
        {
            NSLog(@"用户已经明确拒绝了应用访问捕获设备:AVAuthorizationStatusDenied");
        }
            break;
        case AVAuthorizationStatusAuthorized:
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (authorizedBlock) {
                    authorizedBlock();
                }
            });
            NSLog(@"用户授予应用访问捕获设备的权限:AVAuthorizationStatusAuthorized");
        }
            break;
        default:
            break;
    }
}

@end
