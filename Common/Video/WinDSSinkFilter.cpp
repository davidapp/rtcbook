#include "WinDSSinkFilter.h"
#include "WinDS.h"

// {AC18ED31-601C-4518-8C2B-417878705453}
DEFINE_GUID(CLSID_WINDS_SINKFILTER, 0xac18ed31, 0x601c, 0x4518, 0x8c, 0x2b, 0x41, 0x78, 0x78, 0x70, 0x54, 0x53);


WinDSSinkFilter::WinDSSinkFilter()
{
    m_input_pin = new WinDSCaptureInputPin(this);
    m_input_pin->AddRef();
}

WinDSSinkFilter::~WinDSSinkFilter() 
{
    m_input_pin->Release();
}

// IUnknown
STDMETHODIMP WinDSSinkFilter::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IPersist || riid == IID_IBaseFilter) 
    {
        *ppv = static_cast<IBaseFilter*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

IFACEMETHODIMP_(ULONG) WinDSSinkFilter::AddRef()
{
    m_refCount++;
    return 1;
}

IFACEMETHODIMP_(ULONG) WinDSSinkFilter::Release()
{
    m_refCount--;
    if (m_refCount == 0) {
        delete this;
        return 0;
    }
    return 1;
}

// IPersist
STDMETHODIMP WinDSSinkFilter::GetClassID(CLSID* clsid) 
{
    *clsid = CLSID_WINDS_SINKFILTER;
    return S_OK;
}

// IMediaFilter
STDMETHODIMP WinDSSinkFilter::GetState(DWORD msecs, FILTER_STATE* state)
{
    *state = m_state;
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::SetSyncSource(IReferenceClock* clock)
{
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::GetSyncSource(IReferenceClock** clock)
{
    return E_NOTIMPL;
}

STDMETHODIMP WinDSSinkFilter::Pause()
{
    m_state = State_Paused;
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::Run(REFERENCE_TIME tStart)
{
    if (m_state == State_Stopped)
        Pause();

    m_state = State_Running;
    m_input_pin->OnFilterActivated();

    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::Stop()
{
    if (m_state == State_Stopped)
        return S_OK;

    m_state = State_Stopped;
    m_input_pin->OnFilterDeactivated();

    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::EnumPins(IEnumPins** pins)
{
    *pins = new WinDSEnumPins(m_input_pin);
    (*pins)->AddRef();
    return S_OK; // E_NOTIMPL
}

STDMETHODIMP WinDSSinkFilter::FindPin(LPCWSTR id, IPin** pin)
{
    *pin = m_input_pin;
    (*pin)->AddRef();
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::QueryFilterInfo(FILTER_INFO* info)
{
    *info = m_info;
    if (info->pGraph)
        info->pGraph->AddRef();
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::JoinFilterGraph(IFilterGraph* graph, LPCWSTR name)
{
    if (m_state != State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }

    m_info.pGraph = graph;
    m_info.achName[0] = L'\0';
    if (name) {
        lstrcpynW(m_info.achName, name, sizeof(m_info.achName));
    }
    return S_OK;
}

STDMETHODIMP WinDSSinkFilter::QueryVendorInfo(LPWSTR* vendor_info)
{
    return E_NOTIMPL;
}
