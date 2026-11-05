//========================================================================
// Lumina Win32 - fork of GLFW 3.4 (www.glfw.org)
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

// We don't need all the fancy stuff
#ifndef NOMINMAX
 #define NOMINMAX
#endif

#ifndef VC_EXTRALEAN
 #define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif

// This is a workaround for the fact that lumina.h needs to export APIENTRY (for
// example to allow applications to correctly declare a GL_KHR_debug callback)
// but windows.h assumes no one will define APIENTRY before it does
#undef APIENTRY

// Lumina on Windows is Unicode only and does not work in MBCS mode
#ifndef UNICODE
 #define UNICODE
#endif

// Lumina requires Windows XP or later
#if WINVER < 0x0501
 #undef WINVER
 #define WINVER 0x0501
#endif
#if _WIN32_WINNT < 0x0501
 #undef _WIN32_WINNT
 #define _WIN32_WINNT 0x0501
#endif

// Lumina uses DirectInput8 interfaces
#define DIRECTINPUT_VERSION 0x0800

// Lumina uses OEM cursor resources
#define OEMRESOURCE

#include <wctype.h>
#include <windows.h>
#include <dinput.h>
#include <xinput.h>
#include <dbt.h>

// HACK: Define macros that some windows.h variants don't
#ifndef WM_MOUSEHWHEEL
 #define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
 #define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
 #define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#ifndef WM_COPYGLOBALDATA
 #define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
 #define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
 #define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
 #define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
 #define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
 #define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
 #define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#ifndef _WIN32_WINNT_WINBLUE
 #define _WIN32_WINNT_WINBLUE 0x0603
#endif
#ifndef _WIN32_WINNT_WIN8
 #define _WIN32_WINNT_WIN8 0x0602
#endif
#ifndef WM_GETDPISCALEDSIZE
 #define WM_GETDPISCALEDSIZE 0x02e4
#endif
#ifndef USER_DEFAULT_SCREEN_DPI
 #define USER_DEFAULT_SCREEN_DPI 96
#endif
#ifndef OCR_HAND
 #define OCR_HAND 32649
#endif

#if WINVER < 0x0601
typedef struct
{
    DWORD cbSize;
    DWORD ExtStatus;
} CHANGEFILTERSTRUCT;
#ifndef MSGFLT_ALLOW
 #define MSGFLT_ALLOW 1
#endif
#endif /*Windows 7*/

#if WINVER < 0x0600
#define DWM_BB_ENABLE 0x00000001
#define DWM_BB_BLURREGION 0x00000002
typedef struct
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;
#else
 #include <dwmapi.h>
#endif /*Windows Vista*/

#ifndef DPI_ENUMS_DECLARED
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef enum
{
    MDT_EFFECTIVE_DPI = 0,
    MDT_ANGULAR_DPI = 1,
    MDT_RAW_DPI = 2,
    MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif /*DPI_ENUMS_DECLARED*/

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE) -4)
#endif /*DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2*/

// HACK: Define versionhelpers.h functions manually as MinGW lacks the header
#define IsWindowsVistaOrGreater()                                     \
    _lumIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_VISTA),   \
                                        LOBYTE(_WIN32_WINNT_VISTA), 0)
#define IsWindows7OrGreater()                                         \
    _lumIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN7),    \
                                        LOBYTE(_WIN32_WINNT_WIN7), 0)
#define IsWindows8OrGreater()                                         \
    _lumIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WIN8),    \
                                        LOBYTE(_WIN32_WINNT_WIN8), 0)
#define IsWindows8Point1OrGreater()                                   \
    _lumIsWindowsVersionOrGreaterWin32(HIBYTE(_WIN32_WINNT_WINBLUE), \
                                        LOBYTE(_WIN32_WINNT_WINBLUE), 0)

#define _lumIsWindows10AnniversaryUpdateOrGreaterWin32() \
    _lumIsWindows10BuildOrGreaterWin32(14393)
#define _lumIsWindows10CreatorsUpdateOrGreaterWin32() \
    _lumIsWindows10BuildOrGreaterWin32(15063)

// HACK: Define macros that some xinput.h variants don't
#ifndef XINPUT_CAPS_WIRELESS
 #define XINPUT_CAPS_WIRELESS 0x0002
#endif
#ifndef XINPUT_DEVSUBTYPE_WHEEL
 #define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
 #define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
 #define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
 #define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
 #define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
 #define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_PAD
 #define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#ifndef XUSER_MAX_COUNT
 #define XUSER_MAX_COUNT 4
#endif

// HACK: Define macros that some dinput.h variants don't
#ifndef DIDFT_OPTIONAL
 #define DIDFT_OPTIONAL 0x80000000
#endif

#define WGL_NUMBER_PIXEL_FORMATS_ARB 0x2000
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202b
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_NO_ACCELERATION_ARB 0x2025
#define WGL_RED_BITS_ARB 0x2015
#define WGL_RED_SHIFT_ARB 0x2016
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_GREEN_SHIFT_ARB 0x2018
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_BLUE_SHIFT_ARB 0x201a
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_ALPHA_SHIFT_ARB 0x201c
#define WGL_ACCUM_BITS_ARB 0x201d
#define WGL_ACCUM_RED_BITS_ARB 0x201e
#define WGL_ACCUM_GREEN_BITS_ARB 0x201f
#define WGL_ACCUM_BLUE_BITS_ARB 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB 0x2021
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_AUX_BUFFERS_ARB 0x2024
#define WGL_STEREO_ARB 0x2012
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_SAMPLES_ARB 0x2042
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20a9
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#define WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define WGL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define WGL_NO_RESET_NOTIFICATION_ARB 0x8261
#define WGL_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define WGL_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define WGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#define WGL_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3
#define WGL_COLORSPACE_EXT 0x309d
#define WGL_COLORSPACE_SRGB_EXT 0x3089

#define ERROR_INVALID_VERSION_ARB 0x2095
#define ERROR_INVALID_PROFILE_ARB 0x2096
#define ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB 0x2054

// xinput.dll function pointer typedefs
typedef DWORD (WINAPI * PFN_XInputGetCapabilities)(DWORD,DWORD,XINPUT_CAPABILITIES*);
typedef DWORD (WINAPI * PFN_XInputGetState)(DWORD,XINPUT_STATE*);
#define XInputGetCapabilities _lum.win32.xinput.GetCapabilities
#define XInputGetState _lum.win32.xinput.GetState

// dinput8.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_DirectInput8Create)(HINSTANCE,DWORD,REFIID,LPVOID*,LPUNKNOWN);
#define DirectInput8Create _lum.win32.dinput8.Create

// user32.dll function pointer typedefs
typedef BOOL (WINAPI * PFN_SetProcessDPIAware)(void);
typedef BOOL (WINAPI * PFN_ChangeWindowMessageFilterEx)(HWND,UINT,DWORD,CHANGEFILTERSTRUCT*);
typedef BOOL (WINAPI * PFN_EnableNonClientDpiScaling)(HWND);
typedef BOOL (WINAPI * PFN_SetProcessDpiAwarenessContext)(HANDLE);
typedef UINT (WINAPI * PFN_GetDpiForWindow)(HWND);
typedef BOOL (WINAPI * PFN_AdjustWindowRectExForDpi)(LPRECT,DWORD,BOOL,DWORD,UINT);
#define SetProcessDPIAware _lum.win32.user32.SetProcessDPIAware_
#define ChangeWindowMessageFilterEx _lum.win32.user32.ChangeWindowMessageFilterEx_
#define EnableNonClientDpiScaling _lum.win32.user32.EnableNonClientDpiScaling_
#define SetProcessDpiAwarenessContext _lum.win32.user32.SetProcessDpiAwarenessContext_
#define GetDpiForWindow _lum.win32.user32.GetDpiForWindow_
#define AdjustWindowRectExForDpi _lum.win32.user32.AdjustWindowRectExForDpi_

// dwmapi.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_DwmIsCompositionEnabled)(BOOL*);
typedef HRESULT (WINAPI * PFN_DwmFlush)(VOID);
typedef HRESULT(WINAPI * PFN_DwmEnableBlurBehindWindow)(HWND,const DWM_BLURBEHIND*);
typedef HRESULT (WINAPI * PFN_DwmGetColorizationColor)(DWORD*,BOOL*);
#define DwmIsCompositionEnabled _lum.win32.dwmapi.IsCompositionEnabled
#define DwmFlush _lum.win32.dwmapi.Flush
#define DwmEnableBlurBehindWindow _lum.win32.dwmapi.EnableBlurBehindWindow
#define DwmGetColorizationColor _lum.win32.dwmapi.GetColorizationColor

// shcore.dll function pointer typedefs
typedef HRESULT (WINAPI * PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);
typedef HRESULT (WINAPI * PFN_GetDpiForMonitor)(HMONITOR,MONITOR_DPI_TYPE,UINT*,UINT*);
#define SetProcessDpiAwareness _lum.win32.shcore.SetProcessDpiAwareness_
#define GetDpiForMonitor _lum.win32.shcore.GetDpiForMonitor_

// ntdll.dll function pointer typedefs
typedef LONG (WINAPI * PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*,ULONG,ULONGLONG);
#define RtlVerifyVersionInfo _lum.win32.ntdll.RtlVerifyVersionInfo_

// WGL extension pointer typedefs
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC,int,int,UINT,const int*,int*);
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);
typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC,HGLRC,const int*);
#define wglSwapIntervalEXT _lum.wgl.SwapIntervalEXT
#define wglGetPixelFormatAttribivARB _lum.wgl.GetPixelFormatAttribivARB
#define wglGetExtensionsStringEXT _lum.wgl.GetExtensionsStringEXT
#define wglGetExtensionsStringARB _lum.wgl.GetExtensionsStringARB
#define wglCreateContextAttribsARB _lum.wgl.CreateContextAttribsARB

// opengl32.dll function pointer typedefs
typedef HGLRC (WINAPI * PFN_wglCreateContext)(HDC);
typedef BOOL (WINAPI * PFN_wglDeleteContext)(HGLRC);
typedef PROC (WINAPI * PFN_wglGetProcAddress)(LPCSTR);
typedef HDC (WINAPI * PFN_wglGetCurrentDC)(void);
typedef HGLRC (WINAPI * PFN_wglGetCurrentContext)(void);
typedef BOOL (WINAPI * PFN_wglMakeCurrent)(HDC,HGLRC);
typedef BOOL (WINAPI * PFN_wglShareLists)(HGLRC,HGLRC);
#define wglCreateContext _lum.wgl.CreateContext
#define wglDeleteContext _lum.wgl.DeleteContext
#define wglGetProcAddress _lum.wgl.GetProcAddress
#define wglGetCurrentDC _lum.wgl.GetCurrentDC
#define wglGetCurrentContext _lum.wgl.GetCurrentContext
#define wglMakeCurrent _lum.wgl.MakeCurrent
#define wglShareLists _lum.wgl.ShareLists

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(VkInstance,const VkWin32SurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
typedef VkBool32 (APIENTRY *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice,uint32_t);

#if !defined(_LUM_WNDCLASSNAME)
 #define _LUM_WNDCLASSNAME L"LUMINA"
#endif

#define LUM_WIN32_WINDOW_STATE         _LUMwindowWin32  win32;
#define LUM_WIN32_LIBRARY_WINDOW_STATE _LUMlibraryWin32 win32;
#define LUM_WIN32_MONITOR_STATE        _LUMmonitorWin32 win32;
#define LUM_WIN32_CURSOR_STATE         _LUMcursorWin32  win32;

#define LUM_WGL_CONTEXT_STATE          _LUMcontextWGL wgl;
#define LUM_WGL_LIBRARY_CONTEXT_STATE  _LUMlibraryWGL wgl;


// WGL-specific per-context data
//
typedef struct _LUMcontextWGL
{
    HDC       dc;
    HGLRC     handle;
    int       interval;
} _LUMcontextWGL;

// WGL-specific global data
//
typedef struct _LUMlibraryWGL
{
    HINSTANCE                           instance;
    PFN_wglCreateContext                CreateContext;
    PFN_wglDeleteContext                DeleteContext;
    PFN_wglGetProcAddress               GetProcAddress;
    PFN_wglGetCurrentDC                 GetCurrentDC;
    PFN_wglGetCurrentContext            GetCurrentContext;
    PFN_wglMakeCurrent                  MakeCurrent;
    PFN_wglShareLists                   ShareLists;

    PFNWGLSWAPINTERVALEXTPROC           SwapIntervalEXT;
    PFNWGLGETPIXELFORMATATTRIBIVARBPROC GetPixelFormatAttribivARB;
    PFNWGLGETEXTENSIONSSTRINGEXTPROC    GetExtensionsStringEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC    GetExtensionsStringARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC   CreateContextAttribsARB;
    LUMbool                            EXT_swap_control;
    LUMbool                            EXT_colorspace;
    LUMbool                            ARB_multisample;
    LUMbool                            ARB_framebuffer_sRGB;
    LUMbool                            EXT_framebuffer_sRGB;
    LUMbool                            ARB_pixel_format;
    LUMbool                            ARB_create_context;
    LUMbool                            ARB_create_context_profile;
    LUMbool                            EXT_create_context_es2_profile;
    LUMbool                            ARB_create_context_robustness;
    LUMbool                            ARB_create_context_no_error;
    LUMbool                            ARB_context_flush_control;
} _LUMlibraryWGL;

// Win32-specific per-window data
//
typedef struct _LUMwindowWin32
{
    HWND                handle;
    HICON               bigIcon;
    HICON               smallIcon;

    LUMbool            cursorTracked;
    LUMbool            frameAction;
    LUMbool            iconified;
    LUMbool            maximized;
    // Whether to enable framebuffer transparency on DWM
    LUMbool            transparent;
    LUMbool            scaleToMonitor;
    LUMbool            keymenu;

    // Cached size used to filter out duplicate events
    int                 width, height;

    // The last received cursor position, regardless of source
    int                 lastCursorPosX, lastCursorPosY;
    // The last received high surrogate when decoding pairs of UTF-16 messages
    WCHAR               highSurrogate;
} _LUMwindowWin32;

// Win32-specific global data
//
typedef struct _LUMlibraryWin32
{
    HWND                helperWindowHandle;
    HDEVNOTIFY          deviceNotificationHandle;
    int                 acquiredMonitorCount;
    char*               clipboardString;
    short int           keycodes[512];
    short int           scancodes[LUM_KEY_LAST + 1];
    char                keynames[LUM_KEY_LAST + 1][5];
    // Where to place the cursor when re-enabled
    double              restoreCursorPosX, restoreCursorPosY;
    // The window whose disabled cursor mode is active
    _LUMwindow*        disabledCursorWindow;
    RAWINPUT*           rawInput;
    int                 rawInputSize;
    UINT                mouseTrailSize;

    struct {
        HINSTANCE                       instance;
        PFN_DirectInput8Create          Create;
        IDirectInput8W*                 api;
    } dinput8;

    struct {
        HINSTANCE                       instance;
        PFN_XInputGetCapabilities       GetCapabilities;
        PFN_XInputGetState              GetState;
    } xinput;

    struct {
        HINSTANCE                       instance;
        PFN_SetProcessDPIAware          SetProcessDPIAware_;
        PFN_ChangeWindowMessageFilterEx ChangeWindowMessageFilterEx_;
        PFN_EnableNonClientDpiScaling   EnableNonClientDpiScaling_;
        PFN_SetProcessDpiAwarenessContext SetProcessDpiAwarenessContext_;
        PFN_GetDpiForWindow             GetDpiForWindow_;
        PFN_AdjustWindowRectExForDpi    AdjustWindowRectExForDpi_;
    } user32;

    struct {
        HINSTANCE                       instance;
        PFN_DwmIsCompositionEnabled     IsCompositionEnabled;
        PFN_DwmFlush                    Flush;
        PFN_DwmEnableBlurBehindWindow   EnableBlurBehindWindow;
        PFN_DwmGetColorizationColor     GetColorizationColor;
    } dwmapi;

    struct {
        HINSTANCE                       instance;
        PFN_SetProcessDpiAwareness      SetProcessDpiAwareness_;
        PFN_GetDpiForMonitor            GetDpiForMonitor_;
    } shcore;

    struct {
        HINSTANCE                       instance;
        PFN_RtlVerifyVersionInfo        RtlVerifyVersionInfo_;
    } ntdll;
} _LUMlibraryWin32;

// Win32-specific per-monitor data
//
typedef struct _LUMmonitorWin32
{
    HMONITOR            handle;
    // This size matches the static size of DISPLAY_DEVICE.DeviceName
    WCHAR               adapterName[32];
    WCHAR               displayName[32];
    char                publicAdapterName[32];
    char                publicDisplayName[32];
    LUMbool            modesPruned;
    LUMbool            modeChanged;
} _LUMmonitorWin32;

// Win32-specific per-cursor data
//
typedef struct _LUMcursorWin32
{
    HCURSOR             handle;
} _LUMcursorWin32;


LUMbool _lumConnectWin32(int platformID, _LUMplatform* platform);
int _lumInitWin32(void);
void _lumTerminateWin32(void);

LUMbool _lumRegisterWindowClassWin32(void);
void _lumUnregisterWindowClassWin32(void);

WCHAR* _lumCreateWideStringFromUTF8Win32(const char* source);
char* _lumCreateUTF8FromWideStringWin32(const WCHAR* source);
BOOL _lumIsWindowsVersionOrGreaterWin32(WORD major, WORD minor, WORD sp);
BOOL _lumIsWindows10BuildOrGreaterWin32(WORD build);
void _lumInputErrorWin32(int error, const char* description);
void _lumUpdateKeyNamesWin32(void);

void _lumPollMonitorsWin32(void);
void _lumSetVideoModeWin32(_LUMmonitor* monitor, const LUMvidmode* desired);
void _lumRestoreVideoModeWin32(_LUMmonitor* monitor);
void _lumGetHMONITORContentScaleWin32(HMONITOR handle, float* xscale, float* yscale);

int _lumCreateWindowWin32(_LUMwindow* window, const _LUMwndconfig* wndconfig, const _LUMctxconfig* ctxconfig, const _LUMfbconfig* fbconfig);
void _lumDestroyWindowWin32(_LUMwindow* window);
void _lumSetWindowTitleWin32(_LUMwindow* window, const char* title);
void _lumSetWindowIconWin32(_LUMwindow* window, int count, const LUMimage* images);
void _lumGetWindowPosWin32(_LUMwindow* window, int* xpos, int* ypos);
void _lumSetWindowPosWin32(_LUMwindow* window, int xpos, int ypos);
void _lumGetWindowSizeWin32(_LUMwindow* window, int* width, int* height);
void _lumSetWindowSizeWin32(_LUMwindow* window, int width, int height);
void _lumSetWindowSizeLimitsWin32(_LUMwindow* window, int minwidth, int minheight, int maxwidth, int maxheight);
void _lumSetWindowAspectRatioWin32(_LUMwindow* window, int numer, int denom);
void _lumGetFramebufferSizeWin32(_LUMwindow* window, int* width, int* height);
void _lumGetWindowFrameSizeWin32(_LUMwindow* window, int* left, int* top, int* right, int* bottom);
void _lumGetWindowContentScaleWin32(_LUMwindow* window, float* xscale, float* yscale);
void _lumIconifyWindowWin32(_LUMwindow* window);
void _lumRestoreWindowWin32(_LUMwindow* window);
void _lumMaximizeWindowWin32(_LUMwindow* window);
void _lumShowWindowWin32(_LUMwindow* window);
void _lumHideWindowWin32(_LUMwindow* window);
void _lumRequestWindowAttentionWin32(_LUMwindow* window);
void _lumFocusWindowWin32(_LUMwindow* window);
void _lumSetWindowMonitorWin32(_LUMwindow* window, _LUMmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate);
int _lumWindowFocusedWin32(_LUMwindow* window);
int _lumWindowIconifiedWin32(_LUMwindow* window);
int _lumWindowVisibleWin32(_LUMwindow* window);
int _lumWindowMaximizedWin32(_LUMwindow* window);
int _lumWindowHoveredWin32(_LUMwindow* window);
int _lumFramebufferTransparentWin32(_LUMwindow* window);
void _lumSetWindowResizableWin32(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowDecoratedWin32(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowFloatingWin32(_LUMwindow* window, LUMbool enabled);
void _lumSetWindowMousePassthroughWin32(_LUMwindow* window, LUMbool enabled);
float _lumGetWindowOpacityWin32(_LUMwindow* window);
void _lumSetWindowOpacityWin32(_LUMwindow* window, float opacity);

void _lumSetRawMouseMotionWin32(_LUMwindow *window, LUMbool enabled);
LUMbool _lumRawMouseMotionSupportedWin32(void);

void _lumPollEventsWin32(void);
void _lumWaitEventsWin32(void);
void _lumWaitEventsTimeoutWin32(double timeout);
void _lumPostEmptyEventWin32(void);

// Hazel
void _lumSetWindowTitlebarWin32(_LUMwindow* window, LUMbool enabled);

void _lumGetCursorPosWin32(_LUMwindow* window, double* xpos, double* ypos);
void _lumSetCursorPosWin32(_LUMwindow* window, double xpos, double ypos);
void _lumSetCursorModeWin32(_LUMwindow* window, int mode);
const char* _lumGetScancodeNameWin32(int scancode);
int _lumGetKeyScancodeWin32(int key);
int _lumCreateCursorWin32(_LUMcursor* cursor, const LUMimage* image, int xhot, int yhot);
int _lumCreateStandardCursorWin32(_LUMcursor* cursor, int shape);
void _lumDestroyCursorWin32(_LUMcursor* cursor);
void _lumSetCursorWin32(_LUMwindow* window, _LUMcursor* cursor);
void _lumSetClipboardStringWin32(const char* string);
const char* _lumGetClipboardStringWin32(void);

EGLenum _lumGetEGLPlatformWin32(EGLint** attribs);
EGLNativeDisplayType _lumGetEGLNativeDisplayWin32(void);
EGLNativeWindowType _lumGetEGLNativeWindowWin32(_LUMwindow* window);

void _lumGetRequiredInstanceExtensionsWin32(char** extensions);
int _lumGetPhysicalDevicePresentationSupportWin32(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily);
VkResult _lumCreateWindowSurfaceWin32(VkInstance instance, _LUMwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);

void _lumFreeMonitorWin32(_LUMmonitor* monitor);
void _lumGetMonitorPosWin32(_LUMmonitor* monitor, int* xpos, int* ypos);
void _lumGetMonitorContentScaleWin32(_LUMmonitor* monitor, float* xscale, float* yscale);
void _lumGetMonitorWorkareaWin32(_LUMmonitor* monitor, int* xpos, int* ypos, int* width, int* height);
LUMvidmode* _lumGetVideoModesWin32(_LUMmonitor* monitor, int* count);
void _lumGetVideoModeWin32(_LUMmonitor* monitor, LUMvidmode* mode);
LUMbool _lumGetGammaRampWin32(_LUMmonitor* monitor, LUMgammaramp* ramp);
void _lumSetGammaRampWin32(_LUMmonitor* monitor, const LUMgammaramp* ramp);

LUMbool _lumInitJoysticksWin32(void);
void _lumTerminateJoysticksWin32(void);
int _lumPollJoystickWin32(_LUMjoystick* js, int mode);
const char* _lumGetMappingNameWin32(void);
void _lumUpdateGamepadGUIDWin32(char* guid);

LUMbool _lumInitWGL(void);
void _lumTerminateWGL(void);
LUMbool _lumCreateContextWGL(_LUMwindow* window,
                               const _LUMctxconfig* ctxconfig,
                               const _LUMfbconfig* fbconfig);

