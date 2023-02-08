#import "MacGLView.h"

#include <CoreVideo/CVOpenGLTexture.h>
#include <CoreVideo/CVOpenGLTextureCache.h>

#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

@interface MacGLView()

@property(strong)CIImage* pFrame;

@end

@implementation MacGLView

- (void) initGL
{
    NSOpenGLPixelFormatAttribute attrs[] = {NSOpenGLPFADoubleBuffer, NSOpenGLPFADepthSize,  24, NSOpenGLPFAAllowOfflineRenderers,  NSOpenGLPFAOpenGLProfile,  NSOpenGLProfileVersionLegacy, 0};
    
    NSOpenGLPixelFormat* egl_pixelformat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    NSOpenGLContext * egl_context_ = [[NSOpenGLContext alloc] initWithFormat:egl_pixelformat  shareContext:nil];
    
    [egl_context_ setView:self];
    
    [egl_context_ makeCurrentContext];
    
    CVOpenGLTextureCacheRef texture_cache_ = nil;
    
    CVOpenGLTextureCacheCreate(kCFAllocatorDefault, NULL, [egl_context_ CGLContextObj], [egl_pixelformat CGLPixelFormatObj], NULL, &texture_cache_);
    
    glClearColor(0, 0.5, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_RECTANGLE_ARB, rgbTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glViewport(10, 10, 10, 10);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // Unbind the texture as a precaution.
    glBindTexture(GL_TEXTURE_RED_TYPE_ARB, 0);

    [egl_context_ flushBuffer];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    

}

- (void)drawFrame:(CIImage*)pFrame
{
    [self setNeedsDisplay:YES];
}

- (void)drawNV12
{
    int height = CVPixelBufferGetHeight(nv12_buffer);
    int width = CVPixelBufferGetWidth(nv12_buffer);
    IOSurfaceRef surface = CVPixelBufferGetIOSurface(nv12_buffer);
    CGLContextObj cgl_context = [(__bridge NSOpenGLContext*)gl_context CGLContextObj];
    // gen texture
    if (texture1_ == 0) {
        glGenTextures(1, &texture1_);
    }
    glBindTexture(GL_TEXTURE_2D, texture1_);
    CGLTexImageIOSurface2D(cgl_context, GL_TEXTURE_2D, GL_R8, width, height,
             GL_RED, GL_UNSIGNED_BYTE, surface, 0);
    
    if (texture2_ == 0) {
        glGenTextures(1, &texture2_);
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, texture2_);
         CGLTexImageIOSurface2D(cgl_context, GL_TEXTURE_2D, GL_LUMINANCE8_ALPHA8, width / 2,
             height / 2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, surface, 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    destNV12Tex[0] = texture1_;
    destNV12Tex[1] = texture2_;
}

@end
