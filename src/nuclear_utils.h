#ifndef __NUCLEAR_COMPOSITOR_NUCLEAR_UTILS_INCLUDED__
#define __NUCLEAR_COMPOSITOR_NUCLEAR_UTILS_INCLUDED__


#define GL_GLEXT_PROTOTYPES

#include <stdint.h>
#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#define radians(r) ((r)/(180.f/3.14159f))
#define degrees(r) ((r)*(180.f/3.14159f))

struct modifier_state {
	uint32_t depressed, latched, locked, group;
};

// Egl
typedef void (*PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) (GLenum target, EGLImage image);

void init_egl();

extern PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
extern PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC c_glEGLImageTargetTexture2DOES;

GLuint loadShaders(const char *vertexShaderName, const char *fragmentShaderName);

#endif
