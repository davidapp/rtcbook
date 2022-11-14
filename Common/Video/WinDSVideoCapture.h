#pragma once

#include "DTypes.h"
#include "VideoDefines.h"
#include "Video/WinDSSinkFilter.h"
#include <dshow.h>
#include <string>


class WinDSVideoCapture
{
public:
    WinDSVideoCapture();
    ~WinDSVideoCapture();

    DBool Init(HWND hWnd);
    DVoid UnInit();
    DBool Start();
    DBool Stop();

private:
    IGraphBuilder* m_graph_builder = nullptr;
    IMediaControl* m_media_control = nullptr;
    IMediaEventEx* m_media_event = nullptr;

    IBaseFilter* m_capture_filter = nullptr;
    IPin* m_capture_output_pin = nullptr;

    WinDSSinkFilter* m_sink_filter = nullptr;
    IPin* m_sink_input_pin = nullptr;
};
