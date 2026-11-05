# Lumina

## Introduction

Lumina is a multi-platform library for OpenGL, OpenGL ES and Vulkan application
development.  It provides a simple, platform-independent API for creating
windows, contexts and surfaces, reading input, handling events, etc.

Lumina is a fork of [GLFW](https://www.glfw.org/) 3.4, picked up after upstream
development stopped.  The API is unchanged apart from its names: every `glfw`
and `GLFW` identifier was renamed to the `lum` / `LUM` prefix.

| GLFW                       | Lumina                           |
| -------------------------- | -------------------------------- |
| `glfwCreateWindow()`       | `lumCreateWindow()`              |
| `GLFWwindow`               | `LUMwindow`                      |
| `GLFW_KEY_ESCAPE`          | `LUM_KEY_ESCAPE`                 |
| `#include <GLFW/glfw3.h>`  | `#include <Lumina/lumina.h>`     |
| link against `glfw3`       | link against `lumina`            |

There is deliberately no GLFW compatibility header, so porting existing code
means applying that same renaming to the call sites.

Lumina natively supports Windows, macOS and Linux and other Unix-like systems.
On Linux both X11 and Wayland are supported.

Lumina keeps GLFW's [zlib/libpng license](LICENSE.md) and exists because of the
work of the [many people](CONTRIBUTORS.md) who built GLFW, whether by reporting
bugs, providing community support, adding features, reviewing or testing code,
debugging, proofreading docs, suggesting features or fixing bugs.

> **A note on the links in this file and in `docs/`.**  They still point at
> GLFW's website, documentation and issue tracker, because Lumina has none of
> its own yet.  They describe the same API under the old `glfw` names.  Replace
> them once Lumina has its own site, tracker and releases.


## Compiling Lumina

Lumina itself requires only the headers and libraries for your OS and window
system.  It does not need the headers for any context creation API (WGL, GLX,
EGL, NSGL, OSMesa) or rendering API (OpenGL, OpenGL ES, Vulkan) to enable
support for them.

Lumina supports compilation on Windows with Visual C++ 2010 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled Windows binaries](https://www.glfw.org/download.html)
available for all supported compilers.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about how to compile Lumina yourself.


## Using Lumina

Lumina's own documentation site lives in [`website/`](website/). It is
generated from the headers and the guides in `docs/`, so it always describes
the code in this repository:

```sh
cd website
npm install
npm run dev
```

It covers the guides, an alphabetical index of all 148 functions, every type
and every constant. See [`website/README.md`](website/README.md) for how the
generation works.

GLFW's [documentation](https://www.glfw.org/docs/latest/) still describes the
same API under the old `glfw` names, and remains a useful reference.


## Contributing to Lumina

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

Lumina supports Windows XP and later and macOS 10.8 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

Lumina itself needs only CMake 3.1 or later and the headers and libraries for your
OS and window system.

The examples and test programs depend on a number of tiny libraries.  These are
located in the `deps/` directory.

 - [getopt\_port](https://github.com/kimgr/getopt_port/) for examples
   with command-line options
 - [TinyCThread](https://github.com/tinycthread/tinycthread) for threaded
   examples
 - [glad2](https://github.com/Dav1dde/glad) for loading OpenGL and Vulkan
   functions
 - [linmath.h](https://github.com/datenwolf/linmath.h) for linear algebra in
   examples
 - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for test and example UI
 - [stb\_image\_write](https://github.com/nothings/stb) for writing images to disk

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Lumina does not have an issue tracker of its own yet; report bugs to whoever
maintains this copy.  GLFW's [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) still
describes what is worth including in a report.

A bug that also affects GLFW 3.4 is worth checking against GLFW's own [issue
tracker](https://github.com/glfw/glfw/issues) first, where it may already be
diagnosed.


## Changelog

 - Added `LUM_PLATFORM` init hint for runtime platform selection (#1958)
 - Added `LUM_ANY_PLATFORM`, `LUM_PLATFORM_WIN32`, `LUM_PLATFORM_COCOA`,
   `LUM_PLATFORM_WAYLAND`, `LUM_PLATFORM_X11` and `LUM_PLATFORM_NULL` symbols to
   specify the desired platform (#1958)
 - Added `lumGetPlatform` function to query what platform was selected (#1655,#1958)
 - Added `lumPlatformSupported` function to query if a platform is supported
   (#1655,#1958)
 - Added `lumInitAllocator` for setting a custom memory allocator (#544,#1628,#1947)
 - Added `LUMallocator` struct and `LUMallocatefun`, `LUMreallocatefun` and
   `LUMdeallocatefun` types (#544,#1628,#1947)
 - Added `lumInitVulkanLoader` for using a non-default Vulkan loader (#1374,#1890)
 - Added `LUM_RESIZE_NWSE_CURSOR`, `LUM_RESIZE_NESW_CURSOR`,
   `LUM_RESIZE_ALL_CURSOR` and `LUM_NOT_ALLOWED_CURSOR` cursor shapes (#427)
 - Added `LUM_RESIZE_EW_CURSOR` alias for `LUM_HRESIZE_CURSOR` (#427)
 - Added `LUM_RESIZE_NS_CURSOR` alias for `LUM_VRESIZE_CURSOR` (#427)
 - Added `LUM_POINTING_HAND_CURSOR` alias for `LUM_HAND_CURSOR` (#427)
 - Added `LUM_MOUSE_PASSTHROUGH` window hint for letting mouse input pass
   through the window (#1236,#1568)
 - Added `LUM_PLATFORM_UNAVAILABLE` error for platform detection failures (#1958)
 - Added `LUM_FEATURE_UNAVAILABLE` error for platform limitations (#1692)
 - Added `LUM_FEATURE_UNIMPLEMENTED` error for incomplete backends (#1692)
 - Added `LUM_ANGLE_PLATFORM_TYPE` init hint and `LUM_ANGLE_PLATFORM_TYPE_*`
   values to select ANGLE backend (#1380)
 - Added `LUM_X11_XCB_VULKAN_SURFACE` init hint for selecting X11 Vulkan
   surface extension (#1793)
 - Added `LUM_BUILD_WIN32` CMake option for enabling Win32 support (#1958)
 - Added `LUM_BUILD_COCOA` CMake option for enabling Cocoa support (#1958)
 - Added `LUM_BUILD_X11` CMake option for enabling X11 support (#1958)
 - Added `LUM_LIBRARY_TYPE` CMake variable for overriding the library type
   (#279,#1307,#1497,#1574,#1928)
 - Added `LUM_PKG_CONFIG_REQUIRES_PRIVATE` and `LUM_PKG_CONFIG_LIBS_PRIVATE` CMake
   variables exposing pkg-config dependencies (#1307)
 - Made joystick subsystem initialize at first use (#1284,#1646)
 - Made `LUM_DOUBLEBUFFER` a read-only window attribute
 - Updated the minimum required CMake version to 3.1
 - Updated gamepad mappings from upstream
 - Disabled tests and examples by default when built as a CMake subdirectory
 - Renamed `LUM_USE_WAYLAND` CMake option to `LUM_BUILD_WAYLAND` (#1958)
 - Removed `LUM_USE_OSMESA` CMake option enabling the Null platform (#1958)
 - Removed CMake generated configuration header
 - Bugfix: The CMake config-file package used an absolute path and was not
   relocatable (#1470)
 - Bugfix: Video modes with a duplicate screen area were discarded (#1555,#1556)
 - Bugfix: Compiling with -Wextra-semi caused warnings (#1440)
 - Bugfix: Built-in mappings failed because some OEMs re-used VID/PID (#1583)
 - Bugfix: Some extension loader headers did not prevent default OpenGL header
   inclusion (#1695)
 - Bugfix: Buffers were swapped at creation on single-buffered windows (#1873)
 - Bugfix: Gamepad mapping updates could spam `LUM_INVALID_VALUE` due to
   incompatible controllers sharing hardware ID (#1763)
 - Bugfix: Native access functions for context handles did not check that the API matched
 - [Win32] Added the `LUM_WIN32_KEYBOARD_MENU` window hint for enabling access
           to the window menu
 - [Win32] Added a version info resource to the Lumina DLL
 - [Win32] Disabled framebuffer transparency on Windows 7 when DWM windows are
   opaque (#1512)
 - [Win32] Bugfix: `LUM_INCLUDE_VULKAN` plus `VK_USE_PLATFORM_WIN32_KHR` caused
   symbol redefinition (#1524)
 - [Win32] Bugfix: The cursor position event was emitted before its cursor enter
   event (#1490)
 - [Win32] Bugfix: The window hint `LUM_MAXIMIZED` did not move or resize the
   window (#1499)
 - [Win32] Bugfix: Disabled cursor mode interfered with some non-client actions
 - [Win32] Bugfix: Super key was not released after Win+V hotkey (#1622)
 - [Win32] Bugfix: `lumGetKeyName` could access out of bounds and return an
   invalid pointer
 - [Win32] Bugfix: Some synthetic key events were reported as `LUM_KEY_UNKNOWN`
   (#1623)
 - [Win32] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
 - [Win32] Bugfix: Monitor functions could return invalid values after
   configuration change (#1761)
 - [Win32] Bugfix: Initialization would segfault on Windows 8 (not 8.1) (#1775)
 - [Win32] Bugfix: Duplicate size events were not filtered (#1610)
 - [Win32] Bugfix: Full screen windows were incorrectly resized by DPI changes
   (#1582)
 - [Win32] Bugfix: `LUM_SCALE_TO_MONITOR` had no effect on systems older than
   Windows 10 version 1703 (#1511)
 - [Win32] Bugfix: `USE_MSVC_RUNTIME_LIBRARY_DLL` had no effect on CMake 3.15 or
   later (#1783,#1796)
 - [Win32] Bugfix: Compilation with LLVM for Windows failed (#1807,#1824,#1874)
 - [Win32] Bugfix: The foreground lock timeout was overridden, ignoring the user
 - [Win32] Bugfix: Content scale queries could fail silently (#1615)
 - [Win32] Bugfix: Content scales could have garbage values if monitor was recently
   disconnected (#1615)
 - [Cocoa] Added support for `VK_EXT_metal_surface` (#1619)
 - [Cocoa] Added locating the Vulkan loader at runtime in an application bundle
 - [Cocoa] Moved main menu creation to Lumina initialization time (#1649)
 - [Cocoa] Changed `EGLNativeWindowType` from `NSView` to `CALayer` (#1169)
 - [Cocoa] Changed F13 key to report Print Screen for cross-platform consistency
   (#1786)
 - [Cocoa] Removed dependency on the CoreVideo framework
 - [Cocoa] Bugfix: `lumSetWindowSize` used a bottom-left anchor point (#1553)
 - [Cocoa] Bugfix: Window remained on screen after destruction until event poll
   (#1412)
 - [Cocoa] Bugfix: Event processing before window creation would assert (#1543)
 - [Cocoa] Bugfix: Undecorated windows could not be iconified on recent macOS
 - [Cocoa] Bugfix: Touching event queue from secondary thread before main thread
   would abort (#1649)
 - [Cocoa] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
   (#1635)
 - [Cocoa] Bugfix: Failing to retrieve the refresh rate of built-in displays
   could leak memory
 - [Cocoa] Bugfix: Objective-C files were compiled as C with CMake 3.19 (#1787)
 - [Cocoa] Bugfix: Duplicate video modes were not filtered out (#1830)
 - [Cocoa] Bugfix: Menu bar was not clickable on macOS 10.15+ until it lost and
   regained focus (#1648,#1802)
 - [Cocoa] Bugfix: Monitor name query could segfault on macOS 11 (#1809,#1833)
 - [Cocoa] Bugfix: The install name of the installed dylib was relative (#1504)
 - [Cocoa] Bugfix: The MoltenVK layer contents scale was updated only after
   related events were emitted
 - [Cocoa] Bugfix: Moving the cursor programmatically would freeze it for
   a fraction of a second (#1962)
 - [Cocoa] Bugfix: `kIOMasterPortDefault` was deprecated in macOS 12.0 (#1980)
 - [Cocoa] Bugfix: `kUTTypeURL` was deprecated in macOS 12.0 (#2003)
 - [X11] Bugfix: The CMake files did not check for the XInput headers (#1480)
 - [X11] Bugfix: Key names were not updated when the keyboard layout changed
   (#1462,#1528)
 - [X11] Bugfix: Decorations could not be enabled after window creation (#1566)
 - [X11] Bugfix: Content scale fallback value could be inconsistent (#1578)
 - [X11] Bugfix: `lumMaximizeWindow` had no effect on hidden windows
 - [X11] Bugfix: Clearing `LUM_FLOATING` on a hidden window caused invalid read
 - [X11] Bugfix: Changing `LUM_FLOATING` on a hidden window could silently fail
 - [X11] Bugfix: Disabled cursor mode was interrupted by indicator windows
 - [X11] Bugfix: Monitor physical dimensions could be reported as zero mm
 - [X11] Bugfix: Window position events were not emitted during resizing (#1613)
 - [X11] Bugfix: `lumFocusWindow` could terminate on older WMs or without a WM
 - [X11] Bugfix: Querying a disconnected monitor could segfault (#1602)
 - [X11] Bugfix: IME input of CJK was broken for "C" locale (#1587,#1636)
 - [X11] Bugfix: Termination would segfault if the IM had been destroyed
 - [X11] Bugfix: Any IM started after initialization would not be detected
 - [X11] Bugfix: Xlib errors caused by other parts of the application could be
   reported as Lumina errors
 - [X11] Bugfix: A handle race condition could cause a `BadWindow` error (#1633)
 - [X11] Bugfix: XKB path used keysyms instead of physical locations for
   non-printable keys (#1598)
 - [X11] Bugfix: Function keys were mapped to `LUM_KEY_UNKNOWN` for some layout
   combinations (#1598)
 - [X11] Bugfix: Keys pressed simultaneously with others were not always
   reported (#1112,#1415,#1472,#1616)
 - [X11] Bugfix: Some window attributes were not applied on leaving fullscreen
   (#1863)
 - [X11] Bugfix: Changing `LUM_FLOATING` could leak memory
 - [X11] Bugfix: Icon pixel format conversion worked only by accident, relying on
   undefined behavior (#1986)
 - [X11] Bugfix: Dynamic loading on OpenBSD failed due to soname differences
 - [Wayland] Added dynamic loading of all Wayland libraries
 - [Wayland] Added support for key names via xkbcommon
 - [Wayland] Removed support for `wl_shell` (#1443)
 - [Wayland] Bugfix: The `LUM_HAND_CURSOR` shape used the wrong image (#1432)
 - [Wayland] Bugfix: `CLOCK_MONOTONIC` was not correctly enabled
 - [Wayland] Bugfix: Repeated keys could be reported with `NULL` window (#1704)
 - [Wayland] Bugfix: Retrieving partial framebuffer size would segfault
 - [Wayland] Bugfix: Scrolling offsets were inverted compared to other platforms
   (#1463)
 - [Wayland] Bugfix: Client-Side Decorations were destroyed in the wrong order
   (#1798)
 - [Wayland] Bugfix: Monitors physical size could report zero (#1784,#1792)
 - [Wayland] Bugfix: Some keys were not repeating in Wayland (#1908)
 - [Wayland] Bugfix: Non-arrow cursors are offset from the hotspot (#1706,#1899)
 - [Wayland] Bugfix: The `O_CLOEXEC` flag was not defined on FreeBSD
 - [Wayland] Bugfix: Key repeat could lead to a race condition (#1710)
 - [Wayland] Bugfix: Activating a window would emit two input focus events
 - [Wayland] Bugfix: Disable key repeat mechanism when window loses input focus
 - [Wayland] Bugfix: Window hiding and showing did not work (#1492,#1731)
 - [Wayland] Bugfix: A key being repeated was not released when window lost focus
 - [Wayland] Bugfix: Showing a hidden window did not emit a window refresh event
 - [Wayland] Bugfix: Full screen window creation did not ignore `LUM_VISIBLE`
 - [Wayland] Bugfix: Some keys were reported as wrong key or `LUM_KEY_UNKNOWN`
 - [Wayland] Bugfix: Text input did not repeat along with key repeat
 - [POSIX] Removed use of deprecated function `gettimeofday`
 - [POSIX] Bugfix: `CLOCK_MONOTONIC` was not correctly tested for or enabled
 - [WGL] Disabled the DWM swap interval hack for Windows 8 and later (#1072)
 - [NSGL] Removed enforcement of forward-compatible flag for core contexts
 - [NSGL] Bugfix: `LUM_COCOA_RETINA_FRAMEBUFFER` had no effect on newer
   macOS versions (#1442)
 - [NSGL] Bugfix: Workaround for swap interval on 10.14 broke on 10.12 (#1483)
 - [NSGL] Bugfix: Defining `GL_SILENCE_DEPRECATION` externally caused
   a duplicate definition warning (#1840)
 - [EGL] Added platform selection via the `EGL_EXT_platform_base` extension
   (#442)
 - [EGL] Added ANGLE backend selection via `EGL_ANGLE_platform_angle` extension
   (#1380)
 - [EGL] Bugfix: The `LUM_DOUBLEBUFFER` context attribute was ignored (#1843)


## Contact

Lumina has no website, forum or chat of its own yet.  Reach whoever maintains
this copy for questions, patches and feature requests.

For questions about the underlying design, GLFW's own channels remain useful,
since Lumina's API is GLFW 3.4's under different names: the
[documentation](https://www.glfw.org/docs/latest/) on
[glfw.org](https://www.glfw.org/), the
[forum](https://discourse.glfw.org/) and the `#glfw` IRC channel on
[Libera.Chat](https://libera.chat/).

