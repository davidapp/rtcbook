#pragma once

#include "DTypes.h"
#include <string>

class DXP
{
public:
    static DCStr GetOSName();
    static DVoid Print(std::string str);

public:
    static DVoid* memcpy(DVoid* dest, const DVoid* src, DSizeT count);

    D_DISALLOW_ALL_DEFAULT(DXP)
};
