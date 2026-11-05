//========================================================================
// Lumina OSMesa - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2017 Camilla Löwy <elmindreda@glfw.org>
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "internal.h"


static void makeContextCurrentOSMesa(_LUMwindow* window)
{
    if (window)
    {
        int width, height;
        _lum.platform.getFramebufferSize(window, &width, &height);

        // Check to see if we need to allocate a new buffer
        if ((window->context.osmesa.buffer == NULL) ||
            (width != window->context.osmesa.width) ||
            (height != window->context.osmesa.height))
        {
            _lum_free(window->context.osmesa.buffer);

            // Allocate the new buffer (width * height * 8-bit RGBA)
            window->context.osmesa.buffer = _lum_calloc(4, (size_t) width * height);
            window->context.osmesa.width  = width;
            window->context.osmesa.height = height;
        }

        if (!OSMesaMakeCurrent(window->context.osmesa.handle,
                               window->context.osmesa.buffer,
                               GL_UNSIGNED_BYTE,
                               width, height))
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "OSMesa: Failed to make context current");
            return;
        }
    }

    _lumPlatformSetTls(&_lum.contextSlot, window);
}

static LUMglproc getProcAddressOSMesa(const char* procname)
{
    return (LUMglproc) OSMesaGetProcAddress(procname);
}

static void destroyContextOSMesa(_LUMwindow* window)
{
    if (window->context.osmesa.handle)
    {
        OSMesaDestroyContext(window->context.osmesa.handle);
        window->context.osmesa.handle = NULL;
    }

    if (window->context.osmesa.buffer)
    {
        _lum_free(window->context.osmesa.buffer);
        window->context.osmesa.width = 0;
        window->context.osmesa.height = 0;
    }
}

static void swapBuffersOSMesa(_LUMwindow* window)
{
    // No double buffering on OSMesa
}

static void swapIntervalOSMesa(int interval)
{
    // No swap interval on OSMesa
}

static int extensionSupportedOSMesa(const char* extension)
{
    // OSMesa does not have extensions
    return LUM_FALSE;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumInitOSMesa(void)
{
    int i;
    const char* sonames[] =
    {
#if defined(_LUM_OSMESA_LIBRARY)
        _LUM_OSMESA_LIBRARY,
#elif defined(_WIN32)
        "libOSMesa.dll",
        "OSMesa.dll",
#elif defined(__APPLE__)
        "libOSMesa.8.dylib",
#elif defined(__CYGWIN__)
        "libOSMesa-8.so",
#elif defined(__OpenBSD__)
        "libOSMesa.so",
#else
        "libOSMesa.so.8",
        "libOSMesa.so.6",
#endif
        NULL
    };

    if (_lum.osmesa.handle)
        return LUM_TRUE;

    for (i = 0;  sonames[i];  i++)
    {
        _lum.osmesa.handle = _lumPlatformLoadModule(sonames[i]);
        if (_lum.osmesa.handle)
            break;
    }

    if (!_lum.osmesa.handle)
    {
        _lumInputError(LUM_API_UNAVAILABLE, "OSMesa: Library not found");
        return LUM_FALSE;
    }

    _lum.osmesa.CreateContextExt = (PFN_OSMesaCreateContextExt)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaCreateContextExt");
    _lum.osmesa.CreateContextAttribs = (PFN_OSMesaCreateContextAttribs)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaCreateContextAttribs");
    _lum.osmesa.DestroyContext = (PFN_OSMesaDestroyContext)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaDestroyContext");
    _lum.osmesa.MakeCurrent = (PFN_OSMesaMakeCurrent)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaMakeCurrent");
    _lum.osmesa.GetColorBuffer = (PFN_OSMesaGetColorBuffer)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaGetColorBuffer");
    _lum.osmesa.GetDepthBuffer = (PFN_OSMesaGetDepthBuffer)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaGetDepthBuffer");
    _lum.osmesa.GetProcAddress = (PFN_OSMesaGetProcAddress)
        _lumPlatformGetModuleSymbol(_lum.osmesa.handle, "OSMesaGetProcAddress");

    if (!_lum.osmesa.CreateContextExt ||
        !_lum.osmesa.DestroyContext ||
        !_lum.osmesa.MakeCurrent ||
        !_lum.osmesa.GetColorBuffer ||
        !_lum.osmesa.GetDepthBuffer ||
        !_lum.osmesa.GetProcAddress)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "OSMesa: Failed to load required entry points");

        _lumTerminateOSMesa();
        return LUM_FALSE;
    }

    return LUM_TRUE;
}

void _lumTerminateOSMesa(void)
{
    if (_lum.osmesa.handle)
    {
        _lumPlatformFreeModule(_lum.osmesa.handle);
        _lum.osmesa.handle = NULL;
    }
}

#define setAttrib(a, v) \
{ \
    assert(((size_t) index + 1) < sizeof(attribs) / sizeof(attribs[0])); \
    attribs[index++] = a; \
    attribs[index++] = v; \
}

LUMbool _lumCreateContextOSMesa(_LUMwindow* window,
                                  const _LUMctxconfig* ctxconfig,
                                  const _LUMfbconfig* fbconfig)
{
    OSMesaContext share = NULL;
    const int accumBits = fbconfig->accumRedBits +
                          fbconfig->accumGreenBits +
                          fbconfig->accumBlueBits +
                          fbconfig->accumAlphaBits;

    if (ctxconfig->client == LUM_OPENGL_ES_API)
    {
        _lumInputError(LUM_API_UNAVAILABLE,
                        "OSMesa: OpenGL ES is not available on OSMesa");
        return LUM_FALSE;
    }

    if (ctxconfig->share)
        share = ctxconfig->share->context.osmesa.handle;

    if (OSMesaCreateContextAttribs)
    {
        int index = 0, attribs[40];

        setAttrib(OSMESA_FORMAT, OSMESA_RGBA);
        setAttrib(OSMESA_DEPTH_BITS, fbconfig->depthBits);
        setAttrib(OSMESA_STENCIL_BITS, fbconfig->stencilBits);
        setAttrib(OSMESA_ACCUM_BITS, accumBits);

        if (ctxconfig->profile == LUM_OPENGL_CORE_PROFILE)
        {
            setAttrib(OSMESA_PROFILE, OSMESA_CORE_PROFILE);
        }
        else if (ctxconfig->profile == LUM_OPENGL_COMPAT_PROFILE)
        {
            setAttrib(OSMESA_PROFILE, OSMESA_COMPAT_PROFILE);
        }

        if (ctxconfig->major != 1 || ctxconfig->minor != 0)
        {
            setAttrib(OSMESA_CONTEXT_MAJOR_VERSION, ctxconfig->major);
            setAttrib(OSMESA_CONTEXT_MINOR_VERSION, ctxconfig->minor);
        }

        if (ctxconfig->forward)
        {
            _lumInputError(LUM_VERSION_UNAVAILABLE,
                            "OSMesa: Forward-compatible contexts not supported");
            return LUM_FALSE;
        }

        setAttrib(0, 0);

        window->context.osmesa.handle =
            OSMesaCreateContextAttribs(attribs, share);
    }
    else
    {
        if (ctxconfig->profile)
        {
            _lumInputError(LUM_VERSION_UNAVAILABLE,
                            "OSMesa: OpenGL profiles unavailable");
            return LUM_FALSE;
        }

        window->context.osmesa.handle =
            OSMesaCreateContextExt(OSMESA_RGBA,
                                   fbconfig->depthBits,
                                   fbconfig->stencilBits,
                                   accumBits,
                                   share);
    }

    if (window->context.osmesa.handle == NULL)
    {
        _lumInputError(LUM_VERSION_UNAVAILABLE,
                        "OSMesa: Failed to create context");
        return LUM_FALSE;
    }

    window->context.makeCurrent = makeContextCurrentOSMesa;
    window->context.swapBuffers = swapBuffersOSMesa;
    window->context.swapInterval = swapIntervalOSMesa;
    window->context.extensionSupported = extensionSupportedOSMesa;
    window->context.getProcAddress = getProcAddressOSMesa;
    window->context.destroy = destroyContextOSMesa;

    return LUM_TRUE;
}

#undef setAttrib


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI int lumGetOSMesaColorBuffer(LUMwindow* handle, int* width,
                                     int* height, int* format, void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaFormat;
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (window->context.source != LUM_OSMESA_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return LUM_FALSE;
    }

    if (!OSMesaGetColorBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaFormat, &mesaBuffer))
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve color buffer");
        return LUM_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (format)
        *format = mesaFormat;
    if (buffer)
        *buffer = mesaBuffer;

    return LUM_TRUE;
}

LUMAPI int lumGetOSMesaDepthBuffer(LUMwindow* handle,
                                     int* width, int* height,
                                     int* bytesPerValue,
                                     void** buffer)
{
    void* mesaBuffer;
    GLint mesaWidth, mesaHeight, mesaBytes;
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(LUM_FALSE);

    if (window->context.source != LUM_OSMESA_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return LUM_FALSE;
    }

    if (!OSMesaGetDepthBuffer(window->context.osmesa.handle,
                              &mesaWidth, &mesaHeight,
                              &mesaBytes, &mesaBuffer))
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "OSMesa: Failed to retrieve depth buffer");
        return LUM_FALSE;
    }

    if (width)
        *width = mesaWidth;
    if (height)
        *height = mesaHeight;
    if (bytesPerValue)
        *bytesPerValue = mesaBytes;
    if (buffer)
        *buffer = mesaBuffer;

    return LUM_TRUE;
}

LUMAPI OSMesaContext lumGetOSMesaContext(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (window->context.source != LUM_OSMESA_CONTEXT_API)
    {
        _lumInputError(LUM_NO_WINDOW_CONTEXT, NULL);
        return NULL;
    }

    return window->context.osmesa.handle;
}

