#pragma once

#include "DTypes.h"
#include "Base/DAtomic.h"
#include "Video/VideoDefines.h"
#include <dshow.h>
#include <string>
#include <Initguid.h>

#define SAFE_RELEASE(p) \
    if (p) {            \
        (p)->Release(); \
        (p) = NULL;     \
    }

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(MEDIASUBTYPE_HDYC, 0x43594448, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);


class WinDS {
public:
    static DVoid ResetMediaType(AM_MEDIA_TYPE* media_type);
    static HRESULT CopyMediaType(AM_MEDIA_TYPE* target, const AM_MEDIA_TYPE* source);
    static DBool MediaType2DShowCapability(const AM_MEDIA_TYPE* media_type, struct tagDVideoFormat* capability);
    static DVoid GetSampleProperties(IMediaSample* sample, AM_SAMPLE2_PROPERTIES* props);

    static std::string MajorTypeName(GUID id);
    static std::string SubTypeName(GUID id);
    static std::string FormatTypeName(GUID id);

    static std::string RECTToStr(RECT rc);
    static std::string GUIDToStr(GUID id);
    static std::string FRArrayToStr(void* p, DUInt32 len);
    static std::string Dump_AM_MEDIA_TYPE(void* amt);
    static std::string Dump_VIDEOINFOHEADER(void* vih);
    static std::string Dump_VIDEOINFOHEADER2(void* vih2);

    static IPin* GetInputPin(IBaseFilter* filter);
    static IPin* GetOutputPin(IBaseFilter* filter, REFGUID Category);
    static DBool PinMatchesCategory(IPin* pPin, REFGUID Category);
    static DVoid FreeMediaType(AM_MEDIA_TYPE& mt);
    static DUInt64 GetMaxOfFrameArray(DInt64* maxFps, DUInt32 size);
};




class WinDSEnumPins : public IEnumPins
{
public:
    WinDSEnumPins(IPin* pin) : m_pin(pin)
    {
        m_pin->AddRef();
    }

    virtual ~WinDSEnumPins() {
        m_pin->Release();
    }

private:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override 
    {
        if (riid == IID_IUnknown || riid == IID_IEnumPins) {
            *ppv = static_cast<IEnumPins*>(this);
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    
    IFACEMETHODIMP_(ULONG) AddRef() override 
    {
        m_refCount++;
        return 1;
    }
    
    IFACEMETHODIMP_(ULONG) Release() override
    {
        m_refCount--;
        if (m_refCount == 0) {
            delete this;
            return 0;
        }
        return 1;
    }

    STDMETHOD(Clone)(IEnumPins** pins)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(Next)(ULONG count, IPin** pins, ULONG* fetched)
    {
        if (m_pos > 0) 
        {
            if (fetched)
                *fetched = 0;
            return S_FALSE;
        }

        ++m_pos;
        pins[0] = m_pin;
        pins[0]->AddRef();
        if (fetched)
            *fetched = 1;
        return count == 1 ? S_OK : S_FALSE;
    }

    STDMETHOD(Skip)(ULONG count)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(Reset)()
    {
        m_pos = 0;
        return S_OK;
    }

    IPin* m_pin = nullptr;
    DInt32 m_pos = 0;
    DAtomInt32 m_refCount;
};

