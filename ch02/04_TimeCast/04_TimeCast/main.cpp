#include <iostream>
#include <chrono>

int main(int argc, const char * argv[]) {
    std::chrono::milliseconds ms(1000);  // 1秒
    std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout << s.count() << std::endl; // 1
    
    std::chrono::microseconds mis = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    std::cout << mis.count() << std::endl; // 1000000
    
    std::chrono::nanoseconds nas = std::chrono::duration_cast<std::chrono::nanoseconds>(ms);
    std::cout << nas.count() << std::endl; // 1000000000
}
