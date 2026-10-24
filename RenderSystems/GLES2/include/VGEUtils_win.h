/////////////////////////////////////////////////////////////////////////////////////////
///
/// @file VGEUtils.h
/// @brief  API Neutral interface for creating windows. Implementation needs to provided
/// per-platform.
///
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef VGEUTILS_WIN_H
#define VGEUTILS_WIN_H


#ifdef __cplusplus
extern "C" {
#endif


/// @brief  Create Win32 instance and window
/// @param  vgeContext Application context
/// @param  title title appearing on window
/// @return GL_TRUE if window creation succeded otherwise GL_FALSE
///
GLboolean WinCreate (VGEContext *vgeContext, const char* title);


#ifdef __cplusplus
}
#endif

#endif //VGEUTILS_WIN_H