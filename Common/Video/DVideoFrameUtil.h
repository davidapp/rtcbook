#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"
#include "DVideoFrame.h"


class DVideoFrameUtil
{
public:
    static DVideoFrame YUY2ToRAW(const DVideoFrame& buf);
    static DVideoFrame YUY2ToI420(const DVideoFrame& buf);

public:
    static DVideoFrame I420Scale(const DVideoFrame& srcFrame);

public:
    static DVideoFrame Rotate(const DVideoFrame& srcFrame, DRotation rotate);

public:
    static DVideoFrame Mirror(const DVideoFrame& srcFrame);
};
