#pragma once

#include "DTypes.h"
#include "Video/DVideoDefines.h"
#include <dshow.h>
#include <dvdmedia.h>
#include <vector>
#include <string>

class DCameraInfo {
public:
    std::string m_device_name;
    std::string m_device_path;
    IBaseFilter* m_device_filter;
};

class DCameraCaps {
public:
    DUInt32 m_width;
    DUInt32 m_height;
    DUInt32 m_frame_rate;
    DPixelFmt m_pixel_format;
    std::string m_amt;
    std::string m_frlist;
};

class WinDSCamera
{
public:
    static DBool Init();
    static DVoid UnInit();

public:
    static std::vector<DCameraInfo> GetDevices();
    static std::wstring GetInfoString(const DCameraInfo& info);
    static DBool ShowSettingDialog(IBaseFilter* filter, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY);
    static std::vector<DCameraCaps> GetDeviceCaps(IBaseFilter* pFilter);
};
