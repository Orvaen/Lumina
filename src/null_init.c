//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <stdlib.h>


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumConnectNull(int platformID, _LUMplatform* platform)
{
    const _LUMplatform null =
    {
        LUM_PLATFORM_NULL,
        _lumInitNull,
        _lumTerminateNull,
        _lumGetCursorPosNull,
        _lumSetCursorPosNull,
        _lumSetCursorModeNull,
        _lumSetRawMouseMotionNull,
        _lumRawMouseMotionSupportedNull,
        _lumCreateCursorNull,
        _lumCreateStandardCursorNull,
        _lumDestroyCursorNull,
        _lumSetCursorNull,
        _lumGetScancodeNameNull,
        _lumGetKeyScancodeNull,
        _lumSetClipboardStringNull,
        _lumGetClipboardStringNull,
        _lumInitJoysticksNull,
        _lumTerminateJoysticksNull,
        _lumPollJoystickNull,
        _lumGetMappingNameNull,
        _lumUpdateGamepadGUIDNull,
        _lumFreeMonitorNull,
        _lumGetMonitorPosNull,
        _lumGetMonitorContentScaleNull,
        _lumGetMonitorWorkareaNull,
        _lumGetVideoModesNull,
        _lumGetVideoModeNull,
        _lumGetGammaRampNull,
        _lumSetGammaRampNull,
        _lumCreateWindowNull,
        _lumDestroyWindowNull,
        _lumSetWindowTitleNull,
        _lumSetWindowIconNull,
        _lumGetWindowPosNull,
        _lumSetWindowPosNull,
        _lumGetWindowSizeNull,
        _lumSetWindowSizeNull,
        _lumSetWindowSizeLimitsNull,
        _lumSetWindowAspectRatioNull,
        _lumGetFramebufferSizeNull,
        _lumGetWindowFrameSizeNull,
        _lumGetWindowContentScaleNull,
        _lumIconifyWindowNull,
        _lumRestoreWindowNull,
        _lumMaximizeWindowNull,
        _lumShowWindowNull,
        _lumHideWindowNull,
        _lumRequestWindowAttentionNull,
        _lumFocusWindowNull,
        _lumSetWindowMonitorNull,
        _lumWindowFocusedNull,
        _lumWindowIconifiedNull,
        _lumWindowVisibleNull,
        _lumWindowMaximizedNull,
        _lumWindowHoveredNull,
        _lumFramebufferTransparentNull,
        _lumGetWindowOpacityNull,
        _lumSetWindowResizableNull,
        _lumSetWindowDecoratedNull,
        _lumSetWindowFloatingNull,
        _lumSetWindowOpacityNull,
        _lumSetWindowMousePassthroughNull,
        _lumPollEventsNull,
        _lumWaitEventsNull,
        _lumWaitEventsTimeoutNull,
        _lumPostEmptyEventNull,
        _lumSetWindowTitlebarNull,
        _lumGetEGLPlatformNull,
        _lumGetEGLNativeDisplayNull,
        _lumGetEGLNativeWindowNull,
        _lumGetRequiredInstanceExtensionsNull,
        _lumGetPhysicalDevicePresentationSupportNull,
        _lumCreateWindowSurfaceNull,
    };

    *platform = null;
    return LUM_TRUE;
}

int _lumInitNull(void)
{
    _lumPollMonitorsNull();
    return LUM_TRUE;
}

void _lumTerminateNull(void)
{
    free(_lum.null.clipboardString);
    _lumTerminateOSMesa();
    _lumTerminateEGL();
}

