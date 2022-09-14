
#include "WinDS.h"
#include "Base/DUtil.h"
#include "File/DBmpFile.h"
#include <dvdmedia.h>

#pragma comment(lib, "strmiids.lib")


/*
class PinMediaTypeEnumerator final : public IEnumMediaTypes
{
public:
    explicit PinMediaTypeEnumerator(VIDEOINFOHEADER* pvi, GUID subtype) : index_(0) {
        subtype_ = subtype;
        vih_ = *pvi;
    }
    // Implement from IUnknown.
    IFACEMETHODIMP QueryInterface(REFIID iid, void** object_ptr) override {
        if (iid == IID_IEnumMediaTypes || iid == IID_IUnknown) {
            AddRef();
            *object_ptr = static_cast<IEnumMediaTypes*>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    IFACEMETHODIMP_(ULONG) AddRef() override {
        return AddRefImpl();
    }
    IFACEMETHODIMP_(ULONG) Release() override {
        return ReleaseImpl();
    }
    // Implement IEnumMediaTypes.
    IFACEMETHODIMP Next(ULONG count,
        AM_MEDIA_TYPE** types,
        ULONG* fetched) override {
        ULONG types_fetched = 0;
        while (types_fetched < count) {
            // Allocate AM_MEDIA_TYPE that we will store the media type in.
            AM_MEDIA_TYPE* type = reinterpret_cast<AM_MEDIA_TYPE*>(
                CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE)));
            if (!type) {
                FreeAllocatedMediaTypes(types_fetched, types);
                return E_OUTOFMEMORY;
            }
            ZeroMemory(type, sizeof(AM_MEDIA_TYPE));
            // Allocate a VIDEOINFOHEADER and connect it to the AM_MEDIA_TYPE.
            type->cbFormat = sizeof(VIDEOINFOHEADER);
            BYTE* format =
                reinterpret_cast<BYTE*>(CoTaskMemAlloc(sizeof(VIDEOINFOHEADER)));
            if (!format) {
                CoTaskMemFree(type);
                FreeAllocatedMediaTypes(types_fetched, types);
                return E_OUTOFMEMORY;
            }
            type->pbFormat = format;
            // Get the media type from the pin.
            if (GetValidMediaType(index_++, type)) {
                types[types_fetched++] = type;
            }
            else {
                CoTaskMemFree(format);
                CoTaskMemFree(type);
                break;
            }
        }
        if (fetched)
            *fetched = types_fetched;
        return types_fetched == count ? S_OK : S_FALSE;
    }
    IFACEMETHODIMP Skip(ULONG count) override {
        index_ += count;
        return S_OK;
    }
    IFACEMETHODIMP Reset() override {
        index_ = 0;
        return S_OK;
    }
    IFACEMETHODIMP Clone(IEnumMediaTypes** clone) override {
        PinMediaTypeEnumerator* type_enum = new PinMediaTypeEnumerator(&this->vih_, this->subtype_);
        type_enum->AddRef();
        type_enum->index_ = index_;
        *clone = type_enum;
        return S_OK;
    }
private:
    DWORD GetArea(VIDEOINFOHEADER vi) {
        return vi.bmiHeader.biWidth * vi.bmiHeader.biHeight;
    }
    bool GetValidMediaType(int index, AM_MEDIA_TYPE* media_type) {
        if (media_type->cbFormat < sizeof(VIDEOINFOHEADER))
            return false;
        VIDEOINFOHEADER* const pvi =
            reinterpret_cast<VIDEOINFOHEADER*>(media_type->pbFormat);
        ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));
        pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pvi->bmiHeader.biPlanes = 1;
        pvi->bmiHeader.biClrImportant = 0;
        pvi->bmiHeader.biClrUsed = 0;
        pvi->AvgTimePerFrame = vih_.AvgTimePerFrame;
        media_type->majortype = MEDIATYPE_Video;
        media_type->formattype = FORMAT_VideoInfo;
        media_type->bTemporalCompression = FALSE;
        if (subtype_ == MEDIASUBTYPE_MJPG) {
            if (index != 0)
                return false;
            pvi->bmiHeader = vih_.bmiHeader;
            return true;
        }
        pvi->bmiHeader.biWidth = vih_.bmiHeader.biWidth;
        pvi->bmiHeader.biHeight = vih_.bmiHeader.biHeight;
        switch (index) {
        case 0: {
            pvi->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');
            pvi->bmiHeader.biBitCount = 12;  // bit per pixel
            pvi->bmiHeader.biSizeImage = GetArea(vih_) * 3 / 2;
            media_type->subtype = MEDIASUBTYPE_I420;
            break;
        }
        case 1: {
            pvi->bmiHeader.biCompression = MAKEFOURCC('Y', 'U', 'Y', '2');
            pvi->bmiHeader.biBitCount = 16;
            pvi->bmiHeader.biSizeImage = GetArea(vih_) * 2;
            media_type->subtype = MEDIASUBTYPE_YUY2;
            break;
        }
        case 2: {
            pvi->bmiHeader.biCompression = MAKEFOURCC('U', 'Y', 'V', 'Y');
            pvi->bmiHeader.biBitCount = 16;
            pvi->bmiHeader.biSizeImage = GetArea(vih_) * 2;
            media_type->subtype = MEDIASUBTYPE_UYVY;
            break;
        }
        case 3: {
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount = 24;
            pvi->bmiHeader.biSizeImage = GetArea(vih_) * 3;
            media_type->subtype = MEDIASUBTYPE_RGB24;
            break;
        }
        case 4: {
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount = 32;
            pvi->bmiHeader.biSizeImage = GetArea(vih_) * 4;
            media_type->subtype = MEDIASUBTYPE_RGB32;
            break;
        }
        default:
            return false;
        }
        media_type->bFixedSizeSamples = TRUE;
        media_type->lSampleSize = pvi->bmiHeader.biSizeImage;
        return true;
    }
    ~PinMediaTypeEnumerator() {}
    void FreeAllocatedMediaTypes(ULONG allocated, AM_MEDIA_TYPE** types) {
        for (ULONG i = 0; i < allocated; ++i) {
            CoTaskMemFree(types[i]->pbFormat);
            CoTaskMemFree(types[i]);
        }
    }
    GUID subtype_;
    VIDEOINFOHEADER vih_;
    int index_;
};*/

/*
DBool WinDS::MediaType2DShowCapability(const AM_MEDIA_TYPE* media_type, DVideoFormat* capability) 
{
    if (!media_type || media_type->majortype != MEDIATYPE_Video || !media_type->pbFormat) {
        return false;
    }

    const BITMAPINFOHEADER* bih = nullptr;
    if (media_type->formattype == FORMAT_VideoInfo) {
        bih = &reinterpret_cast<VIDEOINFOHEADER*>(media_type->pbFormat)->bmiHeader;
    }
    else if (media_type->formattype != FORMAT_VideoInfo2) {
        bih = &reinterpret_cast<VIDEOINFOHEADER2*>(media_type->pbFormat)->bmiHeader;
    }
    else {
        return false;
    }

    const GUID& sub_type = media_type->subtype;
    if (sub_type == MEDIASUBTYPE_MJPG && bih->biCompression == MAKEFOURCC('M', 'J', 'P', 'G')) {
        capability->format = DPixelFmt::MJPG;
    }
    else if (sub_type == MEDIASUBTYPE_I420 && bih->biCompression == MAKEFOURCC('I', '4', '2', '0')) {
        capability->format = DPixelFmt::I420;
    }
    else if (sub_type == MEDIASUBTYPE_YUY2 && bih->biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) {
        capability->format = DPixelFmt::YUY2;
    }
    else if (sub_type == MEDIASUBTYPE_RGB32) {
        capability->format = DPixelFmt::BGRA;
    }
    else if (sub_type == MEDIASUBTYPE_RGB24 && bih->biCompression == BI_RGB) {
        capability->format = DPixelFmt::RGB24;
    }
    else {
        return false;
    }

    capability->width = bih->biWidth;

    if (sub_type == MEDIASUBTYPE_RGB24 && bih->biHeight > 0)
    {
        capability->height = -(bih->biHeight);
    }
    else 
    {
        capability->height = abs(bih->biHeight);
    }

    return true;
}
*/

DVoid WinDS::ResetMediaType(AM_MEDIA_TYPE* media_type)
{
    if (!media_type)
        return;

    if (media_type->cbFormat != 0) 
    {
        CoTaskMemFree(media_type->pbFormat);
        media_type->cbFormat = 0;
        media_type->pbFormat = nullptr;
    }

    if (media_type->pUnk) 
    {
        media_type->pUnk->Release();
        media_type->pUnk = nullptr;
    }
}

HRESULT WinDS::CopyMediaType(AM_MEDIA_TYPE* target, const AM_MEDIA_TYPE* source)
{
    *target = *source;
    if (source->cbFormat != 0) {
        target->pbFormat = reinterpret_cast<BYTE*>(CoTaskMemAlloc(source->cbFormat));
        if (target->pbFormat == nullptr) {
            target->cbFormat = 0;
            return E_OUTOFMEMORY;
        }
        else {
            CopyMemory(target->pbFormat, source->pbFormat, target->cbFormat);
        }
    }

    if (target->pUnk != nullptr)
        target->pUnk->AddRef();

    return S_OK;
}

bool WinDS::MediaType2DShowCapability(const AM_MEDIA_TYPE* media_type, struct tagDVideoFormat* capability) {
    if (!media_type || media_type->majortype != MEDIATYPE_Video || !media_type->pbFormat) {
        return false;
    }

    const BITMAPINFOHEADER* bih = nullptr;
    if (media_type->formattype == FORMAT_VideoInfo) {
        bih = &reinterpret_cast<VIDEOINFOHEADER*>(media_type->pbFormat)->bmiHeader;
    }
    else if (media_type->formattype != FORMAT_VideoInfo2) {
        bih = &reinterpret_cast<VIDEOINFOHEADER2*>(media_type->pbFormat)->bmiHeader;
    }
    else {
        return false;
    }

    
    const GUID& sub_type = media_type->subtype;
    /*
    if (sub_type == MEDIASUBTYPE_MJPG && bih->biCompression == MAKEFOURCC('M', 'J', 'P', 'G')) {
        capability->format = WINCapPixelFormat::kMJPEG;
    }
    else if (sub_type == MEDIASUBTYPE_I420 && bih->biCompression == MAKEFOURCC('I', '4', '2', '0')) {
        capability->format = WINCapPixelFormat::kI420;
    }
    else if (sub_type == MEDIASUBTYPE_YUY2 && bih->biCompression == MAKEFOURCC('Y', 'U', 'Y', '2')) {
        capability->format = WINCapPixelFormat::kYUY2;
    }
    else if (sub_type == MEDIASUBTYPE_UYVY && bih->biCompression == MAKEFOURCC('U', 'Y', 'V', 'Y')) {
        capability->format = WINCapPixelFormat::kUYVY;
    }
    else if (sub_type == MEDIASUBTYPE_NV12 && bih->biCompression == MAKEFOURCC('N', 'V', '1', '2')) {
        capability->format = WINCapPixelFormat::kNV12;
    }
    else if (sub_type == MEDIASUBTYPE_HDYC) {
        capability->format = WINCapPixelFormat::kUYVY;
    }
    else if (sub_type == MEDIASUBTYPE_RGB32) {
        capability->format = WINCapPixelFormat::kBGRA;
    }
    else if (sub_type == MEDIASUBTYPE_RGB24 && bih->biCompression == BI_RGB) {
        capability->format = WINCapPixelFormat::kRGB24;
    }
    else {
        return false;
    }
    */

    // Store the incoming width and height
    capability->width = bih->biWidth;

    // Store the incoming height,
    // for RGB24 we assume the frame to be upside down
    if (sub_type == MEDIASUBTYPE_RGB24 && bih->biHeight > 0) {
        capability->height = -(bih->biHeight);
    }
    else {
        capability->height = abs(bih->biHeight);
    }

    return true;
}

DVoid WinDS::GetSampleProperties(IMediaSample* sample, AM_SAMPLE2_PROPERTIES* props)
{
    HRESULT hr = S_OK;
    IMediaSample2* sample2;
    hr = sample->QueryInterface(IID_IMediaSample2, (void**)& sample2);
    if (SUCCEEDED(hr)) {
        sample2->GetProperties(sizeof(*props), reinterpret_cast<BYTE*>(props));
        return;
    }

    props->cbData = sizeof(*props);
    props->dwTypeSpecificFlags = 0;
    props->dwStreamId = AM_STREAM_MEDIA;
    props->dwSampleFlags = 0;

    if (sample->IsDiscontinuity() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;

    if (sample->IsPreroll() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_PREROLL;

    if (sample->IsSyncPoint() == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;

    if (SUCCEEDED(sample->GetTime(&props->tStart, &props->tStop)))
        props->dwSampleFlags |= AM_SAMPLE_TIMEVALID | AM_SAMPLE_STOPVALID;

    if (sample->GetMediaType(&props->pMediaType) == S_OK)
        props->dwSampleFlags |= AM_SAMPLE_TYPECHANGED;

    sample->GetPointer(&props->pbBuffer);
    props->lActual = sample->GetActualDataLength();
    props->cbBuffer = sample->GetSize();
}


DVoid WinDS::FreeMediaType(AM_MEDIA_TYPE& mt)
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

DUInt64 WinDS::GetMaxOfFrameArray(DUInt64* maxFps, DUInt32 size)
{
    DUInt64 maxFPS = maxFps[0];
    for (DUInt32 i = 0; i < size; i++)
    {
        if (maxFPS > maxFps[i])
            maxFPS = maxFps[i];
    }
    return maxFPS;
}

std::string WinDS::MajorTypeName(GUID id)
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

std::string WinDS::SubTypeName(GUID id)
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

std::string WinDS::FormatTypeName(GUID id)
{
    if (id == FORMAT_VideoInfo) return "FORMAT_VideoInfo";
    else if (id == FORMAT_VideoInfo2) return "FORMAT_VideoInfo2";
    return "FORMAT_Unknown";
}

std::string WinDS::RECTToStr(RECT rc)
{
    char buf[30] = {};
    sprintf_s(buf, 30, "(%d, %d, %d, %d)", rc.left, rc.top, rc.right, rc.bottom);
    std::string str = buf;
    return str;
}

std::string WinDS::GUIDToStr(GUID id)
{
    WCHAR strGuid[39];
    int res = ::StringFromGUID2(id, strGuid, 39);
    std::wstring wstr = strGuid;
    std::string str = DUtil::ws2s(wstr);
    return str;
}

std::string WinDS::FRArrayToStr(void* p, DUInt32 len)
{
    DUInt64* ptr = (DUInt64*)p;
    std::string ret;
    ret += "[";
    for (DUInt32 i = 0; i < len; i++) {
        ret += DUtil::UInt32ToStr((DUInt32)(10000000 / ptr[i]));
        if (i != len - 1) {
            ret += ", ";
        }
    }
    ret += "]\r\n";
    return ret;
}

std::string WinDS::Dump_AM_MEDIA_TYPE(void* amt)
{
    // https://docs.microsoft.com/en-us/previous-versions/ms779120(v=vs.85)
    std::string ret, temp;
    AM_MEDIA_TYPE* p = (AM_MEDIA_TYPE*)amt;
    if (p == nullptr) return ret;

    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(AM_MEDIA_TYPE) = %d\r\n", (DInt32)sizeof(AM_MEDIA_TYPE));
    ret += buf;

    //DBuffer bufTemp(p, sizeof(AM_MEDIA_TYPE));
    //ret += bufTemp.ToHexList();

    temp = "majortype(16 bytes): ";
    ret += temp;
    temp = WinDS::MajorTypeName(p->majortype);
    ret += temp;
    temp = GUIDToStr(p->majortype);
    ret += temp;
    ret += "\r\n";

    temp = "subtype(16 bytes): ";
    ret += temp;
    temp = WinDS::SubTypeName(p->subtype);
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
    temp = WinDS::FormatTypeName(p->formattype);
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

std::string WinDS::Dump_VIDEOINFOHEADER(void* vih)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd407325(v=vs.85).aspx
    std::string ret, temp;

    VIDEOINFOHEADER* h = (VIDEOINFOHEADER*)vih;

    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(VIDEOINFOHEADER) = %d\r\n", (DInt32)sizeof(VIDEOINFOHEADER));
    ret += buf;

    temp = "rcSource(4*4 bytes):";
    ret += temp;
    ret += RECTToStr(h->rcSource);
    ret += "\r\n";

    temp = "rcTarget(4*4 bytes):";
    ret += temp;
    ret += RECTToStr(h->rcTarget);
    ret += "\r\n";

    temp = "dwBitRate(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwBitRate); // biSizeImage * MaxFRS * 8
    ret += "\r\n";

    temp = "dwBitErrorRate(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwBitErrorRate);
    ret += "\r\n";

    temp = "AvgTimePerFrame(8 bytes):";
    ret += temp;
    ret += DUtil::Int64ToStr(h->AvgTimePerFrame);
    ret += "\r\n";

    temp = "bmiHeader(40 bytes):\r\n";
    ret += temp;
    ret += DBmpFile::DumpBitmapInfoHeader(&(h->bmiHeader));
    ret += "\r\n";

    return ret;
}

std::string WinDS::Dump_VIDEOINFOHEADER2(void* vih2)
{
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd407326(v=vs.85).aspx
    std::string ret, temp;

    VIDEOINFOHEADER2* h = (VIDEOINFOHEADER2*)vih2;

    char buf[128] = {};
    sprintf_s(buf, 128, "sizeof(VIDEOINFOHEADER2) = %d\r\n", (DInt32)sizeof(VIDEOINFOHEADER2));
    ret += buf;

    temp = "rcSource(4*4 bytes):";
    ret += temp;
    ret += RECTToStr(h->rcSource);
    ret += "\r\n";

    temp = "rcTarget(4*4 bytes):";
    ret += temp;
    ret += RECTToStr(h->rcTarget);
    ret += "\r\n";

    temp = "dwBitRate(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwBitRate);
    ret += "\r\n";

    temp = "dwBitErrorRate(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwBitErrorRate);
    ret += "\r\n";

    temp = "AvgTimePerFrame(8 bytes):";
    ret += temp;
    ret += DUtil::Int64ToStr(h->AvgTimePerFrame);
    ret += "\r\n";

    temp = "dwInterlaceFlags(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwInterlaceFlags);
    ret += "\r\n";

    temp = "dwCopyProtectFlags(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwCopyProtectFlags);
    ret += "\r\n";

    temp = "dwPictAspectRatioX(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwPictAspectRatioX);
    ret += "\r\n";

    temp = "dwPictAspectRatioY(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwPictAspectRatioY);
    ret += "\r\n";

    temp = "dwControlFlags(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwControlFlags);
    ret += "\r\n";

    temp = "dwReserved2(4 bytes):";
    ret += temp;
    ret += DUtil::UInt32ToStr(h->dwReserved2);
    ret += "\r\n";

    temp = "bmiHeader(40 bytes):\r\n";
    ret += temp;
    ret += DBmpFile::DumpBitmapInfoHeader(&(h->bmiHeader));
    ret += "\r\n";

    return ret;
}


// ÄÃµ½ IBaseFilter µÄ INPUT IPin
IPin* WinDS::GetInputPin(IBaseFilter* filter)
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

IPin* WinDS::GetOutputPin(IBaseFilter* filter, REFGUID Category)
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

DBool WinDS::PinMatchesCategory(IPin* pPin, REFGUID Category)
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
