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

#include "null_platform.h"

#if defined(_LUM_WIN32)
 #include "win32_platform.h"
#else
 #define LUM_WIN32_WINDOW_STATE
 #define LUM_WIN32_MONITOR_STATE
 #define LUM_WIN32_CURSOR_STATE
 #define LUM_WIN32_LIBRARY_WINDOW_STATE
 #define LUM_WGL_CONTEXT_STATE
 #define LUM_WGL_LIBRARY_CONTEXT_STATE
#endif

#if defined(_LUM_COCOA)
 #include "cocoa_platform.h"
#else
 #define LUM_COCOA_WINDOW_STATE
 #define LUM_COCOA_MONITOR_STATE
 #define LUM_COCOA_CURSOR_STATE
 #define LUM_COCOA_LIBRARY_WINDOW_STATE
 #define LUM_NSGL_CONTEXT_STATE
 #define LUM_NSGL_LIBRARY_CONTEXT_STATE
#endif

#if defined(_LUM_WAYLAND)
 #include "wl_platform.h"
#else
 #define LUM_WAYLAND_WINDOW_STATE
 #define LUM_WAYLAND_MONITOR_STATE
 #define LUM_WAYLAND_CURSOR_STATE
 #define LUM_WAYLAND_LIBRARY_WINDOW_STATE
#endif

#if defined(_LUM_X11)
 #include "x11_platform.h"
#else
 #define LUM_X11_WINDOW_STATE
 #define LUM_X11_MONITOR_STATE
 #define LUM_X11_CURSOR_STATE
 #define LUM_X11_LIBRARY_WINDOW_STATE
 #define LUM_GLX_CONTEXT_STATE
 #define LUM_GLX_LIBRARY_CONTEXT_STATE
#endif

#include "null_joystick.h"

#if defined(_LUM_WIN32)
 #include "win32_joystick.h"
#else
 #define LUM_WIN32_JOYSTICK_STATE
 #define LUM_WIN32_LIBRARY_JOYSTICK_STATE
#endif

#if defined(_LUM_COCOA)
 #include "cocoa_joystick.h"
#else
 #define LUM_COCOA_JOYSTICK_STATE
 #define LUM_COCOA_LIBRARY_JOYSTICK_STATE
#endif

#if (defined(_LUM_X11) || defined(_LUM_WAYLAND)) && defined(__linux__)
 #include "linux_joystick.h"
#else
 #define LUM_LINUX_JOYSTICK_STATE
 #define LUM_LINUX_LIBRARY_JOYSTICK_STATE
#endif

#if defined(_WIN32)
 #include "win32_thread.h"
 #define LUM_POSIX_TLS_STATE
 #define LUM_POSIX_MUTEX_STATE
#else
 #include "posix_thread.h"
 #define LUM_WIN32_TLS_STATE
 #define LUM_WIN32_MUTEX_STATE
#endif

#if defined(_WIN32)
 #include "win32_time.h"
 #define LUM_POSIX_LIBRARY_TIMER_STATE
 #define LUM_COCOA_LIBRARY_TIMER_STATE
#elif defined(__APPLE__)
 #include "cocoa_time.h"
 #define LUM_WIN32_LIBRARY_TIMER_STATE
 #define LUM_POSIX_LIBRARY_TIMER_STATE
#else
 #include "posix_time.h"
 #define LUM_WIN32_LIBRARY_TIMER_STATE
 #define LUM_COCOA_LIBRARY_TIMER_STATE
#endif

#define LUM_PLATFORM_WINDOW_STATE \
        LUM_WIN32_WINDOW_STATE \
        LUM_COCOA_WINDOW_STATE \
        LUM_WAYLAND_WINDOW_STATE \
        LUM_X11_WINDOW_STATE \
        LUM_NULL_WINDOW_STATE \

#define LUM_PLATFORM_MONITOR_STATE \
        LUM_WIN32_MONITOR_STATE \
        LUM_COCOA_MONITOR_STATE \
        LUM_WAYLAND_MONITOR_STATE \
        LUM_X11_MONITOR_STATE \
        LUM_NULL_MONITOR_STATE \

#define LUM_PLATFORM_CURSOR_STATE \
        LUM_WIN32_CURSOR_STATE \
        LUM_COCOA_CURSOR_STATE \
        LUM_WAYLAND_CURSOR_STATE \
        LUM_X11_CURSOR_STATE \
        LUM_NULL_CURSOR_STATE \

#define LUM_PLATFORM_JOYSTICK_STATE \
        LUM_WIN32_JOYSTICK_STATE \
        LUM_COCOA_JOYSTICK_STATE \
        LUM_LINUX_JOYSTICK_STATE

#define LUM_PLATFORM_TLS_STATE \
        LUM_WIN32_TLS_STATE \
        LUM_POSIX_TLS_STATE \

#define LUM_PLATFORM_MUTEX_STATE \
        LUM_WIN32_MUTEX_STATE \
        LUM_POSIX_MUTEX_STATE \

#define LUM_PLATFORM_LIBRARY_WINDOW_STATE \
        LUM_WIN32_LIBRARY_WINDOW_STATE \
        LUM_COCOA_LIBRARY_WINDOW_STATE \
        LUM_WAYLAND_LIBRARY_WINDOW_STATE \
        LUM_X11_LIBRARY_WINDOW_STATE \
        LUM_NULL_LIBRARY_WINDOW_STATE \

#define LUM_PLATFORM_LIBRARY_JOYSTICK_STATE \
        LUM_WIN32_LIBRARY_JOYSTICK_STATE \
        LUM_COCOA_LIBRARY_JOYSTICK_STATE \
        LUM_LINUX_LIBRARY_JOYSTICK_STATE

#define LUM_PLATFORM_LIBRARY_TIMER_STATE \
        LUM_WIN32_LIBRARY_TIMER_STATE \
        LUM_COCOA_LIBRARY_TIMER_STATE \
        LUM_POSIX_LIBRARY_TIMER_STATE \

#define LUM_PLATFORM_CONTEXT_STATE \
        LUM_WGL_CONTEXT_STATE \
        LUM_NSGL_CONTEXT_STATE \
        LUM_GLX_CONTEXT_STATE

#define LUM_PLATFORM_LIBRARY_CONTEXT_STATE \
        LUM_WGL_LIBRARY_CONTEXT_STATE \
        LUM_NSGL_LIBRARY_CONTEXT_STATE \
        LUM_GLX_LIBRARY_CONTEXT_STATE

