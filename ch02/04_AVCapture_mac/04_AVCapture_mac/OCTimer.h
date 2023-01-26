#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface OCTimer : NSObject
+(void)Start:(int)index;
+(void)Stop:(int)index;
+(void)Print:(int)index;
@end

NS_ASSUME_NONNULL_END
