#include "WinDSCamera.h"
#include <Initguid.h>
#include "Base/DUTF8.h"
#include "Base/DUtil.h"
#include "Video/VideoDefines.h"

#pragma comment(lib, "strmiids.lib")

#define SAFE_RELEASE(p) \
    if (p) {            \
        (p)->Release(); \
        (p) = NULL;     \
    }

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(MEDIASUBTYPE_HDYC, 0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

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
                dev.m_device_name = DUTF8::UCS2ToUTF8((DUInt16*)varName.bstrVal, wcslen(varName.bstrVal)*2);
                SysFreeString(varName.bstrVal);
            }

            // 2.设备的路径，"DevicePath"，用这个当作设备ID
            hr = pBag->Read(L"DevicePath", &varName, 0);
            if (SUCCEEDED(hr)) 
            {
                dev.m_device_path = DUTF8::UCS2ToUTF8((DUInt16*)varName.bstrVal, wcslen(varName.bstrVal) * 2);
                SysFreeString(varName.bstrVal);
            }

            // 3.拿到设备的 IBaseFilter*
            IBaseFilter* pFilter = nullptr;
            hr = pM->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pFilter);
            if (SUCCEEDED(hr))
            {
                dev.m_filter = pFilter;
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

        outputCapturePin = GetOutputPin(captureDevice, GUID_NULL);
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
            cap.m_amt = Dump_AM_MEDIA_TYPE(pmt);
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
                cap.m_amt += Dump_VIDEOINFOHEADER(h);
            }
            else if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == FORMAT_VideoInfo2)
            {
                VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(pmt->pbFormat);
                cap.m_width = h->bmiHeader.biWidth;
                cap.m_height = h->bmiHeader.biHeight;
                avgTimePerFrame = h->AvgTimePerFrame;
                cap.m_amt += "\r\n";
                cap.m_amt += Dump_VIDEOINFOHEADER2(h);
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
                if (hrVC == S_OK && listSize > 0 && 0 != (max_fps = GetMaxOfFrameArray(frameDurationList, listSize)))
                {
                    cap.m_frame_rate = (DUInt32)(10000000 / max_fps);
                    cap.m_frlist = DUtil::FRArrayToStr(frameDurationList, listSize);
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
                    cap.m_frlist = DUtil::FRArrayToStr(&fr, 1); 
                }
                else
                {
                    cap.m_frame_rate = 0;
                    cap.m_frlist = "[]";
                }
            }

            cap.m_amt += "\r\nFrameRate: ";
            cap.m_amt += cap.m_frlist;
            cap.m_amt += "\r\n";

            caps.push_back(cap);

            if (pmt) {
                FreeMediaType(*pmt);
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

DVoid WinDSCamera::FreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL) {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

DUInt64 WinDSCamera::GetMaxOfFrameArray(DUInt64* maxFps, DUInt32 size) 
{
    DUInt64 maxFPS = maxFps[0];
    for (DUInt32 i = 0; i < size; i++) 
    {
        if (maxFPS > maxFps[i])
            maxFPS = maxFps[i];
    }
    return maxFPS;
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

std::string WinDSCamera::MajorTypeName(GUID id)
{
    // https://docs.microsoft.com/en-us/windows/win32/directshow/major-types
    if (id == MEDIATYPE_Video) return "MEDIATYPE_Video";
    else if (id == MEDIATYPE_Audio) return "MEDIATYPE_Audio";
    else if (id == MEDIATYPE_AnalogAudio) return "MEDIATYPE_AnalogAudio";
    else if (id == MEDIATYPE_AnalogVideo) return "MEDIATYPE_AnalogVideo";
    else if (id == MEDIATYPE_AUXLine21Data) return "MEDIATYPE_AUXLine21Data";
    else if (id == MEDIATYPE_Interleaved) return "MEDIATYPE_Interleaved";
    else if (id == MEDIATYPE_Midi) return "MEDIATYPE_Midi";
    else if (id == MEDIATYPE_MPEG2_PES) return "MEDIATYPE_MPEG2_PES";
    else if (id == MEDIATYPE_MPEG2_SECTIONS) return "MEDIATYPE_MPEG2_SECTIONS";
    else if (id == MEDIATYPE_ScriptCommand) return "MEDIATYPE_ScriptCommand";
    else if (id == MEDIATYPE_Stream) return "MEDIATYPE_Stream";
    else if (id == MEDIATYPE_Text) return "MEDIATYPE_Text";
    else if (id == MEDIATYPE_VBI) return "MEDIATYPE_VBI";
    return "MEDIATYPE_Unknown";
}

std::string WinDSCamera::SubTypeName(GUID id)
{
    // https://docs.microsoft.com/en-us/windows/win32/directshow/video-subtypes
    if (id == MEDIASUBTYPE_RGB565) return "MEDIASUBTYPE_RGB565";
    else if (id == MEDIASUBTYPE_RGB24) return "MEDIASUBTYPE_RGB24";
    else if (id == MEDIASUBTYPE_RGB32) return "MEDIASUBTYPE_RGB32";
    else if (id == MEDIASUBTYPE_ARGB32) return "MEDIASUBTYPE_ARGB32";
    // https://docs.microsoft.com/en-us/windows/win32/directshow/yuv-video-subtypes
    else if (id == MEDIASUBTYPE_AYUV) return "MEDIASUBTYPE_AYUV";
    else if (id == MEDIASUBTYPE_YUY2) return "MEDIASUBTYPE_YUY2";
    else if (id == MEDIASUBTYPE_UYVY) return "MEDIASUBTYPE_UYVY";
    else if (id == MEDIASUBTYPE_YV12) return "MEDIASUBTYPE_YV12";
    else if (id == MEDIASUBTYPE_NV12) return "MEDIASUBTYPE_NV12";
    else if (id == MEDIASUBTYPE_I420) return "MEDIASUBTYPE_I420";
    else if (id == MEDIASUBTYPE_IYUV) return "MEDIASUBTYPE_IYUV";
    else if (id == MEDIASUBTYPE_Y411) return "MEDIASUBTYPE_Y411";
    else if (id == MEDIASUBTYPE_YVU9) return "MEDIASUBTYPE_YVU9";
    else if (id == MEDIASUBTYPE_YVYU) return "MEDIASUBTYPE_YVYU";
    else if (id == MEDIASUBTYPE_MJPG) return "MEDIASUBTYPE_MJPG";
    return "MEDIASUBTYPE_Unknown";
}

std::string WinDSCamera::FormatTypeName(GUID id)
{
    if (id == FORMAT_VideoInfo) return "FORMAT_VideoInfo";
    else if (id == FORMAT_VideoInfo2) return "FORMAT_VideoInfo2";
    return "FORMAT_Unknown";
}

std::string WinDSCamera::RECTToStr(RECT rc)
{
    char buf[30] = {};
    sprintf_s(buf, 30, "(%d, %d, %d, %d)", rc.left, rc.top, rc.right, rc.bottom);
    std::string str = buf;
    return str;
}

std::string WinDSCamera::GUIDToStr(GUID id)
{
    WCHAR strGuid[39];
    int res = ::StringFromGUID2(id, strGuid, 39);
    std::wstring wstr = strGuid;
    std::string str = DUtil::ws2s(wstr);
    return str;
}

std::string WinDSCamera::Dump_AM_MEDIA_TYPE(void* amt)
{
    // https://docs.microsoft.com/en-us/previous-versions/ms779120(v=vs.85)
    std::string ret, temp;
    AM_MEDIA_TYPE* p = (AM_MEDIA_TYPE*)amt;
    if (p == nullptr) return ret;

    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(AM_MEDIA_TYPE) = %d\r\n", sizeof(AM_MEDIA_TYPE));
    ret += buf;

    //DBuffer bufTemp(p, sizeof(AM_MEDIA_TYPE));
    //ret += bufTemp.ToHexList();

    temp = "majortype(16 bytes): ";
    ret += temp;
    temp = WinDSCamera::MajorTypeName(p->majortype);
    ret += temp;
    temp = GUIDToStr(p->majortype);
    ret += temp;
    ret += "\r\n";

    temp = "subtype(16 bytes): ";
    ret += temp;
    temp = WinDSCamera::SubTypeName(p->subtype);
    ret += temp;
    temp = GUIDToStr(p->subtype);
    ret += temp;
    ret += "\r\n";

    // BOOL is int on Windows
    temp = "bFixedSizeSamples(4 byte): ";
    ret += temp;
    temp = DUtil::UInt32ToStr(p->bFixedSizeSamples);
    ret += temp;
    ret += "\r\n";

    temp = "bTemporalCompression(4 byte): ";
    ret += temp;

    temp = DUtil::UInt32ToStr(p->bTemporalCompression);
    ret += temp;
    ret += "\r\n";

    temp = "lSampleSize(4 bytes): ";
    ret += temp;
    temp = DUtil::UInt32ToStr(p->lSampleSize);
    ret += temp;
    ret += "\r\n";

    temp = "formattype(16 bytes): ";
    ret += temp;
    temp = WinDSCamera::FormatTypeName(p->formattype);
    ret += temp;
    temp = GUIDToStr(p->formattype);
    ret += temp;
    ret += "\r\n";

    temp = "pUnk(4/8 bytes): ";
    ret += temp;
    temp = DUtil::AddrToStr(p->pUnk);
    ret += temp;
    ret += "\r\n";

    temp = "cbFormat(4 bytes): ";
    ret += temp;
    temp = DUtil::UInt32ToStr(p->cbFormat);
    ret += temp;
    ret += "\r\n";

    temp = "pbFormat(4/8 bytes): ";
    ret += temp;
    temp = DUtil::AddrToStr(p->pbFormat);
    ret += temp;
    ret += "\r\n";

    //DBuffer bufFormat(p->pbFormat, p->cbFormat);
    //ret += "pbFormat Buffer:\r\n";
    //ret += bufFormat.ToHexList();

    return ret;
}


std::string WinDSCamera::Dump_VIDEOINFOHEADER(void* vih)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd407325(v=vs.85).aspx
    std::string ret, temp;
    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(VIDEOINFOHEADER) = %d\r\n", sizeof(VIDEOINFOHEADER));
    ret += buf;

    return ret;
}

std::string WinDSCamera::Dump_VIDEOINFOHEADER2(void* vih2)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd407326(v=vs.85).aspx
    std::string ret, temp;
    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(VIDEOINFOHEADER2) = %d\r\n", sizeof(VIDEOINFOHEADER2));
    ret += buf;

    return ret;
}


// 拿到 IBaseFilter 的 INPUT IPin
IPin* WinDSCamera::GetInputPin(IBaseFilter* filter) 
{
    HRESULT hr;
    IPin* pin = NULL;
    IEnumPins* pPinEnum = NULL;
    filter->EnumPins(&pPinEnum);
    if (pPinEnum == NULL) {
        return NULL;
    }

    // get first unconnected pin
    hr = pPinEnum->Reset();  // set to first pin

    while (S_OK == pPinEnum->Next(1, &pin, NULL)) 
    {
        PIN_DIRECTION pPinDir;
        pin->QueryDirection(&pPinDir);
        if (PINDIR_INPUT == pPinDir) {  // This is an input pin
            IPin* tempPin = NULL;
            if (S_OK != pin->ConnectedTo(&tempPin)) {  // The pint is not connected
                pPinEnum->Release();
                return pin;
            }
        }
        pin->Release();
    }
    pPinEnum->Release();
    return NULL;
}

IPin* WinDSCamera::GetOutputPin(IBaseFilter* filter, REFGUID Category) 
{
    HRESULT hr;
    IPin* pin = NULL;
    IEnumPins* pPinEnum = NULL;
    filter->EnumPins(&pPinEnum);
    if (pPinEnum == NULL) {
        return NULL;
    }
    // get first unconnected pin
    hr = pPinEnum->Reset();  // set to first pin
    while (S_OK == pPinEnum->Next(1, &pin, NULL)) 
    {
        PIN_DIRECTION pPinDir;
        pin->QueryDirection(&pPinDir);
        if (PINDIR_OUTPUT == pPinDir) {  // This is an output pin
            if (Category == GUID_NULL || PinMatchesCategory(pin, Category)) {
                pPinEnum->Release();
                return pin;
            }
        }
        pin->Release();
        pin = NULL;
    }
    pPinEnum->Release();
    return NULL;
}

DBool WinDSCamera::PinMatchesCategory(IPin* pPin, REFGUID Category) 
{
    BOOL bFound = FALSE;
    IKsPropertySet* pKs = NULL;
    HRESULT hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
    if (SUCCEEDED(hr)) {
        GUID PinCategory;
        DWORD cbReturned;
        hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &PinCategory, sizeof(GUID), &cbReturned);
        if (SUCCEEDED(hr) && (cbReturned == sizeof(GUID))) {
            bFound = (PinCategory == Category);
        }
        pKs->Release();
    }
    return bFound;
}
