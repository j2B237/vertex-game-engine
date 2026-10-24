/////////////////////////////////////////////////////////////////////////
///
///
/// @file VGEUtils.c
/// @brief A utility interface for OpenGL ES 3.0. This interface provides
/// a basic common framework for rendering operations.
///
/////////////////////////////////////////////////////////////////////////

//
/// Includes
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "VGEUtils.h"
#include "VGEUtils_win.h"


#ifdef ANDROID
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
typedef AAsset veFile;
#else
typedef FILE veFile;
#endif

#ifdef __APPLE__
#include "FileWrapper.h"
#endif


//
/// MACROS
//
#define INVERTED_BIT        (1 << 5)

//
/// Types
//
#ifndef __APPLE__
#pragma pack(push, x1)                  // Byte alignment (8-bit)
#pragma pack(1)
#endif

/// @brief This structure represents header of image file TARGA (Truevision TGA). It allow
/// the program to read informations describing an image before loading pixels data.

typedef struct 
#ifdef __APPLE__
__attribute__ ( ( packed ) )
#endif
{

    unsigned char  IdSize,
            MapType,
            ImageType;
    unsigned short PaletteStart,
            PaletteSize;
    unsigned char  PaletteEntryDepth;
    unsigned short X,
            Y,
            Width,
            Height;
   unsigned char  ColorDepth,
            Descriptor;

} TGA_HEADER;

#ifdef __APPLE__
#pragma pack (pop, x1)
#endif

#ifndef __APPLE__

////////////////////////////////////////////////////////////////////////////
///
///     Private Functions
///
////////////////////////////////////////////////////////////////////////////


/// @brief Check whether EGL_KHR_create_context extension is supported.
///         If so,
/// @param  eglDisplay EGL handler to the native display
/// @return EGL_OPENGL_ES3_BIT_KHR instead of EGL_OPENGL_ES2_BIT
//
EGLint GetContextRenderableType ( EGLDisplay eglDisplay )
{
#ifdef EGL_KHR_create_context
   const char *extensions = eglQueryString ( eglDisplay, EGL_EXTENSIONS );

   // check whether EGL_KHR_create_context is in the extension string
   if ( extensions != NULL && strstr( extensions, "EGL_KHR_create_context" ) )
   {
      // extension is supported
      return EGL_OPENGL_ES3_BIT_KHR;
   }
#endif
   // extension is not supported
   return EGL_OPENGL_ES2_BIT;
}
#endif


////////////////////////////////////////////////////////////////////////////
//  
//      Public Functions
//
//

GLboolean VGE_API vgeCreateWindow (VGEContext *vgeContext, const char *title, GLint width, GLint height, GLuint flags)
{
#ifndef __APPLE__
    EGLConfig config;
    GLint majorVersion;
    GLint minorVersion;
    
    GLint contextAttribs[] = 
    {
        // Designate notation for array component
        [0] = EGL_CONTEXT_CLIENT_VERSION, 
        [1] = 3, 
        [2] = EGL_NONE
    };

    if ( vgeContext == NULL )
    {
        return EGL_FALSE;
    }

#ifdef ANDROID
    // For android get the width and height from the window rather than what
    // the application requested.
    vgeContext->width    = ANativeWindow_getWidth (vgeContext->eglNativeWindow);
    vgeContext->height   = ANativeWindow_getHeight (vgeContext->eglNativeWindow);
#else
    vgeContext->width    = width;
    vgeContext->height   = height;
#endif

    if ( !WinCreate ( vgeContext, title ) )
    {
        return EGL_FALSE;
    }

    // Get EGL Handler to the native display
    vgeContext->eglDisplay = eglGetDisplay ( vgeContext->eglNativeDisplay );

    if ( vgeContext->eglDisplay == EGL_NO_DISPLAY )
    {
        return EGL_FALSE;
    }

    // Initialize EGL
    if ( !eglInitialize ( vgeContext->eglDisplay, &majorVersion, &minorVersion ) )
    {
        return EGL_FALSE;
    }

    {
        EGLint numConfigs = 0;
        EGLint attribList[] = 
        {
            [0]     = EGL_RED_SIZE,         [1]     = 5,
            [2]     = EGL_GREEN_SIZE,       [3]     = 6,
            [4]     = EGL_BLUE_SIZE,        [5]     = 5,
            [6]     = EGL_ALPHA_SIZE,       [7]     = ( flags & VGE_WINDOW_ALPHA ) ? 8 : EGL_DONT_CARE,
            [8]     = EGL_DEPTH_SIZE,       [9]     = ( flags & VGE_WINDOW_DEPTH ) ? 8 : EGL_DONT_CARE,
            [10]    = EGL_STENCIL_SIZE,     [11]    = ( flags & VGE_WINDOW_STENCIL ) ? 8 : EGL_DONT_CARE,
            [12]    = EGL_SAMPLE_BUFFERS,   [13]    = ( flags & VGE_WINDOW_MULTISAMPLE ) ? 1 : 0,
            [14]    = EGL_RENDERABLE_TYPE,  [15]    = GetContextRenderableType ( vgeContext->eglDisplay),
            [16]    = EGL_NONE
        };

        // ChooseConfig
        if ( !eglChooseConfig ( vgeContext->eglDisplay, attribList, &config, 1, &numConfigs) )
        {
            return EGL_FALSE;
        }

        if ( numConfigs < 1) 
        {
            return EGL_FALSE;
        }
    }

#ifdef ANDROID
    // For Android, need to get the EGL_NATIVE_VISUAL_ID and set it
    // using ANativeWindow_setBuffersGeometry
    {
        EGLint format = 0;
        eglGetConfigAttrib ( vgeContext->eglDisplay, config, EGL_NATIVE_VISUAL_ID, &format );
        ANativeWindow_setBuffersGeometry ( vgeContext->eglNativeWindow, 0, 0, format );
    }
#endif  // ANDROID

    // Create a surface
    vgeContext->eglSurface = eglCreateWindowSurface (vgeContext->eglDisplay, config, 
                                                    vgeContext->eglNativeWindow, NULL );

    if ( vgeContext->eglSurface == EGL_NO_SURFACE )
    {
        return EGL_FALSE;
    }

    // Create a GL context
    vgeContext->eglContext = eglCreateContext (vgeContext->eglDisplay, config, EGL_NO_CONTEXT, contextAttribs );
    if ( vgeContext->eglContext == EGL_NO_CONTEXT )
    {
        return EGL_FALSE;
    }

    // Make the context current
    if ( !eglMakeCurrent (vgeContext->eglDisplay, vgeContext->eglSurface,
                        vgeContext->eglSurface, vgeContext->eglContext ))
    {
        return EGL_FALSE;    
    }

#endif // end __APPLE__

    return EGL_TRUE;
}


///
//  vgeRegisterDrawFunc()
//
void VGE_API vgeRegisterDrawFunc (VGEContext *vgeContext, void (VGECALLBACK *drawFunc) (VGEContext*))
{
   vgeContext->drawFunc = drawFunc;
}

///
//  vgeRegisterShutdownFunc()
//
void VGE_API vgeRegisterShutdownFunc (VGEContext *vgeContext, void (VGECALLBACK *shutdownFunc) (VGEContext*))
{
   vgeContext->shutdownFunc = shutdownFunc;
}

///
//  vgeRegisterUpdateFunc()
//
void VGE_API vgeRegisterUpdateFunc (VGEContext *vgeContext, void (VGECALLBACK *updateFunc) (VGEContext*, float))
{
   vgeContext->updateFunc = updateFunc;
}


///
//  vgeRegisterKeyFunc()
//
void VGE_API vgeRegisterKeyFunc (VGEContext *vgeContext, void (VGECALLBACK *keyFunc) (VGEContext*, unsigned char, int, int))
{
   vgeContext->keyFunc = keyFunc;
}



///
// vgeLogMessage()
//
//    Log an error message to the debug output for the platform
//
void VGE_API vgeLogMessage (const char *formatStr, ... )
{
   va_list params;
   char buf[BUFSIZ];

   va_start ( params, formatStr );
   vsprintf ( buf, formatStr, params );

#ifdef ANDROID
   __android_log_print ( ANDROID_LOG_INFO, "vgeUtil" , "%s", buf );
#else
   printf ( "%s", buf );
#endif

   va_end ( params );
}