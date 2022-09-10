
#include "WinDS.h"
#include "WinDSCapturePin.h"
#include <dvdmedia.h>
#include "VideoDefines.h"

WinDSCaptureInputPin::WinDSCaptureInputPin(IBaseFilter* filter)
{
    m_info.pFilter = filter;
    m_info.dir = PINDIR_INPUT;
    m_refCount = 0;
}

WinDSCaptureInputPin::~WinDSCaptureInputPin() 
{
    WinDS::ResetMediaType(&m_media_type);
}

// IUnknown
STDMETHODIMP WinDSCaptureInputPin::QueryInterface(REFIID riid, void** ppv)
{
    (*ppv) = nullptr;

    if (riid == IID_IUnknown || riid == IID_IMemInputPin) {
        *ppv = static_cast<IMemInputPin*>(this);
    }
    else if (riid == IID_IPin) {
        *ppv = static_cast<IPin*>(this);
    }

    if (!(*ppv))
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

IFACEMETHODIMP_(ULONG) WinDSCaptureInputPin::AddRef()
{
    m_refCount++;
    return 1;
}

IFACEMETHODIMP_(ULONG) WinDSCaptureInputPin::Release()
{
    m_refCount--;
    if (m_refCount == 0) {
        delete this;
        return 0;
    }
    return 1;
}

// IPin
STDMETHODIMP WinDSCaptureInputPin::Connect(IPin* receive_pin, const AM_MEDIA_TYPE* media_type)
{
    return E_NOTIMPL;
}

STDMETHODIMP WinDSCaptureInputPin::ReceiveConnection(IPin* connector, const AM_MEDIA_TYPE* media_type)
{
    if (!IsStopped()) {
        return VFW_E_WRONG_STATE;
    }

    if (m_connected_pin) {
        return VFW_E_ALREADY_CONNECTED;
    }

    HRESULT hr = CheckDirection(connector);
    if (FAILED(hr)) {
        return hr;
    }

    //if (!WinDS::MediaType2DShowCapability(media_type, &m_final_fmt)) {
    //    return VFW_E_TYPE_NOT_ACCEPTED;
    //}

    m_connected_pin = connector;
    WinDS::ResetMediaType(&m_media_type);
    WinDS::CopyMediaType(&m_media_type, media_type);

    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::Disconnect()
{
    if (!IsStopped())
        return VFW_E_NOT_STOPPED;

    if (!m_connected_pin)
        return S_FALSE;

    ClearAllocator(true);
    m_connected_pin = nullptr;

    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::ConnectedTo(IPin** pin)
{
    if (!m_connected_pin)
        return VFW_E_NOT_CONNECTED;

    *pin = m_connected_pin;
    m_connected_pin->AddRef();

    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::ConnectionMediaType(AM_MEDIA_TYPE* media_type)
{
    if (!m_connected_pin)
        return VFW_E_NOT_CONNECTED;

    WinDS::CopyMediaType(media_type, &m_media_type);

    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::QueryPinInfo(PIN_INFO* info)
{
    *info = m_info;
    if (m_info.pFilter)
        m_info.pFilter->AddRef();
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::QueryDirection(PIN_DIRECTION* pin_dir)
{
    *pin_dir = m_info.dir;
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::QueryId(LPWSTR* id)
{
    size_t len = lstrlenW(m_info.achName);
    *id = reinterpret_cast<LPWSTR>(CoTaskMemAlloc((len + 1) * sizeof(wchar_t)));
    lstrcpyW(*id, m_info.achName);
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::QueryAccept(const AM_MEDIA_TYPE* media_type)
{
    if (!IsStopped()) {
        return VFW_E_WRONG_STATE;
    }

    DVideoFormat capability(m_final_fmt);
    return true;// WinDS::MediaType2DShowCapability(media_type, &capability) ? S_FALSE : S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::EnumMediaTypes(IEnumMediaTypes** types)
{
    //*types = new PinMediaTypeEnumerator(&req_vih_, req_subtype_);
    //(*types)->AddRef();
    //return S_OK;
    return E_NOTIMPL;
}

STDMETHODIMP WinDSCaptureInputPin::QueryInternalConnections(IPin** pins, ULONG* count)
{
    return E_NOTIMPL;
}

STDMETHODIMP WinDSCaptureInputPin::EndOfStream()
{
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::BeginFlush()
{
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::EndFlush() 
{
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::NewSegment(REFERENCE_TIME start, REFERENCE_TIME stop, double rate)
{
    return S_OK;
}

// IMemInputPin
STDMETHODIMP WinDSCaptureInputPin::GetAllocator(IMemAllocator** allocator)
{
    if (allocator_ == nullptr)
    {
        HRESULT hr = CoCreateInstance(CLSID_MemoryAllocator, 0, CLSCTX_INPROC_SERVER, IID_IMemAllocator, (void**)&allocator_);
        if (FAILED(hr))
            return hr;
    }
    *allocator = allocator_;
    allocator_->AddRef();
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::NotifyAllocator(IMemAllocator* allocator, BOOL read_only)
{
    if (allocator_)
        allocator_->Release();
    allocator_ = allocator;
    if (allocator_)
        allocator_->AddRef();
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* props)
{
    return E_NOTIMPL;
}

STDMETHODIMP WinDSCaptureInputPin::Receive(IMediaSample* media_sample)
{
    if (IsStopped()) {
        return S_FALSE;
    }

    if (media_sample->IsPreroll() == S_OK) {
        return S_OK;
    }

    AM_SAMPLE2_PROPERTIES sample_props = {};
    WinDS::GetSampleProperties(media_sample, &sample_props);

    //DVideoFrame frame((DByte*)sample_props.pbBuffer, sample_props.lActual, m_final_fmt.width, m_final_fmt.height, m_final_fmt.format);


    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::ReceiveMultiple(IMediaSample** samples, long count, long* processed)
{
    HRESULT hr = S_OK;
    *processed = 0;
    while (count-- > 0) 
    {
        hr = Receive(samples[*processed]);
        if (hr != S_OK)
            break;
        ++(*processed);
    }
    return hr;
}

STDMETHODIMP WinDSCaptureInputPin::ReceiveCanBlock()
{
    return S_FALSE;
}


DBool WinDSCaptureInputPin::IsStopped() const 
{
    FILTER_STATE sta = State_Running;
    m_info.pFilter->GetState(0, &sta);
    return (sta == State_Stopped);
}

void WinDSCaptureInputPin::OnFilterActivated() 
{

}

void WinDSCaptureInputPin::OnFilterDeactivated() 
{
    if (allocator_)
    {
        allocator_->Decommit();
        allocator_ = nullptr;
    }
}

void WinDSCaptureInputPin::ClearAllocator(bool decommit)
{
    if (!allocator_)
        return;

    if (decommit)
        allocator_->Decommit();
    allocator_->Release();
    allocator_ = nullptr;
}

HRESULT WinDSCaptureInputPin::CheckDirection(IPin* pin) const
{
    PIN_DIRECTION pd;
    pin->QueryDirection(&pd);
    return pd == m_info.dir ? VFW_E_INVALID_DIRECTION : S_OK;
}
