#include "Base/DXP.h"

int main()
{
    std::string strHello = std::string("Hello ") + DXP::GetOSName() + D_LINES;
    DXP::Print(strHello);
    return 0;
}
