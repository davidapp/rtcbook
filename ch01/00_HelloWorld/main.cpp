#include "Base/DUtil.h"

int main()
{
    std::string strHello = std::string("Hello ") + DUtil::GetOSName();
    DUtil::Print(strHello);
    return 0;
}