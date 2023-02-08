//
//  ViewController.m
//  02_Render_mac
//
//  Created by bytedance on 2023/2/8.
//

#import "ViewController.h"
#import "MacOCCamera.h"
#import "MacGLView.h"

@interface ViewController()

@property (strong) MacOCCamera* camera;
@property (strong) MacGLView* glview;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    NSLog(@"%@", self.view);
    
    _camera = [[MacOCCamera alloc] initWithHanlder: NULL];
    _glview = [[MacGLView alloc] initWithFrame:CGRectMake(10,10,320,240)];
    [_glview initGL];
    
    [self.view addSubview:_glview];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onFrame:) name:@"onFrame" object:nil];

}

-(void)onFrame:(NSNotification *)notification{
    CIImage* pFrame = notification.object;
    //[_cgview drawFrame:pFrame];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
}

- (IBAction)pressAuth:(id)sender {
    [MacOCCamera getAuthorizationStatus: nil];
}

- (IBAction)pressStart:(id)sender {
    [_camera start];
}

- (IBAction)pressStop:(id)sender {
    [_camera stop];
}

@end
