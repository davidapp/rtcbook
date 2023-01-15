#include <iostream>
#include <time.h>
#include <chrono>

int main(int argc, const char * argv[]) {
    auto now_time = std::chrono::system_clock::now();
    int64_t nano_time = std::chrono::duration_cast<std::chrono::nanoseconds>(now_time.time_since_epoch()).count();
    std::cout << nano_time << std::endl;
    
    time_t timep;
    time(&timep);
    struct tm *p = localtime(&timep);
    printf("%d/%d/%d %d:%d:%d\n", 1900+p->tm_year,1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    
    std::time_t now_t = std::chrono::system_clock::to_time_t(now_time);
    std::cout << "now: " << std::ctime(&now_t);
    return 0;
}
