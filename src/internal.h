//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
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

#pragma once

#if defined(_LUM_USE_CONFIG_H)
 #include "lum_config.h"
#endif

#if defined(LUM_INCLUDE_GLCOREARB) || \
    defined(LUM_INCLUDE_ES1)       || \
    defined(LUM_INCLUDE_ES2)       || \
    defined(LUM_INCLUDE_ES3)       || \
    defined(LUM_INCLUDE_ES31)      || \
    defined(LUM_INCLUDE_ES32)      || \
    defined(LUM_INCLUDE_NONE)      || \
    defined(LUM_INCLUDE_GLEXT)     || \
    defined(LUM_INCLUDE_GLU)       || \
    defined(LUM_INCLUDE_VULKAN)    || \
    defined(LUM_DLL)
 #error "You must not define any header option macros when compiling Lumina"
#endif

#define LUM_INCLUDE_NONE
#include "../include/Lumina/lumina.h"

#define _LUM_INSERT_FIRST      0
#define _LUM_INSERT_LAST       1

#define _LUM_POLL_PRESENCE     0
#define _LUM_POLL_AXES         1
#define _LUM_POLL_BUTTONS      2
#define _LUM_POLL_ALL          (_LUM_POLL_AXES | _LUM_POLL_BUTTONS)

#define _LUM_MESSAGE_SIZE      1024

typedef int LUMbool;
typedef void (*LUMproc)(void);

typedef struct _LUMerror       _LUMerror;
typedef struct _LUMinitconfig  _LUMinitconfig;
typedef struct _LUMwndconfig   _LUMwndconfig;
typedef struct _LUMctxconfig   _LUMctxconfig;
typedef struct _LUMfbconfig    _LUMfbconfig;
typedef struct _LUMcontext     _LUMcontext;
typedef struct _LUMwindow      _LUMwindow;
typedef struct _LUMplatform    _LUMplatform;
typedef struct _LUMlibrary     _LUMlibrary;
typedef struct _LUMmonitor     _LUMmonitor;
typedef struct _LUMcursor      _LUMcursor;
typedef struct _LUMmapelement  _LUMmapelement;
typedef struct _LUMmapping     _LUMmapping;
typedef struct _LUMjoystick    _LUMjoystick;
typedef struct _LUMtls         _LUMtls;
typedef struct _LUMmutex       _LUMmutex;

#define GL_VERSION 0x1f02
#define GL_NONE 0
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_UNSIGNED_BYTE 0x1401
#define GL_EXTENSIONS 0x1f03
#define GL_NUM_EXTENSIONS 0x821d
#define GL_CONTEXT_FLAGS 0x821e
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#define GL_CONTEXT_PROFILE_MASK 0x9126
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#define GL_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GL_NO_RESET_NOTIFICATION_ARB 0x8261
#define GL_CONTEXT_RELEASE_BEHAVIOR 0x82fb
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82fc
#define GL_CONTEXT_FLAG_NO_ERROR_BIT_KHR 0x00000008

typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned char GLubyte;

typedef void (APIENTRY * PFNGLCLEARPROC)(GLbitfield);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGPROC)(GLenum);
typedef void (APIENTRY * PFNGLGETINTEGERVPROC)(GLenum,GLint*);
typedef const GLubyte* (APIENTRY * PFNGLGETSTRINGIPROC)(GLenum,GLuint);

#if defined(_LUM_WIN32)
 #define EGLAPIENTRY __stdcall
#else
 #define EGLAPIENTRY
#endif

#define EGL_SUCCESS 0x3000
#define EGL_NOT_INITIALIZED 0x3001
#define EGL_BAD_ACCESS 0x3002
#define EGL_BAD_ALLOC 0x3003
#define EGL_BAD_ATTRIBUTE 0x3004
#define EGL_BAD_CONFIG 0x3005
#define EGL_BAD_CONTEXT 0x3006
#define EGL_BAD_CURRENT_SURFACE 0x3007
#define EGL_BAD_DISPLAY 0x3008
#define EGL_BAD_MATCH 0x3009
#define EGL_BAD_NATIVE_PIXMAP 0x300a
#define EGL_BAD_NATIVE_WINDOW 0x300b
#define EGL_BAD_PARAMETER 0x300c
#define EGL_BAD_SURFACE 0x300d
#define EGL_CONTEXT_LOST 0x300e
#define EGL_COLOR_BUFFER_TYPE 0x303f
#define EGL_RGB_BUFFER 0x308e
#define EGL_SURFACE_TYPE 0x3033
#define EGL_WINDOW_BIT 0x0004
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_BIT 0x0008
#define EGL_ALPHA_SIZE 0x3021
#define EGL_BLUE_SIZE 0x3022
#define EGL_GREEN_SIZE 0x3023
#define EGL_RED_SIZE 0x3024
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3026
#define EGL_SAMPLES 0x3031
#define EGL_OPENGL_ES_API 0x30a0
#define EGL_OPENGL_API 0x30a2
#define EGL_NONE 0x3038
#define EGL_RENDER_BUFFER 0x3086
#define EGL_SINGLE_BUFFER 0x3085
#define EGL_EXTENSIONS 0x3055
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_NATIVE_VISUAL_ID 0x302e
#define EGL_NO_SURFACE ((EGLSurface) 0)
#define EGL_NO_DISPLAY ((EGLDisplay) 0)
#define EGL_NO_CONTEXT ((EGLContext) 0)
#define EGL_DEFAULT_DISPLAY ((EGLNativeDisplayType) 0)

#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE_BIT_KHR 0x00000002
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR 0x00000002
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR 0x00000001
#define EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_KHR 0x31bd
#define EGL_NO_RESET_NOTIFICATION_KHR 0x31be
#define EGL_LOSE_CONTEXT_ON_RESET_KHR 0x31bf
#define EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR 0x00000004
#define EGL_CONTEXT_MAJOR_VERSION_KHR 0x3098
#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30fb
#define EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR 0x30fd
#define EGL_CONTEXT_FLAGS_KHR 0x30fc
#define EGL_CONTEXT_OPENGL_NO_ERROR_KHR 0x31b3
#define EGL_GL_COLORSPACE_KHR 0x309d
#define EGL_GL_COLORSPACE_SRGB_KHR 0x3089
#define EGL_CONTEXT_RELEASE_BEHAVIOR_KHR 0x2097
#define EGL_CONTEXT_RELEASE_BEHAVIOR_NONE_KHR 0
#define EGL_CONTEXT_RELEASE_BEHAVIOR_FLUSH_KHR 0x2098
#define EGL_PLATFORM_X11_EXT 0x31d5
#define EGL_PLATFORM_WAYLAND_EXT 0x31d8
#define EGL_PRESENT_OPAQUE_EXT 0x31df
#define EGL_PLATFORM_ANGLE_ANGLE 0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE 0x3203
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE 0x320d
#define EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE 0x320e
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE 0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE 0x3208
#define EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE 0x3450
#define EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE 0x3489
#define EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE 0x348f

typedef int EGLint;
typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void* EGLConfig;
typedef void* EGLContext;
typedef void* EGLDisplay;
typedef void* EGLSurface;

typedef void* EGLNativeDisplayType;
typedef void* EGLNativeWindowType;

// EGL function pointer typedefs
typedef EGLBoolean (EGLAPIENTRY * PFN_eglGetConfigAttrib)(EGLDisplay,EGLConfig,EGLint,EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglGetConfigs)(EGLDisplay,EGLConfig*,EGLint,EGLint*);
typedef EGLDisplay (EGLAPIENTRY * PFN_eglGetDisplay)(EGLNativeDisplayType);
typedef EGLint (EGLAPIENTRY * PFN_eglGetError)(void);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglInitialize)(EGLDisplay,EGLint*,EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglTerminate)(EGLDisplay);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglBindAPI)(EGLenum);
typedef EGLContext (EGLAPIENTRY * PFN_eglCreateContext)(EGLDisplay,EGLConfig,EGLContext,const EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglDestroySurface)(EGLDisplay,EGLSurface);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglDestroyContext)(EGLDisplay,EGLContext);
typedef EGLSurface (EGLAPIENTRY * PFN_eglCreateWindowSurface)(EGLDisplay,EGLConfig,EGLNativeWindowType,const EGLint*);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglMakeCurrent)(EGLDisplay,EGLSurface,EGLSurface,EGLContext);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglSwapBuffers)(EGLDisplay,EGLSurface);
typedef EGLBoolean (EGLAPIENTRY * PFN_eglSwapInterval)(EGLDisplay,EGLint);
typedef const char* (EGLAPIENTRY * PFN_eglQueryString)(EGLDisplay,EGLint);
typedef LUMglproc (EGLAPIENTRY * PFN_eglGetProcAddress)(const char*);
#define eglGetConfigAttrib _lum.egl.GetConfigAttrib
#define eglGetConfigs _lum.egl.GetConfigs
#define eglGetDisplay _lum.egl.GetDisplay
#define eglGetError _lum.egl.GetError
#define eglInitialize _lum.egl.Initialize
#define eglTerminate _lum.egl.Terminate
#define eglBindAPI _lum.egl.BindAPI
#define eglCreateContext _lum.egl.CreateContext
#define eglDestroySurface _lum.egl.DestroySurface
#define eglDestroyContext _lum.egl.DestroyContext
#define eglCreateWindowSurface _lum.egl.CreateWindowSurface
#define eglMakeCurrent _lum.egl.MakeCurrent
#define eglSwapBuffers _lum.egl.SwapBuffers
#define eglSwapInterval _lum.egl.SwapInterval
#define eglQueryString _lum.egl.QueryString
#define eglGetProcAddress _lum.egl.GetProcAddress

typedef EGLDisplay (EGLAPIENTRY * PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum,void*,const EGLint*);
typedef EGLSurface (EGLAPIENTRY * PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)(EGLDisplay,EGLConfig,void*,const EGLint*);
#define eglGetPlatformDisplayEXT _lum.egl.GetPlatformDisplayEXT
#define eglCreatePlatformWindowSurfaceEXT _lum.egl.CreatePlatformWindowSurfaceEXT

#define OSMESA_RGBA 0x1908
#define OSMESA_FORMAT 0x22
#define OSMESA_DEPTH_BITS 0x30
#define OSMESA_STENCIL_BITS 0x31
#define OSMESA_ACCUM_BITS 0x32
#define OSMESA_PROFILE 0x33
#define OSMESA_CORE_PROFILE 0x34
#define OSMESA_COMPAT_PROFILE 0x35
#define OSMESA_CONTEXT_MAJOR_VERSION 0x36
#define OSMESA_CONTEXT_MINOR_VERSION 0x37

typedef void* OSMesaContext;
typedef void (*OSMESAproc)(void);

typedef OSMesaContext (GLAPIENTRY * PFN_OSMesaCreateContextExt)(GLenum,GLint,GLint,GLint,OSMesaContext);
typedef OSMesaContext (GLAPIENTRY * PFN_OSMesaCreateContextAttribs)(const int*,OSMesaContext);
typedef void (GLAPIENTRY * PFN_OSMesaDestroyContext)(OSMesaContext);
typedef int (GLAPIENTRY * PFN_OSMesaMakeCurrent)(OSMesaContext,void*,int,int,int);
typedef int (GLAPIENTRY * PFN_OSMesaGetColorBuffer)(OSMesaContext,int*,int*,int*,void**);
typedef int (GLAPIENTRY * PFN_OSMesaGetDepthBuffer)(OSMesaContext,int*,int*,int*,void**);
typedef LUMglproc (GLAPIENTRY * PFN_OSMesaGetProcAddress)(const char*);
#define OSMesaCreateContextExt _lum.osmesa.CreateContextExt
#define OSMesaCreateContextAttribs _lum.osmesa.CreateContextAttribs
#define OSMesaDestroyContext _lum.osmesa.DestroyContext
#define OSMesaMakeCurrent _lum.osmesa.MakeCurrent
#define OSMesaGetColorBuffer _lum.osmesa.GetColorBuffer
#define OSMesaGetDepthBuffer _lum.osmesa.GetDepthBuffer
#define OSMesaGetProcAddress _lum.osmesa.GetProcAddress

#define VK_NULL_HANDLE 0

typedef void* VkInstance;
typedef void* VkPhysicalDevice;
typedef uint64_t VkSurfaceKHR;
typedef uint32_t VkFlags;
typedef uint32_t VkBool32;

typedef enum VkStructureType
{
    VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR = 1000004000,
    VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR = 1000005000,
    VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR = 1000006000,
    VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR = 1000009000,
    VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK = 1000123000,
    VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT = 1000217000,
    VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} VkStructureType;

typedef enum VkResult
{
    VK_SUCCESS = 0,
    VK_NOT_READY = 1,
    VK_TIMEOUT = 2,
    VK_EVENT_SET = 3,
    VK_EVENT_RESET = 4,
    VK_INCOMPLETE = 5,
    VK_ERROR_OUT_OF_HOST_MEMORY = -1,
    VK_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    VK_ERROR_INITIALIZATION_FAILED = -3,
    VK_ERROR_DEVICE_LOST = -4,
    VK_ERROR_MEMORY_MAP_FAILED = -5,
    VK_ERROR_LAYER_NOT_PRESENT = -6,
    VK_ERROR_EXTENSION_NOT_PRESENT = -7,
    VK_ERROR_FEATURE_NOT_PRESENT = -8,
    VK_ERROR_INCOMPATIBLE_DRIVER = -9,
    VK_ERROR_TOO_MANY_OBJECTS = -10,
    VK_ERROR_FORMAT_NOT_SUPPORTED = -11,
    VK_ERROR_SURFACE_LOST_KHR = -1000000000,
    VK_SUBOPTIMAL_KHR = 1000001003,
    VK_ERROR_OUT_OF_DATE_KHR = -1000001004,
    VK_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000003001,
    VK_ERROR_NATIVE_WINDOW_IN_USE_KHR = -1000000001,
    VK_ERROR_VALIDATION_FAILED_EXT = -1000011001,
    VK_RESULT_MAX_ENUM = 0x7FFFFFFF
} VkResult;

typedef struct VkAllocationCallbacks VkAllocationCallbacks;

typedef struct VkExtensionProperties
{
    char            extensionName[256];
    uint32_t        specVersion;
} VkExtensionProperties;

typedef void (APIENTRY * PFN_vkVoidFunction)(void);

typedef PFN_vkVoidFunction (APIENTRY * PFN_vkGetInstanceProcAddr)(VkInstance,const char*);
typedef VkResult (APIENTRY * PFN_vkEnumerateInstanceExtensionProperties)(const char*,uint32_t*,VkExtensionProperties*);
#define vkGetInstanceProcAddr _lum.vk.GetInstanceProcAddr

#include "platform.h"

// Constructs a version number string from the public header macros
#define _LUM_CONCAT_VERSION(m, n, r) #m "." #n "." #r
#define _LUM_MAKE_VERSION(m, n, r) _LUM_CONCAT_VERSION(m, n, r)
#define _LUM_VERSION_NUMBER _LUM_MAKE_VERSION(LUM_VERSION_MAJOR, \
                                                LUM_VERSION_MINOR, \
                                                LUM_VERSION_REVISION)

// Checks for whether the library has been initialized
#define _LUM_REQUIRE_INIT()                         \
    if (!_lum.initialized)                          \
    {                                                \
        _lumInputError(LUM_NOT_INITIALIZED, NULL); \
        return;                                      \
    }
#define _LUM_REQUIRE_INIT_OR_RETURN(x)              \
    if (!_lum.initialized)                          \
    {                                                \
        _lumInputError(LUM_NOT_INITIALIZED, NULL); \
        return x;                                    \
    }

// Swaps the provided pointers
#define _LUM_SWAP(type, x, y) \
    {                          \
        type t;                \
        t = x;                 \
        x = y;                 \
        y = t;                 \
    }

// Per-thread error structure
//
struct _LUMerror
{
    _LUMerror*     next;
    int             code;
    char            description[_LUM_MESSAGE_SIZE];
};

// Initialization configuration
//
// Parameters relating to the initialization of the library
//
struct _LUMinitconfig
{
    LUMbool      hatButtons;
    int           angleType;
    int           platformID;
    PFN_vkGetInstanceProcAddr vulkanLoader;
    struct {
        LUMbool  menubar;
        LUMbool  chdir;
    } ns;
    struct {
        LUMbool  xcbVulkanSurface;
    } x11;
};

// Window configuration
//
// Parameters relating to the creation of the window but not directly related
// to the framebuffer.  This is used to pass window creation parameters from
// shared code to the platform API.
//
struct _LUMwndconfig
{
    int           width;
    int           height;
    const char*   title;
    LUMbool      resizable;
    LUMbool      visible;
    LUMbool      decorated;
    LUMbool      titlebar;
    LUMbool      focused;
    LUMbool      autoIconify;
    LUMbool      floating;
    LUMbool      maximized;
    LUMbool      centerCursor;
    LUMbool      focusOnShow;
    LUMbool      mousePassthrough;
    LUMbool      scaleToMonitor;
    struct {
        LUMbool  retina;
        char      frameName[256];
    } ns;
    struct {
        char      className[256];
        char      instanceName[256];
    } x11;
    struct {
        LUMbool  keymenu;
    } win32;
};

// Context configuration
//
// Parameters relating to the creation of the context but not directly related
// to the framebuffer.  This is used to pass context creation parameters from
// shared code to the platform API.
//
struct _LUMctxconfig
{
    int           client;
    int           source;
    int           major;
    int           minor;
    LUMbool      forward;
    LUMbool      debug;
    LUMbool      noerror;
    int           profile;
    int           robustness;
    int           release;
    _LUMwindow*  share;
    struct {
        LUMbool  offline;
    } nsgl;
};

// Framebuffer configuration
//
// This describes buffers and their sizes.  It also contains
// a platform-specific ID used to map back to the backend API object.
//
// It is used to pass framebuffer parameters from shared code to the platform
// API and also to enumerate and select available framebuffer configs.
//
struct _LUMfbconfig
{
    int         redBits;
    int         greenBits;
    int         blueBits;
    int         alphaBits;
    int         depthBits;
    int         stencilBits;
    int         accumRedBits;
    int         accumGreenBits;
    int         accumBlueBits;
    int         accumAlphaBits;
    int         auxBuffers;
    LUMbool    stereo;
    int         samples;
    LUMbool    sRGB;
    LUMbool    doublebuffer;
    LUMbool    transparent;
    uintptr_t   handle;
};

// Context structure
//
struct _LUMcontext
{
    int                 client;
    int                 source;
    int                 major, minor, revision;
    LUMbool            forward, debug, noerror;
    int                 profile;
    int                 robustness;
    int                 release;

    PFNGLGETSTRINGIPROC  GetStringi;
    PFNGLGETINTEGERVPROC GetIntegerv;
    PFNGLGETSTRINGPROC   GetString;

    void (*makeCurrent)(_LUMwindow*);
    void (*swapBuffers)(_LUMwindow*);
    void (*swapInterval)(int);
    int (*extensionSupported)(const char*);
    LUMglproc (*getProcAddress)(const char*);
    void (*destroy)(_LUMwindow*);

    struct {
        EGLConfig       config;
        EGLContext      handle;
        EGLSurface      surface;
        void*           client;
    } egl;

    struct {
        OSMesaContext   handle;
        int             width;
        int             height;
        void*           buffer;
    } osmesa;

    // This is defined in platform.h
    LUM_PLATFORM_CONTEXT_STATE
};

// Window and context structure
//
struct _LUMwindow
{
    struct _LUMwindow* next;

    // Window settings and state
    LUMbool            resizable;
    LUMbool            decorated;
    LUMbool            autoIconify;
    LUMbool            floating;
    LUMbool            focusOnShow;
    LUMbool            mousePassthrough;
    LUMbool            shouldClose;
    void*               userPointer;
    LUMbool            doublebuffer;
    LUMvidmode         videoMode;
    _LUMmonitor*       monitor;
    _LUMcursor*        cursor;

    int                 minwidth, minheight;
    int                 maxwidth, maxheight;
    int                 numer, denom;

    LUMbool            stickyKeys;
    LUMbool            stickyMouseButtons;
    LUMbool            lockKeyMods;
    int                 cursorMode;
    char                mouseButtons[LUM_MOUSE_BUTTON_LAST + 1];
    char                keys[LUM_KEY_LAST + 1];
    // Virtual cursor position when cursor is disabled
    double              virtualCursorPosX, virtualCursorPosY;
    LUMbool            rawMouseMotion;

    _LUMcontext        context;

    struct {
        LUMwindowposfun        pos;
        LUMtitlebarhittestfun  tbhittest;
        LUMwindowsizefun       size;
        LUMwindowclosefun      close;
        LUMwindowrefreshfun    refresh;
        LUMwindowfocusfun      focus;
        LUMwindowiconifyfun    iconify;
        LUMwindowmaximizefun   maximize;
        LUMframebuffersizefun  fbsize;
        LUMwindowcontentscalefun scale;
        LUMmousebuttonfun        mouseButton;
        LUMcursorposfun          cursorPos;
        LUMcursorenterfun        cursorEnter;
        LUMscrollfun             scroll;
        LUMkeyfun                key;
        LUMcharfun               character;
        LUMcharmodsfun           charmods;
        LUMdropfun               drop;
    } callbacks;

    // This is defined in platform.h
    LUM_PLATFORM_WINDOW_STATE
};

// Monitor structure
//
struct _LUMmonitor
{
    char            name[128];
    void*           userPointer;

    // Physical dimensions in millimeters.
    int             widthMM, heightMM;

    // The window whose video mode is current on this monitor
    _LUMwindow*    window;

    LUMvidmode*    modes;
    int             modeCount;
    LUMvidmode     currentMode;

    LUMgammaramp   originalRamp;
    LUMgammaramp   currentRamp;

    // This is defined in platform.h
    LUM_PLATFORM_MONITOR_STATE
};

// Cursor structure
//
struct _LUMcursor
{
    _LUMcursor*    next;
    // This is defined in platform.h
    LUM_PLATFORM_CURSOR_STATE
};

// Gamepad mapping element structure
//
struct _LUMmapelement
{
    uint8_t         type;
    uint8_t         index;
    int8_t          axisScale;
    int8_t          axisOffset;
};

// Gamepad mapping structure
//
struct _LUMmapping
{
    char            name[128];
    char            guid[33];
    _LUMmapelement buttons[15];
    _LUMmapelement axes[6];
};

// Joystick structure
//
struct _LUMjoystick
{
    LUMbool        present;
    float*          axes;
    int             axisCount;
    unsigned char*  buttons;
    int             buttonCount;
    unsigned char*  hats;
    int             hatCount;
    char            name[128];
    void*           userPointer;
    char            guid[33];
    _LUMmapping*   mapping;

    // This is defined in platform.h
    LUM_PLATFORM_JOYSTICK_STATE
};

// Thread local storage structure
//
struct _LUMtls
{
    // This is defined in platform.h
    LUM_PLATFORM_TLS_STATE
};

// Mutex structure
//
struct _LUMmutex
{
    // This is defined in platform.h
    LUM_PLATFORM_MUTEX_STATE
};

// Platform API structure
//
struct _LUMplatform
{
    int platformID;
    // init
    LUMbool (*init)(void);
    void (*terminate)(void);
    // input
    void (*getCursorPos)(_LUMwindow*,double*,double*);
    void (*setCursorPos)(_LUMwindow*,double,double);
    void (*setCursorMode)(_LUMwindow*,int);
    void (*setRawMouseMotion)(_LUMwindow*,LUMbool);
    LUMbool (*rawMouseMotionSupported)(void);
    int (*createCursor)(_LUMcursor*,const LUMimage*,int,int);
    int (*createStandardCursor)(_LUMcursor*,int);
    void (*destroyCursor)(_LUMcursor*);
    void (*setCursor)(_LUMwindow*,_LUMcursor*);
    const char* (*getScancodeName)(int);
    int (*getKeyScancode)(int);
    void (*setClipboardString)(const char*);
    const char* (*getClipboardString)(void);
    LUMbool (*initJoysticks)(void);
    void (*terminateJoysticks)(void);
    int (*pollJoystick)(_LUMjoystick*,int);
    const char* (*getMappingName)(void);
    void (*updateGamepadGUID)(char*);
    // monitor
    void (*freeMonitor)(_LUMmonitor*);
    void (*getMonitorPos)(_LUMmonitor*,int*,int*);
    void (*getMonitorContentScale)(_LUMmonitor*,float*,float*);
    void (*getMonitorWorkarea)(_LUMmonitor*,int*,int*,int*,int*);
    LUMvidmode* (*getVideoModes)(_LUMmonitor*,int*);
    void (*getVideoMode)(_LUMmonitor*,LUMvidmode*);
    LUMbool (*getGammaRamp)(_LUMmonitor*,LUMgammaramp*);
    void (*setGammaRamp)(_LUMmonitor*,const LUMgammaramp*);
    // window
    int (*createWindow)(_LUMwindow*,const _LUMwndconfig*,const _LUMctxconfig*,const _LUMfbconfig*);
    void (*destroyWindow)(_LUMwindow*);
    void (*setWindowTitle)(_LUMwindow*,const char*);
    void (*setWindowIcon)(_LUMwindow*,int,const LUMimage*);
    void (*getWindowPos)(_LUMwindow*,int*,int*);
    void (*setWindowPos)(_LUMwindow*,int,int);
    void (*getWindowSize)(_LUMwindow*,int*,int*);
    void (*setWindowSize)(_LUMwindow*,int,int);
    void (*setWindowSizeLimits)(_LUMwindow*,int,int,int,int);
    void (*setWindowAspectRatio)(_LUMwindow*,int,int);
    void (*getFramebufferSize)(_LUMwindow*,int*,int*);
    void (*getWindowFrameSize)(_LUMwindow*,int*,int*,int*,int*);
    void (*getWindowContentScale)(_LUMwindow*,float*,float*);
    void (*iconifyWindow)(_LUMwindow*);
    void (*restoreWindow)(_LUMwindow*);
    void (*maximizeWindow)(_LUMwindow*);
    void (*showWindow)(_LUMwindow*);
    void (*hideWindow)(_LUMwindow*);
    void (*requestWindowAttention)(_LUMwindow*);
    void (*focusWindow)(_LUMwindow*);
    void (*setWindowMonitor)(_LUMwindow*,_LUMmonitor*,int,int,int,int,int);
    int (*windowFocused)(_LUMwindow*);
    int (*windowIconified)(_LUMwindow*);
    int (*windowVisible)(_LUMwindow*);
    int (*windowMaximized)(_LUMwindow*);
    int (*windowHovered)(_LUMwindow*);
    int (*framebufferTransparent)(_LUMwindow*);
    float (*getWindowOpacity)(_LUMwindow*);
    void (*setWindowResizable)(_LUMwindow*,LUMbool);
    void (*setWindowDecorated)(_LUMwindow*,LUMbool);
    void (*setWindowFloating)(_LUMwindow*,LUMbool);
    void (*setWindowOpacity)(_LUMwindow*,float);
    void (*setWindowMousePassthrough)(_LUMwindow*,LUMbool);
    void (*pollEvents)(void);
    void (*waitEvents)(void);
    void (*waitEventsTimeout)(double);
    void (*postEmptyEvent)(void);

    // Hazel
    void (*setWindowTitleBar)(_LUMwindow*,LUMbool);

    // EGL
    EGLenum (*getEGLPlatform)(EGLint**);
    EGLNativeDisplayType (*getEGLNativeDisplay)(void);
    EGLNativeWindowType (*getEGLNativeWindow)(_LUMwindow*);
    // vulkan
    void (*getRequiredInstanceExtensions)(char**);
    int (*getPhysicalDevicePresentationSupport)(VkInstance,VkPhysicalDevice,uint32_t);
    VkResult (*createWindowSurface)(VkInstance,_LUMwindow*,const VkAllocationCallbacks*,VkSurfaceKHR*);
};

// Library global data
//
struct _LUMlibrary
{
    LUMbool            initialized;
    LUMallocator       allocator;

    _LUMplatform       platform;

    struct {
        _LUMinitconfig init;
        _LUMfbconfig   framebuffer;
        _LUMwndconfig  window;
        _LUMctxconfig  context;
        int             refreshRate;
    } hints;

    _LUMerror*         errorListHead;
    _LUMcursor*        cursorListHead;
    _LUMwindow*        windowListHead;

    _LUMmonitor**      monitors;
    int                 monitorCount;

    LUMbool            joysticksInitialized;
    _LUMjoystick       joysticks[LUM_JOYSTICK_LAST + 1];
    _LUMmapping*       mappings;
    int                 mappingCount;

    _LUMtls            errorSlot;
    _LUMtls            contextSlot;
    _LUMmutex          errorLock;

    struct {
        uint64_t        offset;
        // This is defined in platform.h
        LUM_PLATFORM_LIBRARY_TIMER_STATE
    } timer;

    struct {
        EGLenum         platform;
        EGLDisplay      display;
        EGLint          major, minor;
        LUMbool        prefix;

        LUMbool        KHR_create_context;
        LUMbool        KHR_create_context_no_error;
        LUMbool        KHR_gl_colorspace;
        LUMbool        KHR_get_all_proc_addresses;
        LUMbool        KHR_context_flush_control;
        LUMbool        EXT_client_extensions;
        LUMbool        EXT_platform_base;
        LUMbool        EXT_platform_x11;
        LUMbool        EXT_platform_wayland;
        LUMbool        EXT_present_opaque;
        LUMbool        ANGLE_platform_angle;
        LUMbool        ANGLE_platform_angle_opengl;
        LUMbool        ANGLE_platform_angle_d3d;
        LUMbool        ANGLE_platform_angle_vulkan;
        LUMbool        ANGLE_platform_angle_metal;

        void*           handle;

        PFN_eglGetConfigAttrib      GetConfigAttrib;
        PFN_eglGetConfigs           GetConfigs;
        PFN_eglGetDisplay           GetDisplay;
        PFN_eglGetError             GetError;
        PFN_eglInitialize           Initialize;
        PFN_eglTerminate            Terminate;
        PFN_eglBindAPI              BindAPI;
        PFN_eglCreateContext        CreateContext;
        PFN_eglDestroySurface       DestroySurface;
        PFN_eglDestroyContext       DestroyContext;
        PFN_eglCreateWindowSurface  CreateWindowSurface;
        PFN_eglMakeCurrent          MakeCurrent;
        PFN_eglSwapBuffers          SwapBuffers;
        PFN_eglSwapInterval         SwapInterval;
        PFN_eglQueryString          QueryString;
        PFN_eglGetProcAddress       GetProcAddress;

        PFNEGLGETPLATFORMDISPLAYEXTPROC GetPlatformDisplayEXT;
        PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC CreatePlatformWindowSurfaceEXT;
    } egl;

    struct {
        void*           handle;

        PFN_OSMesaCreateContextExt      CreateContextExt;
        PFN_OSMesaCreateContextAttribs  CreateContextAttribs;
        PFN_OSMesaDestroyContext        DestroyContext;
        PFN_OSMesaMakeCurrent           MakeCurrent;
        PFN_OSMesaGetColorBuffer        GetColorBuffer;
        PFN_OSMesaGetDepthBuffer        GetDepthBuffer;
        PFN_OSMesaGetProcAddress        GetProcAddress;

    } osmesa;

    struct {
        LUMbool        available;
        void*           handle;
        char*           extensions[2];
        PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
        LUMbool        KHR_surface;
        LUMbool        KHR_win32_surface;
        LUMbool        MVK_macos_surface;
        LUMbool        EXT_metal_surface;
        LUMbool        KHR_xlib_surface;
        LUMbool        KHR_xcb_surface;
        LUMbool        KHR_wayland_surface;
    } vk;

    struct {
        LUMmonitorfun  monitor;
        LUMjoystickfun joystick;
    } callbacks;

    // These are defined in platform.h
    LUM_PLATFORM_LIBRARY_WINDOW_STATE
    LUM_PLATFORM_LIBRARY_CONTEXT_STATE
    LUM_PLATFORM_LIBRARY_JOYSTICK_STATE
};

// Global state shared between compilation units of Lumina
//
extern _LUMlibrary _lum;


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

void _lumPlatformInitTimer(void);
uint64_t _lumPlatformGetTimerValue(void);
uint64_t _lumPlatformGetTimerFrequency(void);

int _lumPlatformCreateWindow(_LUMwindow* window,
                              const _LUMwndconfig* wndconfig,
                              const _LUMctxconfig* ctxconfig,
                              const _LUMfbconfig* fbconfig);
void _lumPlatformDestroyWindow(_LUMwindow* window);
void _lumPlatformSetWindowTitle(_LUMwindow* window, const char* title);
void _lumPlatformSetWindowIcon(_LUMwindow* window,
                                int count, const LUMimage* images);
void _lumPlatformGetWindowPos(_LUMwindow* window, int* xpos, int* ypos);
void _lumPlatformSetWindowPos(_LUMwindow* window, int xpos, int ypos);
void _lumPlatformGetWindowSize(_LUMwindow* window, int* width, int* height);
void _lumPlatformSetWindowSize(_LUMwindow* window, int width, int height);
void _lumPlatformSetWindowSizeLimits(_LUMwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight);
void _lumPlatformSetWindowAspectRatio(_LUMwindow* window, int numer, int denom);
void _lumPlatformGetFramebufferSize(_LUMwindow* window, int* width, int* height);
void _lumPlatformGetWindowFrameSize(_LUMwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom);
void _lumPlatformGetWindowContentScale(_LUMwindow* window,
                                        float* xscale, float* yscale);
void _lumPlatformIconifyWindow(_LUMwindow* window);
void _lumPlatformRestoreWindow(_LUMwindow* window);
void _lumPlatformMaximizeWindow(_LUMwindow* window);
void _lumPlatformShowWindow(_LUMwindow* window);
void _lumPlatformHideWindow(_LUMwindow* window);
void _lumPlatformRequestWindowAttention(_LUMwindow* window);
void _lumPlatformFocusWindow(_LUMwindow* window);
void _lumPlatformSetWindowMonitor(_LUMwindow* window, _LUMmonitor* monitor,
                                   int xpos, int ypos, int width, int height,
                                   int refreshRate);
int _lumPlatformWindowFocused(_LUMwindow* window);
int _lumPlatformWindowIconified(_LUMwindow* window);
int _lumPlatformWindowVisible(_LUMwindow* window);
int _lumPlatformWindowMaximized(_LUMwindow* window);
int _lumPlatformWindowHovered(_LUMwindow* window);
int _lumPlatformFramebufferTransparent(_LUMwindow* window);
float _lumPlatformGetWindowOpacity(_LUMwindow* window);
void _lumPlatformSetWindowResizable(_LUMwindow* window, LUMbool enabled);
void _lumPlatformSetWindowDecorated(_LUMwindow* window, LUMbool enabled);
void _lumPlatformSetWindowTitlebar(_LUMwindow* window, LUMbool enabled);
void _lumPlatformSetWindowFloating(_LUMwindow* window, LUMbool enabled);
void _lumPlatformSetWindowOpacity(_LUMwindow* window, float opacity);

void _lumPlatformPollEvents(void);
void _lumPlatformWaitEvents(void);
void _lumPlatformWaitEventsTimeout(double timeout);
void _lumPlatformPostEmptyEvent(void);

void _lumPlatformGetRequiredInstanceExtensions(char** extensions);
int _lumPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily);
VkResult _lumPlatformCreateWindowSurface(VkInstance instance,
                                          _LUMwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface);

LUMbool _lumPlatformCreateTls(_LUMtls* tls);
void _lumPlatformDestroyTls(_LUMtls* tls);
void* _lumPlatformGetTls(_LUMtls* tls);
void _lumPlatformSetTls(_LUMtls* tls, void* value);

LUMbool _lumPlatformCreateMutex(_LUMmutex* mutex);
void _lumPlatformDestroyMutex(_LUMmutex* mutex);
void _lumPlatformLockMutex(_LUMmutex* mutex);
void _lumPlatformUnlockMutex(_LUMmutex* mutex);

void* _lumPlatformLoadModule(const char* path);
void _lumPlatformFreeModule(void* module);
LUMproc _lumPlatformGetModuleSymbol(void* module, const char* name);


//////////////////////////////////////////////////////////////////////////
//////                         Lumina event API                       //////
//////////////////////////////////////////////////////////////////////////

void _lumInputWindowFocus(_LUMwindow* window, LUMbool focused);
void _lumInputWindowPos(_LUMwindow* window, int xpos, int ypos);
void _lumInputWindowSize(_LUMwindow* window, int width, int height);
void _lumInputTitleBarHitTest(_LUMwindow* window, int posX, int posY, int* hit);
void _lumInputFramebufferSize(_LUMwindow* window, int width, int height);
void _lumInputWindowContentScale(_LUMwindow* window,
                                  float xscale, float yscale);
void _lumInputWindowIconify(_LUMwindow* window, LUMbool iconified);
void _lumInputWindowMaximize(_LUMwindow* window, LUMbool maximized);
void _lumInputWindowDamage(_LUMwindow* window);
void _lumInputWindowCloseRequest(_LUMwindow* window);
void _lumInputWindowMonitor(_LUMwindow* window, _LUMmonitor* monitor);

void _lumInputKey(_LUMwindow* window,
                   int key, int scancode, int action, int mods);
void _lumInputChar(_LUMwindow* window,
                    uint32_t codepoint, int mods, LUMbool plain);
void _lumInputScroll(_LUMwindow* window, double xoffset, double yoffset);
void _lumInputMouseClick(_LUMwindow* window, int button, int action, int mods);
void _lumInputCursorPos(_LUMwindow* window, double xpos, double ypos);
void _lumInputCursorEnter(_LUMwindow* window, LUMbool entered);
void _lumInputDrop(_LUMwindow* window, int count, const char** names);
void _lumInputJoystick(_LUMjoystick* js, int event);
void _lumInputJoystickAxis(_LUMjoystick* js, int axis, float value);
void _lumInputJoystickButton(_LUMjoystick* js, int button, char value);
void _lumInputJoystickHat(_LUMjoystick* js, int hat, char value);

void _lumInputMonitor(_LUMmonitor* monitor, int action, int placement);
void _lumInputMonitorWindow(_LUMmonitor* monitor, _LUMwindow* window);

#if defined(__GNUC__)
void _lumInputError(int code, const char* format, ...)
    __attribute__((format(printf, 2, 3)));
#else
void _lumInputError(int code, const char* format, ...);
#endif


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumSelectPlatform(int platformID, _LUMplatform* platform);

LUMbool _lumStringInExtensionString(const char* string, const char* extensions);
const _LUMfbconfig* _lumChooseFBConfig(const _LUMfbconfig* desired,
                                         const _LUMfbconfig* alternatives,
                                         unsigned int count);
LUMbool _lumRefreshContextAttribs(_LUMwindow* window,
                                    const _LUMctxconfig* ctxconfig);
LUMbool _lumIsValidContextConfig(const _LUMctxconfig* ctxconfig);

const LUMvidmode* _lumChooseVideoMode(_LUMmonitor* monitor,
                                        const LUMvidmode* desired);
int _lumCompareVideoModes(const LUMvidmode* first, const LUMvidmode* second);
_LUMmonitor* _lumAllocMonitor(const char* name, int widthMM, int heightMM);
void _lumFreeMonitor(_LUMmonitor* monitor);
void _lumAllocGammaArrays(LUMgammaramp* ramp, unsigned int size);
void _lumFreeGammaArrays(LUMgammaramp* ramp);
void _lumSplitBPP(int bpp, int* red, int* green, int* blue);

void _lumInitGamepadMappings(void);
_LUMjoystick* _lumAllocJoystick(const char* name,
                                  const char* guid,
                                  int axisCount,
                                  int buttonCount,
                                  int hatCount);
void _lumFreeJoystick(_LUMjoystick* js);
void _lumCenterCursorInContentArea(_LUMwindow* window);

LUMbool _lumInitEGL(void);
void _lumTerminateEGL(void);
LUMbool _lumCreateContextEGL(_LUMwindow* window,
                               const _LUMctxconfig* ctxconfig,
                               const _LUMfbconfig* fbconfig);
#if defined(_LUM_X11)
LUMbool _lumChooseVisualEGL(const _LUMwndconfig* wndconfig,
                              const _LUMctxconfig* ctxconfig,
                              const _LUMfbconfig* fbconfig,
                              Visual** visual, int* depth);
#endif /*_LUM_X11*/

LUMbool _lumInitOSMesa(void);
void _lumTerminateOSMesa(void);
LUMbool _lumCreateContextOSMesa(_LUMwindow* window,
                                  const _LUMctxconfig* ctxconfig,
                                  const _LUMfbconfig* fbconfig);

LUMbool _lumInitVulkan(int mode);
void _lumTerminateVulkan(void);
const char* _lumGetVulkanResultString(VkResult result);

size_t _lumEncodeUTF8(char* s, uint32_t codepoint);

char* _lum_strdup(const char* source);
float _lum_fminf(float a, float b);
float _lum_fmaxf(float a, float b);

void* _lum_calloc(size_t count, size_t size);
void* _lum_realloc(void* pointer, size_t size);
void _lum_free(void* pointer);

