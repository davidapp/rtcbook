#import "IOSCGView.h"

@interface IOSCGView()

@property(strong)CIImage* pFrame;

@end

@implementation IOSCGView

- (void)drawRect:(CGRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    if (_pFrame.CGImage) {
        CGContextDrawImage(context, self.bounds, _pFrame.CGImage);
        CGImageRelease(_pFrame.CGImage);
    }
}

- (void)drawFrame:(CIImage*)pFrame
{
    _pFrame = pFrame;
    [self setNeedsDisplay];
}

@end
