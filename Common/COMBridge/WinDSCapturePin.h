#pragma once

#include "DTypes.h"
#include <dshow.h>
#include <atomic>
#include "Video/DVideoDefines.h"
#include "Base/DAtomic.h"


class WinDSCaptureInputPin : public IMemInputPin, public IPin
{
public:
    WinDSCaptureInputPin(IBaseFilter* filter, DVoid* pCallback, DVoid* pUserData);
    virtual ~WinDSCaptureInputPin();

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) override;
    IFACEMETHODIMP_(ULONG) AddRef() override;
    IFACEMETHODIMP_(ULONG) Release() override;

    // IPin
    STDMETHOD(Connect)(IPin* receive_pin, const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(ReceiveConnection)(IPin* connector, const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(Disconnect)() override;
    STDMETHOD(ConnectedTo)(IPin** pin) override;
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(QueryPinInfo)(PIN_INFO* info) override;
    STDMETHOD(QueryDirection)(PIN_DIRECTION* pin_dir) override;
    STDMETHOD(QueryId)(LPWSTR* id) override;
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE* media_type) override;
    STDMETHOD(EnumMediaTypes)(IEnumMediaTypes** types) override;
    STDMETHOD(QueryInternalConnections)(IPin** pins, ULONG* count) override;
    STDMETHOD(EndOfStream)() override;
    STDMETHOD(BeginFlush)() override;
    STDMETHOD(EndFlush)() override;
    STDMETHOD(NewSegment)(REFERENCE_TIME start, REFERENCE_TIME stop, double rate) override;

    // IMemInputPin
    STDMETHOD(GetAllocator)(IMemAllocator** allocator) override;
    STDMETHOD(NotifyAllocator)(IMemAllocator* allocator, BOOL read_only) override;
    STDMETHOD(GetAllocatorRequirements)(ALLOCATOR_PROPERTIES* props) override;
    STDMETHOD(Receive)(IMediaSample* sample) override;
    STDMETHOD(ReceiveMultiple)(IMediaSample** samples, long count, long* processed) override;
    STDMETHOD(ReceiveCanBlock)() override;

    DVoid OnFilterActivated();
    DVoid OnFilterDeactivated();

private:
    DVoid ClearAllocator(bool decommit);
    HRESULT CheckDirection(IPin* pin) const;
    DBool IsStopped() const;

    IMemAllocator* allocator_ = nullptr;

    DAtomInt32 m_refCount;
    PIN_INFO m_info = {};
    IPin* m_connected_pin = nullptr;

    AM_MEDIA_TYPE m_media_type = {};
    DVideoFormat m_final_fmt;

    DVoid* m_pCallback;
    DVoid* m_pUserData;
};
