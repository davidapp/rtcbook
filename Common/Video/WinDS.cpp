
#include "WinDS.h"
#include <dvdmedia.h>

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(MEDIASUBTYPE_HDYC, 0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);


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
