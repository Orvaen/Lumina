//========================================================================
// Lumina - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
// Copyright (c) 2012 Torsten Walluhn <tw@mad-cad.net>
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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>


//////////////////////////////////////////////////////////////////////////
//////                         Lumina event API                       //////
//////////////////////////////////////////////////////////////////////////

// Notifies shared code that a window has lost or received input focus
//
void _lumInputWindowFocus(_LUMwindow* window, LUMbool focused)
{
    if (window->callbacks.focus)
        window->callbacks.focus((LUMwindow*) window, focused);

    if (!focused)
    {
        int key, button;

        for (key = 0;  key <= LUM_KEY_LAST;  key++)
        {
            if (window->keys[key] == LUM_PRESS)
            {
                const int scancode = _lum.platform.getKeyScancode(key);
                _lumInputKey(window, key, scancode, LUM_RELEASE, 0);
            }
        }

        for (button = 0;  button <= LUM_MOUSE_BUTTON_LAST;  button++)
        {
            if (window->mouseButtons[button] == LUM_PRESS)
                _lumInputMouseClick(window, button, LUM_RELEASE, 0);
        }
    }
}

// Notifies shared code that a window has moved
// The position is specified in content area relative screen coordinates
//
void _lumInputWindowPos(_LUMwindow* window, int x, int y)
{
    if (window->callbacks.pos)
        window->callbacks.pos((LUMwindow*) window, x, y);
}

// Notifies shared code that a window has been resized
// The size is specified in screen coordinates
//
void _lumInputWindowSize(_LUMwindow* window, int width, int height)
{
    if (window->callbacks.size)
        window->callbacks.size((LUMwindow*) window, width, height);
}

// Notifies shared code that mouse hittest needs to be resolved
//
void _lumInputTitleBarHitTest(_LUMwindow* window, int posX, int posY, int* hit)
{
    if (window->callbacks.tbhittest)
        window->callbacks.tbhittest((LUMwindow*)window, posX, posY, hit);
}

// Notifies shared code that a window has been iconified or restored
//
void _lumInputWindowIconify(_LUMwindow* window, LUMbool iconified)
{
    if (window->callbacks.iconify)
        window->callbacks.iconify((LUMwindow*) window, iconified);
}

// Notifies shared code that a window has been maximized or restored
//
void _lumInputWindowMaximize(_LUMwindow* window, LUMbool maximized)
{
    if (window->callbacks.maximize)
        window->callbacks.maximize((LUMwindow*) window, maximized);
}

// Notifies shared code that a window framebuffer has been resized
// The size is specified in pixels
//
void _lumInputFramebufferSize(_LUMwindow* window, int width, int height)
{
    if (window->callbacks.fbsize)
        window->callbacks.fbsize((LUMwindow*) window, width, height);
}

// Notifies shared code that a window content scale has changed
// The scale is specified as the ratio between the current and default DPI
//
void _lumInputWindowContentScale(_LUMwindow* window, float xscale, float yscale)
{
    if (window->callbacks.scale)
        window->callbacks.scale((LUMwindow*) window, xscale, yscale);
}

// Notifies shared code that the window contents needs updating
//
void _lumInputWindowDamage(_LUMwindow* window)
{
    if (window->callbacks.refresh)
        window->callbacks.refresh((LUMwindow*) window);
}

// Notifies shared code that the user wishes to close a window
//
void _lumInputWindowCloseRequest(_LUMwindow* window)
{
    window->shouldClose = LUM_TRUE;

    if (window->callbacks.close)
        window->callbacks.close((LUMwindow*) window);
}

// Notifies shared code that a window has changed its desired monitor
//
void _lumInputWindowMonitor(_LUMwindow* window, _LUMmonitor* monitor)
{
    window->monitor = monitor;
}

//////////////////////////////////////////////////////////////////////////
//////                        Lumina public API                       //////
//////////////////////////////////////////////////////////////////////////

// Fills the creation configs with the library defaults.
//
// Single source of truth for every default, shared by lumDefaultWindowHints
// and lumCreateWindowEx so the two paths cannot drift apart.
//
static void setDefaultConfigs(_LUMwndconfig* wndconfig,
                              _LUMctxconfig* ctxconfig,
                              _LUMfbconfig* fbconfig,
                              int* refreshRate)
{
    // The default is OpenGL with minimum version 1.0
    memset(ctxconfig, 0, sizeof(_LUMctxconfig));
    ctxconfig->client = LUM_OPENGL_API;
    ctxconfig->source = LUM_NATIVE_CONTEXT_API;
    ctxconfig->major  = 1;
    ctxconfig->minor  = 0;

    // The default is a focused, visible, resizable window with decorations
    memset(wndconfig, 0, sizeof(_LUMwndconfig));
    wndconfig->resizable    = LUM_TRUE;
    wndconfig->visible      = LUM_TRUE;
    wndconfig->decorated    = LUM_TRUE;
    wndconfig->titlebar     = LUM_TRUE;
    wndconfig->focused      = LUM_TRUE;
    wndconfig->autoIconify  = LUM_TRUE;
    wndconfig->centerCursor = LUM_TRUE;
    wndconfig->focusOnShow  = LUM_TRUE;

    // The default is to use full Retina resolution framebuffers
    wndconfig->ns.retina = LUM_TRUE;

    // The default is 24 bits of color, 24 bits of depth and 8 bits of stencil,
    // double buffered
    memset(fbconfig, 0, sizeof(_LUMfbconfig));
    fbconfig->redBits      = 8;
    fbconfig->greenBits    = 8;
    fbconfig->blueBits     = 8;
    fbconfig->alphaBits    = 8;
    fbconfig->depthBits    = 24;
    fbconfig->stencilBits  = 8;
    fbconfig->doublebuffer = LUM_TRUE;

    // The default is to select the highest available refresh rate
    *refreshRate = LUM_DONT_CARE;
}

// Resolves a descriptor switch against the library default
//
static LUMbool descToggle(LUMtoggle value, LUMbool fallback)
{
    if (value == LUM_ON)
        return LUM_TRUE;
    if (value == LUM_OFF)
        return LUM_FALSE;

    return fallback;
}

// Resolves a descriptor integer, where zero means "use the default"
//
static int descInt(int value, int fallback)
{
    return value ? value : fallback;
}

// As descInt, but LUM_BITS_NONE asks for a buffer of exactly zero bits
//
static int descBits(int value, int fallback)
{
    if (value == LUM_BITS_NONE)
        return 0;

    return value ? value : fallback;
}

// Copies an optional descriptor string into a fixed size config field
//
static void descString(char* target, size_t size, const char* source)
{
    if (!source)
        return;

    strncpy(target, source, size - 1);
    target[size - 1] = 0;
}

// The shared tail of window creation
//
// Both public creation functions end up here once their parameters have been
// turned into the three configuration structs the platform layer expects.
//
static LUMwindow* createWindow(const _LUMwndconfig* wndconfig,
                                const _LUMctxconfig* ctxconfig,
                                const _LUMfbconfig* fbconfig,
                                LUMmonitor* monitor,
                                int refreshRate)
{
    _LUMwindow* window;

    if (wndconfig->width <= 0 || wndconfig->height <= 0)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        wndconfig->width, wndconfig->height);

        return NULL;
    }

    if (!_lumIsValidContextConfig(ctxconfig))
        return NULL;

    window = _lum_calloc(1, sizeof(_LUMwindow));
    window->next = _lum.windowListHead;
    _lum.windowListHead = window;

    window->videoMode.width       = wndconfig->width;
    window->videoMode.height      = wndconfig->height;
    window->videoMode.redBits     = fbconfig->redBits;
    window->videoMode.greenBits   = fbconfig->greenBits;
    window->videoMode.blueBits    = fbconfig->blueBits;
    window->videoMode.refreshRate = refreshRate;

    window->monitor          = (_LUMmonitor*) monitor;
    window->resizable        = wndconfig->resizable;
    window->decorated        = wndconfig->decorated;
    window->autoIconify      = wndconfig->autoIconify;
    window->floating         = wndconfig->floating;
    window->focusOnShow      = wndconfig->focusOnShow;
    window->mousePassthrough = wndconfig->mousePassthrough;
    window->cursorMode       = LUM_CURSOR_NORMAL;

    window->doublebuffer = fbconfig->doublebuffer;

    window->minwidth    = LUM_DONT_CARE;
    window->minheight   = LUM_DONT_CARE;
    window->maxwidth    = LUM_DONT_CARE;
    window->maxheight   = LUM_DONT_CARE;
    window->numer       = LUM_DONT_CARE;
    window->denom       = LUM_DONT_CARE;

    // Open the actual window and create its context
    if (!_lum.platform.createWindow(window, wndconfig, ctxconfig, fbconfig))
    {
        lumDestroyWindow((LUMwindow*) window);
        return NULL;
    }

    if (ctxconfig->client != LUM_NO_API)
    {
        if (!_lumRefreshContextAttribs(window, ctxconfig))
        {
            lumDestroyWindow((LUMwindow*) window);
            return NULL;
        }
    }

    if (wndconfig->mousePassthrough)
        _lum.platform.setWindowMousePassthrough(window, LUM_TRUE);

    if (window->monitor)
    {
        if (wndconfig->centerCursor)
            _lumCenterCursorInContentArea(window);
    }
    else
    {
        if (wndconfig->visible)
        {
            _lum.platform.showWindow(window);
            if (wndconfig->focused)
                _lum.platform.focusWindow(window);
        }
    }

    return (LUMwindow*) window;
}

LUMAPI LUMwindow* lumCreateWindow(int width, int height,
                                     const char* title,
                                     LUMmonitor* monitor,
                                     LUMwindow* share)
{
    _LUMfbconfig fbconfig;
    _LUMctxconfig ctxconfig;
    _LUMwndconfig wndconfig;

    assert(title != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    fbconfig  = _lum.hints.framebuffer;
    ctxconfig = _lum.hints.context;
    wndconfig = _lum.hints.window;

    wndconfig.width   = width;
    wndconfig.height  = height;
    wndconfig.title   = title;
    ctxconfig.share   = (_LUMwindow*) share;

    return createWindow(&wndconfig, &ctxconfig, &fbconfig,
                        monitor, _lum.hints.refreshRate);
}

LUMAPI LUMwindow* lumCreateWindowEx(const LUMwindowdesc* desc)
{
    _LUMfbconfig fbconfig;
    _LUMctxconfig ctxconfig;
    _LUMwndconfig wndconfig;
    LUMwindow* window;
    int refreshRate;

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (desc == NULL)
    {
        _lumInputError(LUM_INVALID_VALUE, "Window descriptor must not be NULL");
        return NULL;
    }

    if (desc->title == NULL)
    {
        _lumInputError(LUM_INVALID_VALUE, "Window title must not be NULL");
        return NULL;
    }

    // Start from the library defaults, then apply whatever the descriptor
    // states.  A field left at zero therefore keeps its default.
    setDefaultConfigs(&wndconfig, &ctxconfig, &fbconfig, &refreshRate);

    wndconfig.width  = desc->width;
    wndconfig.height = desc->height;
    wndconfig.title  = desc->title;

    wndconfig.resizable        = descToggle(desc->resizable, wndconfig.resizable);
    wndconfig.visible          = descToggle(desc->visible, wndconfig.visible);
    wndconfig.decorated        = descToggle(desc->decorated, wndconfig.decorated);
    wndconfig.titlebar         = descToggle(desc->titlebar, wndconfig.titlebar);
    wndconfig.focused          = descToggle(desc->focused, wndconfig.focused);
    wndconfig.autoIconify      = descToggle(desc->autoIconify, wndconfig.autoIconify);
    wndconfig.floating         = descToggle(desc->floating, wndconfig.floating);
    wndconfig.maximized        = descToggle(desc->maximized, wndconfig.maximized);
    wndconfig.centerCursor     = descToggle(desc->centerCursor, wndconfig.centerCursor);
    wndconfig.focusOnShow      = descToggle(desc->focusOnShow, wndconfig.focusOnShow);
    wndconfig.mousePassthrough = descToggle(desc->mousePassthrough,
                                            wndconfig.mousePassthrough);
    wndconfig.scaleToMonitor   = descToggle(desc->scaleToMonitor,
                                            wndconfig.scaleToMonitor);

    wndconfig.ns.retina     = descToggle(desc->cocoaRetinaFramebuffer,
                                         wndconfig.ns.retina);
    wndconfig.win32.keymenu = descToggle(desc->win32KeyboardMenu,
                                         wndconfig.win32.keymenu);

    descString(wndconfig.ns.frameName, sizeof(wndconfig.ns.frameName),
               desc->cocoaFrameName);
    descString(wndconfig.x11.className, sizeof(wndconfig.x11.className),
               desc->x11ClassName);
    descString(wndconfig.x11.instanceName, sizeof(wndconfig.x11.instanceName),
               desc->x11InstanceName);

    // A descriptor says "no context" with a switch, because LUM_NO_API is zero
    // and zero already means "use the default" everywhere else.
    if (desc->context == LUM_OFF)
        ctxconfig.client = LUM_NO_API;
    else
        ctxconfig.client = descInt(desc->clientApi, ctxconfig.client);

    ctxconfig.source       = descInt(desc->contextCreationApi, ctxconfig.source);
    ctxconfig.major        = descInt(desc->glMajor, ctxconfig.major);
    ctxconfig.minor        = desc->glMinor;
    ctxconfig.profile      = descInt(desc->glProfile, ctxconfig.profile);
    ctxconfig.robustness   = descInt(desc->robustness, ctxconfig.robustness);
    ctxconfig.release      = descInt(desc->releaseBehavior, ctxconfig.release);
    ctxconfig.forward      = descToggle(desc->forwardCompatible, ctxconfig.forward);
    ctxconfig.debug        = descToggle(desc->debugContext, ctxconfig.debug);
    ctxconfig.noerror      = descToggle(desc->noErrorContext, ctxconfig.noerror);
    ctxconfig.nsgl.offline = descToggle(desc->cocoaGraphicsSwitching,
                                        ctxconfig.nsgl.offline);
    ctxconfig.share        = (_LUMwindow*) desc->share;

    fbconfig.redBits        = descBits(desc->redBits, fbconfig.redBits);
    fbconfig.greenBits      = descBits(desc->greenBits, fbconfig.greenBits);
    fbconfig.blueBits       = descBits(desc->blueBits, fbconfig.blueBits);
    fbconfig.alphaBits      = descBits(desc->alphaBits, fbconfig.alphaBits);
    fbconfig.depthBits      = descBits(desc->depthBits, fbconfig.depthBits);
    fbconfig.stencilBits    = descBits(desc->stencilBits, fbconfig.stencilBits);
    fbconfig.accumRedBits   = descBits(desc->accumRedBits, fbconfig.accumRedBits);
    fbconfig.accumGreenBits = descBits(desc->accumGreenBits, fbconfig.accumGreenBits);
    fbconfig.accumBlueBits  = descBits(desc->accumBlueBits, fbconfig.accumBlueBits);
    fbconfig.accumAlphaBits = descBits(desc->accumAlphaBits, fbconfig.accumAlphaBits);
    fbconfig.auxBuffers     = descBits(desc->auxBuffers, fbconfig.auxBuffers);
    fbconfig.samples        = descBits(desc->samples, fbconfig.samples);
    fbconfig.stereo         = descToggle(desc->stereo, fbconfig.stereo);
    fbconfig.sRGB           = descToggle(desc->srgbCapable, fbconfig.sRGB);
    fbconfig.doublebuffer   = descToggle(desc->doublebuffer, fbconfig.doublebuffer);
    fbconfig.transparent    = descToggle(desc->transparentFramebuffer,
                                         fbconfig.transparent);

    refreshRate = descInt(desc->refreshRate, refreshRate);

    window = createWindow(&wndconfig, &ctxconfig, &fbconfig,
                          desc->monitor, refreshRate);
    if (!window)
        return NULL;

    if (desc->makeCurrent == LUM_ON && ctxconfig.client != LUM_NO_API)
        lumMakeContextCurrent(window);

    return window;
}

void lumDefaultWindowHints(void)
{
    _LUM_REQUIRE_INIT();

    setDefaultConfigs(&_lum.hints.window,
                      &_lum.hints.context,
                      &_lum.hints.framebuffer,
                      &_lum.hints.refreshRate);
}

LUMAPI void lumWindowHint(int hint, int value)
{
    _LUM_REQUIRE_INIT();

    switch (hint)
    {
        case LUM_RED_BITS:
            _lum.hints.framebuffer.redBits = value;
            return;
        case LUM_GREEN_BITS:
            _lum.hints.framebuffer.greenBits = value;
            return;
        case LUM_BLUE_BITS:
            _lum.hints.framebuffer.blueBits = value;
            return;
        case LUM_ALPHA_BITS:
            _lum.hints.framebuffer.alphaBits = value;
            return;
        case LUM_DEPTH_BITS:
            _lum.hints.framebuffer.depthBits = value;
            return;
        case LUM_STENCIL_BITS:
            _lum.hints.framebuffer.stencilBits = value;
            return;
        case LUM_ACCUM_RED_BITS:
            _lum.hints.framebuffer.accumRedBits = value;
            return;
        case LUM_ACCUM_GREEN_BITS:
            _lum.hints.framebuffer.accumGreenBits = value;
            return;
        case LUM_ACCUM_BLUE_BITS:
            _lum.hints.framebuffer.accumBlueBits = value;
            return;
        case LUM_ACCUM_ALPHA_BITS:
            _lum.hints.framebuffer.accumAlphaBits = value;
            return;
        case LUM_AUX_BUFFERS:
            _lum.hints.framebuffer.auxBuffers = value;
            return;
        case LUM_STEREO:
            _lum.hints.framebuffer.stereo = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_DOUBLEBUFFER:
            _lum.hints.framebuffer.doublebuffer = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_TRANSPARENT_FRAMEBUFFER:
            _lum.hints.framebuffer.transparent = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_SAMPLES:
            _lum.hints.framebuffer.samples = value;
            return;
        case LUM_SRGB_CAPABLE:
            _lum.hints.framebuffer.sRGB = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_RESIZABLE:
            _lum.hints.window.resizable = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_DECORATED:
            _lum.hints.window.decorated = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_TITLEBAR:
            _lum.hints.window.titlebar = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_FOCUSED:
            _lum.hints.window.focused = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_AUTO_ICONIFY:
            _lum.hints.window.autoIconify = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_FLOATING:
            _lum.hints.window.floating = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_MAXIMIZED:
            _lum.hints.window.maximized = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_VISIBLE:
            _lum.hints.window.visible = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_COCOA_RETINA_FRAMEBUFFER:
            _lum.hints.window.ns.retina = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_WIN32_KEYBOARD_MENU:
            _lum.hints.window.win32.keymenu = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_COCOA_GRAPHICS_SWITCHING:
            _lum.hints.context.nsgl.offline = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_SCALE_TO_MONITOR:
            _lum.hints.window.scaleToMonitor = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_CENTER_CURSOR:
            _lum.hints.window.centerCursor = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_FOCUS_ON_SHOW:
            _lum.hints.window.focusOnShow = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_MOUSE_PASSTHROUGH:
            _lum.hints.window.mousePassthrough = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_CLIENT_API:
            _lum.hints.context.client = value;
            return;
        case LUM_CONTEXT_CREATION_API:
            _lum.hints.context.source = value;
            return;
        case LUM_CONTEXT_VERSION_MAJOR:
            _lum.hints.context.major = value;
            return;
        case LUM_CONTEXT_VERSION_MINOR:
            _lum.hints.context.minor = value;
            return;
        case LUM_CONTEXT_ROBUSTNESS:
            _lum.hints.context.robustness = value;
            return;
        case LUM_OPENGL_FORWARD_COMPAT:
            _lum.hints.context.forward = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_CONTEXT_DEBUG:
            _lum.hints.context.debug = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_CONTEXT_NO_ERROR:
            _lum.hints.context.noerror = value ? LUM_TRUE : LUM_FALSE;
            return;
        case LUM_OPENGL_PROFILE:
            _lum.hints.context.profile = value;
            return;
        case LUM_CONTEXT_RELEASE_BEHAVIOR:
            _lum.hints.context.release = value;
            return;
        case LUM_REFRESH_RATE:
            _lum.hints.refreshRate = value;
            return;
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid window hint 0x%08X", hint);
}

LUMAPI void lumWindowHintString(int hint, const char* value)
{
    assert(value != NULL);

    _LUM_REQUIRE_INIT();

    switch (hint)
    {
        case LUM_COCOA_FRAME_NAME:
            strncpy(_lum.hints.window.ns.frameName, value,
                    sizeof(_lum.hints.window.ns.frameName) - 1);
            return;
        case LUM_X11_CLASS_NAME:
            strncpy(_lum.hints.window.x11.className, value,
                    sizeof(_lum.hints.window.x11.className) - 1);
            return;
        case LUM_X11_INSTANCE_NAME:
            strncpy(_lum.hints.window.x11.instanceName, value,
                    sizeof(_lum.hints.window.x11.instanceName) - 1);
            return;
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid window hint string 0x%08X", hint);
}

LUMAPI void lumDestroyWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;

    _LUM_REQUIRE_INIT();

    // Allow closing of NULL (to match the behavior of free)
    if (window == NULL)
        return;

    // Clear all callbacks to avoid exposing a half torn-down window object
    memset(&window->callbacks, 0, sizeof(window->callbacks));

    // The window's context must not be current on another thread when the
    // window is destroyed
    if (window == _lumPlatformGetTls(&_lum.contextSlot))
        lumMakeContextCurrent(NULL);

    _lum.platform.destroyWindow(window);

    // Unlink window from global linked list
    {
        _LUMwindow** prev = &_lum.windowListHead;

        while (*prev != window)
            prev = &((*prev)->next);

        *prev = window->next;
    }

    _lum_free(window);
}

LUMAPI int lumWindowShouldClose(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(0);
    return window->shouldClose;
}

LUMAPI void lumSetWindowShouldClose(LUMwindow* handle, int value)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();
    window->shouldClose = value;
}

LUMAPI void lumSetWindowTitle(LUMwindow* handle, const char* title)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);
    assert(title != NULL);

    _LUM_REQUIRE_INIT();
    _lum.platform.setWindowTitle(window, title);
}

LUMAPI void lumSetWindowIcon(LUMwindow* handle,
                               int count, const LUMimage* images)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);
    assert(count >= 0);
    assert(count == 0 || images != NULL);

    _LUM_REQUIRE_INIT();
    _lum.platform.setWindowIcon(window, count, images);
}

LUMAPI void lumGetWindowPos(LUMwindow* handle, int* xpos, int* ypos)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (xpos)
        *xpos = 0;
    if (ypos)
        *ypos = 0;

    _LUM_REQUIRE_INIT();
    _lum.platform.getWindowPos(window, xpos, ypos);
}

LUMAPI void lumSetWindowPos(LUMwindow* handle, int xpos, int ypos)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (window->monitor)
        return;

    _lum.platform.setWindowPos(window, xpos, ypos);
}

LUMAPI void lumGetWindowSize(LUMwindow* handle, int* width, int* height)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LUM_REQUIRE_INIT();
    _lum.platform.getWindowSize(window, width, height);
}

LUMAPI void lumSetWindowSize(LUMwindow* handle, int width, int height)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LUM_REQUIRE_INIT();

    window->videoMode.width  = width;
    window->videoMode.height = height;

    _lum.platform.setWindowSize(window, width, height);
}

LUMAPI void lumSetWindowSizeLimits(LUMwindow* handle,
                                     int minwidth, int minheight,
                                     int maxwidth, int maxheight)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (minwidth != LUM_DONT_CARE && minheight != LUM_DONT_CARE)
    {
        if (minwidth < 0 || minheight < 0)
        {
            _lumInputError(LUM_INVALID_VALUE,
                            "Invalid window minimum size %ix%i",
                            minwidth, minheight);
            return;
        }
    }

    if (maxwidth != LUM_DONT_CARE && maxheight != LUM_DONT_CARE)
    {
        if (maxwidth < 0 || maxheight < 0 ||
            maxwidth < minwidth || maxheight < minheight)
        {
            _lumInputError(LUM_INVALID_VALUE,
                            "Invalid window maximum size %ix%i",
                            maxwidth, maxheight);
            return;
        }
    }

    window->minwidth  = minwidth;
    window->minheight = minheight;
    window->maxwidth  = maxwidth;
    window->maxheight = maxheight;

    if (window->monitor || !window->resizable)
        return;

    _lum.platform.setWindowSizeLimits(window,
                                       minwidth, minheight,
                                       maxwidth, maxheight);
}

LUMAPI void lumSetWindowAspectRatio(LUMwindow* handle, int numer, int denom)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);
    assert(numer != 0);
    assert(denom != 0);

    _LUM_REQUIRE_INIT();

    if (numer != LUM_DONT_CARE && denom != LUM_DONT_CARE)
    {
        if (numer <= 0 || denom <= 0)
        {
            _lumInputError(LUM_INVALID_VALUE,
                            "Invalid window aspect ratio %i:%i",
                            numer, denom);
            return;
        }
    }

    window->numer = numer;
    window->denom = denom;

    if (window->monitor || !window->resizable)
        return;

    _lum.platform.setWindowAspectRatio(window, numer, denom);
}

LUMAPI void lumGetFramebufferSize(LUMwindow* handle, int* width, int* height)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    _LUM_REQUIRE_INIT();
    _lum.platform.getFramebufferSize(window, width, height);
}

LUMAPI void lumGetWindowFrameSize(LUMwindow* handle,
                                    int* left, int* top,
                                    int* right, int* bottom)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (left)
        *left = 0;
    if (top)
        *top = 0;
    if (right)
        *right = 0;
    if (bottom)
        *bottom = 0;

    _LUM_REQUIRE_INIT();
    _lum.platform.getWindowFrameSize(window, left, top, right, bottom);
}

LUMAPI void lumGetWindowContentScale(LUMwindow* handle,
                                       float* xscale, float* yscale)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    if (xscale)
        *xscale = 0.f;
    if (yscale)
        *yscale = 0.f;

    _LUM_REQUIRE_INIT();
    _lum.platform.getWindowContentScale(window, xscale, yscale);
}

LUMAPI float lumGetWindowOpacity(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(1.f);
    return _lum.platform.getWindowOpacity(window);
}

LUMAPI void lumSetWindowOpacity(LUMwindow* handle, float opacity)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);
    assert(opacity == opacity);
    assert(opacity >= 0.f);
    assert(opacity <= 1.f);

    _LUM_REQUIRE_INIT();

    if (opacity != opacity || opacity < 0.f || opacity > 1.f)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid window opacity %f", opacity);
        return;
    }

    _lum.platform.setWindowOpacity(window, opacity);
}

LUMAPI void lumIconifyWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();
    _lum.platform.iconifyWindow(window);
}

LUMAPI void lumRestoreWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();
    _lum.platform.restoreWindow(window);
}

LUMAPI void lumMaximizeWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (window->monitor)
        return;

    _lum.platform.maximizeWindow(window);
}

LUMAPI void lumShowWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (window->monitor)
        return;

    _lum.platform.showWindow(window);

    if (window->focusOnShow)
        _lum.platform.focusWindow(window);
}

LUMAPI void lumRequestWindowAttention(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    _lum.platform.requestWindowAttention(window);
}

LUMAPI void lumHideWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    if (window->monitor)
        return;

    _lum.platform.hideWindow(window);
}

LUMAPI void lumFocusWindow(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    _lum.platform.focusWindow(window);
}

LUMAPI int lumGetWindowAttrib(LUMwindow* handle, int attrib)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(0);

    switch (attrib)
    {
        case LUM_FOCUSED:
            return _lum.platform.windowFocused(window);
        case LUM_ICONIFIED:
            return _lum.platform.windowIconified(window);
        case LUM_VISIBLE:
            return _lum.platform.windowVisible(window);
        case LUM_MAXIMIZED:
            return _lum.platform.windowMaximized(window);
        case LUM_HOVERED:
            return _lum.platform.windowHovered(window);
        case LUM_FOCUS_ON_SHOW:
            return window->focusOnShow;
        case LUM_MOUSE_PASSTHROUGH:
            return window->mousePassthrough;
        case LUM_TRANSPARENT_FRAMEBUFFER:
            return _lum.platform.framebufferTransparent(window);
        case LUM_RESIZABLE:
            return window->resizable;
        case LUM_DECORATED:
            return window->decorated;
        case LUM_TITLEBAR:
            return _lum.hints.window.titlebar;
        case LUM_FLOATING:
            return window->floating;
        case LUM_AUTO_ICONIFY:
            return window->autoIconify;
        case LUM_DOUBLEBUFFER:
            return window->doublebuffer;
        case LUM_CLIENT_API:
            return window->context.client;
        case LUM_CONTEXT_CREATION_API:
            return window->context.source;
        case LUM_CONTEXT_VERSION_MAJOR:
            return window->context.major;
        case LUM_CONTEXT_VERSION_MINOR:
            return window->context.minor;
        case LUM_CONTEXT_REVISION:
            return window->context.revision;
        case LUM_CONTEXT_ROBUSTNESS:
            return window->context.robustness;
        case LUM_OPENGL_FORWARD_COMPAT:
            return window->context.forward;
        case LUM_CONTEXT_DEBUG:
            return window->context.debug;
        case LUM_OPENGL_PROFILE:
            return window->context.profile;
        case LUM_CONTEXT_RELEASE_BEHAVIOR:
            return window->context.release;
        case LUM_CONTEXT_NO_ERROR:
            return window->context.noerror;
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
    return 0;
}

LUMAPI void lumSetWindowAttrib(LUMwindow* handle, int attrib, int value)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();

    value = value ? LUM_TRUE : LUM_FALSE;

    switch (attrib)
    {
        case LUM_AUTO_ICONIFY:
            window->autoIconify = value;
            return;

        case LUM_RESIZABLE:
            window->resizable = value;
            if (!window->monitor)
                _lum.platform.setWindowResizable(window, value);
            return;

        case LUM_DECORATED:
            window->decorated = value;
            if (!window->monitor)
                _lum.platform.setWindowDecorated(window, value);
		case LUM_TITLEBAR:
			if (_lum.hints.window.titlebar == value)
            	return;

            _lum.hints.window.titlebar = value;
    	    if (!window->monitor)
                _lum.platform.setWindowTitleBar(window, value);
            return;
        case LUM_FLOATING:
            window->floating = value;
            if (!window->monitor)
                _lum.platform.setWindowFloating(window, value);
            return;

        case LUM_FOCUS_ON_SHOW:
            window->focusOnShow = value;
            return;

        case LUM_MOUSE_PASSTHROUGH:
            window->mousePassthrough = value;
            _lum.platform.setWindowMousePassthrough(window, value);
            return;
    }

    _lumInputError(LUM_INVALID_ENUM, "Invalid window attribute 0x%08X", attrib);
}

LUMAPI LUMmonitor* lumGetWindowMonitor(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return (LUMmonitor*) window->monitor;
}

LUMAPI void lumSetWindowMonitor(LUMwindow* wh,
                                  LUMmonitor* mh,
                                  int xpos, int ypos,
                                  int width, int height,
                                  int refreshRate)
{
    _LUMwindow* window = (_LUMwindow*) wh;
    _LUMmonitor* monitor = (_LUMmonitor*) mh;
    assert(window != NULL);
    assert(width >= 0);
    assert(height >= 0);

    _LUM_REQUIRE_INIT();

    if (width <= 0 || height <= 0)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Invalid window size %ix%i",
                        width, height);
        return;
    }

    if (refreshRate < 0 && refreshRate != LUM_DONT_CARE)
    {
        _lumInputError(LUM_INVALID_VALUE,
                        "Invalid refresh rate %i",
                        refreshRate);
        return;
    }

    window->videoMode.width       = width;
    window->videoMode.height      = height;
    window->videoMode.refreshRate = refreshRate;

    _lum.platform.setWindowMonitor(window, monitor,
                                    xpos, ypos, width, height,
                                    refreshRate);
}

LUMAPI void lumSetWindowUserPointer(LUMwindow* handle, void* pointer)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT();
    window->userPointer = pointer;
}

LUMAPI void* lumGetWindowUserPointer(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    return window->userPointer;
}

LUMAPI LUMwindowposfun lumSetWindowPosCallback(LUMwindow* handle,
                                                  LUMwindowposfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowposfun, window->callbacks.pos, cbfun);
    return cbfun;
}

LUMAPI LUMtitlebarhittestfun lumSetTitlebarHitTestCallback(LUMwindow* handle, LUMtitlebarhittestfun tbhtfun)
{
    _LUMwindow* window = (_LUMwindow*)handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMtitlebarhittestfun, window->callbacks.tbhittest, tbhtfun);
    return tbhtfun;
}

LUMAPI LUMwindowsizefun lumSetWindowSizeCallback(LUMwindow* handle,
                                                    LUMwindowsizefun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowsizefun, window->callbacks.size, cbfun);
    return cbfun;
}

LUMAPI LUMwindowclosefun lumSetWindowCloseCallback(LUMwindow* handle,
                                                      LUMwindowclosefun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowclosefun, window->callbacks.close, cbfun);
    return cbfun;
}

LUMAPI LUMwindowrefreshfun lumSetWindowRefreshCallback(LUMwindow* handle,
                                                          LUMwindowrefreshfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowrefreshfun, window->callbacks.refresh, cbfun);
    return cbfun;
}

LUMAPI LUMwindowfocusfun lumSetWindowFocusCallback(LUMwindow* handle,
                                                      LUMwindowfocusfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowfocusfun, window->callbacks.focus, cbfun);
    return cbfun;
}

LUMAPI LUMwindowiconifyfun lumSetWindowIconifyCallback(LUMwindow* handle,
                                                          LUMwindowiconifyfun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowiconifyfun, window->callbacks.iconify, cbfun);
    return cbfun;
}

LUMAPI LUMwindowmaximizefun lumSetWindowMaximizeCallback(LUMwindow* handle,
                                                            LUMwindowmaximizefun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowmaximizefun, window->callbacks.maximize, cbfun);
    return cbfun;
}

LUMAPI LUMframebuffersizefun lumSetFramebufferSizeCallback(LUMwindow* handle,
                                                              LUMframebuffersizefun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMframebuffersizefun, window->callbacks.fbsize, cbfun);
    return cbfun;
}

LUMAPI LUMwindowcontentscalefun lumSetWindowContentScaleCallback(LUMwindow* handle,
                                                                    LUMwindowcontentscalefun cbfun)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    assert(window != NULL);

    _LUM_REQUIRE_INIT_OR_RETURN(NULL);
    _LUM_SWAP(LUMwindowcontentscalefun, window->callbacks.scale, cbfun);
    return cbfun;
}

LUMAPI void lumPollEvents(void)
{
    _LUM_REQUIRE_INIT();
    _lum.platform.pollEvents();
}

LUMAPI void lumWaitEvents(void)
{
    _LUM_REQUIRE_INIT();
    _lum.platform.waitEvents();
}

LUMAPI void lumWaitEventsTimeout(double timeout)
{
    _LUM_REQUIRE_INIT();
    assert(timeout == timeout);
    assert(timeout >= 0.0);
    assert(timeout <= DBL_MAX);

    if (timeout != timeout || timeout < 0.0 || timeout > DBL_MAX)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid time %f", timeout);
        return;
    }

    _lum.platform.waitEventsTimeout(timeout);
}

LUMAPI void lumPostEmptyEvent(void)
{
    _LUM_REQUIRE_INIT();
    _lum.platform.postEmptyEvent();
}

