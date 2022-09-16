#include "WinDSVideoCapture.h"
#include "WinDSCamera.h"
#include "WinDS.h"

WinDSVideoCapture::WinDSVideoCapture()
{

}

WinDSVideoCapture::~WinDSVideoCapture()
{

}

DBool WinDSVideoCapture::Init(HWND hWnd)
{
    WinDSCamera::Init();
    std::vector<DCameraInfo> info = WinDSCamera::GetDevices();
    if (info.size() == 0) return false;

    m_capture_filter = info[0].m_device_filter;
    m_capture_output_pin = WinDS::GetOutputPin(m_capture_filter, GUID_NULL);

    m_sink_filter = new WinDSSinkFilter(hWnd);
    m_sink_input_pin = m_sink_filter->m_input_pin;

    CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_graph_builder);
    m_graph_builder->QueryInterface(IID_IMediaEventEx, (void**)&m_media_event);
    m_graph_builder->QueryInterface(IID_IMediaControl, (void**)&m_media_control);
    m_graph_builder->AddFilter(m_capture_filter, L"VideoCaptureFilter");
    m_graph_builder->AddFilter(m_sink_filter, L"SinkFilter");
    
    IAMStreamConfig* streamConfig = nullptr;
    AM_MEDIA_TYPE* pmt = nullptr;
    VIDEO_STREAM_CONFIG_CAPS caps;
    m_capture_output_pin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);
    streamConfig->GetStreamCaps(18, &pmt, reinterpret_cast<BYTE*>(&caps)); // Use 0 Index
    streamConfig->SetFormat(pmt);

    m_graph_builder->ConnectDirect(m_capture_output_pin, m_sink_input_pin, NULL);

    return true;
}

DVoid WinDSVideoCapture::UnInit()
{
    WinDSCamera::UnInit();
}

DBool WinDSVideoCapture::Start()
{
    m_media_control->Run();
    return true;
}

DBool WinDSVideoCapture::Stop()
{
    m_media_control->Stop();
    return true;
}
