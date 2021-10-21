//
//  WinDSCamera.cpp
//
//  Created by Dai Wei(bmw.dai@gmail.com) on 05/03/2021 for Dream.
//  Copyright 2021. All rights reserved.
//

#include "WinDSCamera.h"
#include "Base/DStdLib.h"
#include <Initguid.h>
#include <mutex>

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
std::recursive_mutex _mutex;

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
// 返回一个 DData 的 DArray，包含：设备的名称，路径(DeviceID)，ProductID
DArray* WinDSCamera::GetDevices() 
{
    if (_dsDevEnum == nullptr) {
        return nullptr;
    }

    // 每次重新创建新的 IEnumMoniker，便于实时更新
    SAFE_RELEASE(_dsMonikerDevEnum)
    HRESULT hr = _dsDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &_dsMonikerDevEnum, 0);
    if (hr != NOERROR || _dsMonikerDevEnum == nullptr) {
        return nullptr;
    }
    _dsMonikerDevEnum->Reset();

    DArray* pArrayRet = DArray::CreateDArray();
    ULONG cFetched = ULONG();
    IMoniker* pM = nullptr;
    // 枚举不到就直接返回空数组
    while (S_OK == _dsMonikerDevEnum->Next(1, &pM, &cFetched)) 
    {
        // 枚举到了就找它的 IPropertyBag 接口
        IPropertyBag* pBag = nullptr;
        hr = pM->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&pBag);
        if (S_OK == hr) 
        {
            DData* pItem = DData::CreateDData(5);

            // 1.设备的名称，"Description"优先，"FriendlyName"兜底，填入UTF8编码
            VARIANT varName;
            VariantInit(&varName);
            hr = pBag->Read(L"Description", &varName, 0);
            if (FAILED(hr)) 
            {
                hr = pBag->Read(L"FriendlyName", &varName, 0);
            }
            if (SUCCEEDED(hr)) 
            {
                DString strName = varName.bstrVal;
                DStringA strNameU8 = strName.ToUTF8();
                pItem->AddStringA(DCAMERA_STRINGA_DEVICE_NAME, strNameU8);
            }

            // 2.设备的路径，"DevicePath"，用这个当作设备ID
            hr = pBag->Read(L"DevicePath", &varName, 0);
            if (SUCCEEDED(hr)) 
            {
                DString strDeviceID = varName.bstrVal;
                DStringA strDeviceIDU8 = strDeviceID.ToUTF8();
                pItem->AddStringA(DCAMERA_STRINGA_DEVICE_ID, strDeviceIDU8);

                // 3.产品ID，通过 Path 中的某一段来获得
                DStringA strProductID = WinDSCamera::GetProductIdFromPath(strDeviceIDU8);
                pItem->AddStringA(DCAMERA_STRINGA_PRODUCT_ID, strProductID);
            }
            VariantClear(&varName);
            pBag->Release();

            pArrayRet->AddData(pItem);
            pItem->Release();
        }
        pM->Release();
    }

	return pArrayRet;
}

// 给定一个视频采集设备的ID，获取这个采集设备的能力
DArray* WinDSCamera::GetDeviceCaps(DStr deviceID) 
{
    DArray* pArrRet = DArray::CreateDArray();
    IBaseFilter* captureDevice = nullptr;
    IPin* outputCapturePin = nullptr;
    IAMStreamConfig* streamConfig = nullptr;
    IAMVideoControl* videoControlConfig = nullptr;

    do
    {
        captureDevice = GetDeviceFilter(deviceID);
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

        int count, size;
        hr = streamConfig->GetNumberOfCapabilities(&count, &size);
        if (FAILED(hr)){
            break;
        }

        // used for FPS
        HRESULT hrVC = captureDevice->QueryInterface(IID_IAMVideoControl, (void**)&videoControlConfig);

        AM_MEDIA_TYPE* pmt = NULL;
        VIDEO_STREAM_CONFIG_CAPS caps = {};
        for (int32_t tmp = 0; tmp < count; ++tmp)
        {
            hr = streamConfig->GetStreamCaps(tmp, &pmt, reinterpret_cast<BYTE*>(&caps));
            if (FAILED(hr)) {
                continue;
            }

            // 枚举视频类型，看视频的格式
            if (pmt->majortype == MEDIATYPE_Video && pmt->formattype == FORMAT_VideoInfo)
            {
                DData* pData = DData::CreateDData(10);
                // 所属的平台
                pData->AddUInt32(DPFINFO_UINT32_OS_PLATFORM, 1); //Windows

                // 支持的分辨率
                VIDEOINFOHEADER* h = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
                pData->AddUInt32(DPFINFO_UINT32_WIDTH, h->bmiHeader.biWidth);
                pData->AddUInt32(DPFINFO_UINT32_HEIGHT, h->bmiHeader.biHeight);
                
                // 支持的帧率
                DInt64 avgTimePerFrame = h->AvgTimePerFrame;
                if (hrVC == S_OK)
                {
                    DUInt64* frameDurationList;
                    DUInt64 max_fps;
                    DUInt32 listSize;
                    SIZE size;
                    size.cx = h->bmiHeader.biWidth;
                    size.cy = h->bmiHeader.biHeight;
                    hrVC = videoControlConfig->GetFrameRateList(outputCapturePin, tmp, size, (long*)&listSize, (LONGLONG**)&frameDurationList);
                    if (hrVC == S_OK && listSize > 0 && 0 != (max_fps = GetMaxOfFrameArray(frameDurationList, listSize)))
                    {
                        pData->AddUInt32(DPFINFO_UINT32_FRAMERATE, static_cast<int>(10000000 / max_fps));
                        pData->AddBool(DPFINFO_BOOL_IS_MAX_FRAMERATE, true);
                    }
                    else
                    {
                        if (avgTimePerFrame > 0)
                            pData->AddUInt32(DPFINFO_UINT32_FRAMERATE, static_cast<int>(10000000 / avgTimePerFrame));
                        else
                            pData->AddUInt32(DPFINFO_UINT32_FRAMERATE, 0);

                        pData->AddBool(DPFINFO_BOOL_IS_MAX_FRAMERATE, false);
                    }
                }
                else
                {   
                    // use existing method in case IAMVideoControl is not supported
                    if (avgTimePerFrame > 0)
                        pData->AddUInt32(DPFINFO_UINT32_FRAMERATE, static_cast<int>(10000000 / avgTimePerFrame));
                    else
                        pData->AddUInt32(DPFINFO_UINT32_FRAMERATE, 0);

                    pData->AddBool(DPFINFO_BOOL_IS_MAX_FRAMERATE, false);
                }

                DPixelFormat mformat = DPixelFormat::kUnknown;
                if (pmt->subtype == MEDIASUBTYPE_RGB24)
                {
                    mformat = DPixelFormat::kRGB;
                }
                else if (pmt->subtype == MEDIASUBTYPE_I420)
                {
                    mformat = DPixelFormat::kI420;
                }
                else if (pmt->subtype == MEDIASUBTYPE_IYUV)
                {
                    mformat = DPixelFormat::kIYUV;
                }
                else if (pmt->subtype == MEDIASUBTYPE_YUY2)
                {
                    mformat = DPixelFormat::kYUY2;
                }
                else
                {
                    WCHAR strGuid[39];
                    StringFromGUID2(pmt->subtype, strGuid, 39);
                    continue;
                }
                pData->AddUInt32(DPFINFO_UINT32_PIXEL_FORMAT, (DUInt32)mformat);

                pArrRet->AddData(pData);
                pData->Release();
            }

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

    return pArrRet;
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

// 通过 DeviceID 拿到一个 IBaseFilter*
// 方法是遍历，匹配 "DevicePath"，BindToObject IID_IBaseFilter
IBaseFilter* WinDSCamera::GetDeviceFilter(DCStr deviceUniqueIdUTF8) 
{
    IBaseFilter* captureFilter = nullptr;

    do
    {
        SAFE_RELEASE(_dsMonikerDevEnum);
        HRESULT hr = _dsDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &_dsMonikerDevEnum, 0);
        if (hr != S_OK)
        {
            break;
        }

        _dsMonikerDevEnum->Reset();
        ULONG cFetched = 0;
        IMoniker* pM = NULL;
        while (S_OK == _dsMonikerDevEnum->Next(1, &pM, &cFetched))
        {
            IPropertyBag* pBag;
            hr = pM->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&pBag);
            if (hr != S_OK)
            {
                pM->Release();
                continue;
            }

            VARIANT varName;
            VariantInit(&varName);
            hr = pBag->Read(L"DevicePath", &varName, 0);
            if (SUCCEEDED(hr))
            {
                DString strName = varName.bstrVal;
                DStringA strNameU8 = strName.ToUTF8();
                if (strNameU8 == deviceUniqueIdUTF8)
                {
                    pM->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&captureFilter);
                }
            }
            VariantClear(&varName);
            pBag->Release();
            pM->Release();

            if (captureFilter)
            {
                break;
            }
        }
    } while (0);

    return captureFilter;
}

// 通过 IBaseFilter 查询到 IID_ISpecifyPropertyPages
DBool WinDSCamera::ShowSettingDialog(DCStr deviceUniqueIdUTF8, DVoid* parentWindow, DUInt32 positionX, DUInt32 positionY) 
{
    std::unique_lock<std::recursive_mutex> ul(_mutex);
    DBool bResult = false;
    IBaseFilter* filter = nullptr;
    do
    {
        filter = GetDeviceFilter(deviceUniqueIdUTF8);
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

// Example of device path:
// "\\?\usb#vid_0408&pid_2010&mi_00#7&258e7aaf&0&0000#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\global"
// "\\?\avc#sony&dv-vcr&camcorder&dv#65b2d50301460008#{65e8773d-8f56-11d0-a3b9-00a0c9223196}\global"
DStringA WinDSCamera::GetProductIdFromPath(DStringA& path) 
{
    DStringA strRet;
    DChar* startPos = path.GetStr() + path.FindOneOf("\\\\?\\");
    if (startPos < path.GetStr()) {
        strRet = "";
        return strRet;
    }
    startPos += 4;

    DChar* pos = DStdLib::strchr(startPos, '&');
    if (!pos || pos >= path.GetStr() + DStdLib::strlen(path.GetStr())) 
    {
        strRet = "";
        return strRet;
    }
    pos = DStdLib::strchr(pos + 1, '&');
    DUInt32 bytesToCopy = (DUInt32)(pos - startPos);
    if (pos && (bytesToCopy <= (DUInt32)path.GetDataLength()) && bytesToCopy <= 1024) 
    {
        strRet.SetStr(startPos, bytesToCopy);
    }
    else 
    {
        strRet = "";
        return strRet;
    }
    return strRet;
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

#if defined(D_INCLUDE_TEST) && (D_INCLUDE_TEST==1)
#include "Base/DTest.h"

DVoid WinDSCamera::TestDevices()
{
    WinDSCamera camera;
    camera.Init();
    DArray* pArr = camera.GetDevices();
    pArr->Dump();
    DStringA strA = pArr->GetData(0)->GetStringA(1);
    DArray* pCapArr = camera.GetDeviceCaps(strA.GetStr());
    pCapArr->Dump();
    pArr->Release();
}


#endif
