//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2016-2019 Camilla Löwy <elmindreda@glfw.org>
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

static void applySizeLimits(_LUMwindow* window, int* width, int* height)
{
    if (window->numer != LUM_DONT_CARE && window->denom != LUM_DONT_CARE)
    {
        const float ratio = (float) window->numer / (float) window->denom;
        *height = (int) (*width / ratio);
    }

    if (window->minwidth != LUM_DONT_CARE && *width < window->minwidth)
        *width = window->minwidth;
    else if (window->maxwidth != LUM_DONT_CARE && *width > window->maxwidth)
        *width = window->maxwidth;

    if (window->minheight != LUM_DONT_CARE && *height < window->minheight)
        *height = window->minheight;
    else if (window->maxheight != LUM_DONT_CARE && *height > window->maxheight)
        *height = window->maxheight;
}

static void fitToMonitor(_LUMwindow* window)
{
    LUMvidmode mode;
    _lumGetVideoModeNull(window->monitor, &mode);
    _lumGetMonitorPosNull(window->monitor,
                           &window->null.xpos,
                           &window->null.ypos);
    window->null.width = mode.width;
    window->null.height = mode.height;
}

static void acquireMonitor(_LUMwindow* window)
{
    _lumInputMonitorWindow(window->monitor, window);
}

static void releaseMonitor(_LUMwindow* window)
{
    if (window->monitor->window != window)
        return;

    _lumInputMonitorWindow(window->monitor, NULL);
}

static int createNativeWindow(_LUMwindow* window,
                              const _LUMwndconfig* wndconfig,
                              const _LUMfbconfig* fbconfig)
{
    if (window->monitor)
        fitToMonitor(window);
    else
    {
        window->null.xpos = 17;
        window->null.ypos = 17;
        window->null.width = wndconfig->width;
        window->null.height = wndconfig->height;
    }

    window->null.visible = wndconfig->visible;
    window->null.decorated = wndconfig->decorated;
    window->null.maximized = wndconfig->maximized;
    window->null.floating = wndconfig->floating;
    window->null.transparent = fbconfig->transparent;
    window->null.opacity = 1.f;

    return LUM_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _lumCreateWindowNull(_LUMwindow* window,
                          const _LUMwndconfig* wndconfig,
                          const _LUMctxconfig* ctxconfig,
                          const _LUMfbconfig* fbconfig)
{
    if (!createNativeWindow(window, wndconfig, fbconfig))
        return LUM_FALSE;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_NATIVE_CONTEXT_API ||
            ctxconfig->source == LUM_OSMESA_CONTEXT_API)
        {
            if (!_lumInitOSMesa())
                return LUM_FALSE;
            if (!_lumCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_EGL_CONTEXT_API)
        {
            if (!_lumInitEGL())
                return LUM_FALSE;
            if (!_lumCreateContextEGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
    }

    if (window->monitor)
    {
        _lumShowWindowNull(window);
        _lumFocusWindowNull(window);
        acquireMonitor(window);
    }

    return LUM_TRUE;
}

void _lumDestroyWindowNull(_LUMwindow* window)
{
    if (window->monitor)
        releaseMonitor(window);

    if (_lum.null.focusedWindow == window)
        _lum.null.focusedWindow = NULL;

    if (window->context.destroy)
        window->context.destroy(window);
}

void _lumSetWindowTitleNull(_LUMwindow* window, const char* title)
{
}

void _lumSetWindowIconNull(_LUMwindow* window, int count, const LUMimage* images)
{
}

void _lumSetWindowMonitorNull(_LUMwindow* window,
                               _LUMmonitor* monitor,
                               int xpos, int ypos,
                               int width, int height,
                               int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (!monitor)
        {
            _lumSetWindowPosNull(window, xpos, ypos);
            _lumSetWindowSizeNull(window, width, height);
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _lumInputWindowMonitor(window, monitor);

    if (window->monitor)
    {
        window->null.visible = LUM_TRUE;
        acquireMonitor(window);
        fitToMonitor(window);
    }
    else
    {
        _lumSetWindowPosNull(window, xpos, ypos);
        _lumSetWindowSizeNull(window, width, height);
    }
}

void _lumGetWindowPosNull(_LUMwindow* window, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = window->null.xpos;
    if (ypos)
        *ypos = window->null.ypos;
}

void _lumSetWindowPosNull(_LUMwindow* window, int xpos, int ypos)
{
    if (window->monitor)
        return;

    if (window->null.xpos != xpos || window->null.ypos != ypos)
    {
        window->null.xpos = xpos;
        window->null.ypos = ypos;
        _lumInputWindowPos(window, xpos, ypos);
    }
}

void _lumGetWindowSizeNull(_LUMwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _lumSetWindowSizeNull(_LUMwindow* window, int width, int height)
{
    if (window->monitor)
        return;

    if (window->null.width != width || window->null.height != height)
    {
        window->null.width = width;
        window->null.height = height;
        _lumInputWindowSize(window, width, height);
        _lumInputFramebufferSize(window, width, height);
    }
}

void _lumSetWindowSizeLimitsNull(_LUMwindow* window,
                                  int minwidth, int minheight,
                                  int maxwidth, int maxheight)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _lumSetWindowSizeNull(window, width, height);
}

void _lumSetWindowAspectRatioNull(_LUMwindow* window, int n, int d)
{
    int width = window->null.width;
    int height = window->null.height;
    applySizeLimits(window, &width, &height);
    _lumSetWindowSizeNull(window, width, height);
}

void _lumGetFramebufferSizeNull(_LUMwindow* window, int* width, int* height)
{
    if (width)
        *width = window->null.width;
    if (height)
        *height = window->null.height;
}

void _lumGetWindowFrameSizeNull(_LUMwindow* window,
                                 int* left, int* top,
                                 int* right, int* bottom)
{
    if (window->null.decorated && !window->monitor)
    {
        if (left)
            *left = 1;
        if (top)
            *top = 10;
        if (right)
            *right = 1;
        if (bottom)
            *bottom = 1;
    }
    else
    {
        if (left)
            *left = 0;
        if (top)
            *top = 0;
        if (right)
            *right = 0;
        if (bottom)
            *bottom = 0;
    }
}

void _lumGetWindowContentScaleNull(_LUMwindow* window, float* xscale, float* yscale)
{
    if (xscale)
        *xscale = 1.f;
    if (yscale)
        *yscale = 1.f;
}

void _lumIconifyWindowNull(_LUMwindow* window)
{
    if (_lum.null.focusedWindow == window)
    {
        _lum.null.focusedWindow = NULL;
        _lumInputWindowFocus(window, LUM_FALSE);
    }

    if (!window->null.iconified)
    {
        window->null.iconified = LUM_TRUE;
        _lumInputWindowIconify(window, LUM_TRUE);

        if (window->monitor)
            releaseMonitor(window);
    }
}

void _lumRestoreWindowNull(_LUMwindow* window)
{
    if (window->null.iconified)
    {
        window->null.iconified = LUM_FALSE;
        _lumInputWindowIconify(window, LUM_FALSE);

        if (window->monitor)
            acquireMonitor(window);
    }
    else if (window->null.maximized)
    {
        window->null.maximized = LUM_FALSE;
        _lumInputWindowMaximize(window, LUM_FALSE);
    }
}

void _lumMaximizeWindowNull(_LUMwindow* window)
{
    if (!window->null.maximized)
    {
        window->null.maximized = LUM_TRUE;
        _lumInputWindowMaximize(window, LUM_TRUE);
    }
}

int _lumWindowMaximizedNull(_LUMwindow* window)
{
    return window->null.maximized;
}

int _lumWindowHoveredNull(_LUMwindow* window)
{
    return _lum.null.xcursor >= window->null.xpos &&
           _lum.null.ycursor >= window->null.ypos &&
           _lum.null.xcursor <= window->null.xpos + window->null.width - 1 &&
           _lum.null.ycursor <= window->null.ypos + window->null.height - 1;
}

int _lumFramebufferTransparentNull(_LUMwindow* window)
{
    return window->null.transparent;
}

void _lumSetWindowResizableNull(_LUMwindow* window, LUMbool enabled)
{
    window->null.resizable = enabled;
}

void _lumSetWindowDecoratedNull(_LUMwindow* window, LUMbool enabled)
{
    window->null.decorated = enabled;
}

void _lumSetWindowTitlebarNull(_LUMwindow* window, LUMbool enabled)
{
    window->null.titlebar = enabled;
}

void _lumSetWindowFloatingNull(_LUMwindow* window, LUMbool enabled)
{
    window->null.floating = enabled;
}

void _lumSetWindowMousePassthroughNull(_LUMwindow* window, LUMbool enabled)
{
}

float _lumGetWindowOpacityNull(_LUMwindow* window)
{
    return window->null.opacity;
}

void _lumSetWindowOpacityNull(_LUMwindow* window, float opacity)
{
    window->null.opacity = opacity;
}

void _lumSetRawMouseMotionNull(_LUMwindow *window, LUMbool enabled)
{
}

LUMbool _lumRawMouseMotionSupportedNull(void)
{
    return LUM_TRUE;
}

void _lumShowWindowNull(_LUMwindow* window)
{
    window->null.visible = LUM_TRUE;
}

void _lumRequestWindowAttentionNull(_LUMwindow* window)
{
}

void _lumHideWindowNull(_LUMwindow* window)
{
    if (_lum.null.focusedWindow == window)
    {
        _lum.null.focusedWindow = NULL;
        _lumInputWindowFocus(window, LUM_FALSE);
    }

    window->null.visible = LUM_FALSE;
}

void _lumFocusWindowNull(_LUMwindow* window)
{
    _LUMwindow* previous;

    if (_lum.null.focusedWindow == window)
        return;

    if (!window->null.visible)
        return;

    previous = _lum.null.focusedWindow;
    _lum.null.focusedWindow = window;

    if (previous)
    {
        _lumInputWindowFocus(previous, LUM_FALSE);
        if (previous->monitor && previous->autoIconify)
            _lumIconifyWindowNull(previous);
    }

    _lumInputWindowFocus(window, LUM_TRUE);
}

int _lumWindowFocusedNull(_LUMwindow* window)
{
    return _lum.null.focusedWindow == window;
}

int _lumWindowIconifiedNull(_LUMwindow* window)
{
    return window->null.iconified;
}

int _lumWindowVisibleNull(_LUMwindow* window)
{
    return window->null.visible;
}

void _lumPollEventsNull(void)
{
}

void _lumWaitEventsNull(void)
{
}

void _lumWaitEventsTimeoutNull(double timeout)
{
}

void _lumPostEmptyEventNull(void)
{
}

void _lumGetCursorPosNull(_LUMwindow* window, double* xpos, double* ypos)
{
    if (xpos)
        *xpos = _lum.null.xcursor - window->null.xpos;
    if (ypos)
        *ypos = _lum.null.ycursor - window->null.ypos;
}

void _lumSetCursorPosNull(_LUMwindow* window, double x, double y)
{
    _lum.null.xcursor = window->null.xpos + (int) x;
    _lum.null.ycursor = window->null.ypos + (int) y;
}

void _lumSetCursorModeNull(_LUMwindow* window, int mode)
{
}

int _lumCreateCursorNull(_LUMcursor* cursor,
                          const LUMimage* image,
                          int xhot, int yhot)
{
    return LUM_TRUE;
}

int _lumCreateStandardCursorNull(_LUMcursor* cursor, int shape)
{
    return LUM_TRUE;
}

void _lumDestroyCursorNull(_LUMcursor* cursor)
{
}

void _lumSetCursorNull(_LUMwindow* window, _LUMcursor* cursor)
{
}

void _lumSetClipboardStringNull(const char* string)
{
    char* copy = _lum_strdup(string);
    _lum_free(_lum.null.clipboardString);
    _lum.null.clipboardString = copy;
}

const char* _lumGetClipboardStringNull(void)
{
    return _lum.null.clipboardString;
}

EGLenum _lumGetEGLPlatformNull(EGLint** attribs)
{
    return 0;
}

EGLNativeDisplayType _lumGetEGLNativeDisplayNull(void)
{
    return 0;
}

EGLNativeWindowType _lumGetEGLNativeWindowNull(_LUMwindow* window)
{
    return 0;
}

const char* _lumGetScancodeNameNull(int scancode)
{
    if (scancode < LUM_KEY_SPACE || scancode > LUM_KEY_LAST)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    switch (scancode)
    {
        case LUM_KEY_APOSTROPHE:
            return "'";
        case LUM_KEY_COMMA:
            return ",";
        case LUM_KEY_MINUS:
        case LUM_KEY_KP_SUBTRACT:
            return "-";
        case LUM_KEY_PERIOD:
        case LUM_KEY_KP_DECIMAL:
            return ".";
        case LUM_KEY_SLASH:
        case LUM_KEY_KP_DIVIDE:
            return "/";
        case LUM_KEY_SEMICOLON:
            return ";";
        case LUM_KEY_EQUAL:
        case LUM_KEY_KP_EQUAL:
            return "=";
        case LUM_KEY_LEFT_BRACKET:
            return "[";
        case LUM_KEY_RIGHT_BRACKET:
            return "]";
        case LUM_KEY_KP_MULTIPLY:
            return "*";
        case LUM_KEY_KP_ADD:
            return "+";
        case LUM_KEY_BACKSLASH:
        case LUM_KEY_WORLD_1:
        case LUM_KEY_WORLD_2:
            return "\\";
        case LUM_KEY_0:
        case LUM_KEY_KP_0:
            return "0";
        case LUM_KEY_1:
        case LUM_KEY_KP_1:
            return "1";
        case LUM_KEY_2:
        case LUM_KEY_KP_2:
            return "2";
        case LUM_KEY_3:
        case LUM_KEY_KP_3:
            return "3";
        case LUM_KEY_4:
        case LUM_KEY_KP_4:
            return "4";
        case LUM_KEY_5:
        case LUM_KEY_KP_5:
            return "5";
        case LUM_KEY_6:
        case LUM_KEY_KP_6:
            return "6";
        case LUM_KEY_7:
        case LUM_KEY_KP_7:
            return "7";
        case LUM_KEY_8:
        case LUM_KEY_KP_8:
            return "8";
        case LUM_KEY_9:
        case LUM_KEY_KP_9:
            return "9";
        case LUM_KEY_A:
            return "a";
        case LUM_KEY_B:
            return "b";
        case LUM_KEY_C:
            return "c";
        case LUM_KEY_D:
            return "d";
        case LUM_KEY_E:
            return "e";
        case LUM_KEY_F:
            return "f";
        case LUM_KEY_G:
            return "g";
        case LUM_KEY_H:
            return "h";
        case LUM_KEY_I:
            return "i";
        case LUM_KEY_J:
            return "j";
        case LUM_KEY_K:
            return "k";
        case LUM_KEY_L:
            return "l";
        case LUM_KEY_M:
            return "m";
        case LUM_KEY_N:
            return "n";
        case LUM_KEY_O:
            return "o";
        case LUM_KEY_P:
            return "p";
        case LUM_KEY_Q:
            return "q";
        case LUM_KEY_R:
            return "r";
        case LUM_KEY_S:
            return "s";
        case LUM_KEY_T:
            return "t";
        case LUM_KEY_U:
            return "u";
        case LUM_KEY_V:
            return "v";
        case LUM_KEY_W:
            return "w";
        case LUM_KEY_X:
            return "x";
        case LUM_KEY_Y:
            return "y";
        case LUM_KEY_Z:
            return "z";
    }

    return NULL;
}

int _lumGetKeyScancodeNull(int key)
{
    return key;
}

void _lumGetRequiredInstanceExtensionsNull(char** extensions)
{
}

int _lumGetPhysicalDevicePresentationSupportNull(VkInstance instance,
                                                  VkPhysicalDevice device,
                                                  uint32_t queuefamily)
{
    return LUM_FALSE;
}

VkResult _lumCreateWindowSurfaceNull(VkInstance instance,
                                      _LUMwindow* window,
                                      const VkAllocationCallbacks* allocator,
                                      VkSurfaceKHR* surface)
{
    // This seems like the most appropriate error to return here
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

