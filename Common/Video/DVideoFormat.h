#pragma once

#include "DTypes.h"
#include "DVideoDefines.h"
#include "DVideoFrame.h"


class DVideoFormat
{
public:
    static DVideoFrame YUY2ToRAW(const DVideoFrame& buf);
    static DVideoFrame YUY2ToI420(const DVideoFrame& buf);
    static DVideoFrame I420ToRAW(const DVideoFrame& buf);

private:
    static DInt32 YUY2ToI420(const DByte* src_yuy2, DInt32 src_stride_yuy2,
        DByte* dst_y, DInt32 dst_stride_y, DByte* dst_u, DInt32 dst_stride_u, DByte* dst_v, DInt32 dst_stride_v,
        DInt32 width, DInt32 height);

    static DInt32 I420ToARGB(const DByte* src_y, DInt32 src_stride_y,
        const DByte* src_u, DInt32 src_stride_u,
        const DByte* src_v, DInt32 src_stride_v,
        DByte* dst_argb, DInt32 dst_stride_argb,
        DInt32 width, DInt32 height);
};
