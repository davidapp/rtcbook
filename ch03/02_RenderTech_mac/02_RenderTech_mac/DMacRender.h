#pragma once

#include "DTypes.h"
#include "Video/DVideoDefines.h"


class DMacRender {
public:
    static bool DrawYuv(const DByte* src_y, DInt32 src_stride_y, const DByte* src_u, DInt32 src_stride_u, const DByte* src_v, DInt32 src_stride_v, DInt32 frame_width, DInt32 frame_height, DRotation rotation, DColorSpace color_space = DColorSpace::BT601LimitedRange);
    
};
