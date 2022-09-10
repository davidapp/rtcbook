#pragma once

#include "DTypes.h"
#include "Base/DAtomic.h"
#include <dshow.h>
#include "VideoDefines.h"

class WinDS {
public:
    static DVoid ResetMediaType(AM_MEDIA_TYPE* media_type);
    static HRESULT CopyMediaType(AM_MEDIA_TYPE* target, const AM_MEDIA_TYPE* source);
    static DBool MediaType2DShowCapability(const AM_MEDIA_TYPE* media_type, DVideoFormat* capability);
    static DVoid GetSampleProperties(IMediaSample* sample, AM_SAMPLE2_PROPERTIES* props);
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

