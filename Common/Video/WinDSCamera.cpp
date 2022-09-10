#include "WinDSCamera.h"
#include "Base/DUTF8.h"
#include "Base/DUtil.h"
#include "Video/VideoDefines.h"
#include "File/DBmpFile.h"
#include "Video/WinDS.h"


ICreateDevEnum* _dsDevEnum;
IEnumMoniker* _dsMonikerDevEnum;

// 初始化COM，并创建 ICreateDevEnum 接口
DBool WinDSCamera::Init() 
{
    _dsDevEnum = nullptr;
    _dsMonikerDevEnum = nullptr;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) 
    {
        return false;
    }

    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void**)&_dsDevEnum);
    if (hr != NOERROR) 
    {
        return false;
    }
    return true;
}

// 销毁 ICreateDevEnum 和 IEnumMoniker 接口
DVoid WinDSCamera::UnInit()
{
    SAFE_RELEASE(_dsMonikerDevEnum)
    SAFE_RELEASE(_dsDevEnum)

}

// 通过 CLSID_SystemDeviceEnum 对象的 IID_ICreateDevEnum 接口，枚举系统的摄像头
// 要枚举的是 CLSID_VideoInputDeviceCategory 类别下的 IEnumMoniker 接口
std::vector<DCameraInfo> WinDSCamera::GetDevices()
{
    std::vector<DCameraInfo> info;

    if (_dsDevEnum == nullptr) {
        return info;
    }

    // 每次重新创建新的 IEnumMoniker，便于实时更新
    SAFE_RELEASE(_dsMonikerDevEnum)
    HRESULT hr = _dsDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &_dsMonikerDevEnum, 0);
    if (hr != NOERROR || _dsMonikerDevEnum == nullptr) {
        return info;
    }
    _dsMonikerDevEnum->Reset();

    ULONG cFetched = ULONG();
    IMoniker* pM = nullptr;
    // 枚举不到就直接返回空数组
    while (S_OK == _dsMonikerDevEnum->Next(1, &pM, &cFetched)) 
    {
        // 枚举到了就找它的 IPropertyBag 接口
        IPropertyBag* pBag = nullptr;
        hr = pM->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&pBag);
        if (SUCCEEDED(hr)) 
        {
            DCameraInfo dev = {};

            VARIANT varName;
            varName.vt = VT_BSTR;

            // 1.设备的名称，"FriendlyName"，填入UTF8编码
            hr = pBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr)) 
            {
                dev.m_device_name = DUTF8::UCS2ToUTF8((DUInt16*)varName.bstrVal, (DUInt32)wcslen(varName.bstrVal)*2);
                SysFreeString(varName.bstrVal);
            }

            // 2.设备的路径，"DevicePath"，用这个当作设备ID
            hr = pBag->Read(L"DevicePath", &varName, 0);
            if (SUCCEEDED(hr)) 
            {
                dev.m_device_path = DUTF8::UCS2ToUTF8((DUInt16*)varName.bstrVal, (DUInt32)wcslen(varName.bstrVal) * 2);
                SysFreeString(varName.bstrVal);
            }

            // 3.拿到设备的 IBaseFilter*
            IBaseFilter* pFilter = nullptr;
            hr = pM->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pFilter);
            if (SUCCEEDED(hr))
            {
                dev.m_device_filter = pFilter;
            }

            info.push_back(dev);
            pBag->Release();
        }

        pM->Release();
    }

	return info;
}

std::wstring WinDSCamera::GetInfoString(const DCameraInfo& info)
{
    std::wstring ret, temp;
    temp = L"device name: \r\n";
    ret += temp;
    temp = DUTF8::UTF8ToUCS2(info.m_device_name) + L"\r\n";
    ret += temp;
    temp = L"device path: \r\n";
    ret += temp;
    temp = DUTF8::UTF8ToUCS2(info.m_device_path) + L"\r\n";
    ret += temp;
    return ret;
}

// 通过 IBaseFilter 查询到 IID_ISpecifyPropertyPages
DBool WinDSCamera::ShowSettingDialog(IBaseFilter* filter, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY)
{
    DBool bResult = false;
    do
    {
        if (!filter) {
            break;
        }

        ISpecifyPropertyPages* pPages = NULL;
        HRESULT hr = filter->QueryInterface(IID_ISpecifyPropertyPages, (LPVOID*)&pPages);
        if (!SUCCEEDED(hr)) {
            break;
        }

        CAUUID uuid;
        hr = pPages->GetPages(&uuid);
        if (!SUCCEEDED(hr)) {
            break;
        }

        hr = OleCreatePropertyFrame((HWND)parentWindow, positionX, positionY,
            L"Capture Settings", 1, (LPUNKNOWN*)&filter, uuid.cElems, uuid.pElems,
            LOCALE_USER_DEFAULT, 0, NULL);

        if (!SUCCEEDED(hr))
        {
            break;
        }

        if (uuid.pElems)
        {
            CoTaskMemFree(uuid.pElems);
        }

        bResult = true;
    } while (0);

    SAFE_RELEASE(filter);
    return bResult;
}

// 给定一个视频采集设备的ID，获取这个采集设备的能力
std::vector<DCameraCaps> WinDSCamera::GetDeviceCaps(IBaseFilter* pFilter)
{
    std::vector<DCameraCaps> caps;

    IBaseFilter* captureDevice = pFilter;
    IPin* outputCapturePin = nullptr;
    IAMStreamConfig* streamConfig = nullptr; 
    // use IAMStreamConfig::SetFormat to set
    // use IAMStreamConfig::GetFormat to get the current format

    IAMVideoControl* videoControlConfig = nullptr; // use IAMVideoControl to get frame rate
    do
    {
        if (!captureDevice) {
            break;
        }

        outputCapturePin = WinDS::GetOutputPin(captureDevice, GUID_NULL);
        if (!outputCapturePin) {
            break;
        }

        HRESULT hr = outputCapturePin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
        if (FAILED(hr)) {
            break;
        }

        // https://docs.microsoft.com/en-us/previous-versions/ms784113(v=vs.85)
        int count, size;
        hr = streamConfig->GetNumberOfCapabilities(&count, &size);
        if (FAILED(hr)){
            break;
        }

        // used for FPS
        HRESULT hrVC = captureDevice->QueryInterface(IID_IAMVideoControl, (void**)&videoControlConfig);

        AM_MEDIA_TYPE* pmt = NULL;
        VIDEO_STREAM_CONFIG_CAPS vscaps = {}; // we should ignore it 
        for (int32_t tmp = 0; tmp < count; ++tmp)
        {
            // https://docs.microsoft.com/en-us/previous-versions/ms784114(v=vs.85)
            hr = streamConfig->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&vscaps));
            if (FAILED(hr)) {
                continue;
            }

            DCameraCaps cap = {};
            cap.m_amt = WinDS::Dump_AM_MEDIA_TYPE(pmt);
            DInt64 avgTimePerFrame = 0;

            // 枚举视频类型，看视频的格式
            if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == FORMAT_VideoInfo)
            {
                // 支持的分辨率
                VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
                cap.m_width = h->bmiHeader.biWidth;
                cap.m_height = h->bmiHeader.biHeight;
                avgTimePerFrame = h->AvgTimePerFrame;
                cap.m_amt += "\r\n";
                cap.m_amt += WinDS::Dump_VIDEOINFOHEADER(h);
            }
            else if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == FORMAT_VideoInfo2)
            {
                VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
                cap.m_width = h->bmiHeader.biWidth;
                cap.m_height = h->bmiHeader.biHeight;
                avgTimePerFrame = h->AvgTimePerFrame;
                cap.m_amt += "\r\n";
                cap.m_amt += WinDS::Dump_VIDEOINFOHEADER2(h);
            }

            // 像素格式
            DPixelFmt mformat = DPixelFmt::Unknown;
            if (pmt->subtype == MEDIASUBTYPE_RGB24)
            {
                mformat = DPixelFmt::RGB24;
            }
            else if (pmt->subtype == MEDIASUBTYPE_I420)
            {
                mformat = DPixelFmt::I420;
            }
            else if (pmt->subtype == MEDIASUBTYPE_IYUV)
            {
                mformat = DPixelFmt::IYUV;
            }
            else if (pmt->subtype == MEDIASUBTYPE_YUY2)
            {
                mformat = DPixelFmt::YUY2;
            }
            else if (pmt->subtype == MEDIASUBTYPE_MJPG)
            {
                mformat = DPixelFmt::MJPG;
            }
            else
            {
                mformat = DPixelFmt::Unknown;
            }
            cap.m_pixel_format = (DUInt32)mformat;

            // 支持的帧率
            if (hrVC == S_OK)
            {
                DUInt64* frameDurationList;
                DUInt64 max_fps;
                DUInt32 listSize;
                SIZE size;
                size.cx = cap.m_width;
                size.cy = cap.m_height;

                // https://docs.microsoft.com/en-us/windows/win32/api/strmif/nf-strmif-iamvideocontrol-getframeratelist
                hrVC = videoControlConfig->GetFrameRateList(outputCapturePin, tmp, size, (long*)&listSize, (LONGLONG**)&frameDurationList);
                if (hrVC == S_OK && listSize > 0 && 0 != (max_fps = WinDS::GetMaxOfFrameArray(frameDurationList, listSize)))
                {
                    cap.m_frame_rate = (DUInt32)(10000000 / max_fps);
                    cap.m_frlist = WinDS::FRArrayToStr(frameDurationList, listSize);
                    CoTaskMemFree(frameDurationList);
                }
            }
            
            if (cap.m_frlist.length()==0)
            {
                // use existing method in case IAMVideoControl is not supported
                if (avgTimePerFrame > 0)
                {
                    cap.m_frame_rate = (DUInt32)(10000000 / avgTimePerFrame);
                    DUInt64 fr = cap.m_frame_rate;
                    cap.m_frlist = WinDS::FRArrayToStr(&fr, 1);
                }
                else
                {
                    cap.m_frame_rate = 0;
                    cap.m_frlist = "[]";
                }
            }

            cap.m_amt += "FrameRate: ";
            cap.m_amt += cap.m_frlist;
            cap.m_amt += "\r\n";

            caps.push_back(cap);

            if (pmt) {
                WinDS::FreeMediaType(*pmt);
            }

            CoTaskMemFree((PVOID)pmt);
            pmt = NULL;
        }

    } while(0);

    SAFE_RELEASE(videoControlConfig);
    SAFE_RELEASE(streamConfig);
    SAFE_RELEASE(outputCapturePin);
    SAFE_RELEASE(captureDevice);

    return caps;
}
