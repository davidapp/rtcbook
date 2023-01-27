//
//  ViewController.m
//  03_AVCapture_ios
//
//  Created by bytedance on 2022/12/27.
//

#import "ViewController.h"
#import "IOSOCCamera.h"
#import "IOSCGView.h"
#import "OCTimer.h"

@interface ViewController ()

@property (strong) IOSOCCamera* camera;
@property (strong) IOSCGView* cgview;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    _camera = [[IOSOCCamera alloc] initWithHanlder: NULL];
    _cgview = [[IOSCGView alloc] initWithFrame:CGRectMake(70,150,180,270)];
    [self.view addSubview:_cgview];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onFrame:) name:@"onFrame" object:nil];
}

-(void)onFrame:(NSNotification *)notification{
    CIImage* pFrame = notification.object;
    [_cgview drawFrame:pFrame];
}

-(IBAction)pressAuth:(id)sender
{
    [IOSOCCamera getAuthorizationStatus: nil];
    NSLog(@"uio = %d", (int)[UIApplication sharedApplication].statusBarOrientation);
}

-(IBAction)pressStart:(id)sender
{
    [OCTimer Start:0];
    [_camera start];
}

-(IBAction)pressStop:(id)sender
{
    [_camera stop];
}

-(IBAction)pressBack:(id)sender
{
    [_camera set_camera_device_by_unique_id:@"com.apple.avfoundation.avcapturedevice.built-in_video:0"];
}

-(IBAction)pressFront:(id)sender
{
    [_camera set_camera_device_by_unique_id:@"com.apple.avfoundation.avcapturedevice.built-in_video:1"];
}
@end
