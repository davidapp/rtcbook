#include "Base/DXP.h"

int main()
{
    std::string strHello = std::string("Hello ") + DXP::GetOSName();
    DXP::Print(strHello);
    return 0;
}