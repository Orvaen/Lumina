//========================================================================
// Lumina EGL - fork of GLFW 3.4 (www.glfw.org)
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


// Return a description of the specified EGL error
//
static const char* getEGLErrorString(EGLint error)
{
    switch (error)
    {
        case EGL_SUCCESS:
            return "Success";
        case EGL_NOT_INITIALIZED:
            return "EGL is not or could not be initialized";
        case EGL_BAD_ACCESS:
            return "EGL cannot access a requested resource";
        case EGL_BAD_ALLOC:
            return "EGL failed to allocate resources for the requested operation";
        case EGL_BAD_ATTRIBUTE:
            return "An unrecognized attribute or attribute value was passed in the attribute list";
        case EGL_BAD_CONTEXT:
            return "An EGLContext argument does not name a valid EGL rendering context";
        case EGL_BAD_CONFIG:
            return "An EGLConfig argument does not name a valid EGL frame buffer configuration";
        case EGL_BAD_CURRENT_SURFACE:
            return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid";
        case EGL_BAD_DISPLAY:
            return "An EGLDisplay argument does not name a valid EGL display connection";
        case EGL_BAD_SURFACE:
            return "An EGLSurface argument does not name a valid surface configured for GL rendering";
        case EGL_BAD_MATCH:
            return "Arguments are inconsistent";
        case EGL_BAD_PARAMETER:
            return "One or more argument values are invalid";
        case EGL_BAD_NATIVE_PIXMAP:
            return "A NativePixmapType argument does not refer to a valid native pixmap";
        case EGL_BAD_NATIVE_WINDOW:
            return "A NativeWindowType argument does not refer to a valid native window";
        case EGL_CONTEXT_LOST:
            return "The application must destroy all contexts and reinitialise";
        default:
            return "ERROR: UNKNOWN EGL ERROR";
    }
}

// Returns the specified attribute of the specified EGLConfig
//
static int getEGLConfigAttrib(EGLConfig config, int attrib)
{
    int value;
    eglGetConfigAttrib(_lum.egl.display, config, attrib, &value);
    return value;
}

// Return the EGLConfig most closely matching the specified hints
//
static LUMbool chooseEGLConfig(const _LUMctxconfig* ctxconfig,
                                const _LUMfbconfig* desired,
                                EGLConfig* result)
{
    EGLConfig* nativeConfigs;
    _LUMfbconfig* usableConfigs;
    const _LUMfbconfig* closest;
    int i, nativeCount, usableCount;

    eglGetConfigs(_lum.egl.display, NULL, 0, &nativeCount);
    if (!nativeCount)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "EGL: No EGLConfigs returned");
        return LUM_FALSE;
    }

    nativeConfigs = _lum_calloc(nativeCount, sizeof(EGLConfig));
    eglGetConfigs(_lum.egl.display, nativeConfigs, nativeCount, &nativeCount);

    usableConfigs = _lum_calloc(nativeCount, sizeof(_LUMfbconfig));
    usableCount = 0;

    for (i = 0;  i < nativeCount;  i++)
    {
        const EGLConfig n = nativeConfigs[i];
        _LUMfbconfig* u = usableConfigs + usableCount;

        // Only consider RGB(A) EGLConfigs
        if (getEGLConfigAttrib(n, EGL_COLOR_BUFFER_TYPE) != EGL_RGB_BUFFER)
            continue;

        // Only consider window EGLConfigs
        if (!(getEGLConfigAttrib(n, EGL_SURFACE_TYPE) & EGL_WINDOW_BIT))
            continue;

#if defined(_LUM_X11)
        if (_lum.platform.platformID == LUM_PLATFORM_X11)
        {
            XVisualInfo vi = {0};

            // Only consider EGLConfigs with associated Visuals
            vi.visualid = getEGLConfigAttrib(n, EGL_NATIVE_VISUAL_ID);
            if (!vi.visualid)
                continue;

            if (desired->transparent)
            {
                int count;
                XVisualInfo* vis =
                    XGetVisualInfo(_lum.x11.display, VisualIDMask, &vi, &count);
                if (vis)
                {
                    u->transparent = _lumIsVisualTransparentX11(vis[0].visual);
                    XFree(vis);
                }
            }
        }
#endif // _LUM_X11

        if (ctxconfig->client == LUM_OPENGL_ES_API)
        {
            if (ctxconfig->major == 1)
            {
                if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES_BIT))
                    continue;
            }
            else
            {
                if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_ES2_BIT))
                    continue;
            }
        }
        else if (ctxconfig->client == LUM_OPENGL_API)
        {
            if (!(getEGLConfigAttrib(n, EGL_RENDERABLE_TYPE) & EGL_OPENGL_BIT))
                continue;
        }

        u->redBits = getEGLConfigAttrib(n, EGL_RED_SIZE);
        u->greenBits = getEGLConfigAttrib(n, EGL_GREEN_SIZE);
        u->blueBits = getEGLConfigAttrib(n, EGL_BLUE_SIZE);

        u->alphaBits = getEGLConfigAttrib(n, EGL_ALPHA_SIZE);
        u->depthBits = getEGLConfigAttrib(n, EGL_DEPTH_SIZE);
        u->stencilBits = getEGLConfigAttrib(n, EGL_STENCIL_SIZE);

        u->samples = getEGLConfigAttrib(n, EGL_SAMPLES);
        u->doublebuffer = desired->doublebuffer;

        u->handle = (uintptr_t) n;
        usableCount++;
    }

    closest = _lumChooseFBConfig(desired, usableConfigs, usableCount);
    if (closest)
        *result = (EGLConfig) closest->handle;

    _lum_free(nativeConfigs);
    _lum_free(usableConfigs);

    return closest != NULL;
}

static void makeContextCurrentEGL(_LUMwindow* window)
{
    if (window)
    {
        if (!eglMakeCurrent(_lum.egl.display,
                            window->context.egl.surface,
                            window->context.egl.surface,
                            window->context.egl.handle))
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "EGL: Failed to make context current: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }
    else
    {
        if (!eglMakeCurrent(_lum.egl.display,
                            EGL_NO_SURFACE,
                            EGL_NO_SURFACE,
                            EGL_NO_CONTEXT))
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "EGL: Failed to clear current context: %s",
                            getEGLErrorString(eglGetError()));
            return;
        }
    }

    _lumPlatformSetTls(&_lum.contextSlot, window);
}

static void swapBuffersEGL(_LUMwindow* window)
{
    if (window != _lumPlatformGetTls(&_lum.contextSlot))
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "EGL: The context must be current on the calling thread when swapping buffers");
        return;
    }

#if defined(_LUM_WAYLAND)
    if (_lum.platform.platformID == LUM_PLATFORM_WAYLAND)
    {
        // NOTE: Swapping buffers on a hidden window on Wayland makes it visible
        if (!window->wl.visible)
            return;
    }
#endif

    eglSwapBuffers(_lum.egl.display, window->context.egl.surface);
}

static void swapIntervalEGL(int interval)
{
    eglSwapInterval(_lum.egl.display, interval);
}

static int extensionSupportedEGL(const char* extension)
{
    const char* extensions = eglQueryString(_lum.egl.display, EGL_EXTENSIONS);
    if (extensions)
    {
        if (_lumStringInExtensionString(extension, extensions))
            return LUM_TRUE;
    }

    return LUM_FALSE;
}

static LUMglproc getProcAddressEGL(const char* procname)
{
    _LUMwindow* window = _lumPlatformGetTls(&_lum.contextSlot);

    if (window->context.egl.client)
    {
        LUMglproc proc = (LUMglproc)
            _lumPlatformGetModuleSymbol(window->context.egl.client, procname);
        if (proc)
            return proc;
    }

    return eglGetProcAddress(procname);
}

static void destroyContextEGL(_LUMwindow* window)
{
    // NOTE: Do not unload libGL.so.1 while the X11 display is still open,
    //       as it will make XCloseDisplay segfault
    if (_lum.platform.platformID != LUM_PLATFORM_X11 ||
        window->context.client != LUM_OPENGL_API)
    {
        if (window->context.egl.client)
        {
            _lumPlatformFreeModule(window->context.egl.client);
            window->context.egl.client = NULL;
        }
    }

    if (window->context.egl.surface)
    {
        eglDestroySurface(_lum.egl.display, window->context.egl.surface);
        window->context.egl.surface = EGL_NO_SURFACE;
    }

    if (window->context.egl.handle)
    {
        eglDestroyContext(_lum.egl.display, window->context.egl.handle);
        window->context.egl.handle = EGL_NO_CONTEXT;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize EGL
//
LUMbool _lumInitEGL(void)
{
    int i;
    EGLint* attribs = NULL;
    const char* extensions;
    const char* sonames[] =
    {
#if defined(_LUM_EGL_LIBRARY)
        _LUM_EGL_LIBRARY,
#elif defined(_LUM_WIN32)
        "libEGL.dll",
        "EGL.dll",
#elif defined(_LUM_COCOA)
        "libEGL.dylib",
#elif defined(__CYGWIN__)
        "libEGL-1.so",
#elif defined(__OpenBSD__)
        "libEGL.so",
#else
        "libEGL.so.1",
#endif
        NULL
    };

    if (_lum.egl.handle)
        return LUM_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _lum.egl.handle = _lumPlatformLoadModule(sonames[i]);
        if (_lum.egl.handle)
            break;
    }

    if (!_lum.egl.handle)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "EGL: Library not found");
        return LUM_FALSE;
    }

    _lum.egl.prefix = (strncmp(sonames[i], "lib", 3) == 0);

    _lum.egl.GetConfigAttrib = (PFN_eglGetConfigAttrib)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglGetConfigAttrib");
    _lum.egl.GetConfigs = (PFN_eglGetConfigs)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglGetConfigs");
    _lum.egl.GetDisplay = (PFN_eglGetDisplay)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglGetDisplay");
    _lum.egl.GetError = (PFN_eglGetError)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglGetError");
    _lum.egl.Initialize = (PFN_eglInitialize)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglInitialize");
    _lum.egl.Terminate = (PFN_eglTerminate)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglTerminate");
    _lum.egl.BindAPI = (PFN_eglBindAPI)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglBindAPI");
    _lum.egl.CreateContext = (PFN_eglCreateContext)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglCreateContext");
    _lum.egl.DestroySurface = (PFN_eglDestroySurface)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglDestroySurface");
    _lum.egl.DestroyContext = (PFN_eglDestroyContext)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglDestroyContext");
    _lum.egl.CreateWindowSurface = (PFN_eglCreateWindowSurface)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglCreateWindowSurface");
    _lum.egl.MakeCurrent = (PFN_eglMakeCurrent)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglMakeCurrent");
    _lum.egl.SwapBuffers = (PFN_eglSwapBuffers)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglSwapBuffers");
    _lum.egl.SwapInterval = (PFN_eglSwapInterval)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglSwapInterval");
    _lum.egl.QueryString = (PFN_eglQueryString)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglQueryString");
    _lum.egl.GetProcAddress = (PFN_eglGetProcAddress)
        _lumPlatformGetModuleSymbol(_lum.egl.handle, "eglGetProcAddress");

    if (!_lum.egl.GetConfigAttrib ||
        !_lum.egl.GetConfigs ||
        !_lum.egl.GetDisplay ||
        !_lum.egl.GetError ||
        !_lum.egl.Initialize ||
        !_lum.egl.Terminate ||
        !_lum.egl.BindAPI ||
        !_lum.egl.CreateContext ||
        !_lum.egl.DestroySurface ||
        !_lum.egl.DestroyContext ||
        !_lum.egl.CreateWindowSurface ||
        !_lum.egl.MakeCurrent ||
        !_lum.egl.SwapBuffers ||
        !_lum.egl.SwapInterval ||
        !_lum.egl.QueryString ||
        !_lum.egl.GetProcAddress)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "EGL: Failed to load required entry points");

        _lumTerminateEGL();
        return LUM_FALSE;
    }

    extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (extensions && eglGetError() == EGL_SUCCESS)
        _lum.egl.EXT_client_extensions = LUM_TRUE;

    if (_lum.egl.EXT_client_extensions)
    {
        _lum.egl.EXT_platform_base =
            _lumStringInExtensionString("EGL_EXT_platform_base", extensions);
        _lum.egl.EXT_platform_x11 =
            _lumStringInExtensionString("EGL_EXT_platform_x11", extensions);
        _lum.egl.EXT_platform_wayland =
            _lumStringInExtensionString("EGL_EXT_platform_wayland", extensions);
        _lum.egl.ANGLE_platform_angle =
            _lumStringInExtensionString("EGL_ANGLE_platform_angle", extensions);
        _lum.egl.ANGLE_platform_angle_opengl =
            _lumStringInExtensionString("EGL_ANGLE_platform_angle_opengl", extensions);
        _lum.egl.ANGLE_platform_angle_d3d =
            _lumStringInExtensionString("EGL_ANGLE_platform_angle_d3d", extensions);
        _lum.egl.ANGLE_platform_angle_vulkan =
            _lumStringInExtensionString("EGL_ANGLE_platform_angle_vulkan", extensions);
        _lum.egl.ANGLE_platform_angle_metal =
            _lumStringInExtensionString("EGL_ANGLE_platform_angle_metal", extensions);
    }

    if (_lum.egl.EXT_platform_base)
    {
        _lum.egl.GetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
            eglGetProcAddress("eglGetPlatformDisplayEXT");
        _lum.egl.CreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)
            eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
    }

    _lum.egl.platform = _lum.platform.getEGLPlatform(&attribs);
    if (_lum.egl.platform)
    {
        _lum.egl.display =
            eglGetPlatformDisplayEXT(_lum.egl.platform,
                                     _lum.platform.getEGLNativeDisplay(),
                                     attribs);
    }
    else
        _lum.egl.display = eglGetDisplay(_lum.platform.getEGLNativeDisplay());

    _lum_free(attribs);

    if (_lum.egl.display == EGL_NO_DISPLAY)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "EGL: Failed to get EGL display: %s",
                        getEGLErrorString(eglGetError()));

        _lumTerminateEGL();
        return LUM_FALSE;
    }

    if (!eglInitialize(_lum.egl.display, &_lum.egl.major, &_lum.egl.minor))
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "EGL: Failed to initialize EGL: %s",
                        getEGLErrorString(eglGetError()));

        _lumTerminateEGL();
        return LUM_FALSE;
    }

    _lum.egl.KHR_create_context =
        extensionSupportedEGL("EGL_KHR_create_context");
    _lum.egl.KHR_create_context_no_error =
        extensionSupportedEGL("EGL_KHR_create_context_no_error");
    _lum.egl.KHR_gl_colorspace =
        extensionSupportedEGL("EGL_KHR_gl_colorspace");
    _lum.egl.KHR_get_all_proc_addresses =
        extensionSupportedEGL("EGL_KHR_get_all_proc_addresses");
    _lum.egl.KHR_context_flush_control =
        extensionSupportedEGL("EGL_KHR_context_flush_control");
    _lum.egl.EXT_present_opaque =
        extensionSupportedEGL("EGL_EXT_present_opaque");

    return LUM_TRUE;
}

// Terminate EGL
//
void _lumTerminateEGL(void)
{
    if (_lum.egl.display)
    {
        eglTerminate(_lum.egl.display);
        _lum.egl.display = EGL_NO_DISPLAY;
    }

    if (_lum.egl.handle)
    {
        _lumPlatformFreeModule(_lum.egl.handle);
        _lum.egl.handle = NULL;
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
LUMbool _lumCreateContextEGL(_LUMwindow* window,
                               const _LUMctxconfig* ctxconfig,
                               const _LUMfbconfig* fbconfig)
{
    EGLint attribs[40];
    EGLConfig config;
    EGLContext share = NULL;
    EGLNativeWindowType native;
    int index = 0;

    if (!_lum.egl.display)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "EGL: API not available");
        return LUM_FALSE;
    }

    if (ctxconfig->share)
        share = ctxconfig->share->context.egl.handle;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &config))
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return LUM_FALSE;
    }

    if (ctxconfig->client == LUM_OPENGL_ES_API)
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL ES: %s",
                            getEGLErrorString(eglGetError()));
            return LUM_FALSE;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "EGL: Failed to bind OpenGL: %s",
                            getEGLErrorString(eglGetError()));
            return LUM_FALSE;
        }
    }

    if (_lum.egl.KHR_create_context)
    {
        int mask = 0, flags = 0;

        if (ctxconfig->client == LUM_OPENGL_API)
        {
            if (ctxconfig->forward)
                flags |= EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR;

            if (ctxconfig->profile == LUM_OPENGL_CORE_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR;
            else if (ctxconfig->profile == LUM_OPENGL_COMPAT_PROFILE)
                mask |= EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR;
        }

        if (ctxconfig->debug)
            flags |= EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;

        if (ctxconfig->robustness)
        {
            if (ctxconfig->robustness == LUM_NO_RESET_NOTIFICATION)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_NO_RESET_NOTIFICATION_KHR);
            }
            else if (ctxconfig->robustness == LUM_LOSE_CONTEXT_ON_RESET)
            {
                setAttrib(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR,
                          EGL_LOSE_CONTEXT_ON_RESET_KHR);
            }

            flags |= EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
        }

        if (ctxconfig->noerror)
        {
            if (_lum.egl.KHR_create_context_no_error)
                setAttrib(EGL_CONTEXT_OPENGL_NO_ERROR_KHR, LUM_TRUE);
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(EGL_CONTEXT_MAJOR_VERSION_KHR, ctxconfig->major);
            setAttrib(EGL_CONTEXT_MINOR_VERSION_KHR, ctxconfig->minor);
        }

        if (mask)
            setAttrib(EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, mask);

        if (flags)
            setAttrib(EGL_CONTEXT_FLAGS_KHR, flags);
    }
    else
    {
        if (ctxconfig->client == LUM_OPENGL_ES_API)
            setAttrib(EGL_CONTEXT_CLIENT_VERSION, ctxconfig->major);
    }

    if (_lum.egl.KHR_context_flush_control)
    {
        if (ctxconfig->release == LUM_RELEASE_BEHAVIOR_NONE)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR);
        }
        else if (ctxconfig->release == LUM_RELEASE_BEHAVIOR_FLUSH)
        {
            setAttrib(EGL_CONTEXT_RELEASE_BEHAVIOR_KHR,
                      EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR);
        }
    }

    setAttrib(EGL_NONE, EGL_NONE);

    window->context.egl.handle = eglCreateContext(_lum.egl.display,
                                                  config, share, attribs);

    if (window->context.egl.handle == EGL_NO_CONTEXT)
    {
        _lumInputError(LUM_VERSION_UNAVAILABLE,
                        "EGL: Failed to create context: %s",
                        getEGLErrorString(eglGetError()));
        return LUM_FALSE;
    }

    // Set up attributes for surface creation
    index = 0;

    if (fbconfig->sRGB)
    {
        if (_lum.egl.KHR_gl_colorspace)
            setAttrib(EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR);
    }

    if (!fbconfig->doublebuffer)
        setAttrib(EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER);

    if (_lum.egl.EXT_present_opaque)
        setAttrib(EGL_PRESENT_OPAQUE_EXT, !fbconfig->transparent);

    setAttrib(EGL_NONE, EGL_NONE);

    native = _lum.platform.getEGLNativeWindow(window);
    // HACK: ANGLE does not implement eglCreatePlatformWindowSurfaceEXT
    //       despite reporting EGL_EXT_platform_base
    if (_lum.egl.platform && _lum.egl.platform != EGL_PLATFORM_ANGLE_ANGLE)
    {
        window->context.egl.surface =
            eglCreatePlatformWindowSurfaceEXT(_lum.egl.display, config, native, attribs);
    }
    else
    {
        window->context.egl.surface =
            eglCreateWindowSurface(_lum.egl.display, config, native, attribs);
    }

    if (window->context.egl.surface == EGL_NO_SURFACE)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "EGL: Failed to create window surface: %s",
                        getEGLErrorString(eglGetError()));
        return LUM_FALSE;
    }

    window->context.egl.config = config;

    // Load the appropriate client library
    if (!_lum.egl.KHR_get_all_proc_addresses)
    {
        int i;
        const char** sonames;
        const char* es1sonames[] =
        {
#if defined(_LUM_GLESV1_LIBRARY)
            _LUM_GLESV1_LIBRARY,
#elif defined(_LUM_WIN32)
            "GLESv1_CM.dll",
            "libGLES_CM.dll",
#elif defined(_LUM_COCOA)
            "libGLESv1_CM.dylib",
#elif defined(__OpenBSD__)
            "libGLESv1_CM.so",
#else
            "libGLESv1_CM.so.1",
            "libGLES_CM.so.1",
#endif
            NULL
        };
        const char* es2sonames[] =
        {
#if defined(_LUM_GLESV2_LIBRARY)
            _LUM_GLESV2_LIBRARY,
#elif defined(_LUM_WIN32)
            "GLESv2.dll",
            "libGLESv2.dll",
#elif defined(_LUM_COCOA)
            "libGLESv2.dylib",
#elif defined(__CYGWIN__)
            "libGLESv2-2.so",
#elif defined(__OpenBSD__)
            "libGLESv2.so",
#else
            "libGLESv2.so.2",
#endif
            NULL
        };
        const char* glsonames[] =
        {
#if defined(_LUM_OPENGL_LIBRARY)
            _LUM_OPENGL_LIBRARY,
#elif defined(_LUM_WIN32)
#elif defined(_LUM_COCOA)
#elif defined(__OpenBSD__)
            "libGL.so",
#else
            "libGL.so.1",
#endif
            NULL
        };

        if (ctxconfig->client == LUM_OPENGL_ES_API)
        {
            if (ctxconfig->major == 1)
                sonames = es1sonames;
            else
                sonames = es2sonames;
        }
        else
            sonames = glsonames;

        for (i = 0;  sonames[i];  i++)
        {
            // HACK: Match presence of lib prefix to increase chance of finding
            //       a matching pair in the jungle that is Win32 EGL/GLES
            if (_lum.egl.prefix != (strncmp(sonames[i], "lib", 3) == 0))
                continue;

            window->context.egl.client = _lumPlatformLoadModule(sonames[i]);
            if (window->context.egl.client)
                break;
        }

        if (!window->context.egl.client)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "EGL: Failed to load client library");
            return LUM_FALSE;
        }
    }

    window->context.makeCurrent = makeContextCurrentEGL;
    window->context.swapBuffers = swapBuffersEGL;
    window->context.swapInterval = swapIntervalEGL;
    window->context.extensionSupported = extensionSupportedEGL;
    window->context.getProcAddress = getProcAddressEGL;
    window->context.destroy = destroyContextEGL;

    return LUM_TRUE;
}

#undef setAttrib

// Returns the Visual and depth of the chosen EGLConfig
//
#if defined(_LUM_X11)
LUMbool _lumChooseVisualEGL(const _LUMwndconfig* wndconfig,
                              const _LUMctxconfig* ctxconfig,
                              const _LUMfbconfig* fbconfig,
                              Visual** visual, int* depth)
{
    XVisualInfo* result;
    XVisualInfo desired;
    EGLConfig native;
    EGLint visualID = 0, count = 0;
    const long vimask = VisualScreenMask | VisualIDMask;

    if (!chooseEGLConfig(ctxconfig, fbconfig, &native))
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "EGL: Failed to find a suitable EGLConfig");
        return LUM_FALSE;
    }

    eglGetConfigAttrib(_lum.egl.display, native,
                       EGL_NATIVE_VISUAL_ID, &visualID);

    desired.screen = _lum.x11.screen;
    desired.visualid = visualID;

    result = XGetVisualInfo(_lum.x11.display, vimask, &desired, &count);
    if (!result)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "EGL: Failed to retrieve Visual for EGLConfig");
        return LUM_FALSE;
    }

    *visual = result->visual;
    *depth = result->depth;

    XFree(result);
    return LUM_TRUE;
}
#endif // _LUM_X11


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI EGLDisplay lumGetEGLDisplay(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(EGL_NO_DISPLAY);
    return _lum.egl.display;
}

LUMAPI EGLContext lumGetEGLContext(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(EGL_NO_CONTEXT);

    if (window->context.source != LUM_EGL_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_CONTEXT;
    }

    return window->context.egl.handle;
}

LUMAPI EGLSurface lumGetEGLSurface(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(EGL_NO_SURFACE);

    if (window->context.source != LUM_EGL_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return EGL_NO_SURFACE;
    }

    return window->context.egl.surface;
}

