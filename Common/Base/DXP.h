#pragma once

#include "DTypes.h"
#include <string>

class DXP
{
public:
    static DCStr GetOSName();
    static DVoid Print(std::string str);

    D_DISALLOW_ALL_DEFAULT(DXP)
};
