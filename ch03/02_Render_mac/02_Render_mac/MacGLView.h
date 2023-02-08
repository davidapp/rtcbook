#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface MacGLView : NSView
- (void)initGL;
- (void)drawFrame:(CIImage*)pFrame;
@end

NS_ASSUME_NONNULL_END
