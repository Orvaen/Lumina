//========================================================================
// Lumina GLX - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifndef GLXBadProfileARB
 #define GLXBadProfileARB 13
#endif


// Returns the specified attribute of the specified GLXFBConfig
//
static int getGLXFBConfigAttrib(GLXFBConfig fbconfig, int attrib)
{
    int value;
    glXGetFBConfigAttrib(_lum.x11.display, fbconfig, attrib, &value);
    return value;
}

// Return the GLXFBConfig most closely matching the specified hints
//
static LUMbool chooseGLXFBConfig(const _LUMfbconfig* desired,
                                  GLXFBConfig* result)
{
    GLXFBConfig* nativeConfigs;
    _LUMfbconfig* usableConfigs;
    const _LUMfbconfig* closest;
    int nativeCount, usableCount;
    const char* vendor;
    LUMbool trustWindowBit = LUM_TRUE;

    // HACK: This is a (hopefully temporary) workaround for Chromium
    //       (VirtualBox GL) not setting the window bit on any GLXFBConfigs
    vendor = glXGetClientString(_lum.x11.display, GLX_VENDOR);
    if (vendor && strcmp(vendor, "Chromium") == 0)
        trustWindowBit = LUM_FALSE;

    nativeConfigs =
        glXGetFBConfigs(_lum.x11.display, _lum.x11.screen, &nativeCount);
    if (!nativeConfigs || !nativeCount)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "GLX: No GLXFBConfigs returned");
        return LUM_FALSE;
    }

    usableConfigs = _lum_calloc(nativeCount, sizeof(_LUMfbconfig));
    usableCount = 0;

    for (int i = 0;  i < nativeCount;  i++)
    {
        const GLXFBConfig n = nativeConfigs[i];
        _LUMfbconfig* u = usableConfigs + usableCount;

        // Only consider RGBA GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_RENDER_TYPE) & GLX_RGBA_BIT))
            continue;

        // Only consider window GLXFBConfigs
        if (!(getGLXFBConfigAttrib(n, GLX_DRAWABLE_TYPE) & GLX_WINDOW_BIT))
        {
            if (trustWindowBit)
                continue;
        }

        if (getGLXFBConfigAttrib(n, GLX_DOUBLEBUFFER) != desired->doublebuffer)
            continue;

        if (desired->transparent)
        {
            XVisualInfo* vi = glXGetVisualFromFBConfig(_lum.x11.display, n);
            if (vi)
            {
                u->transparent = _lumIsVisualTransparentX11(vi->visual);
                XFree(vi);
            }
        }

        u->redBits = getGLXFBConfigAttrib(n, GLX_RED_SIZE);
        u->greenBits = getGLXFBConfigAttrib(n, GLX_GREEN_SIZE);
        u->blueBits = getGLXFBConfigAttrib(n, GLX_BLUE_SIZE);

        u->alphaBits = getGLXFBConfigAttrib(n, GLX_ALPHA_SIZE);
        u->depthBits = getGLXFBConfigAttrib(n, GLX_DEPTH_SIZE);
        u->stencilBits = getGLXFBConfigAttrib(n, GLX_STENCIL_SIZE);

        u->accumRedBits = getGLXFBConfigAttrib(n, GLX_ACCUM_RED_SIZE);
        u->accumGreenBits = getGLXFBConfigAttrib(n, GLX_ACCUM_GREEN_SIZE);
        u->accumBlueBits = getGLXFBConfigAttrib(n, GLX_ACCUM_BLUE_SIZE);
        u->accumAlphaBits = getGLXFBConfigAttrib(n, GLX_ACCUM_ALPHA_SIZE);

        u->auxBuffers = getGLXFBConfigAttrib(n, GLX_AUX_BUFFERS);

        if (getGLXFBConfigAttrib(n, GLX_STEREO))
            u->stereo = LUM_TRUE;

        if (_lum.glx.ARB_multisample)
            u->samples = getGLXFBConfigAttrib(n, GLX_SAMPLES);

        if (_lum.glx.ARB_framebuffer_sRGB || _lum.glx.EXT_framebuffer_sRGB)
            u->sRGB = getGLXFBConfigAttrib(n, GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB);

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _lumChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (GLXFBConfig) closest->handle;

    XFree(nativeConfigs);
    _lum_free(usableConfigs);

    return closest != NULL;
}

// Create the OpenGL context using legacy API
//
static GLXContext createLegacyContextGLX(_LUMwindow* window,
                                         GLXFBConfig fbconfig,
                                         GLXContext share)
{
    return glXCreateNewContext(_lum.x11.display,
                               fbconfig,
                               GLX_RGBA_TYPE,
                               share,
                               True);
}

static void makeContextCurrentGLX(_LUMwindow* window)
{
    if (window)
    {
        if (!glXMakeCurrent(_lum.x11.display,
                            window->context.glx.window,
                            window->context.glx.handle))
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "GLX: Failed to make context current");
            return;
        }
    }
    else
    {
        if (!glXMakeCurrent(_lum.x11.display, None, NULL))
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "GLX: Failed to clear current context");
            return;
        }
    }

    _lumPlatformSetTls(&_lum.contextSlot, window);
}

static void swapBuffersGLX(_LUMwindow* window)
{
    glXSwapBuffers(_lum.x11.display, window->context.glx.window);
}

static void swapIntervalGLX(int interval)
{
    _LUMwindow* window = _lumPlatformGetTls(&_lum.contextSlot);

    if (_lum.glx.EXT_swap_control)
    {
        _lum.glx.SwapIntervalEXT(_lum.x11.display,
                                  window->context.glx.window,
                                  interval);
    }
    else if (_lum.glx.MESA_swap_control)
        _lum.glx.SwapIntervalMESA(interval);
    else if (_lum.glx.SGI_swap_control)
    {
        if (interval > 0)
            _lum.glx.SwapIntervalSGI(interval);
    }
}

static int extensionSupportedGLX(const char* extension)
{
    const char* extensions =
        glXQueryExtensionsString(_lum.x11.display, _lum.x11.screen);
    if (extensions)
    {
        if (_lumStringInExtensionString(extension, extensions))
            return LUM_TRUE;
    }

    return LUM_FALSE;
}

static LUMglproc getProcAddressGLX(const char* procname)
{
    if (_lum.glx.GetProcAddress)
        return _lum.glx.GetProcAddress((const GLubyte*) procname);
    else if (_lum.glx.GetProcAddressARB)
        return _lum.glx.GetProcAddressARB((const GLubyte*) procname);
    else
        return _lumPlatformGetModuleSymbol(_lum.glx.handle, procname);
}

static void destroyContextGLX(_LUMwindow* window)
{
    if (window->context.glx.window)
    {
        glXDestroyWindow(_lum.x11.display, window->context.glx.window);
        window->context.glx.window = None;
    }

    if (window->context.glx.handle)
    {
        glXDestroyContext(_lum.x11.display, window->context.glx.handle);
        window->context.glx.handle = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize GLX
//
LUMbool _lumInitGLX(void)
{
    const char* sonames[] =
    {
#if defined(_LUM_GLX_LIBRARY)
        _LUM_GLX_LIBRARY,
#elif defined(__CYGWIN__)
        "libGL-1.so",
#elif defined(__OpenBSD__)
        "libGL.so",
#else
        "libGL.so.1",
        "libGL.so",
#endif
        NULL
    };

    if (_lum.glx.handle)
        return LUM_TRUE;

    for (int i = 0;  sonames[i];  i++)
    {
        _lum.glx.handle = _lumPlatformLoadModule(sonames[i]);
        if (_lum.glx.handle)
            break;
    }

    if (!_lum.glx.handle)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "GLX: Failed to load GLX");
        return LUM_FALSE;
    }

    _lum.glx.GetFBConfigs = (PFNGLXGETFBCONFIGSPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetFBConfigs");
    _lum.glx.GetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetFBConfigAttrib");
    _lum.glx.GetClientString = (PFNGLXGETCLIENTSTRINGPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetClientString");
    _lum.glx.QueryExtension = (PFNGLXQUERYEXTENSIONPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXQueryExtension");
    _lum.glx.QueryVersion = (PFNGLXQUERYVERSIONPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXQueryVersion");
    _lum.glx.DestroyContext = (PFNGLXDESTROYCONTEXTPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXDestroyContext");
    _lum.glx.MakeCurrent = (PFNGLXMAKECURRENTPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXMakeCurrent");
    _lum.glx.SwapBuffers = (PFNGLXSWAPBUFFERSPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXSwapBuffers");
    _lum.glx.QueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXQueryExtensionsString");
    _lum.glx.CreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXCreateNewContext");
    _lum.glx.CreateWindow = (PFNGLXCREATEWINDOWPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXCreateWindow");
    _lum.glx.DestroyWindow = (PFNGLXDESTROYWINDOWPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXDestroyWindow");
    _lum.glx.GetProcAddress = (PFNGLXGETPROCADDRESSPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetProcAddress");
    _lum.glx.GetProcAddressARB = (PFNGLXGETPROCADDRESSPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetProcAddressARB");
    _lum.glx.GetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)
        _lumPlatformGetModuleSymbol(_lum.glx.handle, "glXGetVisualFromFBConfig");

    if (!_lum.glx.GetFBConfigs ||
        !_lum.glx.GetFBConfigAttrib ||
        !_lum.glx.GetClientString ||
        !_lum.glx.QueryExtension ||
        !_lum.glx.QueryVersion ||
        !_lum.glx.DestroyContext ||
        !_lum.glx.MakeCurrent ||
        !_lum.glx.SwapBuffers ||
        !_lum.glx.QueryExtensionsString ||
        !_lum.glx.CreateNewContext ||
        !_lum.glx.CreateWindow ||
        !_lum.glx.DestroyWindow ||
        !_lum.glx.GetProcAddress ||
        !_lum.glx.GetProcAddressARB ||
        !_lum.glx.GetVisualFromFBConfig)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "GLX: Failed to load required entry points");
        return LUM_FALSE;
    }

    if (!glXQueryExtension(_lum.x11.display,
                           &_lum.glx.errorBase,
                           &_lum.glx.eventBase))
    {
        _lumInputError(LUM_API_UNAVAILABLE, "GLX: GLX extension not found");
        return LUM_FALSE;
    }

    if (!glXQueryVersion(_lum.x11.display, &_lum.glx.major, &_lum.glx.minor))
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "GLX: Failed to query GLX version");
        return LUM_FALSE;
    }

    if (_lum.glx.major == 1 && _lum.glx.minor < 3)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "GLX: GLX version 1.3 is required");
        return LUM_FALSE;
    }

    if (extensionSupportedGLX("GLX_EXT_swap_control"))
    {
        _lum.glx.SwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
            getProcAddressGLX("glXSwapIntervalEXT");

        if (_lum.glx.SwapIntervalEXT)
            _lum.glx.EXT_swap_control = LUM_TRUE;
    }

    if (extensionSupportedGLX("GLX_SGI_swap_control"))
    {
        _lum.glx.SwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)
            getProcAddressGLX("glXSwapIntervalSGI");

        if (_lum.glx.SwapIntervalSGI)
            _lum.glx.SGI_swap_control = LUM_TRUE;
    }

    if (extensionSupportedGLX("GLX_MESA_swap_control"))
    {
        _lum.glx.SwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC)
            getProcAddressGLX("glXSwapIntervalMESA");

        if (_lum.glx.SwapIntervalMESA)
            _lum.glx.MESA_swap_control = LUM_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_multisample"))
        _lum.glx.ARB_multisample = LUM_TRUE;

    if (extensionSupportedGLX("GLX_ARB_framebuffer_sRGB"))
        _lum.glx.ARB_framebuffer_sRGB = LUM_TRUE;

    if (extensionSupportedGLX("GLX_EXT_framebuffer_sRGB"))
        _lum.glx.EXT_framebuffer_sRGB = LUM_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context"))
    {
        _lum.glx.CreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            getProcAddressGLX("glXCreateContextAttribsARB");

        if (_lum.glx.CreateContextAttribsARB)
            _lum.glx.ARB_create_context = LUM_TRUE;
    }

    if (extensionSupportedGLX("GLX_ARB_create_context_robustness"))
        _lum.glx.ARB_create_context_robustness = LUM_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_profile"))
        _lum.glx.ARB_create_context_profile = LUM_TRUE;

    if (extensionSupportedGLX("GLX_EXT_create_context_es2_profile"))
        _lum.glx.EXT_create_context_es2_profile = LUM_TRUE;

    if (extensionSupportedGLX("GLX_ARB_create_context_no_error"))
        _lum.glx.ARB_create_context_no_error = LUM_TRUE;

    if (extensionSupportedGLX("GLX_ARB_context_flush_control"))
        _lum.glx.ARB_context_flush_control = LUM_TRUE;

    return LUM_TRUE;
}

// Terminate GLX
//
void _lumTerminateGLX(void)
{
    // NOTE: This function must not call any X11 functions, as it is called
    //       after XCloseDisplay (see _lumTerminateX11 for details)

    if (_lum.glx.handle)
    {
        _lumPlatformFreeModule(_lum.glx.handle);
        _lum.glx.handle = NULL;
    }
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

// Create the OpenGL or OpenGL ES context
//
LUMbool _lumCreateContextGLX(_LUMwindow* window,
                               const _LUMctxconfig* ctxconfig,
                               const _LUMfbconfig* fbconfig)
{
    int attribs[40];
    GLXFBConfig native = NULL;
    GLXContext share = NULL;

    if (ctxconfig->share)
        share = ctxconfig->share->context.glx.handle;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return LUM_FALSE;
    }

    if (ctxconfig->client == LUM_OPENGL_ES_API)
    {
        if (!_lum.glx.ARB_create_context ||
            !_lum.glx.ARB_create_context_profile ||
            !_lum.glx.EXT_create_context_es2_profile)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "GLX: OpenGL ES requested but GLX_EXT_create_context_es2_profile is unavailable");
            return LUM_FALSE;
        }
    }

    if (ctxconfig->forward)
    {
        if (!_lum.glx.ARB_create_context)
        {
            _lumInputError(LUM_VERSION_UNAVAILABLE,
                            "GLX: Forward compatibility requested but GLX_ARB_create_context_profile is unavailable");
            return LUM_FALSE;
        }
    }

    if (ctxconfig->profile)
    {
        if (!_lum.glx.ARB_create_context ||
            !_lum.glx.ARB_create_context_profile)
        {
            _lumInputError(LUM_VERSION_UNAVAILABLE,
                            "GLX: An OpenGL profile requested but GLX_ARB_create_context_profile is unavailable");
            return LUM_FALSE;
        }
    }

    _lumGrabErrorHandlerX11();

    if (_lum.glx.ARB_create_context)
    {
        int index = 0, mask = 0, flags = 0;

        if (ctxconfig->client == LUM_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

            if (ctxconfig->profile == LUM_OPENGL_CORE_PROFILE)
                mask |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
            else if (ctxconfig->profile == LUM_OPENGL_COMPAT_PROFILE)
                mask |= GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
        }
        else
            mask |= GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

        if (ctxconfig->debug)
            flags |= GLX_CONTEXT_DEBUG_BIT_ARB;

        if (ctxconfig->robustness)
        {
            if (_lum.glx.ARB_create_context_robustness)
            {
                if (ctxconfig->robustness == LUM_NO_RESET_NOTIFICATION)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_NO_RESET_NOTIFICATION_ARB);
                }
                else if (ctxconfig->robustness == LUM_LOSE_CONTEXT_ON_RESET)
                {
                    setAttrib(GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB,
                              GLX_LOSE_CONTEXT_ON_RESET_ARB);
                }

                flags |= GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
            }
        }

        if (ctxconfig->release)
        {
            if (_lum.glx.ARB_context_flush_control)
            {
                if (ctxconfig->release == LUM_RELEASE_BEHAVIOR_NONE)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB);
                }
                else if (ctxconfig->release == LUM_RELEASE_BEHAVIOR_FLUSH)
                {
                    setAttrib(GLX_CONTEXT_RELEASE_BEHAVIOR_ARB,
                              GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB);
                }
            }
        }

        if (ctxconfig->noerror)
        {
            if (_lum.glx.ARB_create_context_no_error)
                setAttrib(GLX_CONTEXT_OPENGL_NO_ERROR_ARB, LUM_TRUE);
        }

        // NOTE: Only request an explicitly versioned context when necessary, as
        //       explicitly requesting version 1.0 does not always return the
        //       highest version supported by the driver
        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(GLX_CONTEXT_MAJOR_VERSION_ARB, ctxconfig->major);
            setAttrib(GLX_CONTEXT_MINOR_VERSION_ARB, ctxconfig->minor);
        }

        if (mask)
            setAttrib(GLX_CONTEXT_PROFILE_MASK_ARB, mask);

        if (flags)
            setAttrib(GLX_CONTEXT_FLAGS_ARB, flags);

        setAttrib(None, None);

        window->context.glx.handle =
            _lum.glx.CreateContextAttribsARB(_lum.x11.display,
                                              native,
                                              share,
                                              True,
                                              attribs);

        // HACK: This is a fallback for broken versions of the Mesa
        //       implementation of GLX_ARB_create_context_profile that fail
        //       default 1.0 context creation with a GLXBadProfileARB error in
        //       violation of the extension spec
        if (!window->context.glx.handle)
        {
            if (_lum.x11.errorCode == _lum.glx.errorBase + GLXBadProfileARB &&
                ctxconfig->client == LUM_OPENGL_API &&
                ctxconfig->profile == LUM_OPENGL_ANY_PROFILE &&
                ctxconfig->forward == LUM_FALSE)
            {
                window->context.glx.handle =
                    createLegacyContextGLX(window, native, share);
            }
        }
    }
    else
    {
        window->context.glx.handle =
            createLegacyContextGLX(window, native, share);
    }

    _lumReleaseErrorHandlerX11();

    if (!window->context.glx.handle)
    {
        _lumInputErrorX11(LUM_VERSION_UNAVAILABLE, "GLX: Failed to create context");
        return LUM_FALSE;
    }

    window->context.glx.window =
        glXCreateWindow(_lum.x11.display, native, window->x11.handle, NULL);
    if (!window->context.glx.window)
    {
        _lumInputError(LUM_PLATFORM_ERROR, "GLX: Failed to create window");
        return LUM_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentGLX;
    window->context.swapBuffers = swapBuffersGLX;
    window->context.swapInterval = swapIntervalGLX;
    window->context.extensionSupported = extensionSupportedGLX;
    window->context.getProcAddress = getProcAddressGLX;
    window->context.destroy = destroyContextGLX;

    return LUM_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen GLXFBConfig
//
LUMbool _lumChooseVisualGLX(const _LUMwndconfig* wndconfig,
                              const _LUMctxconfig* ctxconfig,
                              const _LUMfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    GLXFBConfig native;
    XVisualInfo* result;

    if (!chooseGLXFBConfig(fbconfig, &native))
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "GLX: Failed to find a suitable GLXFBConfig");
        return LUM_FALSE;
    }

    result = glXGetVisualFromFBConfig(_lum.x11.display, native);
    if (!result)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "GLX: Failed to retrieve Visual for GLXFBConfig");
        return LUM_FALSE;
    }

    *visual = result->visual;
    *depth  = result->depth;

    XFree(result);
    return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI GLXContext lumGetGLXContext(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized");
        return NULL;
    }

    if (window->context.source != LUM_NATIVE_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.glx.handle;
}

LUMAPI GLXWindow lumGetGLXWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(None);

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "GLX: Platform not initialized");
        return None;
    }

    if (window->context.source != LUM_NATIVE_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return None;
    }

    return window->context.glx.window;
}

