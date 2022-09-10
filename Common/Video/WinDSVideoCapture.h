#pragma once

#include "DTypes.h"
#include "VideoDefines.h"

enum class WinDSError {

};

class WinDSVideoCaptureSink {
public:
    virtual DVoid OnFrame(const DVideoFormat& frame) {};
    virtual DVoid OnError(DUInt32 errorCode) {};
};

class WinDSVideoCapture
{
public:
    WinDSVideoCapture();
    ~WinDSVideoCapture();
    DVoid SetSink(WinDSVideoCaptureSink* pSink);
    DBool Start();
    DBool Stop();

private:
    
};
