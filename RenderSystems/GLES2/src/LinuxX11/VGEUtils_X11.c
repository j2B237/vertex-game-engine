/////////////////////////////////////////////////////////////////////////
//
/// @file veUtil_X11.c
/// @brief This file contains the LinuxX11 implementation of the windowing
//  functions.
//

//
// Includes
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "VGEUtils.h"

// X11 relate local variables
static Display *x_display = NULL;
static Atom s_wmDeleteMessage;


//////////////////////////////////////////////////////////////////
//
//  Private Functions
//
//


//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

///
/// @brief WinCreate(). This function initialized the native X11 display and 
//  window for EGL
//
GLboolean WinCreate ( VGEContext *vgeContext, const char *title)
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig config;
    EGLint numConfig;
    Window win;

    /**
     * X11 native display initialization
     */

    x_display = XOpenDisplay (NULL);
    if ( x_display == NULL )
    {
        return EGL_FALSE;
    }

    root = DefaultRootWindow (x_display);

    swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow (x_display, root, 
                        0, 0, vgeContext->width, vgeContext->height, 0, 
                        CopyFromParent, InputOutput,
                        CopyFromParent, CWEventMask, &swa);

    s_wmDeleteMessage = XInternAtom (x_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols (x_display, win, &s_wmDeleteMessage, 1);

    xattr.override_redirect = GL_FALSE;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = GL_TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);


    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", GL_FALSE);

    memset ( &xev, 0, sizeof(xev) );

    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = GL_FALSE;

    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       GL_FALSE,
       SubstructureNotifyMask,
       &xev );

    vgeContext->eglNativeWindow = (EGLNativeWindowType) win;
    vgeContext->eglNativeDisplay = (EGLNativeDisplayType) x_display;

    return EGL_TRUE;

}

///
/// @brief userInterrupt(). Reads from X11 event loop and interrupt program if 
//  there is a keypress, or window close action.
//
//
GLboolean userInterrupt(VGEContext *vgeContext)
{
    XEvent xev;
    KeySym key;
    GLboolean userinterrupt = EGL_FALSE;
    char text;

    // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );

        if ( xev.type == KeyPress )
        {
            if (XLookupString ( &xev.xkey, &text,1, &key, 0 ) == 1 )
            {
                if (vgeContext->keyFunc != NULL )
                    vgeContext->keyFunc ( vgeContext, text, 0, 0 );
            }
        }
        if ( xev.type == ClientMessage ) {
            if ( xev.xclient.data.l[0] == s_wmDeleteMessage) {
                userinterrupt = GL_TRUE;
            }
        }
        if ( xev.type == DestroyNotify )
            userinterrupt = GL_TRUE;
    }
    return userinterrupt;
}

//
/// @brief WinLoop (). Start main windows loop
//
void WinLoop (VGEContext *vgeContext )
{
    struct timeval t1, t2;
    struct timezone tz;         // definittion wille be complete at compile time
    float deltatime;

    gettimeofday ( &t1, &tz);

    while (userInterrupt (vgeContext) == EGL_FALSE ) 
    {
        gettimeofday (&t2, &tz);
        deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_sec - t1.tv_usec) * 1E06 );
        t1 = t2;

        if (vgeContext->updateFunc != NULL)
        {
            vgeContext->updateFunc (vgeContext, deltatime);
        }
        if (vgeContext->drawFunc != NULL)
        {
            vgeContext->drawFunc (vgeContext);
        }

        eglSwapBuffers (vgeContext->eglDisplay, vgeContext->eglSurface);
    }
}


/// @brief Global extern.  The application must declsare this function
/// that runs the application.
///
extern int esMain( VGEContext *vgeContext );


/// @brief Main entrypoint for application
////
int main ( int argc, char *argv[] )
{
    VGEContext vgeContext;
    memset ( &vgeContext, 0, sizeof ( VGEContext ) );

    if ( esMain ( &vgeContext ) != GL_TRUE ) 
    {
        return EXIT_FAILURE;
    }

    WinLoop ( &vgeContext );

    if ( vgeContext.shutdownFunc != NULL )
    {
        vgeContext.shutdownFunc ( &vgeContext );
    }

    if ( vgeContext.userData != NULL )
    {
        free ( vgeContext.userData );
    }

    return EXIT_SUCCESS;
}
