#include "WinDSVideoCapture.h"
#include "WinDSCamera.h"
#include "WinDS.h"

WinDSVideoCapture::WinDSVideoCapture()
{

}

WinDSVideoCapture::~WinDSVideoCapture()
{

}

DBool WinDSVideoCapture::Init(DUInt32 nIndex, DVoid* pCallback, DVoid* pUserData)
{
    WinDSCamera::Init();
    std::vector<DCameraInfo> info = WinDSCamera::GetDevices();
    if (info.size() == 0) return false;

    m_capture_filter = info[nIndex].m_device_filter;
    m_capture_output_pin = WinDS::GetOutputPin(m_capture_filter, GUID_NULL);

    m_sink_filter = new WinDSSinkFilter(pCallback, pUserData);
    m_sink_input_pin = m_sink_filter->m_input_pin;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&m_graph_builder);
    m_graph_builder->QueryInterface(IID_IMediaEventEx, (void**)&m_media_event);
    m_graph_builder->QueryInterface(IID_IMediaControl, (void**)&m_media_control);
    m_graph_builder->AddFilter(m_capture_filter, L"VideoCaptureFilter");
    m_graph_builder->AddFilter(m_sink_filter, L"SinkFilter");
    
    IAMStreamConfig* streamConfig = nullptr;
    AM_MEDIA_TYPE* pmt = nullptr;
    VIDEO_STREAM_CONFIG_CAPS caps;
    m_capture_output_pin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfig);

    // Select RGB24 or YUY2 with 640*480 mode
    std::vector<DCameraCaps> config = WinDSCamera::GetDeviceCaps(m_capture_filter);
    DInt32 selectIndex = -1;
    DUInt32 configsize = config.size();
    for (DUInt32 i = 0; i < configsize; i++) {
        if (config[i].m_width == 640 && 
               (config[i].m_pixel_format == DPixelFmt::RGB24) 
            || (config[i].m_pixel_format == DPixelFmt::YUY2))
        {
            selectIndex = i;
            break;
        }
    }
    if (selectIndex == -1) {
        return false;
    }

    streamConfig->GetStreamCaps(selectIndex, &pmt, reinterpret_cast<BYTE*>(&caps));
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
