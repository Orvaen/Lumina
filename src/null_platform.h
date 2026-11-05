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

#define LUM_NULL_WINDOW_STATE          _LUMwindowNull null;
#define LUM_NULL_LIBRARY_WINDOW_STATE  _LUMlibraryNull null;
#define LUM_NULL_MONITOR_STATE         _LUMmonitorNull null;

#define LUM_NULL_CONTEXT_STATE
#define LUM_NULL_CURSOR_STATE
#define LUM_NULL_LIBRARY_CONTEXT_STATE


// Null-specific per-window data
//
typedef struct _LUMwindowNull
{
    int             xpos;
    int             ypos;
    int             width;
    int             height;
    char*           title;
    LUMbool        visible;
    LUMbool        iconified;
    LUMbool        maximized;
    LUMbool        resizable;
    LUMbool        decorated;
    LUMbool        titlebar;
    LUMbool        floating;
    LUMbool        transparent;
    float           opacity;
} _LUMwindowNull;

// Null-specific per-monitor data
//
typedef struct _LUMmonitorNull
{
    LUMgammaramp   ramp;
} _LUMmonitorNull;

// Null-specific global data
//
typedef struct _LUMlibraryNull
{
    int             xcursor;
    int             ycursor;
    char*           clipboardString;
    _LUMwindow*    focusedWindow;
} _LUMlibraryNull;

void _lumPollMonitorsNull(void);

LUMbool _lumConnectNull(int platformID, _LUMplatform* platform);
int _lumInitNull(void);
void _lumTerminateNull(void);

void _lumFreeMonitorNull(_LUMmonitor* monitor);
void _lumGetMonitorPosNull(_LUMmonitor* monitor, int* xpos, int* ypos);
void _lumGetMonitorContentScaleNull(_LUMmonitor* monitor, float* xscale, float* yscale);
void _lumGetMonitorWorkareaNull(_LUMmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
LUMvidmode* _lumGetVideoModesNull(_LUMmonitor* monitor, int* found);
void _lumGetVideoModeNull(_LUMmonitor* monitor, LUMvidmode* mode);
LUMbool _lumGetGammaRampNull(_LUMmonitor* monitor, LUMgammaramp* ramp);
void _lumSetGammaRampNull(_LUMmonitor* monitor, const LUMgammaramp* ramp);

int _lumCreateWindowNull(_LUMwindow* window, const _LUMwndconfig* wndconfig, const _LUMctxconfig* ctxconfig, const _LUMfbconfig* fbconfig);
void _lumDestroyWindowNull(_LUMwindow* window);
void _lumSetWindowTitleNull(_LUMwindow* window, const char* title);
void _lumSetWindowIconNull(_LUMwindow* window, int count, const LUMimage* images);
void _lumSetWindowMonitorNull(_LUMwindow* window, _LUMmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
void _lumGetWindowPosNull(_LUMwindow* window, int* xpos, int* ypos);
void _lumSetWindowPosNull(_LUMwindow* window, int xpos, int ypos);
void _lumGetWindowSizeNull(_LUMwindow* window, int* width, int* height);
void _lumSetWindowSizeNull(_LUMwindow* window, int width, int height);
void _lumSetWindowSizeLimitsNull(_LUMwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _lumSetWindowAspectRatioNull(_LUMwindow* window, int n, int d);
void _lumGetFramebufferSizeNull(_LUMwindow* window, int* width, int* height);
void _lumGetWindowFrameSizeNull(_LUMwindow* window, int* left, int* top, int* right, int* bottom);
void _lumGetWindowContentScaleNull(_LUMwindow* window, float* xscale, float* yscale);
void _lumIconifyWindowNull(_LUMwindow* window);
void _lumRestoreWindowNull(_LUMwindow* window);
void _lumMaximizeWindowNull(_LUMwindow* window);
int _lumWindowMaximizedNull(_LUMwindow* window);
int _lumWindowHoveredNull(_LUMwindow* window);
int _lumFramebufferTransparentNull(_LUMwindow* window);
void _lumSetWindowResizableNull(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowDecoratedNull(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowFloatingNull(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowMousePassthroughNull(_LUMwindow* window, LUMbool enabled);
float _lumGetWindowOpacityNull(_LUMwindow* window);
void _lumSetWindowOpacityNull(_LUMwindow* window, float opacity);
void _lumSetRawMouseMotionNull(_LUMwindow *window, LUMbool enabled);
LUMbool _lumRawMouseMotionSupportedNull(void);
void _lumShowWindowNull(_LUMwindow* window);
void _lumRequestWindowAttentionNull(_LUMwindow* window);
void _lumRequestWindowAttentionNull(_LUMwindow* window);
void _lumHideWindowNull(_LUMwindow* window);
void _lumFocusWindowNull(_LUMwindow* window);
int _lumWindowFocusedNull(_LUMwindow* window);
int _lumWindowIconifiedNull(_LUMwindow* window);
int _lumWindowVisibleNull(_LUMwindow* window);
void _lumPollEventsNull(void);
void _lumWaitEventsNull(void);
void _lumWaitEventsTimeoutNull(double timeout);
void _lumPostEmptyEventNull(void);

// Hazel
void _lumSetWindowTitlebarNull(_LUMwindow* window, LUMbool enabled);

void _lumGetCursorPosNull(_LUMwindow* window, double* xpos, double* ypos);
void _lumSetCursorPosNull(_LUMwindow* window, double x, double y);
void _lumSetCursorModeNull(_LUMwindow* window, int mode);
int _lumCreateCursorNull(_LUMcursor* cursor, const LUMimage* image, int xhot, int yhot);
int _lumCreateStandardCursorNull(_LUMcursor* cursor, int shape);
void _lumDestroyCursorNull(_LUMcursor* cursor);
void _lumSetCursorNull(_LUMwindow* window, _LUMcursor* cursor);
void _lumSetClipboardStringNull(const char* string);
const char* _lumGetClipboardStringNull(void);
const char* _lumGetScancodeNameNull(int scancode);
int _lumGetKeyScancodeNull(int key);

EGLenum _lumGetEGLPlatformNull(EGLint** attribs);
EGLNativeDisplayType _lumGetEGLNativeDisplayNull(void);
EGLNativeWindowType _lumGetEGLNativeWindowNull(_LUMwindow* window);

void _lumGetRequiredInstanceExtensionsNull(char** extensions);
int _lumGetPhysicalDevicePresentationSupportNull(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _lumCreateWindowSurfaceNull(VkInstance instance, _LUMwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _lumPollMonitorsNull(void);

