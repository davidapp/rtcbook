
#include "WinDS.h"
#include "WinDSCapturePin.h"
#include <dvdmedia.h>

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

    if (!WinDS::MediaType2DShowCapability(media_type, &m_final_fmt)) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

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
    return WinDS::MediaType2DShowCapability(media_type, &capability) ? S_FALSE : S_OK;
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
    flushing_ = true;
    return S_OK;
}

STDMETHODIMP WinDSCaptureInputPin::EndFlush() 
{
    flushing_ = false;
    runtime_error_ = false;
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

    if (flushing_.load(std::memory_order_relaxed))
        return S_FALSE;

    if (runtime_error_.load(std::memory_order_relaxed))
        return VFW_E_RUNTIME_ERROR;

    if (media_sample->IsPreroll() == S_OK) {
        return S_OK;
    }

    AM_SAMPLE2_PROPERTIES sample_props = {};
    WinDS::GetSampleProperties(media_sample, &sample_props);

    if (frame_count_.load() < 5) { // limit logs
        if (sample_props.pbBuffer == NULL || sample_props.lActual == 0) {
            return S_OK;
        }
        if (sample_props.dwSampleFlags & AM_SAMPLE_PREROLL) {
            return S_OK;
        }
    }
    frame_count_++;

    if (sample_props.dwSampleFlags & AM_SAMPLE_TYPECHANGED) {
        if (!WinDS::MediaType2DShowCapability(sample_props.pMediaType, &m_final_fmt))
        {
            // Raise a runtime error if we fail the media type
            runtime_error_ = true;
            EndOfStream();
            //   Filter()->NotifyEvent(EC_ERRORABORT, VFW_E_TYPE_NOT_ACCEPTED, 0);
            return VFW_E_INVALIDMEDIATYPE;
        }
    }

    /*if (callback_) {
        WINCapVideoFrame frame;
        frame.fmt = m_final_fmt.format;
        frame.width = m_final_fmt.width;
        frame.height = m_final_fmt.height;
        frame.data = (uint8_t*)sample_props.pbBuffer;
        frame.data_size = sample_props.lActual;
        frame.stride = WINCapDefaultStride(frame.width, frame.fmt);
        frame.sample_type = ST_IMediaSample;
        frame.sample_ptr = media_sample;
        callback_->OnFrame(frame);
    }*/

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
    // 看对应 Filter 的 state
    FILTER_STATE sta = State_Running;
    m_info.pFilter->GetState(0, &sta);
    return (sta == State_Stopped);
}

void WinDSCaptureInputPin::OnFilterActivated() 
{
    runtime_error_ = false;
    flushing_ = false;
}

void WinDSCaptureInputPin::OnFilterDeactivated() 
{
    flushing_ = true;
    if (allocator_)
    {
        allocator_->Decommit();
        allocator_ = nullptr;
    }
}

void WinDSCaptureInputPin::SetRequiredFormat(AM_MEDIA_TYPE* ptype) 
{
    ZeroMemory(&req_vih_, sizeof(VIDEOINFOHEADER));
    if (ptype->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* h = reinterpret_cast<VIDEOINFOHEADER2*>(ptype->pbFormat);
        req_vih_.AvgTimePerFrame = h->AvgTimePerFrame;
        req_vih_.bmiHeader = h->bmiHeader;
        req_vih_.dwBitErrorRate = h->dwBitErrorRate;
        req_vih_.dwBitRate = h->dwBitRate;
        req_vih_.rcSource = h->rcSource;
        req_vih_.rcTarget = h->rcTarget;
    }
    else 
    {
        VIDEOINFOHEADER* const h = reinterpret_cast<VIDEOINFOHEADER*>(ptype->pbFormat);
        req_vih_ = *h;
    }
    req_subtype_ = ptype->subtype;
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
