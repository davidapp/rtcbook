#include <iostream>
#include <chrono>
#include <ctime>
using namespace std::chrono;

int main()
{
    // epoch ��ʲôʱ��
    std::chrono::time_point<std::chrono::system_clock> epoch;
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(epoch);
    std::cout << "epoch: " << std::ctime(&epoch_time);
    // epoch: Thu Jan  1 08:00:00 1970

    // ��ǰ��ʱ���
    const time_point<system_clock> now = system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "now: " << std::ctime(&now_time);
    // now: Sun Oct  9 17:06:42 2022
    auto now_timestamp = now.time_since_epoch().count();
    std::cout << "now ts: " << now_timestamp << std::endl;
    // now ts: 16653064020116445

    // ��ǰʱ�����ƫ��
    std::chrono::milliseconds ms{ 3 }; // 3 ����
    std::chrono::microseconds us = 2 * ms; // 6000 ΢��
    std::chrono::duration<double, std::ratio<1, 30>> hz(3.5); // ʱ��������Ϊ 1/30 ��

    std::cout << "3 ms duration has " << ms.count() << " ticks\n"
        << "6000 us duration has " << us.count() << " ticks\n"
        << "3.5 hz duration has " << hz.count() << " ticks\n";

    //
    // Linux �ϵ�ʱ���
    // clock() : ms
    // gettimeofday(time_val*, NULL) : us
    // 
    // C++ 11 �ṩ��ʱ��
    // system_clock::now() : ns
    // steady_clock::now() : ns

    // gettimeofday������ѣ�����3�ַ�ʽ���ܲ�඼����ܴ�
    // gettimeofday����ֵ��std::chrono::system_clock::now()һ�£�������ȫ���gettimeofday��
    // ����windows��֧��gettimeofday�������Ƽ���ȡʱ���ʹ��std::chrono::system_clock::now()��ʽ��
}
