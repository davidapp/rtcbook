#include <iostream>
#include <chrono>
#include "DConfig.h"

#if defined(BUILD_FOR_MAC)
#include <mach/mach_time.h>
#endif

int main(int argc, const char * argv[]) {
    auto now_time = std::chrono::steady_clock::now();
    int64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now_time.time_since_epoch()).count();
    std::cout << nano_time << std::endl;
    
#if defined(BUILD_FOR_MAC)
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    int64_t ticks = mach_absolute_time() * timebase.numer / timebase.denom;
    std::cout << ticks << std::endl;
#endif
    return 0;
}
