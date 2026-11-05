//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2018 Camilla Löwy <elmindreda@glfw.org>
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

//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

static const struct
{
    int ID;
    LUMbool (*connect)(int,_LUMplatform*);
} supportedPlatforms[] =
{
#if defined(_LUM_WIN32)
    { LUM_PLATFORM_WIN32, _lumConnectWin32 },
#endif
#if defined(_LUM_COCOA)
    { LUM_PLATFORM_COCOA, _lumConnectCocoa },
#endif
#if defined(_LUM_X11)
    { LUM_PLATFORM_X11, _lumConnectX11 },
#endif
#if defined(_LUM_WAYLAND)
    { LUM_PLATFORM_WAYLAND, _lumConnectWayland },
#endif
};

LUMbool _lumSelectPlatform(int desiredID, _LUMplatform* platform)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (desiredID != LUM_ANY_PLATFORM &&
        desiredID != LUM_PLATFORM_WIN32 &&
        desiredID != LUM_PLATFORM_COCOA &&
        desiredID != LUM_PLATFORM_WAYLAND &&
        desiredID != LUM_PLATFORM_X11 &&
        desiredID != LUM_PLATFORM_NULL)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid platform ID 0x%08X", desiredID);
        return LUM_FALSE;
    }

    // Only allow the Null platform if specifically requested
    if (desiredID == LUM_PLATFORM_NULL)
        return _lumConnectNull(desiredID, platform);
    else if (count == 0)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "This binary only supports the Null platform");
        return LUM_FALSE;
    }

    if (desiredID == LUM_ANY_PLATFORM)
    {
        // If there is exactly one platform available for auto-selection, let it emit the
        // error on failure as the platform-specific error description may be more helpful
        if (count == 1)
            return supportedPlatforms[0].connect(supportedPlatforms[0].ID, platform);

        for (i = 0;  i < count;  i++)
        {
            if (supportedPlatforms[i].connect(desiredID, platform))
                return LUM_TRUE;
        }

        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "Failed to detect any supported platform");
    }
    else
    {
        for (i = 0;  i < count;  i++)
        {
            if (supportedPlatforms[i].ID == desiredID)
                return supportedPlatforms[i].connect(desiredID, platform);
        }

        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "The requested platform is not supported");
    }

    return LUM_FALSE;
}

//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI int lumGetPlatform(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(0);
    return _lum.platform.platformID;
}

LUMAPI int lumPlatformSupported(int platformID)
{
    const size_t count = sizeof(supportedPlatforms) / sizeof(supportedPlatforms[0]);
    size_t i;

    if (platformID != LUM_PLATFORM_WIN32 &&
        platformID != LUM_PLATFORM_COCOA &&
        platformID != LUM_PLATFORM_WAYLAND &&
        platformID != LUM_PLATFORM_X11 &&
        platformID != LUM_PLATFORM_NULL)
    {
        _lumInputError(LUM_INVALID_ENUM, "Invalid platform ID 0x%08X", platformID);
        return LUM_FALSE;
    }

    if (platformID == LUM_PLATFORM_NULL)
        return LUM_TRUE;

    for (i = 0;  i < count;  i++)
    {
        if (platformID == supportedPlatforms[i].ID)
            return LUM_TRUE;
    }

    return LUM_FALSE;
}

LUMAPI const char* lumGetVersionString(void)
{
    return _LUM_VERSION_NUMBER
#if defined(_LUM_WIN32)
        " Win32 WGL"
#endif
#if defined(_LUM_COCOA)
        " Cocoa NSGL"
#endif
#if defined(_LUM_WAYLAND)
        " Wayland"
#endif
#if defined(_LUM_X11)
        " X11 GLX"
#endif
        " Null"
        " EGL"
        " OSMesa"
#if defined(__MINGW64_VERSION_MAJOR)
        " MinGW-w64"
#elif defined(__MINGW32__)
        " MinGW"
#elif defined(_MSC_VER)
        " VisualC"
#endif
#if defined(_LUM_USE_HYBRID_HPG) || defined(_LUM_USE_OPTIMUS_HPG)
        " hybrid-GPU"
#endif
#if defined(_POSIX_MONOTONIC_CLOCK)
        " monotonic"
#endif
#if defined(_LUM_BUILD_DLL)
#if defined(_WIN32)
        " DLL"
#elif defined(__APPLE__)
        " dynamic"
#else
        " shared"
#endif
#endif
        ;
}

