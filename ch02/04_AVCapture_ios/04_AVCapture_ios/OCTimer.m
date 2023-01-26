#import "OCTimer.h"
#import <mach/mach_time.h>

@interface OCTimer()

@end

@implementation OCTimer

int64_t time_record[100];

+(void)Start:(int)index
{
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    int64_t ticks = mach_absolute_time() * timebase.numer / timebase.denom;
    time_record[index] = ticks;
}

+(void)Stop:(int)index
{
    if (time_record[index] == 0) return;
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    int64_t ticks = mach_absolute_time() * timebase.numer / timebase.denom;
    time_record[index] = ticks - time_record[index];
}

+(void)Print:(int)index
{
    if (time_record[index] == 0) return;
    NSLog(@"slot[%d] = %lld us", index, time_record[index] / 1000);
    time_record[index] = 0;
}

@end
