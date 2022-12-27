#import "ViewController.h"
#import "MacOCCamera.h"
#import "MacCGView.h"

@interface ViewController()

@property (strong) MacOCCamera* camera;
@property (strong) MacCGView* cgview;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    _camera = [[MacOCCamera alloc] initWithHanlder: NULL];
    _cgview = [[MacCGView alloc] initWithFrame:CGRectMake(0,0,320,240)];
    [self.view addSubview:_cgview];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onFrame:) name:@"onFrame" object:nil];
}

-(void)onFrame:(NSNotification *)notification{
    CIImage* pFrame = notification.object;
    [_cgview drawFrame:pFrame];
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
