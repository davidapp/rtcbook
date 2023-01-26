#import "MacCGView.h"

@interface MacCGView()

@property(strong)CIImage* pFrame;

@end

@implementation MacCGView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    if (_pFrame.CGImage) {
        CGContextDrawImage(context, self.bounds, _pFrame.CGImage);
        CGImageRelease(_pFrame.CGImage);
    }
}

- (void)drawFrame:(CIImage*)pFrame
{
    _pFrame = pFrame;
    [self setNeedsDisplay:YES];
}

@end
