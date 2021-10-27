#pragma once

#include "DTypes.h"
#include <dshow.h>
#include <vector>
#include <string>

class DCameraInfo {
public:
    std::string m_device_name;
    std::string m_device_path;
    std::string m_device_desc;
};

class DCameraCaps {
    DUInt32 m_platform; 
    DUInt32 m_width;
    DUInt32 m_height;
    DUInt32 m_frame_rate;
    DBool   m_is_max_fr;
    DUInt32 m_pixel_format;
};


class WinDSCamera
{
public:
    // 初始化COM，并创建 ICreateDevEnum 接口
    static DBool Init();
    // 销毁 ICreateDevEnum 和 IEnumMoniker 接口
    static DVoid UnInit();

public:
    // 有哪些采集设备，每次重新创建一个 IEnumMoniker 接口来枚举
    static std::vector<DCameraInfo> GetDevices();
    static std::wstring GetInfoString(const DCameraInfo& info);
    static std::string GetProductIdFromPath(std::string& path);

    // 指定的设备，有哪些采集能力
    static std::vector<DCameraCaps> GetDeviceCaps(DStr deviceID);

    // 拿到对应采集设备的 Filter
    static IBaseFilter* GetDeviceFilter(DCStr deviceID);

    // 显示对应设备的设置窗口
    static DBool ShowSettingDialog(DCStr deviceID, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY);

private:
    // Helper Functions
    static IPin* GetInputPin(IBaseFilter* filter);
    static IPin* GetOutputPin(IBaseFilter* filter, REFGUID Category);
    static DBool PinMatchesCategory(IPin* pPin, REFGUID Category);
    static DVoid FreeMediaType(AM_MEDIA_TYPE& mt);
    static DUInt64 GetMaxOfFrameArray(DUInt64* maxFps, DUInt32 size);
};
