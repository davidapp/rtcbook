#pragma once

#include "DTypes.h"
#include <dshow.h>
#include "WinDSCapturePin.h"

class WinDSSinkFilter : public IBaseFilter 
{
public:
    WinDSSinkFilter(HWND hWnd);
    virtual ~WinDSSinkFilter();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IPersist
    STDMETHOD(GetClassID)(CLSID* clsid) override;

    // IMediaFilter
    STDMETHOD(GetState)(DWORD msecs, FILTER_STATE* state) override;
    STDMETHOD(SetSyncSource)(IReferenceClock* clock) override;
    STDMETHOD(GetSyncSource)(IReferenceClock** clock) override;
    STDMETHOD(Pause)() override;
    STDMETHOD(Run)(REFERENCE_TIME start) override;
    STDMETHOD(Stop)() override;

    // IBaseFilter
    STDMETHOD(EnumPins)(IEnumPins** pins) override;
    STDMETHOD(FindPin)(LPCWSTR id, IPin** pin) override;
    STDMETHOD(QueryFilterInfo)(FILTER_INFO* info) override;
    STDMETHOD(JoinFilterGraph)(IFilterGraph* graph, LPCWSTR name) override;
    STDMETHOD(QueryVendorInfo)(LPWSTR* vendor_info) override;

public:
    FILTER_STATE m_state = State_Stopped;
    FILTER_INFO m_info = {};
    WinDSCaptureInputPin* m_input_pin;
    HWND m_hWnd;

private:
    DAtomInt32 m_refCount;
};
