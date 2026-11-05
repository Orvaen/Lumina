//========================================================================
// Lumina macOS - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@glfw.org>
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

#include <stdint.h>

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>

// NOTE: All of NSGL was deprecated in the 10.14 SDK
//       This disables the pointless warnings for every symbol we use
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#if defined(__OBJC__)
#import <Cocoa/Cocoa.h>
#else
typedef void* id;
#endif

// NOTE: Many Cocoa enum values have been renamed and we need to build across
//       SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the base SDK does not provide the newer names.

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101400
 #define NSOpenGLContextParameterSwapInterval NSOpenGLCPSwapInterval
 #define NSOpenGLContextParameterSurfaceOpacity NSOpenGLCPSurfaceOpacity
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
 #define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
 #define NSEventMaskAny NSAnyEventMask
 #define NSEventMaskKeyUp NSKeyUpMask
 #define NSEventModifierFlagCapsLock NSAlphaShiftKeyMask
 #define NSEventModifierFlagCommand NSCommandKeyMask
 #define NSEventModifierFlagControl NSControlKeyMask
 #define NSEventModifierFlagDeviceIndependentFlagsMask NSDeviceIndependentModifierFlagsMask
 #define NSEventModifierFlagOption NSAlternateKeyMask
 #define NSEventModifierFlagShift NSShiftKeyMask
 #define NSEventTypeApplicationDefined NSApplicationDefined
 #define NSWindowStyleMaskBorderless NSBorderlessWindowMask
 #define NSWindowStyleMaskClosable NSClosableWindowMask
 #define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
 #define NSWindowStyleMaskResizable NSResizableWindowMask
 #define NSWindowStyleMaskTitled NSTitledWindowMask
#endif

// NOTE: Many Cocoa dynamically linked constants have been renamed and we need
//       to build across SDK versions where one is unavailable or deprecated.
//       We use the newer names in code and replace them with the older names if
//       the deployment target is older than the newer names.

#if MAC_OS_X_VERSION_MIN_REQUIRED < 101300
 #define NSPasteboardTypeURL NSURLPboardType
#endif

typedef VkFlags VkMacOSSurfaceCreateFlagsMVK;
typedef VkFlags VkMetalSurfaceCreateFlagsEXT;

typedef struct VkMacOSSurfaceCreateInfoMVK
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMacOSSurfaceCreateFlagsMVK    flags;
    const void*                     pView;
} VkMacOSSurfaceCreateInfoMVK;

typedef struct VkMetalSurfaceCreateInfoEXT
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkMetalSurfaceCreateFlagsEXT    flags;
    const void*                     pLayer;
} VkMetalSurfaceCreateInfoEXT;

typedef VkResult (APIENTRY *PFN_vkCreateMacOSSurfaceMVK)(VkInstance,const VkMacOSSurfaceCreateInfoMVK*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkResult (APIENTRY *PFN_vkCreateMetalSurfaceEXT)(VkInstance,const VkMetalSurfaceCreateInfoEXT*,const VkAllocationCallbacks*,VkSurfaceKHR*);

#define LUM_COCOA_WINDOW_STATE         _LUMwindowNS  ns;
#define LUM_COCOA_LIBRARY_WINDOW_STATE _LUMlibraryNS ns;
#define LUM_COCOA_MONITOR_STATE        _LUMmonitorNS ns;
#define LUM_COCOA_CURSOR_STATE         _LUMcursorNS  ns;

#define LUM_NSGL_CONTEXT_STATE         _LUMcontextNSGL nsgl;
#define LUM_NSGL_LIBRARY_CONTEXT_STATE _LUMlibraryNSGL nsgl;

// HIToolbox.framework pointer typedefs
#define kTISPropertyUnicodeKeyLayoutData _lum.ns.tis.kPropertyUnicodeKeyLayoutData
typedef TISInputSourceRef (*PFN_TISCopyCurrentKeyboardLayoutInputSource)(void);
#define TISCopyCurrentKeyboardLayoutInputSource _lum.ns.tis.CopyCurrentKeyboardLayoutInputSource
typedef void* (*PFN_TISGetInputSourceProperty)(TISInputSourceRef,CFStringRef);
#define TISGetInputSourceProperty _lum.ns.tis.GetInputSourceProperty
typedef UInt8 (*PFN_LMGetKbdType)(void);
#define LMGetKbdType _lum.ns.tis.GetKbdType


// NSGL-specific per-context data
//
typedef struct _LUMcontextNSGL
{
    id                pixelFormat;
    id                object;
} _LUMcontextNSGL;

// NSGL-specific global data
//
typedef struct _LUMlibraryNSGL
{
    // dlopen handle for OpenGL.framework (for lumGetProcAddress)
    CFBundleRef     framework;
} _LUMlibraryNSGL;

// Cocoa-specific per-window data
//
typedef struct _LUMwindowNS
{
    id              object;
    id              delegate;
    id              view;
    id              layer;

    LUMbool        maximized;
    LUMbool        occluded;
    LUMbool        retina;

    // Cached window properties to filter out duplicate events
    int             width, height;
    int             fbWidth, fbHeight;
    float           xscale, yscale;

    // The total sum of the distances the cursor has been warped
    // since the last cursor motion event was processed
    // This is kept to counteract Cocoa doing the same internally
    double          cursorWarpDeltaX, cursorWarpDeltaY;
} _LUMwindowNS;

// Cocoa-specific global data
//
typedef struct _LUMlibraryNS
{
    CGEventSourceRef    eventSource;
    id                  delegate;
    LUMbool            cursorHidden;
    TISInputSourceRef   inputSource;
    IOHIDManagerRef     hidManager;
    id                  unicodeData;
    id                  helper;
    id                  keyUpMonitor;
    id                  nibObjects;

    char                keynames[LUM_KEY_LAST + 1][17];
    short int           keycodes[256];
    short int           scancodes[LUM_KEY_LAST + 1];
    char*               clipboardString;
    CGPoint             cascadePoint;
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _LUMwindow*        disabledCursorWindow;

    struct {
        CFBundleRef     bundle;
        PFN_TISCopyCurrentKeyboardLayoutInputSource CopyCurrentKeyboardLayoutInputSource;
        PFN_TISGetInputSourceProperty GetInputSourceProperty;
        PFN_LMGetKbdType GetKbdType;
        CFStringRef     kPropertyUnicodeKeyLayoutData;
    } tis;
} _LUMlibraryNS;

// Cocoa-specific per-monitor data
//
typedef struct _LUMmonitorNS
{
    CGDirectDisplayID   displayID;
    CGDisplayModeRef    previousMode;
    uint32_t            unitNumber;
    id                  screen;
    double              fallbackRefreshRate;
} _LUMmonitorNS;

// Cocoa-specific per-cursor data
//
typedef struct _LUMcursorNS
{
    id              object;
} _LUMcursorNS;


LUMbool _lumConnectCocoa(int platformID, _LUMplatform* platform);
int _lumInitCocoa(void);
void _lumTerminateCocoa(void);

int _lumCreateWindowCocoa(_LUMwindow* window, const _LUMwndconfig* wndconfig, const _LUMctxconfig* ctxconfig, const _LUMfbconfig* fbconfig);
void _lumDestroyWindowCocoa(_LUMwindow* window);
void _lumSetWindowTitleCocoa(_LUMwindow* window, const char* title);
void _lumSetWindowIconCocoa(_LUMwindow* window, int count, const LUMimage* images);
void _lumGetWindowPosCocoa(_LUMwindow* window, int* xpos, int* ypos);
void _lumSetWindowPosCocoa(_LUMwindow* window, int xpos, int ypos);
void _lumGetWindowSizeCocoa(_LUMwindow* window, int* width, int* height);
void _lumSetWindowSizeCocoa(_LUMwindow* window, int width, int height);
void _lumSetWindowSizeLimitsCocoa(_LUMwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _lumSetWindowAspectRatioCocoa(_LUMwindow* window, int numer, int denom);
void _lumGetFramebufferSizeCocoa(_LUMwindow* window, int* width, int* height);
void _lumGetWindowFrameSizeCocoa(_LUMwindow* window, int* left, int* top, int* right, int* bottom);
void _lumGetWindowContentScaleCocoa(_LUMwindow* window, float* xscale, float* yscale);
void _lumIconifyWindowCocoa(_LUMwindow* window);
void _lumRestoreWindowCocoa(_LUMwindow* window);
void _lumMaximizeWindowCocoa(_LUMwindow* window);
void _lumShowWindowCocoa(_LUMwindow* window);
void _lumHideWindowCocoa(_LUMwindow* window);
void _lumRequestWindowAttentionCocoa(_LUMwindow* window);
void _lumFocusWindowCocoa(_LUMwindow* window);
void _lumSetWindowMonitorCocoa(_LUMwindow* window, _LUMmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
int _lumWindowFocusedCocoa(_LUMwindow* window);
int _lumWindowIconifiedCocoa(_LUMwindow* window);
int _lumWindowVisibleCocoa(_LUMwindow* window);
int _lumWindowMaximizedCocoa(_LUMwindow* window);
int _lumWindowHoveredCocoa(_LUMwindow* window);
int _lumFramebufferTransparentCocoa(_LUMwindow* window);
void _lumSetWindowResizableCocoa(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowDecoratedCocoa(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowFloatingCocoa(_LUMwindow* window, LUMbool enabled);
float _lumGetWindowOpacityCocoa(_LUMwindow* window);
void _lumSetWindowOpacityCocoa(_LUMwindow* window, float opacity);
void _lumSetWindowMousePassthroughCocoa(_LUMwindow* window, LUMbool enabled);

void _lumSetRawMouseMotionCocoa(_LUMwindow *window, LUMbool enabled);
LUMbool _lumRawMouseMotionSupportedCocoa(void);

void _lumPollEventsCocoa(void);
void _lumWaitEventsCocoa(void);
void _lumWaitEventsTimeoutCocoa(double timeout);
void _lumPostEmptyEventCocoa(void);

void _lumGetCursorPosCocoa(_LUMwindow* window, double* xpos, double* ypos);
void _lumSetCursorPosCocoa(_LUMwindow* window, double xpos, double ypos);
void _lumSetCursorModeCocoa(_LUMwindow* window, int mode);
const char* _lumGetScancodeNameCocoa(int scancode);
int _lumGetKeyScancodeCocoa(int key);
int _lumCreateCursorCocoa(_LUMcursor* cursor, const LUMimage* image, int xhot, int yhot);
int _lumCreateStandardCursorCocoa(_LUMcursor* cursor, int shape);
void _lumDestroyCursorCocoa(_LUMcursor* cursor);
void _lumSetCursorCocoa(_LUMwindow* window, _LUMcursor* cursor);
void _lumSetClipboardStringCocoa(const char* string);
const char* _lumGetClipboardStringCocoa(void);

EGLenum _lumGetEGLPlatformCocoa(EGLint** attribs);
EGLNativeDisplayType _lumGetEGLNativeDisplayCocoa(void);
EGLNativeWindowType _lumGetEGLNativeWindowCocoa(_LUMwindow* window);

void _lumGetRequiredInstanceExtensionsCocoa(char** extensions);
int _lumGetPhysicalDevicePresentationSupportCocoa(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _lumCreateWindowSurfaceCocoa(VkInstance instance, _LUMwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _lumFreeMonitorCocoa(_LUMmonitor* monitor);
void _lumGetMonitorPosCocoa(_LUMmonitor* monitor, int* xpos, int* ypos);
void _lumGetMonitorContentScaleCocoa(_LUMmonitor* monitor, float* xscale, float* yscale);
void _lumGetMonitorWorkareaCocoa(_LUMmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
LUMvidmode* _lumGetVideoModesCocoa(_LUMmonitor* monitor, int* count);
void _lumGetVideoModeCocoa(_LUMmonitor* monitor, LUMvidmode* mode);
LUMbool _lumGetGammaRampCocoa(_LUMmonitor* monitor, LUMgammaramp* ramp);
void _lumSetGammaRampCocoa(_LUMmonitor* monitor, const LUMgammaramp* ramp);

void _lumPollMonitorsCocoa(void);
void _lumSetVideoModeCocoa(_LUMmonitor* monitor, const LUMvidmode* desired);
void _lumRestoreVideoModeCocoa(_LUMmonitor* monitor);

float _lumTransformYCocoa(float y);

void* _lumLoadLocalVulkanLoaderCocoa(void);

LUMbool _lumInitNSGL(void);
void _lumTerminateNSGL(void);
LUMbool _lumCreateContextNSGL(_LUMwindow* window,
                                const _LUMctxconfig* ctxconfig,
                                const _LUMfbconfig* fbconfig);
void _lumDestroyContextNSGL(_LUMwindow* window);

