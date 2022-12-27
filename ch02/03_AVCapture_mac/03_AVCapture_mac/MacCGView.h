#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface MacCGView : NSView
- (void)drawFrame:(CIImage*)pFrame;
@end

NS_ASSUME_NONNULL_END
