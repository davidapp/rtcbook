#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"
#include "DVideoFrame.h"

typedef enum FilterMode {
    kFilterNone = 0,      // Point sample; Fastest.
    kFilterLinear = 1,    // Filter horizontally only.
    kFilterBilinear = 2,  // Faster than box, but lower quality scaling down.
    kFilterBox = 3        // Highest quality.
} FilterModeEnum;


class DScale
{
public:
    static DVideoFrame I420Scale(const DVideoFrame& srcFrame);

public:
    static DInt32 I420Scale(const DByte* src_y, DInt32 src_stride_y,
        const DByte* src_u, DInt32 src_stride_u,
        const DByte* src_v, DInt32 src_stride_v,
        DInt32 src_width, DInt32 src_height,
        DByte* dst_y, DInt32 dst_stride_y,
        DByte* dst_u, DInt32 dst_stride_u, 
        DByte* dst_v, DInt32 dst_stride_v,
        DInt32 dst_width, DInt32 dst_height, enum FilterMode filtering);
};
