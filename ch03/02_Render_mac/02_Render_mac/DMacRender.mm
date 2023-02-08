
#include "DMacRender.h"
#import <AppKit/NSOpenGL.h>


bool DMacRender::DrawYuv(const DByte* src_y, DInt32 src_stride_y, const DByte* src_u, DInt32 src_stride_u, const DByte* src_v, DInt32 src_stride_v, int frame_width, DInt32 frame_height, DRotation rotation, DColorSpace color_space)
{
    NSOpenGLContext* current = [NSOpenGLContext currentContext];
    
    NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize,  24, NSOpenGLPFAAllowOfflineRenderers,  NSOpenGLPFAOpenGLProfile,  NSOpenGLProfileVersionLegacy, 0};
    
    NSOpenGLPixelFormat* egl_pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    
  
    return true;
}
