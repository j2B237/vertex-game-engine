////////////////////////////////////////////////////////////////////////////
///
/// @file VGEUtils.h
/// @brief A utility interface for OpenGL ES 3.0. This interface provides
/// a basic common framework for rendering operations.
///
////////////////////////////////////////////////////////////////////////////


#ifndef VGEUTIL_H
#define VGEUTIL_H

#include <stdlib.h>

#ifdef __APPLE__
#include <OpenglES/ES3/gl.h>
#else
#include "./GLES3/gl3.h"
#include "./EGL/egl.h"
#include "./EGL/eglext.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// MACROS
#ifdef WIN32
#define VGE_API         __cdecl
#define VGECALLBACK     __cdecl 
#else
#define VGE_API
#define VGECALLBACK
#endif


/// @brief vgeCreateWindow flag - RGB color buffer
#define VGE_WINDOW_RGB          0
/// @brief vgeCreateWindow flag - ALPHA color buffer
#define VGE_WINDOW_ALPHA        1
/// @brief vgeCreareWindow flag - depth buffer
#define VGE_WINDOW_DEPTH        2
/// @brief vgecreateWindow flag - stencil buffer
#define VGE_WINDOW_STENCIL      4
/// @brief vgeCreateWindow flag - multi-sample buffer
#define VGE_WINDOW_MULTISAMPLE  8

typedef struct VGEContext VGEContext;

struct VGEContext
{
    /// Platform specific data
    void        *platformData;
    /// User data
    void        *userData;
    /// Window width
    GLint       width;
    /// Window height
    GLint       height;

#ifndef __APPLE__
    /// Display Handle
    EGLNativeDisplayType    eglNativeDisplay;
    /// Window handle
    EGLNativeWindowType     eglNativeWindow;
    /// EGL Display
    EGLDisplay              eglDisplay;
    /// EGL context
    EGLContext              eglContext;
    /// EGL surface
    EGLSurface              eglSurface;
#endif

    /// Pointer function callbacks 
    void (VGECALLBACK *drawFunc) (VGEContext*); 
    void (VGECALLBACK *shutdownFunc) (VGEContext*);
    void (VGECALLBACK *keyFunc) (VGEContext*, unsigned char, int, int);
    void (VGECALLBACK *updateFunc) (VGEContext*, float deltaTime);
};


/////////////////////////////
///  PUBLIC FUNCTIONS
////////////////////////////

/// @brief  Create a window with the specified parameters
/// @param  vgeContext Application context
/// @param  title Name of title bar of the window
/// @param  width Width in pixels of window to create
/// @param  height height in pixels of window to create
/// @param  flags Bitfield for the window creation flags
///         VGE_WINDOW_RGB   
///         VGE_WINDOW_ALPHA
///         VGE_WINDOW_DEPTH
///         VGE_WINDOW_STENCIL
///         VGE_WINDOW_MULTISAMPLE
/// @return GL_TRUE if window creation is succesful, GL_FALSE otherwise
///
GLboolean VGE_API vgeCreateWindow (VGEContext *vgeContext, const char *title, GLint width, GLint height, GLuint flags);

/// @brief  Register a draw callback function to be used to render each frame
/// @param  vgeContext Application context
/// @param  drawFunc Draw callback function that will be used to render the scene
/// @return void
///
void VGE_API vgeRegisterDrawFunc (VGEContext *vgeContext, void (VGECALLBACK *drawFunc) (VGEContext*));


/// @brief  Register a callback function to be called on shutdown
/// @param  vgeContext Application context
/// @param  shutdownFunc Shutdown callback function
/// @return void
///
void VGE_API vgeRegisterShutdownFunc (VGEContext *vgeContext, void (VGECALLBACK *shutdownFunc) (VGEContext*));


/// @brief  Register a keyboard input processing callback function
/// @param  vgeContext Application context
/// @param  keyFunc Key callback function for application processing of keyboard input
/// @return void
///
void VGE_API vgeRegisterKeyFunc (VGEContext *vgeContext, void (VGECALLBACK *keyFunc) (VGEContext*, unsigned char, int, int));


/// @brief  Register an update callback function to be used to update on each time step
/// @param  vgeContext Application context
/// @param  updateFunc Update callback function for application processing of keyboard input
/// @return void
///
void VGE_API vgeRegisterUpdateFunc (VGEContext *vgeContext, void (VGECALLBACK *updateFunc) (VGEContext*, float));


/// @brief  Log a message to the debug output for the platform
/// @param  formatStr Format string for error Log.
/// @return void
///
void VGE_API vgeLogMessage (const char *formatStr, ... );


#ifdef __cplusplus
}
#endif

#endif      // VGEUTIL_H