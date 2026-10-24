/////////////////////////////////////////////////////////////////////////////////////
//
/// @file veUtil_andrdoid.c
/// @brief This file contains the android implementation of the windowing functions.
//
//

//
/// Includes
//
#include <android/log.h>
#include <android_native_app_glue.h>
#include <time.h>
#include <string.h>
#include "VGEUtils.h"

#define LOGI ( ... )((void)__android_log_print (ANDROID_LOG_INFO, "veUtil", __VA_ARGS__))

/////////////////////////////////////////////////////////////
//
//      Private Functions
//
////////////////////////////////////////////////////////////

//
/// @brief GetCurrentTime ()
/// @return returm time pasted in seconds since the first start of our application
//
static float GetCurrentTime ()
{
    struct timespec clockRealTime;
    clock_gettime ( CLOCK_MONOTONIC, &clockRealTime );
    double curTimeInSeconds = clockRealTime.tv_sec + ( double ) clockRealTime.tv_nsec / 1.E+9;
    return ( float ) curTimeInSeconds;
}


//
/// @brief Android callback for onAppCmd.
/// @param pApp* Pointer to an android_app struct data type.
/// @param cmd Identifier of the command
/// @return void
static void HandleCommand ( struct android_app *pApp, int32_t cmd) 
{
    VGEContext *vgeContext = (VGEContext *) pApp->userData;

    switch ( cmd )
    {
        case APP_CMD_SAVE_STATE:
            // the OS ask us to save the state of the app
            break;

        case APP_CMD_INIT_WINDOW:

            vgeContext->eglNativeDisplay = EGL_DEFAULT_DISPLAY;
            vgeContext->eglNativeWindow = pApp->window;

            // Call the main entry point for the app
            if (esMain ( vgeContext ) != EGL_TRUE )
            {
                exit (0);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            
            // cleanup on shutdown
            if ( vgeContext->shutdownFunc != NULL )
            {
                vgeContext->shutdownFunc (vgeContext);
            }

            if ( vgeContext->userData != NULL )
            {
                free (vgeContext->userData);
            }

            memset (vgeContext, 0, sizeof (VGEContext) );
            break;
        
        case APP_CMD_LOST_FOCUS:
            
            // if the APP lost focus, avoid unnecessary processing (like monitoring the accelerometer)
            break;

        case APP_CMD_GAINED_FOCUS:
            // bring back a certain functionnality, like monitoring the accelerometer
            break;
    }
}

//
/// @brief Global extern. The application must declare this function
//  that runs the application
extern int esMain ( VGEContext *vgeContext );



///////////////////////////////////////////////////////////////////////
//
//      PUBLIC Functions
//
///////////////////////////////////////////////////////////////////////

/// @brief Main entrypoint for native android application
/// @param pApp. Pointer to android_app data struct
/// @return void
///
void android_main ( struct android_app *pApp)
{
    VGEContext vgeContext;
    float lastTime;

    /*
        * Make sure native_app_glue is not stripped by the linker.
        *
        * When developing a native Android application using NativeActivity,
        * Android expects a specific entry-point function named
        * ANativeActivity_onCreate.
        *
        * This function initializes the native activity and receives the
        * activity instance, along with optional saved state data.
        *
        * Its prototype is:
        *
        * void ANativeActivity_onCreate(
        *     ANativeActivity* activity,
        *     void* savedState,
        *     size_t savedStateSize
        * );
        *
        * The linker's -u option forces the specified symbol to be treated
        * as undefined, causing the linker to include its definition when
        * resolving symbols. This is useful when the implementation is
        * contained in a static library and might otherwise be discarded.
        *
        * When using native_app_glue, android_main() is the entry point
        * for the application's native code. native_app_glue provides
        * the mechanism that connects the NativeActivity lifecycle to
        * android_main().
        *
        * To ensure the native_app_glue entry-point code is linked,
        * add the following linker option to CMakeLists.txt:
        *
        * -Wl,-u,ANativeActivity_onCreate
    */ 
    // app_dummy ();

    // Initialize the context
    memset ( &vgeContext, 0, sizeof ( VGEContext ) );

    vgeContext.platformData = ( void *) pApp->activity->assetManager;

    pApp->onAppCmd = HandleCommand;
    pApp->userData = &vgeContext;

    lastTime = GetCurrentTime ();

    while (1)
    {
        int ident;
        int events;
        struct android_poll_source *pSource;

        while ( ( ident = ALooper_pollAll (0, NULL, &events, ( void **) &pSource ) ) >= 0 )
        {
            if ( pSource != NULL )
            {
                pSource->process ( pApp, pSource );
            }

            if ( pApp->destroyRequested != 0 )
            {
                return;
            }
        }

        if ( vgeContext.eglNativeWindow == NULL )
        {
            continue;
        }

        // Call app update function
        if ( vgeContext.updateFunc != NULL )
        {
            float curTime = GetCurrentTime();
            float deltaTime = ( curTime - lastTime );
            lastTime = curTime;

            vgeContext.updateFunc (&vgeContext, deltaTime );
        }

        if ( vgeContext.drawFunc != NULL )
        {
            vgeContext.drawFunc ( &vgeContext );
            eglSwapBuffers (vgeContext.eglDisplay, vgeContext.eglSurface );
        }
    }
}

/// @brief Create Win32 instance and window
/// @param vgeContext
/// @param title 
/// @return boolean value
///
GLboolean WinCreate ( VGEContext *vgeContext, const char *title )
{
   // On Android, this does not need to do anything.  Everything happens from
   // android_main()
   return GL_TRUE;
}


