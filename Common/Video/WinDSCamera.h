#pragma once

#include "DTypes.h"
#include <dshow.h>
#include <dvdmedia.h>
#include <vector>
#include <string>

class DCameraInfo {
public:
    std::string m_device_name;
    std::string m_device_path;
    IBaseFilter* m_filter;
};

class DCameraCaps {
public:
    DUInt32 m_width;
    DUInt32 m_height;
    DUInt32 m_frame_rate;
    DUInt32 m_pixel_format;
    std::string m_amt;
    std::string m_frlist;
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

    // 显示对应设备的设置窗口
    static DBool ShowSettingDialog(IBaseFilter* filter, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY);

    // 指定的设备，有哪些采集能力
    static std::vector<DCameraCaps> GetDeviceCaps(IBaseFilter* pFilter);

    static std::string MajorTypeName(GUID id);
    static std::string SubTypeName(GUID id);
    static std::string FormatTypeName(GUID id);

    static std::string GetVideoInfo(VIDEOINFOHEADER* pInfo);
    static std::string GetVideoInfo2(VIDEOINFOHEADER2* pInfo);

private:
    // Helper Functions
    static IPin* GetInputPin(IBaseFilter* filter);
    static IPin* GetOutputPin(IBaseFilter* filter, REFGUID Category);
    static DBool PinMatchesCategory(IPin* pPin, REFGUID Category);
    static DVoid FreeMediaType(AM_MEDIA_TYPE& mt);
    static DUInt64 GetMaxOfFrameArray(DUInt64* maxFps, DUInt32 size);
};
