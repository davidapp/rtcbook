#include "DTestBuffer.h"

int main(int argc, const char * argv[])
{
    DTestBuffer::Test();

    DTestGrowBuffer::Test();
    DTestReadBuffer::Test();
    return 0;
}
