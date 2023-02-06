#include "DVideoI420.h"
#include "Base/DXP.h"

#define SUBSAMPLE(v, a, s) (v < 0) ? (-((-v + a) >> s)) : ((v + a) >> s)
#define CENTERSTART(dx, s) (dx < 0) ? -((-dx >> 1) + s) : ((dx >> 1) + s)

DVideoFrame DVideoI420::Scale(const DVideoFrame& srcFrame, DInt32 w, DInt32 h, FilterMode filter)
{
    DInt32 src_w = srcFrame.GetWidth();
    DInt32 src_h = srcFrame.GetHeight();
    DInt32 lineSize = srcFrame.GetLineSize();

    DVideoFrame frameRet(w, h, DPixelFmt::I420);
    DByte* src_y = srcFrame.GetBuf();
    DInt32 src_stride_y = lineSize;
    DByte* src_u = src_y + src_h * lineSize;
    DInt32 src_stride_u = src_stride_y / 2;
    DByte* src_v = src_u + src_h * lineSize / 4;
    DInt32 src_stride_v = src_stride_y / 2;

    DInt32 dst_lineSize = frameRet.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = dst_lineSize;
    DByte* dst_u = dst_y + h * dst_lineSize;
    DInt32 dst_stride_u = dst_lineSize / 2;
    DByte* dst_v = dst_u + h * dst_lineSize / 4;
    DInt32 dst_stride_v = dst_lineSize / 2;

    DVideoI420::I420Scale(src_y, src_stride_y, 
        src_u, src_stride_u, 
        src_v, src_stride_v, 
        srcFrame.GetWidth(), srcFrame.GetHeight(),
        dst_y, dst_stride_y, 
        dst_u, dst_stride_u, 
        dst_v, dst_stride_v, 
        w, h, filter);

    return frameRet;
}

DVideoFrame DVideoI420::Rotate(const DVideoFrame& srcFrame, DRotation rotate)
{
    DInt32 w = srcFrame.GetWidth();
    DInt32 h = srcFrame.GetHeight();
    if (rotate == DRotation::DEGREE_90 || rotate == DRotation::DEGREE_270) {
        DInt32 temp = w;
        w = h;
        h = temp;
    }
    DVideoFrame frameRet(w, h, DPixelFmt::I420);

    DInt32 src_w = srcFrame.GetWidth();
    DInt32 src_h = srcFrame.GetHeight();
    DInt32 lineSize = srcFrame.GetLineSize();

    DByte* src_y = srcFrame.GetBuf();
    DInt32 src_stride_y = lineSize;
    DByte* src_u = src_y + src_h * lineSize;
    DInt32 src_stride_u = src_stride_y / 2;
    DByte* src_v = src_u + src_h * lineSize / 4;
    DInt32 src_stride_v = src_stride_y / 2;

    DInt32 dst_lineSize = frameRet.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = dst_lineSize;
    DByte* dst_u = dst_y + h * dst_lineSize;
    DInt32 dst_stride_u = dst_lineSize / 2;
    DByte* dst_v = dst_u + h * dst_lineSize / 4;
    DInt32 dst_stride_v = dst_lineSize / 2;

    DVideoI420::I420Rotate(src_y, src_stride_y,
        src_u, src_stride_u,
        src_v, src_stride_v,
        dst_y, dst_stride_y,
        dst_u, dst_stride_u,
        dst_v, dst_stride_v,
        w, h, rotate);
    return frameRet;
}

DVideoFrame DVideoI420::Mirror(const DVideoFrame& srcFrame)
{
    DInt32 w = srcFrame.GetWidth();
    DInt32 h = srcFrame.GetHeight();
    DVideoFrame frameRet(w, h, DPixelFmt::I420);

    DInt32 src_w = srcFrame.GetWidth();
    DInt32 src_h = srcFrame.GetHeight();
    DInt32 lineSize = srcFrame.GetLineSize();

    DByte* src_y = srcFrame.GetBuf();
    DInt32 src_stride_y = lineSize;
    DByte* src_u = src_y + src_h * lineSize;
    DInt32 src_stride_u = src_stride_y / 2;
    DByte* src_v = src_u + src_h * lineSize / 4;
    DInt32 src_stride_v = src_stride_y / 2;

    DInt32 dst_lineSize = frameRet.GetLineSize();
    DByte* dst_y = frameRet.GetBuf();
    DInt32 dst_stride_y = dst_lineSize;
    DByte* dst_u = dst_y + h * dst_lineSize;
    DInt32 dst_stride_u = dst_lineSize / 2;
    DByte* dst_v = dst_u + h * dst_lineSize / 4;
    DInt32 dst_stride_v = dst_lineSize / 2;

    DVideoI420::I420Mirror(src_y, src_stride_y,
        src_u, src_stride_u,
        src_v, src_stride_v,
        dst_y, dst_stride_y,
        dst_u, dst_stride_u,
        dst_v, dst_stride_v,
        w, h);

    return frameRet;
}

DVideoFrame DVideoI420::Crop(const DVideoFrame& srcFrame, const DRect& cropRect)
{
    DInt32 w = cropRect.Width();
    DInt32 h = cropRect.Height();
    DVideoFrame frameRet(w, h, DPixelFmt::I420);
    return frameRet;
}

enum FilterMode ScaleFilterReduce(DInt32 src_width, DInt32 src_height,
    DInt32 dst_width, DInt32 dst_height, enum FilterMode filtering)
{
    if (src_width < 0) {
        src_width = -src_width;
    }
    if (src_height < 0) {
        src_height = -src_height;
    }
    if (filtering == kFilterBox) {
        if (dst_width * 2 >= src_width && dst_height * 2 >= src_height) {
            filtering = kFilterBilinear;
        }
    }
    if (filtering == kFilterBilinear) {
        if (src_height == 1) {
            filtering = kFilterLinear;
        }
        if (dst_height == src_height || dst_height * 3 == src_height) {
            filtering = kFilterLinear;
        }
        if (src_width == 1) {
            filtering = kFilterNone;
        }
    }
    if (filtering == kFilterLinear) {
        if (src_width == 1) {
            filtering = kFilterNone;
        }
        if (dst_width == src_width || dst_width * 3 == src_width) {
            filtering = kFilterNone;
        }
    }
    return filtering;
}

DVoid CopyPlane(const DByte* src_y, DInt32 src_stride_y,
    DByte* dst_y, DInt32 dst_stride_y, DInt32 width, DInt32 height)
{
    if (height < 0) {
        height = -height;
        dst_y = dst_y + (height - 1) * dst_stride_y;
        dst_stride_y = -dst_stride_y;
    }
    if (src_stride_y == width && dst_stride_y == width) {
        width *= height;
        height = 1;
        src_stride_y = dst_stride_y = 0;
    }
    if (src_y == dst_y && src_stride_y == dst_stride_y) {
        return;
    }
    for (DInt32 y = 0; y < height; ++y) {
        DXP::memcpy(dst_y, src_y, width);
        src_y += src_stride_y;
        dst_y += dst_stride_y;
    }
}

DInt32 FixedDiv(DInt32 num, DInt32 div) {
    return (DInt32)(((int64_t)(num) << 16) / div);
}

DInt32 FixedDiv1(DInt32 num, DInt32 div) {
    return (DInt32)((((DInt64)(num) << 16) - 0x00010001) / (div - 1));
}

DInt32 Abs(DInt32 v) 
{
    return v >= 0 ? v : -v;
}

DVoid HalfRow(const DUInt8* src_uv,
    DPtrDiff src_uv_stride,
    DUInt8* dst_uv,
    DInt32 width) 
{
    DInt32 x;
    for (x = 0; x < width; ++x) {
        dst_uv[x] = (src_uv[x] + src_uv[src_uv_stride + x] + 1) >> 1;
    }
}

DVoid InterpolateRow(DUInt8* dst_ptr, const DUInt8* src_ptr, DPtrDiff src_stride,
    DInt32 width, DInt32 source_y_fraction) 
{
    DInt32 y1_fraction = source_y_fraction;
    DInt32 y0_fraction = 256 - y1_fraction;
    const DUInt8* src_ptr1 = src_ptr + src_stride;
    DInt32 x;
    if (y1_fraction == 0) {
        DXP::memcpy(dst_ptr, src_ptr, width);
        return;
    }
    if (y1_fraction == 128) {
        HalfRow(src_ptr, src_stride, dst_ptr, width);
        return;
    }
    for (x = 0; x < width - 1; x += 2) {
        dst_ptr[0] =
            (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
        dst_ptr[1] =
            (src_ptr[1] * y0_fraction + src_ptr1[1] * y1_fraction + 128) >> 8;
        src_ptr += 2;
        src_ptr1 += 2;
        dst_ptr += 2;
    }
    if (width & 1) {
        dst_ptr[0] =
            (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
    }
}

DVoid ScalePlaneVertical(DInt32 src_height, DInt32 dst_width, DInt32 dst_height,
    DInt32 src_stride, DInt32 dst_stride, const DUInt8* src_argb,
    DUInt8* dst_argb, DInt32 x, DInt32 y, DInt32 dy,
    DInt32 bpp, enum FilterMode filtering) 
{
    DInt32 dst_width_bytes = dst_width * bpp;
    const DInt32 max_y = (src_height > 1) ? ((src_height - 1) << 16) - 1 : 0;

    assert(bpp >= 1 && bpp <= 4);
    assert(src_height != 0);
    assert(dst_width > 0);
    assert(dst_height > 0);
    src_argb += (x >> 16) * bpp;

    for (DInt32 j = 0; j < dst_height; ++j) 
    {
        if (y > max_y) {
            y = max_y;
        }
        DInt32 yi = y >> 16;
        DInt32 yf = filtering ? ((y >> 8) & 255) : 0;
        InterpolateRow(dst_argb, src_argb + yi * src_stride, src_stride, dst_width_bytes, yf);
        dst_argb += dst_stride;
        y += dy;
    }
}

DVoid ScaleRowDown34(const DUInt8* src_ptr, DPtrDiff src_stride, 
    DUInt8* dst, DInt32 dst_width) 
{
    (DVoid)src_stride;
    assert((dst_width % 3 == 0) && (dst_width > 0));
    for (DInt32 x = 0; x < dst_width; x += 3) {
        dst[0] = src_ptr[0];
        dst[1] = src_ptr[1];
        dst[2] = src_ptr[3];
        dst += 3;
        src_ptr += 4;
    }
}

DVoid ScaleRowDown34_0_Box(const DUInt8* src_ptr, DPtrDiff src_stride,
    DUInt8* d, DInt32 dst_width) 
{
    const DUInt8* s = src_ptr;
    const DUInt8* t = src_ptr + src_stride;
    assert((dst_width % 3 == 0) && (dst_width > 0));
    for (DInt32 x = 0; x < dst_width; x += 3) 
    {
        DUInt8 a0 = (s[0] * 3 + s[1] * 1 + 2) >> 2;
        DUInt8 a1 = (s[1] * 1 + s[2] * 1 + 1) >> 1;
        DUInt8 a2 = (s[2] * 1 + s[3] * 3 + 2) >> 2;
        DUInt8 b0 = (t[0] * 3 + t[1] * 1 + 2) >> 2;
        DUInt8 b1 = (t[1] * 1 + t[2] * 1 + 1) >> 1;
        DUInt8 b2 = (t[2] * 1 + t[3] * 3 + 2) >> 2;
        d[0] = (a0 * 3 + b0 + 2) >> 2;
        d[1] = (a1 * 3 + b1 + 2) >> 2;
        d[2] = (a2 * 3 + b2 + 2) >> 2;
        d += 3;
        s += 4;
        t += 4;
    }
}

DVoid ScaleRowDown34_1_Box(const DUInt8* src_ptr, DPtrDiff src_stride,
    DUInt8* d, DInt32 dst_width) 
{
    const DUInt8* s = src_ptr;
    const DUInt8* t = src_ptr + src_stride;
    DInt32 x;
    assert((dst_width % 3 == 0) && (dst_width > 0));
    for (x = 0; x < dst_width; x += 3) {
        DUInt8 a0 = (s[0] * 3 + s[1] * 1 + 2) >> 2;
        DUInt8 a1 = (s[1] * 1 + s[2] * 1 + 1) >> 1;
        DUInt8 a2 = (s[2] * 1 + s[3] * 3 + 2) >> 2;
        DUInt8 b0 = (t[0] * 3 + t[1] * 1 + 2) >> 2;
        DUInt8 b1 = (t[1] * 1 + t[2] * 1 + 1) >> 1;
        DUInt8 b2 = (t[2] * 1 + t[3] * 3 + 2) >> 2;
        d[0] = (a0 + b0 + 1) >> 1;
        d[1] = (a1 + b1 + 1) >> 1;
        d[2] = (a2 + b2 + 1) >> 1;
        d += 3;
        s += 4;
        t += 4;
    }
}

static DVoid ScalePlaneDown34(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr,
    enum FilterMode filtering) {
    DInt32 y;
    DVoid (*ScaleRowDown34_0)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width);
    DVoid (*ScaleRowDown34_1)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width);
    const DInt32 filter_stride = (filtering == kFilterLinear) ? 0 : src_stride;
    (DVoid)src_width;
    (DVoid)src_height;
    assert(dst_width % 3 == 0);
    if (!filtering) {
        ScaleRowDown34_0 = ScaleRowDown34;
        ScaleRowDown34_1 = ScaleRowDown34;
    }
    else {
        ScaleRowDown34_0 = ScaleRowDown34_0_Box;
        ScaleRowDown34_1 = ScaleRowDown34_1_Box;
    }

    for (y = 0; y < dst_height - 2; y += 3) 
    {
        ScaleRowDown34_0(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride;
        dst_ptr += dst_stride;
        ScaleRowDown34_1(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride;
        dst_ptr += dst_stride;
        ScaleRowDown34_0(src_ptr + src_stride, -filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride * 2;
        dst_ptr += dst_stride;
    }

    if ((dst_height % 3) == 2) {
        ScaleRowDown34_0(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride;
        dst_ptr += dst_stride;
        ScaleRowDown34_1(src_ptr, 0, dst_ptr, dst_width);
    }
    else if ((dst_height % 3) == 1) {
        ScaleRowDown34_0(src_ptr, 0, dst_ptr, dst_width);
    }
}

DVoid ScaleRowDown2(const DUInt8* src_ptr, DPtrDiff src_stride,
    DUInt8* dst, DInt32 dst_width) 
{
    D_UNUSED(src_stride);
    for (DInt32 x = 0; x < dst_width - 1; x += 2) 
    {
        dst[0] = src_ptr[1];
        dst[1] = src_ptr[3];
        dst += 2;
        src_ptr += 4;
    }
    if (dst_width & 1) 
    {
        dst[0] = src_ptr[1];
    }
}

DVoid ScaleRowDown2Linear(const DUInt8* src_ptr, DPtrDiff src_stride,
    DUInt8* dst, DInt32 dst_width) 
{
    D_UNUSED(src_stride);
    const DUInt8* s = src_ptr;
    for (DInt32 x = 0; x < dst_width - 1; x += 2) 
    {
        dst[0] = (s[0] + s[1] + 1) >> 1;
        dst[1] = (s[2] + s[3] + 1) >> 1;
        dst += 2;
        s += 4;
    }
    if (dst_width & 1) 
    {
        dst[0] = (s[0] + s[1] + 1) >> 1;
    }
}

DVoid ScaleRowDown2Box(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst,
    DInt32 dst_width) {
    const DUInt8* s = src_ptr;
    const DUInt8* t = src_ptr + src_stride;
    DInt32 x;
    for (x = 0; x < dst_width - 1; x += 2) {
        dst[0] = (s[0] + s[1] + t[0] + t[1] + 2) >> 2;
        dst[1] = (s[2] + s[3] + t[2] + t[3] + 2) >> 2;
        dst += 2;
        s += 4;
        t += 4;
    }
    if (dst_width & 1) {
        dst[0] = (s[0] + s[1] + t[0] + t[1] + 2) >> 2;
    }
}

DVoid ScalePlaneDown2(DInt32 src_width, DInt32 src_height,
    DInt32 dst_width, DInt32 dst_height,
    DInt32 src_stride, DInt32 dst_stride,
    const DUInt8* src_ptr, DUInt8* dst_ptr, enum FilterMode filtering) 
{
    DVoid (*ScaleRowDown)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width) =
        filtering == kFilterNone
        ? ScaleRowDown2
        : (filtering == kFilterLinear ? ScaleRowDown2Linear
            : ScaleRowDown2Box);
    DInt32 row_stride = src_stride << 1;
    (DVoid)src_width;
    (DVoid)src_height;
    if (!filtering) {
        src_ptr += src_stride;  // Point to odd rows.
        src_stride = 0;
    }

    if (filtering == kFilterLinear) {
        src_stride = 0;
    }

    for (DInt32 y = 0; y < dst_height; ++y) {
        ScaleRowDown(src_ptr, src_stride, dst_ptr, dst_width);
        src_ptr += row_stride;
        dst_ptr += dst_stride;
    }
}


DVoid ScaleRowDown38_C(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst,
    DInt32 dst_width) {
    DInt32 x;
    (DVoid)src_stride;
    assert(dst_width % 3 == 0);
    for (x = 0; x < dst_width; x += 3) {
        dst[0] = src_ptr[0];
        dst[1] = src_ptr[3];
        dst[2] = src_ptr[6];
        dst += 3;
        src_ptr += 8;
    }
}

DVoid ScaleRowDown38_3_Box_C(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst_ptr,
    DInt32 dst_width) {
    intptr_t stride = src_stride;
    DInt32 i;
    assert((dst_width % 3 == 0) && (dst_width > 0));
    for (i = 0; i < dst_width; i += 3) {
        dst_ptr[0] =
            (src_ptr[0] + src_ptr[1] + src_ptr[2] + src_ptr[stride + 0] +
                src_ptr[stride + 1] + src_ptr[stride + 2] + src_ptr[stride * 2 + 0] +
                src_ptr[stride * 2 + 1] + src_ptr[stride * 2 + 2]) *
            (65536 / 9) >>
            16;
        dst_ptr[1] =
            (src_ptr[3] + src_ptr[4] + src_ptr[5] + src_ptr[stride + 3] +
                src_ptr[stride + 4] + src_ptr[stride + 5] + src_ptr[stride * 2 + 3] +
                src_ptr[stride * 2 + 4] + src_ptr[stride * 2 + 5]) *
            (65536 / 9) >>
            16;
        dst_ptr[2] =
            (src_ptr[6] + src_ptr[7] + src_ptr[stride + 6] + src_ptr[stride + 7] +
                src_ptr[stride * 2 + 6] + src_ptr[stride * 2 + 7]) *
            (65536 / 6) >>
            16;
        src_ptr += 8;
        dst_ptr += 3;
    }
}

DVoid ScaleRowDown38_2_Box_C(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst_ptr,
    DInt32 dst_width) {
    intptr_t stride = src_stride;
    DInt32 i;
    assert((dst_width % 3 == 0) && (dst_width > 0));
    for (i = 0; i < dst_width; i += 3) {
        dst_ptr[0] = (src_ptr[0] + src_ptr[1] + src_ptr[2] + src_ptr[stride + 0] +
            src_ptr[stride + 1] + src_ptr[stride + 2]) *
            (65536 / 6) >>
            16;
        dst_ptr[1] = (src_ptr[3] + src_ptr[4] + src_ptr[5] + src_ptr[stride + 3] +
            src_ptr[stride + 4] + src_ptr[stride + 5]) *
            (65536 / 6) >>
            16;
        dst_ptr[2] =
            (src_ptr[6] + src_ptr[7] + src_ptr[stride + 6] + src_ptr[stride + 7]) *
            (65536 / 4) >>
            16;
        src_ptr += 8;
        dst_ptr += 3;
    }
}


DVoid ScalePlaneDown38(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr,
    enum FilterMode filtering) {
    DInt32 y;
    DVoid (*ScaleRowDown38_3)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width);
    DVoid (*ScaleRowDown38_2)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width);
    const DInt32 filter_stride = (filtering == kFilterLinear) ? 0 : src_stride;
    assert(dst_width % 3 == 0);
    (DVoid)src_width;
    (DVoid)src_height;
    if (!filtering) {
        ScaleRowDown38_3 = ScaleRowDown38_C;
        ScaleRowDown38_2 = ScaleRowDown38_C;
    }
    else {
        ScaleRowDown38_3 = ScaleRowDown38_3_Box_C;
        ScaleRowDown38_2 = ScaleRowDown38_2_Box_C;
    }

    for (y = 0; y < dst_height - 2; y += 3) {
        ScaleRowDown38_3(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride * 3;
        dst_ptr += dst_stride;
        ScaleRowDown38_3(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride * 3;
        dst_ptr += dst_stride;
        ScaleRowDown38_2(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride * 2;
        dst_ptr += dst_stride;
    }

    // Remainder 1 or 2 rows with last row vertically unfiltered
    if ((dst_height % 3) == 2) {
        ScaleRowDown38_3(src_ptr, filter_stride, dst_ptr, dst_width);
        src_ptr += src_stride * 3;
        dst_ptr += dst_stride;
        ScaleRowDown38_3(src_ptr, 0, dst_ptr, dst_width);
    }
    else if ((dst_height % 3) == 1) {
        ScaleRowDown38_3(src_ptr, 0, dst_ptr, dst_width);
    }
}

DVoid ScaleRowDown4_C(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst,
    DInt32 dst_width) {
    DInt32 x;
    (DVoid)src_stride;
    for (x = 0; x < dst_width - 1; x += 2) {
        dst[0] = src_ptr[2];
        dst[1] = src_ptr[6];
        dst += 2;
        src_ptr += 8;
    }
    if (dst_width & 1) {
        dst[0] = src_ptr[2];
    }
}

DVoid ScaleRowDown4Box_C(const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DUInt8* dst,
    DInt32 dst_width) {
    intptr_t stride = src_stride;
    DInt32 x;
    for (x = 0; x < dst_width - 1; x += 2) {
        dst[0] = (src_ptr[0] + src_ptr[1] + src_ptr[2] + src_ptr[3] +
            src_ptr[stride + 0] + src_ptr[stride + 1] + src_ptr[stride + 2] +
            src_ptr[stride + 3] + src_ptr[stride * 2 + 0] +
            src_ptr[stride * 2 + 1] + src_ptr[stride * 2 + 2] +
            src_ptr[stride * 2 + 3] + src_ptr[stride * 3 + 0] +
            src_ptr[stride * 3 + 1] + src_ptr[stride * 3 + 2] +
            src_ptr[stride * 3 + 3] + 8) >>
            4;
        dst[1] = (src_ptr[4] + src_ptr[5] + src_ptr[6] + src_ptr[7] +
            src_ptr[stride + 4] + src_ptr[stride + 5] + src_ptr[stride + 6] +
            src_ptr[stride + 7] + src_ptr[stride * 2 + 4] +
            src_ptr[stride * 2 + 5] + src_ptr[stride * 2 + 6] +
            src_ptr[stride * 2 + 7] + src_ptr[stride * 3 + 4] +
            src_ptr[stride * 3 + 5] + src_ptr[stride * 3 + 6] +
            src_ptr[stride * 3 + 7] + 8) >>
            4;
        dst += 2;
        src_ptr += 8;
    }
    if (dst_width & 1) {
        dst[0] = (src_ptr[0] + src_ptr[1] + src_ptr[2] + src_ptr[3] +
            src_ptr[stride + 0] + src_ptr[stride + 1] + src_ptr[stride + 2] +
            src_ptr[stride + 3] + src_ptr[stride * 2 + 0] +
            src_ptr[stride * 2 + 1] + src_ptr[stride * 2 + 2] +
            src_ptr[stride * 2 + 3] + src_ptr[stride * 3 + 0] +
            src_ptr[stride * 3 + 1] + src_ptr[stride * 3 + 2] +
            src_ptr[stride * 3 + 3] + 8) >>
            4;
    }
}


DVoid ScalePlaneDown4(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr,
    enum FilterMode filtering) {
    DInt32 y;
    DVoid (*ScaleRowDown4)(const DUInt8 * src_ptr, DPtrDiff src_stride,
        DUInt8 * dst_ptr, DInt32 dst_width) =
        filtering ? ScaleRowDown4Box_C : ScaleRowDown4_C;
    DInt32 row_stride = src_stride << 2;
    (DVoid)src_width;
    (DVoid)src_height;
    if (!filtering) {
        src_ptr += src_stride * 2;  // Point to row 2.
        src_stride = 0;
    }

    if (filtering == kFilterLinear) {
        src_stride = 0;
    }
    for (y = 0; y < dst_height; ++y) {
        ScaleRowDown4(src_ptr, src_stride, dst_ptr, dst_width);
        src_ptr += row_stride;
        dst_ptr += dst_stride;
    }
}

DVoid ScaleSlope(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    enum FilterMode filtering,
    DInt32* x,
    DInt32* y,
    DInt32* dx,
    DInt32* dy) {
    assert(x != NULL);
    assert(y != NULL);
    assert(dx != NULL);
    assert(dy != NULL);
    assert(src_width != 0);
    assert(src_height != 0);
    assert(dst_width > 0);
    assert(dst_height > 0);

    if (dst_width == 1 && src_width >= 32768) {
        dst_width = src_width;
    }
    if (dst_height == 1 && src_height >= 32768) {
        dst_height = src_height;
    }
    if (filtering == kFilterBox) {
        *dx = FixedDiv(Abs(src_width), dst_width);
        *dy = FixedDiv(src_height, dst_height);
        *x = 0;
        *y = 0;
    }
    else if (filtering == kFilterBilinear) {
        // Scale step for bilinear sampling renders last pixel once for upsample.
        if (dst_width <= Abs(src_width)) {
            *dx = FixedDiv(Abs(src_width), dst_width);
            *x = CENTERSTART(*dx, -32768);  // Subtract 0.5 (32768) to center filter.
        }
        else if (dst_width > 1) {
            *dx = FixedDiv1(Abs(src_width), dst_width);
            *x = 0;
        }
        if (dst_height <= src_height) {
            *dy = FixedDiv(src_height, dst_height);
            *y = CENTERSTART(*dy, -32768);  // Subtract 0.5 (32768) to center filter.
        }
        else if (dst_height > 1) {
            *dy = FixedDiv1(src_height, dst_height);
            *y = 0;
        }
    }
    else if (filtering == kFilterLinear) {
        // Scale step for bilinear sampling renders last pixel once for upsample.
        if (dst_width <= Abs(src_width)) {
            *dx = FixedDiv(Abs(src_width), dst_width);
            *x = CENTERSTART(*dx, -32768);  // Subtract 0.5 (32768) to center filter.
        }
        else if (dst_width > 1) {
            *dx = FixedDiv1(Abs(src_width), dst_width);
            *x = 0;
        }
        *dy = FixedDiv(src_height, dst_height);
        *y = *dy >> 1;
    }
    else {
        // Scale step for point sampling duplicates all pixels equally.
        *dx = FixedDiv(Abs(src_width), dst_width);
        *dy = FixedDiv(src_height, dst_height);
        *x = CENTERSTART(*dx, 0);
        *y = CENTERSTART(*dy, 0);
    }
    // Negative src_width means horizontally mirror.
    if (src_width < 0) {
        *x += (dst_width - 1) * *dx;
        *dx = -*dx;
        // src_width = -src_width;   // Caller must do this.
    }
}

#define IS_ALIGNED(p, a) (!((uintptr_t)(p) & ((a)-1)))

#define align_buffer_64(var, size)                                           \
  DUInt8* var##_mem = (DUInt8*)(malloc((size) + 63));         /* NOLINT */ \
  DUInt8* var = (DUInt8*)(((intptr_t)(var##_mem) + 63) & ~63) /* NOLINT */

#define free_aligned_buffer_64(var) \
  free(var##_mem);                  \
  var = 0

#define MIN1(x) ((x) < 1 ? 1 : (x))

static __inline uint32_t SumPixels(DInt32 iboxwidth, const uint16_t* src_ptr) {
    uint32_t sum = 0u;
    DInt32 x;
    assert(iboxwidth > 0);
    for (x = 0; x < iboxwidth; ++x) {
        sum += src_ptr[x];
    }
    return sum;
}

static DVoid ScaleAddCols2_C(DInt32 dst_width,
    DInt32 boxheight,
    DInt32 x,
    DInt32 dx,
    const uint16_t* src_ptr,
    DUInt8* dst_ptr) {
    DInt32 i;
    DInt32 scaletbl[2];
    DInt32 minboxwidth = dx >> 16;
    DInt32 boxwidth;
    scaletbl[0] = 65536 / (MIN1(minboxwidth) * boxheight);
    scaletbl[1] = 65536 / (MIN1(minboxwidth + 1) * boxheight);
    for (i = 0; i < dst_width; ++i) {
        DInt32 ix = x >> 16;
        x += dx;
        boxwidth = MIN1((x >> 16) - ix);
        *dst_ptr++ =
            SumPixels(boxwidth, src_ptr + ix) * scaletbl[boxwidth - minboxwidth] >>
            16;
    }
}

static DVoid ScaleAddCols1_C(DInt32 dst_width,
    DInt32 boxheight,
    DInt32 x,
    DInt32 dx,
    const uint16_t* src_ptr,
    DUInt8* dst_ptr) {
    DInt32 boxwidth = MIN1(dx >> 16);
    DInt32 scaleval = 65536 / (boxwidth * boxheight);
    DInt32 i;
    x >>= 16;
    for (i = 0; i < dst_width; ++i) {
        *dst_ptr++ = SumPixels(boxwidth, src_ptr + x) * scaleval >> 16;
        x += boxwidth;
    }
}

static DVoid ScaleAddCols0_C(DInt32 dst_width,
    DInt32 boxheight,
    DInt32 x,
    DInt32 dx,
    const uint16_t* src_ptr,
    DUInt8* dst_ptr) {
    DInt32 scaleval = 65536 / boxheight;
    DInt32 i;
    (DVoid)dx;
    src_ptr += (x >> 16);
    for (i = 0; i < dst_width; ++i) {
        *dst_ptr++ = src_ptr[i] * scaleval >> 16;
    }
}

DVoid ScaleAddRow_C(const DUInt8* src_ptr, uint16_t* dst_ptr, DInt32 src_width) {
    DInt32 x;
    assert(src_width > 0);
    for (x = 0; x < src_width - 1; x += 2) {
        dst_ptr[0] += src_ptr[0];
        dst_ptr[1] += src_ptr[1];
        src_ptr += 2;
        dst_ptr += 2;
    }
    if (src_width & 1) {
        dst_ptr[0] += src_ptr[0];
    }
}

// Scale plane down to any dimensions, with interpolation.
// (boxfilter).
//
// Same method as SimpleScale, which is fixed point, outputting
// one pixel of destination using fixed point (16.16) to step
// through source, sampling a box of pixel with simple
// averaging.
static DVoid ScalePlaneBox(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr) {
    DInt32 j, k;
    // Initial source x/y coordinate and step values as 16.16 fixed point.
    DInt32 x = 0;
    DInt32 y = 0;
    DInt32 dx = 0;
    DInt32 dy = 0;
    const DInt32 max_y = (src_height << 16);
    ScaleSlope(src_width, src_height, dst_width, dst_height, kFilterBox, &x, &y,
        &dx, &dy);
    src_width = Abs(src_width);
    {
        // Allocate a row buffer of uint16_t.
        align_buffer_64(row16, src_width * 2);
        DVoid (*ScaleAddCols)(DInt32 dst_width, DInt32 boxheight, DInt32 x, DInt32 dx,
            const uint16_t * src_ptr, DUInt8 * dst_ptr) =
            (dx & 0xffff) ? ScaleAddCols2_C
            : ((dx != 0x10000) ? ScaleAddCols1_C : ScaleAddCols0_C);
        DVoid (*ScaleAddRow)(const DUInt8 * src_ptr, uint16_t * dst_ptr,
            DInt32 src_width) = ScaleAddRow_C;

        for (j = 0; j < dst_height; ++j) {
            DInt32 boxheight;
            DInt32 iy = y >> 16;
            const DUInt8* src = src_ptr + iy * src_stride;
            y += dy;
            if (y > max_y) {
                y = max_y;
            }
            boxheight = MIN1((y >> 16) - iy);
            memset(row16, 0, src_width * 2);
            for (k = 0; k < boxheight; ++k) {
                ScaleAddRow(src, (uint16_t*)(row16), src_width);
                src += src_stride;
            }
            ScaleAddCols(dst_width, boxheight, x, dx, (uint16_t*)(row16), dst_ptr);
            dst_ptr += dst_stride;
        }
        free_aligned_buffer_64(row16);
    }
}

// Blend 2 rows into 1.
static DVoid HalfRow_C(const DUInt8* src_uv,
    DPtrDiff src_uv_stride,
    DUInt8* dst_uv,
    DInt32 width) {
    DInt32 x;
    for (x = 0; x < width; ++x) {
        dst_uv[x] = (src_uv[x] + src_uv[src_uv_stride + x] + 1) >> 1;
    }
}

// C version 2x2 -> 2x1.
DVoid InterpolateRow_C(DUInt8* dst_ptr,
    const DUInt8* src_ptr,
    DPtrDiff src_stride,
    DInt32 width,
    DInt32 source_y_fraction) {
    DInt32 y1_fraction = source_y_fraction;
    DInt32 y0_fraction = 256 - y1_fraction;
    const DUInt8* src_ptr1 = src_ptr + src_stride;
    DInt32 x;
    if (y1_fraction == 0) {
        memcpy(dst_ptr, src_ptr, width);
        return;
    }
    if (y1_fraction == 128) {
        HalfRow_C(src_ptr, src_stride, dst_ptr, width);
        return;
    }
    for (x = 0; x < width - 1; x += 2) {
        dst_ptr[0] =
            (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
        dst_ptr[1] =
            (src_ptr[1] * y0_fraction + src_ptr1[1] * y1_fraction + 128) >> 8;
        src_ptr += 2;
        src_ptr1 += 2;
        dst_ptr += 2;
    }
    if (width & 1) {
        dst_ptr[0] =
            (src_ptr[0] * y0_fraction + src_ptr1[0] * y1_fraction + 128) >> 8;
    }
}

#define BLENDER(a, b, f) \
  (DUInt8)((DInt32)(a) + (((DInt32)((f) >> 9) * ((DInt32)(b) - (DInt32)(a)) + 0x40) >> 7))

DVoid ScaleFilterCols_C(DUInt8* dst_ptr,
    const DUInt8* src_ptr,
    DInt32 dst_width,
    DInt32 x,
    DInt32 dx) {
    DInt32 j;
    for (j = 0; j < dst_width - 1; j += 2) {
        DInt32 xi = x >> 16;
        DInt32 a = src_ptr[xi];
        DInt32 b = src_ptr[xi + 1];
        dst_ptr[0] = BLENDER(a, b, x & 0xffff);
        x += dx;
        xi = x >> 16;
        a = src_ptr[xi];
        b = src_ptr[xi + 1];
        dst_ptr[1] = BLENDER(a, b, x & 0xffff);
        x += dx;
        dst_ptr += 2;
    }
    if (dst_width & 1) {
        DInt32 xi = x >> 16;
        DInt32 a = src_ptr[xi];
        DInt32 b = src_ptr[xi + 1];
        dst_ptr[0] = BLENDER(a, b, x & 0xffff);
    }
}

DVoid ScaleFilterCols64_C(DUInt8* dst_ptr,
    const DUInt8* src_ptr,
    DInt32 dst_width,
    DInt32 x32,
    DInt32 dx) {
    int64_t x = (int64_t)(x32);
    DInt32 j;
    for (j = 0; j < dst_width - 1; j += 2) {
        int64_t xi = x >> 16;
        DInt32 a = src_ptr[xi];
        DInt32 b = src_ptr[xi + 1];
        dst_ptr[0] = BLENDER(a, b, x & 0xffff);
        x += dx;
        xi = x >> 16;
        a = src_ptr[xi];
        b = src_ptr[xi + 1];
        dst_ptr[1] = BLENDER(a, b, x & 0xffff);
        x += dx;
        dst_ptr += 2;
    }
    if (dst_width & 1) {
        int64_t xi = x >> 16;
        DInt32 a = src_ptr[xi];
        DInt32 b = src_ptr[xi + 1];
        dst_ptr[0] = BLENDER(a, b, x & 0xffff);
    }
}

// Scales a single row of pixels up by 2x using point sampling.
DVoid ScaleColsUp2_C(DUInt8* dst_ptr,
    const DUInt8* src_ptr,
    DInt32 dst_width,
    DInt32 x,
    DInt32 dx) {
    DInt32 j;
    (DVoid)x;
    (DVoid)dx;
    for (j = 0; j < dst_width - 1; j += 2) {
        dst_ptr[1] = dst_ptr[0] = src_ptr[0];
        src_ptr += 1;
        dst_ptr += 2;
    }
    if (dst_width & 1) {
        dst_ptr[0] = src_ptr[0];
    }
}

// Scales a single row of pixels using point sampling.
DVoid ScaleCols_C(DUInt8* dst_ptr,
    const DUInt8* src_ptr,
    DInt32 dst_width,
    DInt32 x,
    DInt32 dx) {
    DInt32 j;
    for (j = 0; j < dst_width - 1; j += 2) {
        dst_ptr[0] = src_ptr[x >> 16];
        x += dx;
        dst_ptr[1] = src_ptr[x >> 16];
        x += dx;
        dst_ptr += 2;
    }
    if (dst_width & 1) {
        dst_ptr[0] = src_ptr[x >> 16];
    }
}

// Scale up down with bilinear interpolation.
DVoid ScalePlaneBilinearUp(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr,
    enum FilterMode filtering) {
    DInt32 j;
    // Initial source x/y coordinate and step values as 16.16 fixed point.
    DInt32 x = 0;
    DInt32 y = 0;
    DInt32 dx = 0;
    DInt32 dy = 0;
    const DInt32 max_y = (src_height - 1) << 16;
    DVoid (*InterpolateRow)(DUInt8 * dst_ptr, const DUInt8 * src_ptr,
        DPtrDiff src_stride, DInt32 dst_width,
        DInt32 source_y_fraction) = InterpolateRow_C;
    DVoid (*ScaleFilterCols)(DUInt8 * dst_ptr, const DUInt8 * src_ptr,
        DInt32 dst_width, DInt32 x, DInt32 dx) =
        filtering ? ScaleFilterCols_C : ScaleCols_C;
    ScaleSlope(src_width, src_height, dst_width, dst_height, filtering, &x, &y,
        &dx, &dy);
    src_width = Abs(src_width);

    if (filtering && src_width >= 32768) {
        ScaleFilterCols = ScaleFilterCols64_C;
    }
    if (!filtering && src_width * 2 == dst_width && x < 0x8000) {
        ScaleFilterCols = ScaleColsUp2_C;
    }

    if (y > max_y) {
        y = max_y;
    }
    {
        DInt32 yi = y >> 16;
        const DUInt8* src = src_ptr + yi * src_stride;

        // Allocate 2 row buffers.
        const DInt32 kRowSize = (dst_width + 31) & ~31;
        align_buffer_64(row, kRowSize * 2);

        DUInt8* rowptr = row;
        DInt32 rowstride = kRowSize;
        DInt32 lasty = yi;

        ScaleFilterCols(rowptr, src, dst_width, x, dx);
        if (src_height > 1) {
            src += src_stride;
        }
        ScaleFilterCols(rowptr + rowstride, src, dst_width, x, dx);
        src += src_stride;

        for (j = 0; j < dst_height; ++j) {
            yi = y >> 16;
            if (yi != lasty) {
                if (y > max_y) {
                    y = max_y;
                    yi = y >> 16;
                    src = src_ptr + yi * src_stride;
                }
                if (yi != lasty) {
                    ScaleFilterCols(rowptr, src, dst_width, x, dx);
                    rowptr += rowstride;
                    rowstride = -rowstride;
                    lasty = yi;
                    src += src_stride;
                }
            }
            if (filtering == kFilterLinear) {
                InterpolateRow(dst_ptr, rowptr, 0, dst_width, 0);
            }
            else {
                DInt32 yf = (y >> 8) & 255;
                InterpolateRow(dst_ptr, rowptr, rowstride, dst_width, yf);
            }
            dst_ptr += dst_stride;
            y += dy;
        }
        free_aligned_buffer_64(row);
    }
}

// Scale plane down with bilinear interpolation.
DVoid ScalePlaneBilinearDown(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr,
    enum FilterMode filtering) {
    // Initial source x/y coordinate and step values as 16.16 fixed point.
    DInt32 x = 0;
    DInt32 y = 0;
    DInt32 dx = 0;
    DInt32 dy = 0;
    // TODO(fbarchard): Consider not allocating row buffer for kFilterLinear.
    // Allocate a row buffer.
    align_buffer_64(row, src_width);

    const DInt32 max_y = (src_height - 1) << 16;
    DInt32 j;
    DVoid (*ScaleFilterCols)(DUInt8 * dst_ptr, const DUInt8 * src_ptr,
        DInt32 dst_width, DInt32 x, DInt32 dx) =
        (src_width >= 32768) ? ScaleFilterCols64_C : ScaleFilterCols_C;
    DVoid (*InterpolateRow)(DUInt8 * dst_ptr, const DUInt8 * src_ptr,
        DPtrDiff src_stride, DInt32 dst_width,
        DInt32 source_y_fraction) = InterpolateRow_C;
    ScaleSlope(src_width, src_height, dst_width, dst_height, filtering, &x, &y,
        &dx, &dy);
    src_width = Abs(src_width);

    if (y > max_y) {
        y = max_y;
    }

    for (j = 0; j < dst_height; ++j) {
        DInt32 yi = y >> 16;
        const DUInt8* src = src_ptr + yi * src_stride;
        if (filtering == kFilterLinear) {
            ScaleFilterCols(dst_ptr, src, dst_width, x, dx);
        }
        else {
            DInt32 yf = (y >> 8) & 255;
            InterpolateRow(row, src, src_stride, src_width, yf);
            ScaleFilterCols(dst_ptr, row, dst_width, x, dx);
        }
        dst_ptr += dst_stride;
        y += dy;
        if (y > max_y) {
            y = max_y;
        }
    }
    free_aligned_buffer_64(row);
}


// Scale Plane to/from any dimensions, without interpolation.
// Fixed point math is used for performance: The upper 16 bits
// of x and dx is the integer part of the source position and
// the lower 16 bits are the fixed decimal part.

static DVoid ScalePlaneSimple(DInt32 src_width,
    DInt32 src_height,
    DInt32 dst_width,
    DInt32 dst_height,
    DInt32 src_stride,
    DInt32 dst_stride,
    const DUInt8* src_ptr,
    DUInt8* dst_ptr) {
    DInt32 i;
    DVoid (*ScaleCols)(DUInt8 * dst_ptr, const DUInt8 * src_ptr, DInt32 dst_width,
        DInt32 x, DInt32 dx) = ScaleCols_C;
    // Initial source x/y coordinate and step values as 16.16 fixed point.
    DInt32 x = 0;
    DInt32 y = 0;
    DInt32 dx = 0;
    DInt32 dy = 0;
    ScaleSlope(src_width, src_height, dst_width, dst_height, kFilterNone, &x, &y,
        &dx, &dy);
    src_width = Abs(src_width);

    if (src_width * 2 == dst_width && x < 0x8000) {
        ScaleCols = ScaleColsUp2_C;
    }

    for (i = 0; i < dst_height; ++i) {
        ScaleCols(dst_ptr, src_ptr + (y >> 16) * src_stride, dst_width, x, dx);
        dst_ptr += dst_stride;
        y += dy;
    }
}

DVoid ScalePlane(const DUInt8* src, DInt32 src_stride, DInt32 src_width, DInt32 src_height, 
    DUInt8* dst, DInt32 dst_stride, DInt32 dst_width, DInt32 dst_height, enum FilterMode filtering) 
{
    // Simplify filtering when possible.
    filtering = ScaleFilterReduce(src_width, src_height, dst_width, dst_height,
        filtering);

    // Negative height means invert the image.
    if (src_height < 0) {
        src_height = -src_height;
        src = src + (src_height - 1) * src_stride;
        src_stride = -src_stride;
    }

    // Use specialized scales to improve performance for common resolutions.
    // For example, all the 1/2 scalings will use ScalePlaneDown2()
    if (dst_width == src_width && dst_height == src_height) {
        // Straight copy.
        CopyPlane(src, src_stride, dst, dst_stride, dst_width, dst_height);
        return;
    }
    if (dst_width == src_width && filtering != kFilterBox) {
        DInt32 dy = FixedDiv(src_height, dst_height);
        // Arbitrary scale vertically, but unscaled horizontally.
        ScalePlaneVertical(src_height, dst_width, dst_height, src_stride,
            dst_stride, src, dst, 0, 0, dy, 1, filtering);
        return;
    }
    if (dst_width <= Abs(src_width) && dst_height <= src_height) {
        // Scale down.
        if (4 * dst_width == 3 * src_width && 4 * dst_height == 3 * src_height) {
            // optimized, 3/4
            ScalePlaneDown34(src_width, src_height, dst_width, dst_height, src_stride,
                dst_stride, src, dst, filtering);
            return;
        }
        if (2 * dst_width == src_width && 2 * dst_height == src_height) {
            // optimized, 1/2
            ScalePlaneDown2(src_width, src_height, dst_width, dst_height, src_stride,
                dst_stride, src, dst, filtering);
            return;
        }
        // 3/8 rounded up for odd sized chroma height.
        if (8 * dst_width == 3 * src_width && 8 * dst_height == 3 * src_height) {
            // optimized, 3/8
            ScalePlaneDown38(src_width, src_height, dst_width, dst_height, src_stride,
                dst_stride, src, dst, filtering);
            return;
        }
        if (4 * dst_width == src_width && 4 * dst_height == src_height &&
            (filtering == kFilterBox || filtering == kFilterNone)) {
            // optimized, 1/4
            ScalePlaneDown4(src_width, src_height, dst_width, dst_height, src_stride,
                dst_stride, src, dst, filtering);
            return;
        }
    }
    if (filtering == kFilterBox && dst_height * 2 < src_height) {
        ScalePlaneBox(src_width, src_height, dst_width, dst_height, src_stride,
            dst_stride, src, dst);
        return;
    }
    if (filtering && dst_height > src_height) {
        ScalePlaneBilinearUp(src_width, src_height, dst_width, dst_height,
            src_stride, dst_stride, src, dst, filtering);
        return;
    }
    if (filtering) {
        ScalePlaneBilinearDown(src_width, src_height, dst_width, dst_height,
            src_stride, dst_stride, src, dst, filtering);
        return;
    }
    ScalePlaneSimple(src_width, src_height, dst_width, dst_height, src_stride,
        dst_stride, src, dst);
}

DInt32 DVideoI420::I420Scale(const DByte* src_y, DInt32 src_stride_y,
    const DByte* src_u, DInt32 src_stride_u,
    const DByte* src_v, DInt32 src_stride_v,
    DInt32 src_width, DInt32 src_height,
    DByte* dst_y, DInt32 dst_stride_y,
    DByte* dst_u, DInt32 dst_stride_u,
    DByte* dst_v, DInt32 dst_stride_v,
    DInt32 dst_width, DInt32 dst_height, enum FilterMode filtering) 
{
    DInt32 src_halfwidth = SUBSAMPLE(src_width, 1, 1);
    DInt32 src_halfheight = SUBSAMPLE(src_height, 1, 1);
    DInt32 dst_halfwidth = SUBSAMPLE(dst_width, 1, 1);
    DInt32 dst_halfheight = SUBSAMPLE(dst_height, 1, 1);
    if (!src_y || !src_u || !src_v || src_width == 0 || src_height == 0 ||
        src_width > 32768 || src_height > 32768 || !dst_y || !dst_u || !dst_v ||
        dst_width <= 0 || dst_height <= 0) {
        return -1;
    }

    ScalePlane(src_y, src_stride_y, src_width, src_height, dst_y, dst_stride_y,
        dst_width, dst_height, filtering);
    ScalePlane(src_u, src_stride_u, src_halfwidth, src_halfheight, dst_u,
        dst_stride_u, dst_halfwidth, dst_halfheight, filtering);
    ScalePlane(src_v, src_stride_v, src_halfwidth, src_halfheight, dst_v,
        dst_stride_v, dst_halfwidth, dst_halfheight, filtering);
    return 0;
}


DInt32 I420Copy(const DUInt8* src_y,
    DInt32 src_stride_y,
    const DUInt8* src_u,
    DInt32 src_stride_u,
    const DUInt8* src_v,
    DInt32 src_stride_v,
    DUInt8* dst_y,
    DInt32 dst_stride_y,
    DUInt8* dst_u,
    DInt32 dst_stride_u,
    DUInt8* dst_v,
    DInt32 dst_stride_v,
    DInt32 width,
    DInt32 height) {
    DInt32 halfwidth = (width + 1) >> 1;
    DInt32 halfheight = (height + 1) >> 1;
    if (!src_u || !src_v || !dst_u || !dst_v || width <= 0 || height == 0) {
        return -1;
    }
    // Negative height means invert the image.
    if (height < 0) {
        height = -height;
        halfheight = (height + 1) >> 1;
        src_y = src_y + (height - 1) * src_stride_y;
        src_u = src_u + (halfheight - 1) * src_stride_u;
        src_v = src_v + (halfheight - 1) * src_stride_v;
        src_stride_y = -src_stride_y;
        src_stride_u = -src_stride_u;
        src_stride_v = -src_stride_v;
    }

    if (dst_y) {
        CopyPlane(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
    }
    // Copy UV planes.
    CopyPlane(src_u, src_stride_u, dst_u, dst_stride_u, halfwidth, halfheight);
    CopyPlane(src_v, src_stride_v, dst_v, dst_stride_v, halfwidth, halfheight);
    return 0;
}

DVoid TransposeWx8(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width) {
    DInt32 i;
    for (i = 0; i < width; ++i) {
        dst[0] = src[0 * src_stride];
        dst[1] = src[1 * src_stride];
        dst[2] = src[2 * src_stride];
        dst[3] = src[3 * src_stride];
        dst[4] = src[4 * src_stride];
        dst[5] = src[5 * src_stride];
        dst[6] = src[6 * src_stride];
        dst[7] = src[7 * src_stride];
        ++src;
        dst += dst_stride;
    }
}

DVoid TransposeWxH(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width,
    DInt32 height) {
    DInt32 i;
    for (i = 0; i < width; ++i) {
        DInt32 j;
        for (j = 0; j < height; ++j) {
            dst[i * dst_stride + j] = src[j * src_stride + i];
        }
    }
}

DVoid TransposePlane(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width,
    DInt32 height) {
    DInt32 i = height;

    // Work across the source in 8x8 tiles
    while (i >= 8) {
        TransposeWx8(src, src_stride, dst, dst_stride, width);
        src += 8 * src_stride;  // Go down 8 rows.
        dst += 8;               // Move over 8 columns.
        i -= 8;
    }

    if (i > 0) {
        TransposeWxH(src, src_stride, dst, dst_stride, width, i);
    }
}

DVoid RotatePlane90(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width,
    DInt32 height) {
    // Rotate by 90 is a transpose with the source read
    // from bottom to top. So set the source pointer to the end
    // of the buffer and flip the sign of the source stride.
    src += src_stride * (height - 1);
    src_stride = -src_stride;
    TransposePlane(src, src_stride, dst, dst_stride, width, height);
}

DVoid RotatePlane270(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width,
    DInt32 height) {
    // Rotate by 270 is a transpose with the destination written
    // from bottom to top. So set the destination pointer to the end
    // of the buffer and flip the sign of the destination stride.
    dst += dst_stride * (width - 1);
    dst_stride = -dst_stride;
    TransposePlane(src, src_stride, dst, dst_stride, width, height);
}

DVoid MirrorRow(const DUInt8* src, DUInt8* dst, DInt32 width) 
{
    src += width - 1;
    for (DInt32 x = 0; x < width - 1; x += 2) {
        dst[x] = src[0];
        dst[x + 1] = src[-1];
        src -= 2;
    }
    if (width & 1) {
        dst[width - 1] = src[0];
    }
}

DVoid CopyRow(const DUInt8* src, DUInt8* dst, DInt32 count) {
    memcpy(dst, src, count);
}

DVoid RotatePlane180(const DUInt8* src,
    DInt32 src_stride,
    DUInt8* dst,
    DInt32 dst_stride,
    DInt32 width,
    DInt32 height) {
    // Swap first and last row and mirror the content. Uses a temporary row.
    align_buffer_64(row, width);
    const DUInt8* src_bot = src + src_stride * (height - 1);
    DUInt8* dst_bot = dst + dst_stride * (height - 1);
    DInt32 half_height = (height + 1) >> 1;
    DInt32 y;

    // Odd height will harmlessly mirror the middle row twice.
    for (y = 0; y < half_height; ++y) {
        MirrorRow(src, row, width);  // Mirror first row into a buffer
        src += src_stride;
        MirrorRow(src_bot, dst, width);  // Mirror last row into first row
        dst += dst_stride;
        CopyRow(row, dst_bot, width);  // Copy first mirrored row into last
        src_bot -= src_stride;
        dst_bot -= dst_stride;
    }
    free_aligned_buffer_64(row);
}

DInt32 DVideoI420::I420Rotate(const DUInt8* src_y, DInt32 src_stride_y,
    const DUInt8* src_u, DInt32 src_stride_u,
    const DUInt8* src_v, DInt32 src_stride_v,
    DUInt8* dst_y, DInt32 dst_stride_y,
    DUInt8* dst_u, DInt32 dst_stride_u,
    DUInt8* dst_v, DInt32 dst_stride_v,
    DInt32 width, DInt32 height, DRotation mode)
{
    DInt32 halfwidth = (width + 1) >> 1;
    DInt32 halfheight = (height + 1) >> 1;
    if (!src_y || !src_u || !src_v || width <= 0 || height == 0 || !dst_y ||
        !dst_u || !dst_v) {
        return -1;
    }

    if (height < 0) {
        height = -height;
        halfheight = (height + 1) >> 1;
        src_y = src_y + (height - 1) * src_stride_y;
        src_u = src_u + (halfheight - 1) * src_stride_u;
        src_v = src_v + (halfheight - 1) * src_stride_v;
        src_stride_y = -src_stride_y;
        src_stride_u = -src_stride_u;
        src_stride_v = -src_stride_v;
    }

    switch (mode) {
    case DRotation::DEGREE_0:
        return I420Copy(src_y, src_stride_y, src_u, src_stride_u, src_v,
            src_stride_v, dst_y, dst_stride_y, dst_u, dst_stride_u,
            dst_v, dst_stride_v, width, height);
    case DRotation::DEGREE_90:
        RotatePlane90(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
        RotatePlane90(src_u, src_stride_u, dst_u, dst_stride_u, halfwidth,
            halfheight);
        RotatePlane90(src_v, src_stride_v, dst_v, dst_stride_v, halfwidth,
            halfheight);
        return 0;
    case DRotation::DEGREE_270:
        RotatePlane270(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
        RotatePlane270(src_u, src_stride_u, dst_u, dst_stride_u, halfwidth,
            halfheight);
        RotatePlane270(src_v, src_stride_v, dst_v, dst_stride_v, halfwidth,
            halfheight);
        return 0;
    case DRotation::DEGREE_180:
        RotatePlane180(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
        RotatePlane180(src_u, src_stride_u, dst_u, dst_stride_u, halfwidth,
            halfheight);
        RotatePlane180(src_v, src_stride_v, dst_v, dst_stride_v, halfwidth,
            halfheight);
        return 0;
    default:
        break;
    }
    return -1;
}

DVoid MirrorPlane(const DUInt8* src_y, DInt32 src_stride_y,
    DUInt8* dst_y, DInt32 dst_stride_y,
    DInt32 width, DInt32 height) 
{
    if (height < 0) {
        height = -height;
        src_y = src_y + (height - 1) * src_stride_y;
        src_stride_y = -src_stride_y;
    }

    for (DInt32 y = 0; y < height; ++y) 
    {
        MirrorRow(src_y, dst_y, width);
        src_y += src_stride_y;
        dst_y += dst_stride_y;
    }
}

DInt32 DVideoI420::I420Mirror(const DUInt8* src_y, DInt32 src_stride_y,
    const DUInt8* src_u, DInt32 src_stride_u,
    const DUInt8* src_v, DInt32 src_stride_v,
    DUInt8* dst_y, DInt32 dst_stride_y,
    DUInt8* dst_u, DInt32 dst_stride_u,
    DUInt8* dst_v, DInt32 dst_stride_v,
    DInt32 width, DInt32 height) 
{
    DInt32 halfwidth = (width + 1) >> 1;
    DInt32 halfheight = (height + 1) >> 1;
    if (!src_y || !src_u || !src_v || !dst_y || !dst_u || !dst_v || width <= 0 ||
        height == 0) {
        return -1;
    }

    if (height < 0) {
        height = -height;
        halfheight = (height + 1) >> 1;
        src_y = src_y + (height - 1) * src_stride_y;
        src_u = src_u + (halfheight - 1) * src_stride_u;
        src_v = src_v + (halfheight - 1) * src_stride_v;
        src_stride_y = -src_stride_y;
        src_stride_u = -src_stride_u;
        src_stride_v = -src_stride_v;
    }

    if (dst_y) {
        MirrorPlane(src_y, src_stride_y, dst_y, dst_stride_y, width, height);
    }
    MirrorPlane(src_u, src_stride_u, dst_u, dst_stride_u, halfwidth, halfheight);
    MirrorPlane(src_v, src_stride_v, dst_v, dst_stride_v, halfwidth, halfheight);
    return 0;
}