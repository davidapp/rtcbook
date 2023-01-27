#import "IOSCGView.h"

@interface IOSCGView()

@property(strong)UIImage* pFrame;

@end

@implementation IOSCGView

- (void)drawRect:(CGRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    if (_pFrame.CGImage) {
        [_pFrame drawInRect:self.bounds];
        CGImageRelease(_pFrame.CGImage);
    }
}

- (void)drawFrame:(UIImage*)pFrame
{
    _pFrame = pFrame;
    [self setNeedsDisplay];
}

@end
