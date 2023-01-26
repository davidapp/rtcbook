#include <iostream>
#include <chrono>
#include <ctime>
using namespace std::chrono;

int main()
{
    // epoch 是什么时候
    std::chrono::time_point<std::chrono::system_clock> epoch;
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(epoch);
    std::cout << "epoch: " << std::ctime(&epoch_time);
    // epoch: Thu Jan  1 08:00:00 1970

    // 当前的时间戳
    const time_point<system_clock> now = system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "now: " << std::ctime(&now_time);
    // now: Sun Oct  9 17:06:42 2022
    auto now_timestamp = now.time_since_epoch().count();
    std::cout << "now ts: " << now_timestamp << std::endl;
    // now ts: 16653064020116445

    // 当前时间戳的偏移
    std::chrono::milliseconds ms{ 3 }; // 3 毫秒
    std::chrono::microseconds us = 2 * ms; // 6000 微秒
    std::chrono::duration<double, std::ratio<1, 30>> hz(3.5); // 时间间隔周期为 1/30 秒

    std::cout << "3 ms duration has " << ms.count() << " ticks\n"
        << "6000 us duration has " << us.count() << " ticks\n"
        << "3.5 hz duration has " << hz.count() << " ticks\n";

    //
    // Linux 上的时间戳
    // clock() : ms
    // gettimeofday(time_val*, NULL) : us
    // 
    // C++ 11 提供的时间
    // system_clock::now() : ns
    // steady_clock::now() : ns

    // gettimeofday性能最佳，但是3种方式性能差距都不算很大。
    // gettimeofday返回值与std::chrono::system_clock::now()一致，可以完全替代gettimeofday。
    // 由于windows不支持gettimeofday函数，推荐获取时间戳使用std::chrono::system_clock::now()方式。
}
