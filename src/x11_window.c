//========================================================================
// Lumina X11 - fork of GLFW 3.4 (www.glfw.org)
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <X11/cursorfont.h>
#include <X11/Xmd.h>

#include <sys/select.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>

// Action for EWMH client messages
#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1
#define _NET_WM_STATE_TOGGLE        2

// Additional mouse button names for XButtonEvent
#define Button6            6
#define Button7            7

// Motif WM hints flags
#define MWM_HINTS_DECORATIONS   2
#define MWM_DECOR_ALL           1

#define _LUM_XDND_VERSION 5


// Wait for data to arrive using select
// This avoids blocking other threads via the per-display Xlib lock that also
// covers GLX functions
//
static LUMbool waitForEvent(double* timeout)
{
    fd_set fds;
    const int fd = ConnectionNumber(_lum.x11.display);
    int count = fd + 1;

#if defined(__linux__)
    if (_lum.linjs.inotify > fd)
        count = _lum.linjs.inotify + 1;
#endif
    for (;;)
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
#if defined(__linux__)
        if (_lum.linjs.inotify > 0)
            FD_SET(_lum.linjs.inotify, &fds);
#endif

        if (timeout)
        {
            const long seconds = (long) *timeout;
            const long microseconds = (long) ((*timeout - seconds) * 1e6);
            struct timeval tv = { seconds, microseconds };
            const uint64_t base = _lumPlatformGetTimerValue();

            const int result = select(count, &fds, NULL, NULL, &tv);
            const int error = errno;

            *timeout -= (_lumPlatformGetTimerValue() - base) /
                (double) _lumPlatformGetTimerFrequency();

            if (result > 0)
                return LUM_TRUE;
            if ((result == -1 && error == EINTR) || *timeout <= 0.0)
                return LUM_FALSE;
        }
        else if (select(count, &fds, NULL, NULL, NULL) != -1 || errno != EINTR)
            return LUM_TRUE;
    }
}

// Waits until a VisibilityNotify event arrives for the specified window or the
// timeout period elapses (ICCCM section 4.2.2)
//
static LUMbool waitForVisibilityNotify(_LUMwindow* window)
{
    XEvent dummy;
    double timeout = 0.1;

    while (!XCheckTypedWindowEvent(_lum.x11.display,
                                   window->x11.handle,
                                   VisibilityNotify,
                                   &dummy))
    {
        if (!waitForEvent(&timeout))
            return LUM_FALSE;
    }

    return LUM_TRUE;
}

// Returns whether the window is iconified
//
static int getWindowState(_LUMwindow* window)
{
    int result = WithdrawnState;
    struct {
        CARD32 state;
        Window icon;
    } *state = NULL;

    if (_lumGetWindowPropertyX11(window->x11.handle,
                                  _lum.x11.WM_STATE,
                                  _lum.x11.WM_STATE,
                                  (unsigned char**) &state) >= 2)
    {
        result = state->state;
    }

    if (state)
        XFree(state);

    return result;
}

// Returns whether the event is a selection event
//
static Bool isSelectionEvent(Display* display, XEvent* event, XPointer pointer)
{
    if (event->xany.window != _lum.x11.helperWindowHandle)
        return False;

    return event->type == SelectionRequest ||
           event->type == SelectionNotify ||
           event->type == SelectionClear;
}

// Returns whether it is a _NET_FRAME_EXTENTS event for the specified window
//
static Bool isFrameExtentsEvent(Display* display, XEvent* event, XPointer pointer)
{
    _LUMwindow* window = (_LUMwindow*) pointer;
    return event->type == PropertyNotify &&
           event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == window->x11.handle &&
           event->xproperty.atom == _lum.x11.NET_FRAME_EXTENTS;
}

// Returns whether it is a property event for the specified selection transfer
//
static Bool isSelPropNewValueNotify(Display* display, XEvent* event, XPointer pointer)
{
    XEvent* notification = (XEvent*) pointer;
    return event->type == PropertyNotify &&
           event->xproperty.state == PropertyNewValue &&
           event->xproperty.window == notification->xselection.requestor &&
           event->xproperty.atom == notification->xselection.property;
}

// Translates an X event modifier state mask
//
static int translateState(int state)
{
    int mods = 0;

    if (state & ShiftMask)
        mods |= LUM_MOD_SHIFT;
    if (state & ControlMask)
        mods |= LUM_MOD_CONTROL;
    if (state & Mod1Mask)
        mods |= LUM_MOD_ALT;
    if (state & Mod4Mask)
        mods |= LUM_MOD_SUPER;
    if (state & LockMask)
        mods |= LUM_MOD_CAPS_LOCK;
    if (state & Mod2Mask)
        mods |= LUM_MOD_NUM_LOCK;

    return mods;
}

// Translates an X11 key code to a Lumina key token
//
static int translateKey(int scancode)
{
    // Use the pre-filled LUT (see createKeyTables() in x11_init.c)
    if (scancode < 0 || scancode > 255)
        return LUM_KEY_UNKNOWN;

    return _lum.x11.keycodes[scancode];
}

// Sends an EWMH or ICCCM event to the window manager
//
static void sendEventToWM(_LUMwindow* window, Atom type,
                          long a, long b, long c, long d, long e)
{
    XEvent event = { ClientMessage };
    event.xclient.window = window->x11.handle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = type;
    event.xclient.data.l[0] = a;
    event.xclient.data.l[1] = b;
    event.xclient.data.l[2] = c;
    event.xclient.data.l[3] = d;
    event.xclient.data.l[4] = e;

    XSendEvent(_lum.x11.display, _lum.x11.root,
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &event);
}

// Updates the normal hints according to the window settings
//
static void updateNormalHints(_LUMwindow* window, int width, int height)
{
    XSizeHints* hints = XAllocSizeHints();

    if (!window->monitor)
    {
        if (window->resizable)
        {
            if (window->minwidth != LUM_DONT_CARE &&
                window->minheight != LUM_DONT_CARE)
            {
                hints->flags |= PMinSize;
                hints->min_width = window->minwidth;
                hints->min_height = window->minheight;
            }

            if (window->maxwidth != LUM_DONT_CARE &&
                window->maxheight != LUM_DONT_CARE)
            {
                hints->flags |= PMaxSize;
                hints->max_width = window->maxwidth;
                hints->max_height = window->maxheight;
            }

            if (window->numer != LUM_DONT_CARE &&
                window->denom != LUM_DONT_CARE)
            {
                hints->flags |= PAspect;
                hints->min_aspect.x = hints->max_aspect.x = window->numer;
                hints->min_aspect.y = hints->max_aspect.y = window->denom;
            }
        }
        else
        {
            hints->flags |= (PMinSize | PMaxSize);
            hints->min_width  = hints->max_width  = width;
            hints->min_height = hints->max_height = height;
        }
    }

    hints->flags |= PWinGravity;
    hints->win_gravity = StaticGravity;

    XSetWMNormalHints(_lum.x11.display, window->x11.handle, hints);
    XFree(hints);
}

// Updates the full screen status of the window
//
static void updateWindowMode(_LUMwindow* window)
{
    if (window->monitor)
    {
        if (_lum.x11.xinerama.available &&
            _lum.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            sendEventToWM(window,
                          _lum.x11.NET_WM_FULLSCREEN_MONITORS,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          window->monitor->x11.index,
                          0);
        }

        if (_lum.x11.NET_WM_STATE && _lum.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window,
                          _lum.x11.NET_WM_STATE,
                          _NET_WM_STATE_ADD,
                          _lum.x11.NET_WM_STATE_FULLSCREEN,
                          0, 1, 0);
        }
        else
        {
            // This is the butcher's way of removing window decorations
            // Setting the override-redirect attribute on a window makes the
            // window manager ignore the window completely (ICCCM, section 4)
            // The good thing is that this makes undecorated full screen windows
            // easy to do; the bad thing is that we have to do everything
            // manually and some things (like iconify/restore) won't work at
            // all, as those are tasks usually performed by the window manager

            XSetWindowAttributes attributes;
            attributes.override_redirect = True;
            XChangeWindowAttributes(_lum.x11.display,
                                    window->x11.handle,
                                    CWOverrideRedirect,
                                    &attributes);

            window->x11.overrideRedirect = LUM_TRUE;
        }

        // Enable compositor bypass
        if (!window->x11.transparent)
        {
            const unsigned long value = 1;

            XChangeProperty(_lum.x11.display,  window->x11.handle,
                            _lum.x11.NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                            PropModeReplace, (unsigned char*) &value, 1);
        }
    }
    else
    {
        if (_lum.x11.xinerama.available &&
            _lum.x11.NET_WM_FULLSCREEN_MONITORS)
        {
            XDeleteProperty(_lum.x11.display, window->x11.handle,
                            _lum.x11.NET_WM_FULLSCREEN_MONITORS);
        }

        if (_lum.x11.NET_WM_STATE && _lum.x11.NET_WM_STATE_FULLSCREEN)
        {
            sendEventToWM(window,
                          _lum.x11.NET_WM_STATE,
                          _NET_WM_STATE_REMOVE,
                          _lum.x11.NET_WM_STATE_FULLSCREEN,
                          0, 1, 0);
        }
        else
        {
            XSetWindowAttributes attributes;
            attributes.override_redirect = False;
            XChangeWindowAttributes(_lum.x11.display,
                                    window->x11.handle,
                                    CWOverrideRedirect,
                                    &attributes);

            window->x11.overrideRedirect = LUM_FALSE;
        }

        // Disable compositor bypass
        if (!window->x11.transparent)
        {
            XDeleteProperty(_lum.x11.display, window->x11.handle,
                            _lum.x11.NET_WM_BYPASS_COMPOSITOR);
        }
    }
}

// Splits and translates a text/uri-list into separate file paths
// NOTE: This function destroys the provided string
//
static char** parseUriList(char* text, int* count)
{
    const char* prefix = "file://";
    char** paths = NULL;
    char* line;

    *count = 0;

    while ((line = strtok(text, "\r\n")))
    {
        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0)
        {
            line += strlen(prefix);
            // TODO: Validate hostname
            while (*line != '/')
                line++;
        }

        (*count)++;

        char* path = _lum_calloc(strlen(line) + 1, 1);
        paths = _lum_realloc(paths, *count * sizeof(char*));
        paths[*count - 1] = path;

        while (*line)
        {
            if (line[0] == '%' && line[1] && line[2])
            {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = strtol(digits, NULL, 16);
                line += 2;
            }
            else
                *path = *line;

            path++;
            line++;
        }
    }

    return paths;
}

// Decode a Unicode code point from a UTF-8 stream
// Based on cutef8 by Jeff Bezanson (Public Domain)
//
static uint32_t decodeUTF8(const char** s)
{
    uint32_t codepoint = 0, count = 0;
    static const uint32_t offsets[] =
    {
        0x00000000u, 0x00003080u, 0x000e2080u,
        0x03c82080u, 0xfa082080u, 0x82082080u
    };

    do
    {
        codepoint = (codepoint << 6) + (unsigned char) **s;
        (*s)++;
        count++;
    } while ((**s & 0xc0) == 0x80);

    assert(count <= 6);
    return codepoint - offsets[count - 1];
}

// Convert the specified Latin-1 string to UTF-8
//
static char* convertLatin1toUTF8(const char* source)
{
    size_t size = 1;
    const char* sp;

    for (sp = source;  *sp;  sp++)
        size += (*sp & 0x80) ? 2 : 1;

    char* target = _lum_calloc(size, 1);
    char* tp = target;

    for (sp = source;  *sp;  sp++)
        tp += _lumEncodeUTF8(tp, *sp);

    return target;
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_LUMwindow* window)
{
    if (window->cursorMode == LUM_CURSOR_NORMAL)
    {
        if (window->cursor)
        {
            XDefineCursor(_lum.x11.display, window->x11.handle,
                          window->cursor->x11.handle);
        }
        else
            XUndefineCursor(_lum.x11.display, window->x11.handle);
    }
    else
    {
        XDefineCursor(_lum.x11.display, window->x11.handle,
                      _lum.x11.hiddenCursorHandle);
    }
}

// Enable XI2 raw mouse motion events
//
static void enableRawMouseMotion(_LUMwindow* window)
{
    XIEventMask em;
    unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;
    XISetMask(mask, XI_RawMotion);

    XISelectEvents(_lum.x11.display, _lum.x11.root, &em, 1);
}

// Disable XI2 raw mouse motion events
//
static void disableRawMouseMotion(_LUMwindow* window)
{
    XIEventMask em;
    unsigned char mask[] = { 0 };

    em.deviceid = XIAllMasterDevices;
    em.mask_len = sizeof(mask);
    em.mask = mask;

    XISelectEvents(_lum.x11.display, _lum.x11.root, &em, 1);
}

// Apply disabled cursor mode to a focused window
//
static void disableCursor(_LUMwindow* window)
{
    if (window->rawMouseMotion)
        enableRawMouseMotion(window);

    _lum.x11.disabledCursorWindow = window;
    _lumGetCursorPosX11(window,
                         &_lum.x11.restoreCursorPosX,
                         &_lum.x11.restoreCursorPosY);
    updateCursorImage(window);
    _lumCenterCursorInContentArea(window);
    XGrabPointer(_lum.x11.display, window->x11.handle, True,
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync,
                 window->x11.handle,
                 _lum.x11.hiddenCursorHandle,
                 CurrentTime);
}

// Exit disabled cursor mode for the specified window
//
static void enableCursor(_LUMwindow* window)
{
    if (window->rawMouseMotion)
        disableRawMouseMotion(window);

    _lum.x11.disabledCursorWindow = NULL;
    XUngrabPointer(_lum.x11.display, CurrentTime);
    _lumSetCursorPosX11(window,
                         _lum.x11.restoreCursorPosX,
                         _lum.x11.restoreCursorPosY);
    updateCursorImage(window);
}

// Clear its handle when the input context has been destroyed
//
static void inputContextDestroyCallback(XIC ic, XPointer clientData, XPointer callData)
{
    _LUMwindow* window = (_LUMwindow*) clientData;
    window->x11.ic = NULL;
}

// Create the X11 window (and its colormap)
//
static LUMbool createNativeWindow(_LUMwindow* window,
                                   const _LUMwndconfig* wndconfig,
                                   Visual* visual, int depth)
{
    int width = wndconfig->width;
    int height = wndconfig->height;

    if (wndconfig->scaleToMonitor)
    {
        width *= _lum.x11.contentScaleX;
        height *= _lum.x11.contentScaleY;
    }

    // Create a colormap based on the visual used by the current context
    window->x11.colormap = XCreateColormap(_lum.x11.display,
                                           _lum.x11.root,
                                           visual,
                                           AllocNone);

    window->x11.transparent = _lumIsVisualTransparentX11(visual);

    XSetWindowAttributes wa = { 0 };
    wa.colormap = window->x11.colormap;
    wa.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                    ExposureMask | FocusChangeMask | VisibilityChangeMask |
                    EnterWindowMask | LeaveWindowMask | PropertyChangeMask;

    _lumGrabErrorHandlerX11();

    window->x11.parent = _lum.x11.root;
    window->x11.handle = XCreateWindow(_lum.x11.display,
                                       _lum.x11.root,
                                       0, 0,   // Position
                                       width, height,
                                       0,      // Border width
                                       depth,  // Color depth
                                       InputOutput,
                                       visual,
                                       CWBorderPixel | CWColormap | CWEventMask,
                                       &wa);

    _lumReleaseErrorHandlerX11();

    if (!window->x11.handle)
    {
        _lumInputErrorX11(LUM_PLATFORM_ERROR,
                           "X11: Failed to create window");
        return LUM_FALSE;
    }

    XSaveContext(_lum.x11.display,
                 window->x11.handle,
                 _lum.x11.context,
                 (XPointer) window);

    if (!wndconfig->decorated)
        _lumSetWindowDecoratedX11(window, LUM_FALSE);

    if (_lum.x11.NET_WM_STATE && !window->monitor)
    {
        Atom states[3];
        int count = 0;

        if (wndconfig->floating)
        {
            if (_lum.x11.NET_WM_STATE_ABOVE)
                states[count++] = _lum.x11.NET_WM_STATE_ABOVE;
        }

        if (wndconfig->maximized)
        {
            if (_lum.x11.NET_WM_STATE_MAXIMIZED_VERT &&
                _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ)
            {
                states[count++] = _lum.x11.NET_WM_STATE_MAXIMIZED_VERT;
                states[count++] = _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ;
                window->x11.maximized = LUM_TRUE;
            }
        }

        if (count)
        {
            XChangeProperty(_lum.x11.display, window->x11.handle,
                            _lum.x11.NET_WM_STATE, XA_ATOM, 32,
                            PropModeReplace, (unsigned char*) states, count);
        }
    }

    // Declare the WM protocols supported by Lumina
    {
        Atom protocols[] =
        {
            _lum.x11.WM_DELETE_WINDOW,
            _lum.x11.NET_WM_PING
        };

        XSetWMProtocols(_lum.x11.display, window->x11.handle,
                        protocols, sizeof(protocols) / sizeof(Atom));
    }

    // Declare our PID
    {
        const long pid = getpid();

        XChangeProperty(_lum.x11.display,  window->x11.handle,
                        _lum.x11.NET_WM_PID, XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char*) &pid, 1);
    }

    if (_lum.x11.NET_WM_WINDOW_TYPE && _lum.x11.NET_WM_WINDOW_TYPE_NORMAL)
    {
        Atom type = _lum.x11.NET_WM_WINDOW_TYPE_NORMAL;
        XChangeProperty(_lum.x11.display,  window->x11.handle,
                        _lum.x11.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &type, 1);
    }

    // Set ICCCM WM_HINTS property
    {
        XWMHints* hints = XAllocWMHints();
        if (!hints)
        {
            _lumInputError(LUM_OUT_OF_MEMORY,
                            "X11: Failed to allocate WM hints");
            return LUM_FALSE;
        }

        hints->flags = StateHint;
        hints->initial_state = NormalState;

        XSetWMHints(_lum.x11.display, window->x11.handle, hints);
        XFree(hints);
    }

    updateNormalHints(window, width, height);

    // Set ICCCM WM_CLASS property
    {
        XClassHint* hint = XAllocClassHint();

        if (strlen(wndconfig->x11.instanceName) &&
            strlen(wndconfig->x11.className))
        {
            hint->res_name = (char*) wndconfig->x11.instanceName;
            hint->res_class = (char*) wndconfig->x11.className;
        }
        else
        {
            const char* resourceName = getenv("RESOURCE_NAME");
            if (resourceName && strlen(resourceName))
                hint->res_name = (char*) resourceName;
            else if (strlen(wndconfig->title))
                hint->res_name = (char*) wndconfig->title;
            else
                hint->res_name = (char*) "lumina-application";

            if (strlen(wndconfig->title))
                hint->res_class = (char*) wndconfig->title;
            else
                hint->res_class = (char*) "Lumina-Application";
        }

        XSetClassHint(_lum.x11.display, window->x11.handle, hint);
        XFree(hint);
    }

    // Announce support for Xdnd (drag and drop)
    {
        const Atom version = _LUM_XDND_VERSION;
        XChangeProperty(_lum.x11.display, window->x11.handle,
                        _lum.x11.XdndAware, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*) &version, 1);
    }

    if (_lum.x11.im)
        _lumCreateInputContextX11(window);

    _lumSetWindowTitleX11(window, wndconfig->title);
    _lumGetWindowPosX11(window, &window->x11.xpos, &window->x11.ypos);
    _lumGetWindowSizeX11(window, &window->x11.width, &window->x11.height);

    return LUM_TRUE;
}

// Set the specified property to the selection converted to the requested target
//
static Atom writeTargetToProperty(const XSelectionRequestEvent* request)
{
    char* selectionString = NULL;
    const Atom formats[] = { _lum.x11.UTF8_STRING, XA_STRING };
    const int formatCount = sizeof(formats) / sizeof(formats[0]);

    if (request->selection == _lum.x11.PRIMARY)
        selectionString = _lum.x11.primarySelectionString;
    else
        selectionString = _lum.x11.clipboardString;

    if (request->property == None)
    {
        // The requester is a legacy client (ICCCM section 2.2)
        // We don't support legacy clients, so fail here
        return None;
    }

    if (request->target == _lum.x11.TARGETS)
    {
        // The list of supported targets was requested

        const Atom targets[] = { _lum.x11.TARGETS,
                                 _lum.x11.MULTIPLE,
                                 _lum.x11.UTF8_STRING,
                                 XA_STRING };

        XChangeProperty(_lum.x11.display,
                        request->requestor,
                        request->property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        sizeof(targets) / sizeof(targets[0]));

        return request->property;
    }

    if (request->target == _lum.x11.MULTIPLE)
    {
        // Multiple conversions were requested

        Atom* targets;
        const unsigned long count =
            _lumGetWindowPropertyX11(request->requestor,
                                      request->property,
                                      _lum.x11.ATOM_PAIR,
                                      (unsigned char**) &targets);

        for (unsigned long i = 0;  i < count;  i += 2)
        {
            int j;

            for (j = 0;  j < formatCount;  j++)
            {
                if (targets[i] == formats[j])
                    break;
            }

            if (j < formatCount)
            {
                XChangeProperty(_lum.x11.display,
                                request->requestor,
                                targets[i + 1],
                                targets[i],
                                8,
                                PropModeReplace,
                                (unsigned char *) selectionString,
                                strlen(selectionString));
            }
            else
                targets[i + 1] = None;
        }

        XChangeProperty(_lum.x11.display,
                        request->requestor,
                        request->property,
                        _lum.x11.ATOM_PAIR,
                        32,
                        PropModeReplace,
                        (unsigned char*) targets,
                        count);

        XFree(targets);

        return request->property;
    }

    if (request->target == _lum.x11.SAVE_TARGETS)
    {
        // The request is a check whether we support SAVE_TARGETS
        // It should be handled as a no-op side effect target

        XChangeProperty(_lum.x11.display,
                        request->requestor,
                        request->property,
                        _lum.x11.NULL_,
                        32,
                        PropModeReplace,
                        NULL,
                        0);

        return request->property;
    }

    // Conversion to a data target was requested

    for (int i = 0;  i < formatCount;  i++)
    {
        if (request->target == formats[i])
        {
            // The requested target is one we support

            XChangeProperty(_lum.x11.display,
                            request->requestor,
                            request->property,
                            request->target,
                            8,
                            PropModeReplace,
                            (unsigned char *) selectionString,
                            strlen(selectionString));

            return request->property;
        }
    }

    // The requested target is not supported

    return None;
}

static void handleSelectionClear(XEvent* event)
{
    if (event->xselectionclear.selection == _lum.x11.PRIMARY)
    {
        _lum_free(_lum.x11.primarySelectionString);
        _lum.x11.primarySelectionString = NULL;
    }
    else
    {
        _lum_free(_lum.x11.clipboardString);
        _lum.x11.clipboardString = NULL;
    }
}

static void handleSelectionRequest(XEvent* event)
{
    const XSelectionRequestEvent* request = &event->xselectionrequest;

    XEvent reply = { SelectionNotify };
    reply.xselection.property = writeTargetToProperty(request);
    reply.xselection.display = request->display;
    reply.xselection.requestor = request->requestor;
    reply.xselection.selection = request->selection;
    reply.xselection.target = request->target;
    reply.xselection.time = request->time;

    XSendEvent(_lum.x11.display, request->requestor, False, 0, &reply);
}

static const char* getSelectionString(Atom selection)
{
    char** selectionString = NULL;
    const Atom targets[] = { _lum.x11.UTF8_STRING, XA_STRING };
    const size_t targetCount = sizeof(targets) / sizeof(targets[0]);

    if (selection == _lum.x11.PRIMARY)
        selectionString = &_lum.x11.primarySelectionString;
    else
        selectionString = &_lum.x11.clipboardString;

    if (XGetSelectionOwner(_lum.x11.display, selection) ==
        _lum.x11.helperWindowHandle)
    {
        // Instead of doing a large number of X round-trips just to put this
        // string into a window property and then read it back, just return it
        return *selectionString;
    }

    _lum_free(*selectionString);
    *selectionString = NULL;

    for (size_t i = 0;  i < targetCount;  i++)
    {
        char* data;
        Atom actualType;
        int actualFormat;
        unsigned long itemCount, bytesAfter;
        XEvent notification, dummy;

        XConvertSelection(_lum.x11.display,
                          selection,
                          targets[i],
                          _lum.x11.LUM_SELECTION,
                          _lum.x11.helperWindowHandle,
                          CurrentTime);

        while (!XCheckTypedWindowEvent(_lum.x11.display,
                                       _lum.x11.helperWindowHandle,
                                       SelectionNotify,
                                       &notification))
        {
            waitForEvent(NULL);
        }

        if (notification.xselection.property == None)
            continue;

        XCheckIfEvent(_lum.x11.display,
                      &dummy,
                      isSelPropNewValueNotify,
                      (XPointer) &notification);

        XGetWindowProperty(_lum.x11.display,
                           notification.xselection.requestor,
                           notification.xselection.property,
                           0,
                           LONG_MAX,
                           True,
                           AnyPropertyType,
                           &actualType,
                           &actualFormat,
                           &itemCount,
                           &bytesAfter,
                           (unsigned char**) &data);

        if (actualType == _lum.x11.INCR)
        {
            size_t size = 1;
            char* string = NULL;

            for (;;)
            {
                while (!XCheckIfEvent(_lum.x11.display,
                                      &dummy,
                                      isSelPropNewValueNotify,
                                      (XPointer) &notification))
                {
                    waitForEvent(NULL);
                }

                XFree(data);
                XGetWindowProperty(_lum.x11.display,
                                   notification.xselection.requestor,
                                   notification.xselection.property,
                                   0,
                                   LONG_MAX,
                                   True,
                                   AnyPropertyType,
                                   &actualType,
                                   &actualFormat,
                                   &itemCount,
                                   &bytesAfter,
                                   (unsigned char**) &data);

                if (itemCount)
                {
                    size += itemCount;
                    string = _lum_realloc(string, size);
                    string[size - itemCount - 1] = '\0';
                    strcat(string, data);
                }

                if (!itemCount)
                {
                    if (targets[i] == XA_STRING)
                    {
                        *selectionString = convertLatin1toUTF8(string);
                        _lum_free(string);
                    }
                    else
                        *selectionString = string;

                    break;
                }
            }
        }
        else if (actualType == targets[i])
        {
            if (targets[i] == XA_STRING)
                *selectionString = convertLatin1toUTF8(data);
            else
                *selectionString = _lum_strdup(data);
        }

        XFree(data);

        if (*selectionString)
            break;
    }

    if (!*selectionString)
    {
        _lumInputError(LUM_FORMAT_UNAVAILABLE,
                        "X11: Failed to convert selection to string");
    }

    return *selectionString;
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_LUMwindow* window)
{
    if (_lum.x11.saver.count == 0)
    {
        // Remember old screen saver settings
        XGetScreenSaver(_lum.x11.display,
                        &_lum.x11.saver.timeout,
                        &_lum.x11.saver.interval,
                        &_lum.x11.saver.blanking,
                        &_lum.x11.saver.exposure);

        // Disable screen saver
        XSetScreenSaver(_lum.x11.display, 0, 0, DontPreferBlanking,
                        DefaultExposures);
    }

    if (!window->monitor->window)
        _lum.x11.saver.count++;

    _lumSetVideoModeX11(window->monitor, &window->videoMode);

    if (window->x11.overrideRedirect)
    {
        int xpos, ypos;
        LUMvidmode mode;

        // Manually position the window over its monitor
        _lumGetMonitorPosX11(window->monitor, &xpos, &ypos);
        _lumGetVideoModeX11(window->monitor, &mode);

        XMoveResizeWindow(_lum.x11.display, window->x11.handle,
                          xpos, ypos, mode.width, mode.height);
    }

    _lumInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_LUMwindow* window)
{
    if (window->monitor->window != window)
        return;

    _lumInputMonitorWindow(window->monitor, NULL);
    _lumRestoreVideoModeX11(window->monitor);

    _lum.x11.saver.count--;

    if (_lum.x11.saver.count == 0)
    {
        // Restore old screen saver settings
        XSetScreenSaver(_lum.x11.display,
                        _lum.x11.saver.timeout,
                        _lum.x11.saver.interval,
                        _lum.x11.saver.blanking,
                        _lum.x11.saver.exposure);
    }
}

// Process the specified X event
//
static void processEvent(XEvent *event)
{
    int keycode = 0;
    Bool filtered = False;

    // HACK: Save scancode as some IMs clear the field in XFilterEvent
    if (event->type == KeyPress || event->type == KeyRelease)
        keycode = event->xkey.keycode;

    filtered = XFilterEvent(event, None);

    if (_lum.x11.randr.available)
    {
        if (event->type == _lum.x11.randr.eventBase + RRNotify)
        {
            XRRUpdateConfiguration(event);
            _lumPollMonitorsX11();
            return;
        }
    }

    if (_lum.x11.xkb.available)
    {
        if (event->type == _lum.x11.xkb.eventBase + XkbEventCode)
        {
            if (((XkbEvent*) event)->any.xkb_type == XkbStateNotify &&
                (((XkbEvent*) event)->state.changed & XkbGroupStateMask))
            {
                _lum.x11.xkb.group = ((XkbEvent*) event)->state.group;
            }

            return;
        }
    }

    if (event->type == GenericEvent)
    {
        if (_lum.x11.xi.available)
        {
            _LUMwindow* window = _lum.x11.disabledCursorWindow;

            if (window &&
                window->rawMouseMotion &&
                event->xcookie.extension == _lum.x11.xi.majorOpcode &&
                XGetEventData(_lum.x11.display, &event->xcookie) &&
                event->xcookie.evtype == XI_RawMotion)
            {
                XIRawEvent* re = event->xcookie.data;
                if (re->valuators.mask_len)
                {
                    const double* values = re->raw_values;
                    double xpos = window->virtualCursorPosX;
                    double ypos = window->virtualCursorPosY;

                    if (XIMaskIsSet(re->valuators.mask, 0))
                    {
                        xpos += *values;
                        values++;
                    }

                    if (XIMaskIsSet(re->valuators.mask, 1))
                        ypos += *values;

                    _lumInputCursorPos(window, xpos, ypos);
                }
            }

            XFreeEventData(_lum.x11.display, &event->xcookie);
        }

        return;
    }

    if (event->type == SelectionClear)
    {
        handleSelectionClear(event);
        return;
    }
    else if (event->type == SelectionRequest)
    {
        handleSelectionRequest(event);
        return;
    }

    _LUMwindow* window = NULL;
    if (XFindContext(_lum.x11.display,
                     event->xany.window,
                     _lum.x11.context,
                     (XPointer*) &window) != 0)
    {
        // This is an event for a window that has already been destroyed
        return;
    }

    switch (event->type)
    {
        case ReparentNotify:
        {
            window->x11.parent = event->xreparent.parent;
            return;
        }

        case KeyPress:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);
            const int plain = !(mods & (LUM_MOD_CONTROL | LUM_MOD_ALT));

            if (window->x11.ic)
            {
                // HACK: Do not report the key press events duplicated by XIM
                //       Duplicate key releases are filtered out implicitly by
                //       the Lumina key repeat logic in _lumInputKey
                //       A timestamp per key is used to handle simultaneous keys
                // NOTE: Always allow the first event for each key through
                //       (the server never sends a timestamp of zero)
                // NOTE: Timestamp difference is compared to handle wrap-around
                Time diff = event->xkey.time - window->x11.keyPressTimes[keycode];
                if (diff == event->xkey.time || (diff > 0 && diff < (1 << 31)))
                {
                    if (keycode)
                        _lumInputKey(window, key, keycode, LUM_PRESS, mods);

                    window->x11.keyPressTimes[keycode] = event->xkey.time;
                }

                if (!filtered)
                {
                    int count;
                    Status status;
                    char buffer[100];
                    char* chars = buffer;

                    count = Xutf8LookupString(window->x11.ic,
                                              &event->xkey,
                                              buffer, sizeof(buffer) - 1,
                                              NULL, &status);

                    if (status == XBufferOverflow)
                    {
                        chars = _lum_calloc(count + 1, 1);
                        count = Xutf8LookupString(window->x11.ic,
                                                  &event->xkey,
                                                  chars, count,
                                                  NULL, &status);
                    }

                    if (status == XLookupChars || status == XLookupBoth)
                    {
                        const char* c = chars;
                        chars[count] = '\0';
                        while (c - chars < count)
                            _lumInputChar(window, decodeUTF8(&c), mods, plain);
                    }

                    if (chars != buffer)
                        _lum_free(chars);
                }
            }
            else
            {
                KeySym keysym;
                XLookupString(&event->xkey, NULL, 0, &keysym, NULL);

                _lumInputKey(window, key, keycode, LUM_PRESS, mods);

                const uint32_t codepoint = _lumKeySym2Unicode(keysym);
                if (codepoint != LUM_INVALID_CODEPOINT)
                    _lumInputChar(window, codepoint, mods, plain);
            }

            return;
        }

        case KeyRelease:
        {
            const int key = translateKey(keycode);
            const int mods = translateState(event->xkey.state);

            if (!_lum.x11.xkb.detectable)
            {
                // HACK: Key repeat events will arrive as KeyRelease/KeyPress
                //       pairs with similar or identical time stamps
                //       The key repeat logic in _lumInputKey expects only key
                //       presses to repeat, so detect and discard release events
                if (XEventsQueued(_lum.x11.display, QueuedAfterReading))
                {
                    XEvent next;
                    XPeekEvent(_lum.x11.display, &next);

                    if (next.type == KeyPress &&
                        next.xkey.window == event->xkey.window &&
                        next.xkey.keycode == keycode)
                    {
                        // HACK: The time of repeat events sometimes doesn't
                        //       match that of the press event, so add an
                        //       epsilon
                        //       Toshiyuki Takahashi can press a button
                        //       16 times per second so it's fairly safe to
                        //       assume that no human is pressing the key 50
                        //       times per second (value is ms)
                        if ((next.xkey.time - event->xkey.time) < 20)
                        {
                            // This is very likely a server-generated key repeat
                            // event, so ignore it
                            return;
                        }
                    }
                }
            }

            _lumInputKey(window, key, keycode, LUM_RELEASE, mods);
            return;
        }

        case ButtonPress:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
                _lumInputMouseClick(window, LUM_MOUSE_BUTTON_LEFT, LUM_PRESS, mods);
            else if (event->xbutton.button == Button2)
                _lumInputMouseClick(window, LUM_MOUSE_BUTTON_MIDDLE, LUM_PRESS, mods);
            else if (event->xbutton.button == Button3)
                _lumInputMouseClick(window, LUM_MOUSE_BUTTON_RIGHT, LUM_PRESS, mods);

            // Modern X provides scroll events as mouse button presses
            else if (event->xbutton.button == Button4)
                _lumInputScroll(window, 0.0, 1.0);
            else if (event->xbutton.button == Button5)
                _lumInputScroll(window, 0.0, -1.0);
            else if (event->xbutton.button == Button6)
                _lumInputScroll(window, 1.0, 0.0);
            else if (event->xbutton.button == Button7)
                _lumInputScroll(window, -1.0, 0.0);

            else
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _lumInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     LUM_PRESS,
                                     mods);
            }

            return;
        }

        case ButtonRelease:
        {
            const int mods = translateState(event->xbutton.state);

            if (event->xbutton.button == Button1)
            {
                _lumInputMouseClick(window,
                                     LUM_MOUSE_BUTTON_LEFT,
                                     LUM_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button2)
            {
                _lumInputMouseClick(window,
                                     LUM_MOUSE_BUTTON_MIDDLE,
                                     LUM_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button == Button3)
            {
                _lumInputMouseClick(window,
                                     LUM_MOUSE_BUTTON_RIGHT,
                                     LUM_RELEASE,
                                     mods);
            }
            else if (event->xbutton.button > Button7)
            {
                // Additional buttons after 7 are treated as regular buttons
                // We subtract 4 to fill the gap left by scroll input above
                _lumInputMouseClick(window,
                                     event->xbutton.button - Button1 - 4,
                                     LUM_RELEASE,
                                     mods);
            }

            return;
        }

        case EnterNotify:
        {
            // XEnterWindowEvent is XCrossingEvent
            const int x = event->xcrossing.x;
            const int y = event->xcrossing.y;

            // HACK: This is a workaround for WMs (KWM, Fluxbox) that otherwise
            //       ignore the defined cursor for hidden cursor mode
            if (window->cursorMode == LUM_CURSOR_HIDDEN)
                updateCursorImage(window);

            _lumInputCursorEnter(window, LUM_TRUE);
            _lumInputCursorPos(window, x, y);

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case LeaveNotify:
        {
            _lumInputCursorEnter(window, LUM_FALSE);
            return;
        }

        case MotionNotify:
        {
            const int x = event->xmotion.x;
            const int y = event->xmotion.y;

            if (x != window->x11.warpCursorPosX ||
                y != window->x11.warpCursorPosY)
            {
                // The cursor was moved by something other than Lumina

                if (window->cursorMode == LUM_CURSOR_DISABLED)
                {
                    if (_lum.x11.disabledCursorWindow != window)
                        return;
                    if (window->rawMouseMotion)
                        return;

                    const int dx = x - window->x11.lastCursorPosX;
                    const int dy = y - window->x11.lastCursorPosY;

                    _lumInputCursorPos(window,
                                        window->virtualCursorPosX + dx,
                                        window->virtualCursorPosY + dy);
                }
                else
                    _lumInputCursorPos(window, x, y);
            }

            window->x11.lastCursorPosX = x;
            window->x11.lastCursorPosY = y;
            return;
        }

        case ConfigureNotify:
        {
            if (event->xconfigure.width != window->x11.width ||
                event->xconfigure.height != window->x11.height)
            {
                _lumInputFramebufferSize(window,
                                          event->xconfigure.width,
                                          event->xconfigure.height);

                _lumInputWindowSize(window,
                                     event->xconfigure.width,
                                     event->xconfigure.height);

                window->x11.width = event->xconfigure.width;
                window->x11.height = event->xconfigure.height;
            }

            int xpos = event->xconfigure.x;
            int ypos = event->xconfigure.y;

            // NOTE: ConfigureNotify events from the server are in local
            //       coordinates, so if we are reparented we need to translate
            //       the position into root (screen) coordinates
            if (!event->xany.send_event && window->x11.parent != _lum.x11.root)
            {
                _lumGrabErrorHandlerX11();

                Window dummy;
                XTranslateCoordinates(_lum.x11.display,
                                      window->x11.parent,
                                      _lum.x11.root,
                                      xpos, ypos,
                                      &xpos, &ypos,
                                      &dummy);

                _lumReleaseErrorHandlerX11();
                if (_lum.x11.errorCode == BadWindow)
                    return;
            }

            if (xpos != window->x11.xpos || ypos != window->x11.ypos)
            {
                _lumInputWindowPos(window, xpos, ypos);
                window->x11.xpos = xpos;
                window->x11.ypos = ypos;
            }

            return;
        }

        case ClientMessage:
        {
            // Custom client message, probably from the window manager

            if (filtered)
                return;

            if (event->xclient.message_type == None)
                return;

            if (event->xclient.message_type == _lum.x11.WM_PROTOCOLS)
            {
                const Atom protocol = event->xclient.data.l[0];
                if (protocol == None)
                    return;

                if (protocol == _lum.x11.WM_DELETE_WINDOW)
                {
                    // The window manager was asked to close the window, for
                    // example by the user pressing a 'close' window decoration
                    // button
                    _lumInputWindowCloseRequest(window);
                }
                else if (protocol == _lum.x11.NET_WM_PING)
                {
                    // The window manager is pinging the application to ensure
                    // it's still responding to events

                    XEvent reply = *event;
                    reply.xclient.window = _lum.x11.root;

                    XSendEvent(_lum.x11.display, _lum.x11.root,
                               False,
                               SubstructureNotifyMask | SubstructureRedirectMask,
                               &reply);
                }
            }
            else if (event->xclient.message_type == _lum.x11.XdndEnter)
            {
                // A drag operation has entered the window
                unsigned long count;
                Atom* formats = NULL;
                const LUMbool list = event->xclient.data.l[1] & 1;

                _lum.x11.xdnd.source  = event->xclient.data.l[0];
                _lum.x11.xdnd.version = event->xclient.data.l[1] >> 24;
                _lum.x11.xdnd.format  = None;

                if (_lum.x11.xdnd.version > _LUM_XDND_VERSION)
                    return;

                if (list)
                {
                    count = _lumGetWindowPropertyX11(_lum.x11.xdnd.source,
                                                      _lum.x11.XdndTypeList,
                                                      XA_ATOM,
                                                      (unsigned char**) &formats);
                }
                else
                {
                    count = 3;
                    formats = (Atom*) event->xclient.data.l + 2;
                }

                for (unsigned int i = 0;  i < count;  i++)
                {
                    if (formats[i] == _lum.x11.text_uri_list)
                    {
                        _lum.x11.xdnd.format = _lum.x11.text_uri_list;
                        break;
                    }
                }

                if (list && formats)
                    XFree(formats);
            }
            else if (event->xclient.message_type == _lum.x11.XdndDrop)
            {
                // The drag operation has finished by dropping on the window
                Time time = CurrentTime;

                if (_lum.x11.xdnd.version > _LUM_XDND_VERSION)
                    return;

                if (_lum.x11.xdnd.format)
                {
                    if (_lum.x11.xdnd.version >= 1)
                        time = event->xclient.data.l[2];

                    // Request the chosen format from the source window
                    XConvertSelection(_lum.x11.display,
                                      _lum.x11.XdndSelection,
                                      _lum.x11.xdnd.format,
                                      _lum.x11.XdndSelection,
                                      window->x11.handle,
                                      time);
                }
                else if (_lum.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _lum.x11.xdnd.source;
                    reply.xclient.message_type = _lum.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = 0; // The drag was rejected
                    reply.xclient.data.l[2] = None;

                    XSendEvent(_lum.x11.display, _lum.x11.xdnd.source,
                               False, NoEventMask, &reply);
                    XFlush(_lum.x11.display);
                }
            }
            else if (event->xclient.message_type == _lum.x11.XdndPosition)
            {
                // The drag operation has moved over the window
                const int xabs = (event->xclient.data.l[2] >> 16) & 0xffff;
                const int yabs = (event->xclient.data.l[2]) & 0xffff;
                Window dummy;
                int xpos, ypos;

                if (_lum.x11.xdnd.version > _LUM_XDND_VERSION)
                    return;

                XTranslateCoordinates(_lum.x11.display,
                                      _lum.x11.root,
                                      window->x11.handle,
                                      xabs, yabs,
                                      &xpos, &ypos,
                                      &dummy);

                _lumInputCursorPos(window, xpos, ypos);

                XEvent reply = { ClientMessage };
                reply.xclient.window = _lum.x11.xdnd.source;
                reply.xclient.message_type = _lum.x11.XdndStatus;
                reply.xclient.format = 32;
                reply.xclient.data.l[0] = window->x11.handle;
                reply.xclient.data.l[2] = 0; // Specify an empty rectangle
                reply.xclient.data.l[3] = 0;

                if (_lum.x11.xdnd.format)
                {
                    // Reply that we are ready to copy the dragged data
                    reply.xclient.data.l[1] = 1; // Accept with no rectangle
                    if (_lum.x11.xdnd.version >= 2)
                        reply.xclient.data.l[4] = _lum.x11.XdndActionCopy;
                }

                XSendEvent(_lum.x11.display, _lum.x11.xdnd.source,
                           False, NoEventMask, &reply);
                XFlush(_lum.x11.display);
            }

            return;
        }

        case SelectionNotify:
        {
            if (event->xselection.property == _lum.x11.XdndSelection)
            {
                // The converted data from the drag operation has arrived
                char* data;
                const unsigned long result =
                    _lumGetWindowPropertyX11(event->xselection.requestor,
                                              event->xselection.property,
                                              event->xselection.target,
                                              (unsigned char**) &data);

                if (result)
                {
                    int count;
                    char** paths = parseUriList(data, &count);

                    _lumInputDrop(window, count, (const char**) paths);

                    for (int i = 0;  i < count;  i++)
                        _lum_free(paths[i]);
                    _lum_free(paths);
                }

                if (data)
                    XFree(data);

                if (_lum.x11.xdnd.version >= 2)
                {
                    XEvent reply = { ClientMessage };
                    reply.xclient.window = _lum.x11.xdnd.source;
                    reply.xclient.message_type = _lum.x11.XdndFinished;
                    reply.xclient.format = 32;
                    reply.xclient.data.l[0] = window->x11.handle;
                    reply.xclient.data.l[1] = result;
                    reply.xclient.data.l[2] = _lum.x11.XdndActionCopy;

                    XSendEvent(_lum.x11.display, _lum.x11.xdnd.source,
                               False, NoEventMask, &reply);
                    XFlush(_lum.x11.display);
                }
            }

            return;
        }

        case FocusIn:
        {
            if (event->xfocus.mode == NotifyGrab ||
                event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == LUM_CURSOR_DISABLED)
                disableCursor(window);

            if (window->x11.ic)
                XSetICFocus(window->x11.ic);

            _lumInputWindowFocus(window, LUM_TRUE);
            return;
        }

        case FocusOut:
        {
            if (event->xfocus.mode == NotifyGrab ||
                event->xfocus.mode == NotifyUngrab)
            {
                // Ignore focus events from popup indicator windows, window menu
                // key chords and window dragging
                return;
            }

            if (window->cursorMode == LUM_CURSOR_DISABLED)
                enableCursor(window);

            if (window->x11.ic)
                XUnsetICFocus(window->x11.ic);

            if (window->monitor && window->autoIconify)
                _lumIconifyWindowX11(window);

            _lumInputWindowFocus(window, LUM_FALSE);
            return;
        }

        case Expose:
        {
            _lumInputWindowDamage(window);
            return;
        }

        case PropertyNotify:
        {
            if (event->xproperty.state != PropertyNewValue)
                return;

            if (event->xproperty.atom == _lum.x11.WM_STATE)
            {
                const int state = getWindowState(window);
                if (state != IconicState && state != NormalState)
                    return;

                const LUMbool iconified = (state == IconicState);
                if (window->x11.iconified != iconified)
                {
                    if (window->monitor)
                    {
                        if (iconified)
                            releaseMonitor(window);
                        else
                            acquireMonitor(window);
                    }

                    window->x11.iconified = iconified;
                    _lumInputWindowIconify(window, iconified);
                }
            }
            else if (event->xproperty.atom == _lum.x11.NET_WM_STATE)
            {
                const LUMbool maximized = _lumWindowMaximizedX11(window);
                if (window->x11.maximized != maximized)
                {
                    window->x11.maximized = maximized;
                    _lumInputWindowMaximize(window, maximized);
                }
            }

            return;
        }

        case DestroyNotify:
            return;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Retrieve a single window property of the specified type
// Inspired by fghGetWindowProperty from freeglut
//
unsigned long _lumGetWindowPropertyX11(Window window,
                                        Atom property,
                                        Atom type,
                                        unsigned char** value)
{
    Atom actualType;
    int actualFormat;
    unsigned long itemCount, bytesAfter;

    XGetWindowProperty(_lum.x11.display,
                       window,
                       property,
                       0,
                       LONG_MAX,
                       False,
                       type,
                       &actualType,
                       &actualFormat,
                       &itemCount,
                       &bytesAfter,
                       value);

    return itemCount;
}

LUMbool _lumIsVisualTransparentX11(Visual* visual)
{
    if (!_lum.x11.xrender.available)
        return LUM_FALSE;

    XRenderPictFormat* pf = XRenderFindVisualFormat(_lum.x11.display, visual);
    return pf && pf->direct.alphaMask;
}

// Push contents of our selection to clipboard manager
//
void _lumPushSelectionToManagerX11(void)
{
    XConvertSelection(_lum.x11.display,
                      _lum.x11.CLIPBOARD_MANAGER,
                      _lum.x11.SAVE_TARGETS,
                      None,
                      _lum.x11.helperWindowHandle,
                      CurrentTime);

    for (;;)
    {
        XEvent event;

        while (XCheckIfEvent(_lum.x11.display, &event, isSelectionEvent, NULL))
        {
            switch (event.type)
            {
                case SelectionRequest:
                    handleSelectionRequest(&event);
                    break;

                case SelectionClear:
                    handleSelectionClear(&event);
                    break;

                case SelectionNotify:
                {
                    if (event.xselection.target == _lum.x11.SAVE_TARGETS)
                    {
                        // This means one of two things; either the selection
                        // was not owned, which means there is no clipboard
                        // manager, or the transfer to the clipboard manager has
                        // completed
                        // In either case, it means we are done here
                        return;
                    }

                    break;
                }
            }
        }

        waitForEvent(NULL);
    }
}

void _lumCreateInputContextX11(_LUMwindow* window)
{
    XIMCallback callback;
    callback.callback = (XIMProc) inputContextDestroyCallback;
    callback.client_data = (XPointer) window;

    window->x11.ic = XCreateIC(_lum.x11.im,
                               XNInputStyle,
                               XIMPreeditNothing | XIMStatusNothing,
                               XNClientWindow,
                               window->x11.handle,
                               XNFocusWindow,
                               window->x11.handle,
                               XNDestroyCallback,
                               &callback,
                               NULL);

    if (window->x11.ic)
    {
        XWindowAttributes attribs;
        XGetWindowAttributes(_lum.x11.display, window->x11.handle, &attribs);

        unsigned long filter = 0;
        if (XGetICValues(window->x11.ic, XNFilterEvents, &filter, NULL) == NULL)
        {
            XSelectInput(_lum.x11.display,
                         window->x11.handle,
                         attribs.your_event_mask | filter);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _lumCreateWindowX11(_LUMwindow* window,
                         const _LUMwndconfig* wndconfig,
                         const _LUMctxconfig* ctxconfig,
                         const _LUMfbconfig* fbconfig)
{
    Visual* visual = NULL;
    int depth;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_NATIVE_CONTEXT_API)
        {
            if (!_lumInitGLX())
                return LUM_FALSE;
            if (!_lumChooseVisualGLX(wndconfig, ctxconfig, fbconfig, &visual, &depth))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_EGL_CONTEXT_API)
        {
            if (!_lumInitEGL())
                return LUM_FALSE;
            if (!_lumChooseVisualEGL(wndconfig, ctxconfig, fbconfig, &visual, &depth))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_OSMESA_CONTEXT_API)
        {
            if (!_lumInitOSMesa())
                return LUM_FALSE;
        }
    }

    if (!visual)
    {
        visual = DefaultVisual(_lum.x11.display, _lum.x11.screen);
        depth = DefaultDepth(_lum.x11.display, _lum.x11.screen);
    }

    if (!createNativeWindow(window, wndconfig, visual, depth))
        return LUM_FALSE;

    if (ctxconfig->client != LUM_NO_API)
    {
        if (ctxconfig->source == LUM_NATIVE_CONTEXT_API)
        {
            if (!_lumCreateContextGLX(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_EGL_CONTEXT_API)
        {
            if (!_lumCreateContextEGL(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
        else if (ctxconfig->source == LUM_OSMESA_CONTEXT_API)
        {
            if (!_lumCreateContextOSMesa(window, ctxconfig, fbconfig))
                return LUM_FALSE;
        }
    }

    if (window->monitor)
    {
        _lumShowWindowX11(window);
        updateWindowMode(window);
        acquireMonitor(window);
    }

    XFlush(_lum.x11.display);
    return LUM_TRUE;
}

void _lumDestroyWindowX11(_LUMwindow* window)
{
    if (_lum.x11.disabledCursorWindow == window)
        _lum.x11.disabledCursorWindow = NULL;

    if (window->monitor)
        releaseMonitor(window);

    if (window->x11.ic)
    {
        XDestroyIC(window->x11.ic);
        window->x11.ic = NULL;
    }

    if (window->context.destroy)
        window->context.destroy(window);

    if (window->x11.handle)
    {
        XDeleteContext(_lum.x11.display, window->x11.handle, _lum.x11.context);
        XUnmapWindow(_lum.x11.display, window->x11.handle);
        XDestroyWindow(_lum.x11.display, window->x11.handle);
        window->x11.handle = (Window) 0;
    }

    if (window->x11.colormap)
    {
        XFreeColormap(_lum.x11.display, window->x11.colormap);
        window->x11.colormap = (Colormap) 0;
    }

    XFlush(_lum.x11.display);
}

void _lumSetWindowTitleX11(_LUMwindow* window, const char* title)
{
    if (_lum.x11.xlib.utf8)
    {
        Xutf8SetWMProperties(_lum.x11.display,
                             window->x11.handle,
                             title, title,
                             NULL, 0,
                             NULL, NULL, NULL);
    }

    XChangeProperty(_lum.x11.display,  window->x11.handle,
                    _lum.x11.NET_WM_NAME, _lum.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title, strlen(title));

    XChangeProperty(_lum.x11.display,  window->x11.handle,
                    _lum.x11.NET_WM_ICON_NAME, _lum.x11.UTF8_STRING, 8,
                    PropModeReplace,
                    (unsigned char*) title, strlen(title));

    XFlush(_lum.x11.display);
}

void _lumSetWindowIconX11(_LUMwindow* window, int count, const LUMimage* images)
{
    if (count)
    {
        int longCount = 0;

        for (int i = 0;  i < count;  i++)
            longCount += 2 + images[i].width * images[i].height;

        unsigned long* icon = _lum_calloc(longCount, sizeof(unsigned long));
        unsigned long* target = icon;

        for (int i = 0;  i < count;  i++)
        {
            *target++ = images[i].width;
            *target++ = images[i].height;

            for (int j = 0;  j < images[i].width * images[i].height;  j++)
            {
                *target++ = (((unsigned long) images[i].pixels[j * 4 + 0]) << 16) |
                            (((unsigned long) images[i].pixels[j * 4 + 1]) <<  8) |
                            (((unsigned long) images[i].pixels[j * 4 + 2]) <<  0) |
                            (((unsigned long) images[i].pixels[j * 4 + 3]) << 24);
            }
        }

        // NOTE: XChangeProperty expects 32-bit values like the image data above to be
        //       placed in the 32 least significant bits of individual longs.  This is
        //       true even if long is 64-bit and a WM protocol calls for "packed" data.
        //       This is because of a historical mistake that then became part of the Xlib
        //       ABI.  Xlib will pack these values into a regular array of 32-bit values
        //       before sending it over the wire.
        XChangeProperty(_lum.x11.display, window->x11.handle,
                        _lum.x11.NET_WM_ICON,
                        XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char*) icon,
                        longCount);

        _lum_free(icon);
    }
    else
    {
        XDeleteProperty(_lum.x11.display, window->x11.handle,
                        _lum.x11.NET_WM_ICON);
    }

    XFlush(_lum.x11.display);
}

void _lumGetWindowPosX11(_LUMwindow* window, int* xpos, int* ypos)
{
    Window dummy;
    int x, y;

    XTranslateCoordinates(_lum.x11.display, window->x11.handle, _lum.x11.root,
                          0, 0, &x, &y, &dummy);

    if (xpos)
        *xpos = x;
    if (ypos)
        *ypos = y;
}

void _lumSetWindowPosX11(_LUMwindow* window, int xpos, int ypos)
{
    // HACK: Explicitly setting PPosition to any value causes some WMs, notably
    //       Compiz and Metacity, to honor the position of unmapped windows
    if (!_lumWindowVisibleX11(window))
    {
        long supplied;
        XSizeHints* hints = XAllocSizeHints();

        if (XGetWMNormalHints(_lum.x11.display, window->x11.handle, hints, &supplied))
        {
            hints->flags |= PPosition;
            hints->x = hints->y = 0;

            XSetWMNormalHints(_lum.x11.display, window->x11.handle, hints);
        }

        XFree(hints);
    }

    XMoveWindow(_lum.x11.display, window->x11.handle, xpos, ypos);
    XFlush(_lum.x11.display);
}

void _lumGetWindowSizeX11(_LUMwindow* window, int* width, int* height)
{
    XWindowAttributes attribs;
    XGetWindowAttributes(_lum.x11.display, window->x11.handle, &attribs);

    if (width)
        *width = attribs.width;
    if (height)
        *height = attribs.height;
}

void _lumSetWindowSizeX11(_LUMwindow* window, int width, int height)
{
    if (window->monitor)
    {
        if (window->monitor->window == window)
            acquireMonitor(window);
    }
    else
    {
        if (!window->resizable)
            updateNormalHints(window, width, height);

        XResizeWindow(_lum.x11.display, window->x11.handle, width, height);
    }

    XFlush(_lum.x11.display);
}

void _lumSetWindowSizeLimitsX11(_LUMwindow* window,
                                 int minwidth, int minheight,
                                 int maxwidth, int maxheight)
{
    int width, height;
    _lumGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_lum.x11.display);
}

void _lumSetWindowAspectRatioX11(_LUMwindow* window, int numer, int denom)
{
    int width, height;
    _lumGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
    XFlush(_lum.x11.display);
}

void _lumGetFramebufferSizeX11(_LUMwindow* window, int* width, int* height)
{
    _lumGetWindowSizeX11(window, width, height);
}

void _lumGetWindowFrameSizeX11(_LUMwindow* window,
                                int* left, int* top,
                                int* right, int* bottom)
{
    long* extents = NULL;

    if (window->monitor || !window->decorated)
        return;

    if (_lum.x11.NET_FRAME_EXTENTS == None)
        return;

    if (!_lumWindowVisibleX11(window) &&
        _lum.x11.NET_REQUEST_FRAME_EXTENTS)
    {
        XEvent event;
        double timeout = 0.5;

        // Ensure _NET_FRAME_EXTENTS is set, allowing lumGetWindowFrameSize to
        // function before the window is mapped
        sendEventToWM(window, _lum.x11.NET_REQUEST_FRAME_EXTENTS,
                      0, 0, 0, 0, 0);

        // HACK: Use a timeout because earlier versions of some window managers
        //       (at least Unity, Fluxbox and Xfwm) failed to send the reply
        //       They have been fixed but broken versions are still in the wild
        //       If you are affected by this and your window manager is NOT
        //       listed above, PLEASE report it to their and our issue trackers
        while (!XCheckIfEvent(_lum.x11.display,
                              &event,
                              isFrameExtentsEvent,
                              (XPointer) window))
        {
            if (!waitForEvent(&timeout))
            {
                _lumInputError(LUM_PLATFORM_ERROR,
                                "X11: The window manager has a broken _NET_REQUEST_FRAME_EXTENTS implementation; please report this issue");
                return;
            }
        }
    }

    if (_lumGetWindowPropertyX11(window->x11.handle,
                                  _lum.x11.NET_FRAME_EXTENTS,
                                  XA_CARDINAL,
                                  (unsigned char**) &extents) == 4)
    {
        if (left)
            *left = extents[0];
        if (top)
            *top = extents[2];
        if (right)
            *right = extents[1];
        if (bottom)
            *bottom = extents[3];
    }

    if (extents)
        XFree(extents);
}

void _lumGetWindowContentScaleX11(_LUMwindow* window, float* xscale, float* yscale)
{
    if (xscale)
        *xscale = _lum.x11.contentScaleX;
    if (yscale)
        *yscale = _lum.x11.contentScaleY;
}

void _lumIconifyWindowX11(_LUMwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    XIconifyWindow(_lum.x11.display, window->x11.handle, _lum.x11.screen);
    XFlush(_lum.x11.display);
}

void _lumRestoreWindowX11(_LUMwindow* window)
{
    if (window->x11.overrideRedirect)
    {
        // Override-redirect windows cannot be iconified or restored, as those
        // tasks are performed by the window manager
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Iconification of full screen windows requires a WM that supports EWMH full screen");
        return;
    }

    if (_lumWindowIconifiedX11(window))
    {
        XMapWindow(_lum.x11.display, window->x11.handle);
        waitForVisibilityNotify(window);
    }
    else if (_lumWindowVisibleX11(window))
    {
        if (_lum.x11.NET_WM_STATE &&
            _lum.x11.NET_WM_STATE_MAXIMIZED_VERT &&
            _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            sendEventToWM(window,
                          _lum.x11.NET_WM_STATE,
                          _NET_WM_STATE_REMOVE,
                          _lum.x11.NET_WM_STATE_MAXIMIZED_VERT,
                          _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ,
                          1, 0);
        }
    }

    XFlush(_lum.x11.display);
}

void _lumMaximizeWindowX11(_LUMwindow* window)
{
    if (!_lum.x11.NET_WM_STATE ||
        !_lum.x11.NET_WM_STATE_MAXIMIZED_VERT ||
        !_lum.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return;
    }

    if (_lumWindowVisibleX11(window))
    {
        sendEventToWM(window,
                    _lum.x11.NET_WM_STATE,
                    _NET_WM_STATE_ADD,
                    _lum.x11.NET_WM_STATE_MAXIMIZED_VERT,
                    _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ,
                    1, 0);
    }
    else
    {
        Atom* states = NULL;
        unsigned long count =
            _lumGetWindowPropertyX11(window->x11.handle,
                                      _lum.x11.NET_WM_STATE,
                                      XA_ATOM,
                                      (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        Atom missing[2] =
        {
            _lum.x11.NET_WM_STATE_MAXIMIZED_VERT,
            _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ
        };
        unsigned long missingCount = 2;

        for (unsigned long i = 0;  i < count;  i++)
        {
            for (unsigned long j = 0;  j < missingCount;  j++)
            {
                if (states[i] == missing[j])
                {
                    missing[j] = missing[missingCount - 1];
                    missingCount--;
                }
            }
        }

        if (states)
            XFree(states);

        if (!missingCount)
            return;

        XChangeProperty(_lum.x11.display, window->x11.handle,
                        _lum.x11.NET_WM_STATE, XA_ATOM, 32,
                        PropModeAppend,
                        (unsigned char*) missing,
                        missingCount);
    }

    XFlush(_lum.x11.display);
}

void _lumShowWindowX11(_LUMwindow* window)
{
    if (_lumWindowVisibleX11(window))
        return;

    XMapWindow(_lum.x11.display, window->x11.handle);
    waitForVisibilityNotify(window);
}

void _lumHideWindowX11(_LUMwindow* window)
{
    XUnmapWindow(_lum.x11.display, window->x11.handle);
    XFlush(_lum.x11.display);
}

void _lumRequestWindowAttentionX11(_LUMwindow* window)
{
    if (!_lum.x11.NET_WM_STATE || !_lum.x11.NET_WM_STATE_DEMANDS_ATTENTION)
        return;

    sendEventToWM(window,
                  _lum.x11.NET_WM_STATE,
                  _NET_WM_STATE_ADD,
                  _lum.x11.NET_WM_STATE_DEMANDS_ATTENTION,
                  0, 1, 0);
}

void _lumFocusWindowX11(_LUMwindow* window)
{
    if (_lum.x11.NET_ACTIVE_WINDOW)
        sendEventToWM(window, _lum.x11.NET_ACTIVE_WINDOW, 1, 0, 0, 0, 0);
    else if (_lumWindowVisibleX11(window))
    {
        XRaiseWindow(_lum.x11.display, window->x11.handle);
        XSetInputFocus(_lum.x11.display, window->x11.handle,
                       RevertToParent, CurrentTime);
    }

    XFlush(_lum.x11.display);
}

void _lumSetWindowMonitorX11(_LUMwindow* window,
                              _LUMmonitor* monitor,
                              int xpos, int ypos,
                              int width, int height,
                              int refreshRate)
{
    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
                acquireMonitor(window);
        }
        else
        {
            if (!window->resizable)
                updateNormalHints(window, width, height);

            XMoveResizeWindow(_lum.x11.display, window->x11.handle,
                              xpos, ypos, width, height);
        }

        XFlush(_lum.x11.display);
        return;
    }

    if (window->monitor)
    {
        _lumSetWindowDecoratedX11(window, window->decorated);
        _lumSetWindowFloatingX11(window, window->floating);
        releaseMonitor(window);
    }

    _lumInputWindowMonitor(window, monitor);
    updateNormalHints(window, width, height);

    if (window->monitor)
    {
        if (!_lumWindowVisibleX11(window))
        {
            XMapRaised(_lum.x11.display, window->x11.handle);
            waitForVisibilityNotify(window);
        }

        updateWindowMode(window);
        acquireMonitor(window);
    }
    else
    {
        updateWindowMode(window);
        XMoveResizeWindow(_lum.x11.display, window->x11.handle,
                          xpos, ypos, width, height);
    }

    XFlush(_lum.x11.display);
}

int _lumWindowFocusedX11(_LUMwindow* window)
{
    Window focused;
    int state;

    XGetInputFocus(_lum.x11.display, &focused, &state);
    return window->x11.handle == focused;
}

int _lumWindowIconifiedX11(_LUMwindow* window)
{
    return getWindowState(window) == IconicState;
}

int _lumWindowVisibleX11(_LUMwindow* window)
{
    XWindowAttributes wa;
    XGetWindowAttributes(_lum.x11.display, window->x11.handle, &wa);
    return wa.map_state == IsViewable;
}

int _lumWindowMaximizedX11(_LUMwindow* window)
{
    Atom* states;
    LUMbool maximized = LUM_FALSE;

    if (!_lum.x11.NET_WM_STATE ||
        !_lum.x11.NET_WM_STATE_MAXIMIZED_VERT ||
        !_lum.x11.NET_WM_STATE_MAXIMIZED_HORZ)
    {
        return maximized;
    }

    const unsigned long count =
        _lumGetWindowPropertyX11(window->x11.handle,
                                  _lum.x11.NET_WM_STATE,
                                  XA_ATOM,
                                  (unsigned char**) &states);

    for (unsigned long i = 0;  i < count;  i++)
    {
        if (states[i] == _lum.x11.NET_WM_STATE_MAXIMIZED_VERT ||
            states[i] == _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ)
        {
            maximized = LUM_TRUE;
            break;
        }
    }

    if (states)
        XFree(states);

    return maximized;
}

int _lumWindowHoveredX11(_LUMwindow* window)
{
    Window w = _lum.x11.root;
    while (w)
    {
        Window root;
        int rootX, rootY, childX, childY;
        unsigned int mask;

        _lumGrabErrorHandlerX11();

        const Bool result = XQueryPointer(_lum.x11.display, w,
                                          &root, &w, &rootX, &rootY,
                                          &childX, &childY, &mask);

        _lumReleaseErrorHandlerX11();

        if (_lum.x11.errorCode == BadWindow)
            w = _lum.x11.root;
        else if (!result)
            return LUM_FALSE;
        else if (w == window->x11.handle)
            return LUM_TRUE;
    }

    return LUM_FALSE;
}

int _lumFramebufferTransparentX11(_LUMwindow* window)
{
    if (!window->x11.transparent)
        return LUM_FALSE;

    return XGetSelectionOwner(_lum.x11.display, _lum.x11.NET_WM_CM_Sx) != None;
}

void _lumSetWindowResizableX11(_LUMwindow* window, LUMbool enabled)
{
    int width, height;
    _lumGetWindowSizeX11(window, &width, &height);
    updateNormalHints(window, width, height);
}

void _lumSetWindowDecoratedX11(_LUMwindow* window, LUMbool enabled)
{
    struct
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long input_mode;
        unsigned long status;
    } hints = {0};

    hints.flags = MWM_HINTS_DECORATIONS;
    hints.decorations = enabled ? MWM_DECOR_ALL : 0;

    XChangeProperty(_lum.x11.display, window->x11.handle,
                    _lum.x11.MOTIF_WM_HINTS,
                    _lum.x11.MOTIF_WM_HINTS, 32,
                    PropModeReplace,
                    (unsigned char*) &hints,
                    sizeof(hints) / sizeof(long));
}

void _lumPlatformSetWindowTitlebar(_LUMwindow* window, LUMbool enabled)
{
    // TODO
    _lumInputError(LUM_PLATFORM_ERROR,
        "X11: Window attribute setting not implemented yet");
}

void _lumSetWindowFloatingX11(_LUMwindow* window, LUMbool enabled)
{
    if (!_lum.x11.NET_WM_STATE || !_lum.x11.NET_WM_STATE_ABOVE)
        return;

    if (_lumWindowVisibleX11(window))
    {
        const long action = enabled ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        sendEventToWM(window,
                      _lum.x11.NET_WM_STATE,
                      action,
                      _lum.x11.NET_WM_STATE_ABOVE,
                      0, 1, 0);
    }
    else
    {
        Atom* states = NULL;
        const unsigned long count =
            _lumGetWindowPropertyX11(window->x11.handle,
                                      _lum.x11.NET_WM_STATE,
                                      XA_ATOM,
                                      (unsigned char**) &states);

        // NOTE: We don't check for failure as this property may not exist yet
        //       and that's fine (and we'll create it implicitly with append)

        if (enabled)
        {
            unsigned long i;

            for (i = 0;  i < count;  i++)
            {
                if (states[i] == _lum.x11.NET_WM_STATE_ABOVE)
                    break;
            }

            if (i == count)
            {
                XChangeProperty(_lum.x11.display, window->x11.handle,
                                _lum.x11.NET_WM_STATE, XA_ATOM, 32,
                                PropModeAppend,
                                (unsigned char*) &_lum.x11.NET_WM_STATE_ABOVE,
                                1);
            }
        }
        else if (states)
        {
            for (unsigned long i = 0;  i < count;  i++)
            {
                if (states[i] == _lum.x11.NET_WM_STATE_ABOVE)
                {
                    states[i] = states[count - 1];
                    XChangeProperty(_lum.x11.display, window->x11.handle,
                                    _lum.x11.NET_WM_STATE, XA_ATOM, 32,
                                    PropModeReplace, (unsigned char*) states, count - 1);
                    break;
                }
            }
        }

        if (states)
            XFree(states);
    }

    XFlush(_lum.x11.display);
}

void _lumSetWindowMousePassthroughX11(_LUMwindow* window, LUMbool enabled)
{
    if (!_lum.x11.xshape.available)
        return;

    if (enabled)
    {
        Region region = XCreateRegion();
        XShapeCombineRegion(_lum.x11.display, window->x11.handle,
                            ShapeInput, 0, 0, region, ShapeSet);
        XDestroyRegion(region);
    }
    else
    {
        XShapeCombineMask(_lum.x11.display, window->x11.handle,
                          ShapeInput, 0, 0, None, ShapeSet);
    }
}

float _lumGetWindowOpacityX11(_LUMwindow* window)
{
    float opacity = 1.f;

    if (XGetSelectionOwner(_lum.x11.display, _lum.x11.NET_WM_CM_Sx))
    {
        CARD32* value = NULL;

        if (_lumGetWindowPropertyX11(window->x11.handle,
                                      _lum.x11.NET_WM_WINDOW_OPACITY,
                                      XA_CARDINAL,
                                      (unsigned char**) &value))
        {
            opacity = (float) (*value / (double) 0xffffffffu);
        }

        if (value)
            XFree(value);
    }

    return opacity;
}

void _lumSetWindowOpacityX11(_LUMwindow* window, float opacity)
{
    const CARD32 value = (CARD32) (0xffffffffu * (double) opacity);
    XChangeProperty(_lum.x11.display, window->x11.handle,
                    _lum.x11.NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*) &value, 1);
}

void _lumSetRawMouseMotionX11(_LUMwindow *window, LUMbool enabled)
{
    if (!_lum.x11.xi.available)
        return;

    if (_lum.x11.disabledCursorWindow != window)
        return;

    if (enabled)
        enableRawMouseMotion(window);
    else
        disableRawMouseMotion(window);
}

LUMbool _lumRawMouseMotionSupportedX11(void)
{
    return _lum.x11.xi.available;
}

void _lumPollEventsX11(void)
{
    _LUMwindow* window;

#if defined(__linux__)
    if (_lum.joysticksInitialized)
        _lumDetectJoystickConnectionLinux();
#endif
    XPending(_lum.x11.display);

    while (QLength(_lum.x11.display))
    {
        XEvent event;
        XNextEvent(_lum.x11.display, &event);
        processEvent(&event);
    }

    window = _lum.x11.disabledCursorWindow;
    if (window)
    {
        int width, height;
        _lumGetWindowSizeX11(window, &width, &height);

        // NOTE: Re-center the cursor only if it has moved since the last call,
        //       to avoid breaking lumWaitEvents with MotionNotify
        if (window->x11.lastCursorPosX != width / 2 ||
            window->x11.lastCursorPosY != height / 2)
        {
            _lumSetCursorPosX11(window, width / 2, height / 2);
        }
    }

    XFlush(_lum.x11.display);
}

void _lumWaitEventsX11(void)
{
    while (!XPending(_lum.x11.display))
        waitForEvent(NULL);

    _lumPollEventsX11();
}

void _lumWaitEventsTimeoutX11(double timeout)
{
    while (!XPending(_lum.x11.display))
    {
        if (!waitForEvent(&timeout))
            break;
    }

    _lumPollEventsX11();
}

void _lumPostEmptyEventX11(void)
{
    XEvent event = { ClientMessage };
    event.xclient.window = _lum.x11.helperWindowHandle;
    event.xclient.format = 32; // Data is 32-bit longs
    event.xclient.message_type = _lum.x11.NULL_;

    XSendEvent(_lum.x11.display, _lum.x11.helperWindowHandle, False, 0, &event);
    XFlush(_lum.x11.display);
}

void _lumGetCursorPosX11(_LUMwindow* window, double* xpos, double* ypos)
{
    Window root, child;
    int rootX, rootY, childX, childY;
    unsigned int mask;

    XQueryPointer(_lum.x11.display, window->x11.handle,
                  &root, &child,
                  &rootX, &rootY, &childX, &childY,
                  &mask);

    if (xpos)
        *xpos = childX;
    if (ypos)
        *ypos = childY;
}

void _lumSetCursorPosX11(_LUMwindow* window, double x, double y)
{
    // Store the new position so it can be recognized later
    window->x11.warpCursorPosX = (int) x;
    window->x11.warpCursorPosY = (int) y;

    XWarpPointer(_lum.x11.display, None, window->x11.handle,
                 0,0,0,0, (int) x, (int) y);
    XFlush(_lum.x11.display);
}

void _lumSetCursorModeX11(_LUMwindow* window, int mode)
{
    if (mode == LUM_CURSOR_DISABLED)
    {
        if (_lumWindowFocusedX11(window))
            disableCursor(window);
    }
    else if (_lum.x11.disabledCursorWindow == window)
        enableCursor(window);
    else
        updateCursorImage(window);

    XFlush(_lum.x11.display);
}

const char* _lumGetScancodeNameX11(int scancode)
{
    if (!_lum.x11.xkb.available)
        return NULL;

    if (scancode < 0 || scancode > 0xff ||
        _lum.x11.keycodes[scancode] == LUM_KEY_UNKNOWN)
    {
        _lumInputError(LUM_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    const int key = _lum.x11.keycodes[scancode];
    const KeySym keysym = XkbKeycodeToKeysym(_lum.x11.display,
                                             scancode, _lum.x11.xkb.group, 0);
    if (keysym == NoSymbol)
        return NULL;

    const uint32_t codepoint = _lumKeySym2Unicode(keysym);
    if (codepoint == LUM_INVALID_CODEPOINT)
        return NULL;

    const size_t count = _lumEncodeUTF8(_lum.x11.keynames[key], codepoint);
    if (count == 0)
        return NULL;

    _lum.x11.keynames[key][count] = '\0';
    return _lum.x11.keynames[key];
}

int _lumGetKeyScancodeX11(int key)
{
    return _lum.x11.scancodes[key];
}

int _lumCreateCursorX11(_LUMcursor* cursor,
                         const LUMimage* image,
                         int xhot, int yhot)
{
    cursor->x11.handle = _lumCreateNativeCursorX11(image, xhot, yhot);
    if (!cursor->x11.handle)
        return LUM_FALSE;

    return LUM_TRUE;
}

int _lumCreateStandardCursorX11(_LUMcursor* cursor, int shape)
{
    if (_lum.x11.xcursor.handle)
    {
        char* theme = XcursorGetTheme(_lum.x11.display);
        if (theme)
        {
            const int size = XcursorGetDefaultSize(_lum.x11.display);
            const char* name = NULL;

            switch (shape)
            {
                case LUM_ARROW_CURSOR:
                    name = "default";
                    break;
                case LUM_IBEAM_CURSOR:
                    name = "text";
                    break;
                case LUM_CROSSHAIR_CURSOR:
                    name = "crosshair";
                    break;
                case LUM_POINTING_HAND_CURSOR:
                    name = "pointer";
                    break;
                case LUM_RESIZE_EW_CURSOR:
                    name = "ew-resize";
                    break;
                case LUM_RESIZE_NS_CURSOR:
                    name = "ns-resize";
                    break;
                case LUM_RESIZE_NWSE_CURSOR:
                    name = "nwse-resize";
                    break;
                case LUM_RESIZE_NESW_CURSOR:
                    name = "nesw-resize";
                    break;
                case LUM_RESIZE_ALL_CURSOR:
                    name = "all-scroll";
                    break;
                case LUM_NOT_ALLOWED_CURSOR:
                    name = "not-allowed";
                    break;
            }

            XcursorImage* image = XcursorLibraryLoadImage(name, theme, size);
            if (image)
            {
                cursor->x11.handle = XcursorImageLoadCursor(_lum.x11.display, image);
                XcursorImageDestroy(image);
            }
        }
    }

    if (!cursor->x11.handle)
    {
        unsigned int native = 0;

        switch (shape)
        {
            case LUM_ARROW_CURSOR:
                native = XC_left_ptr;
                break;
            case LUM_IBEAM_CURSOR:
                native = XC_xterm;
                break;
            case LUM_CROSSHAIR_CURSOR:
                native = XC_crosshair;
                break;
            case LUM_POINTING_HAND_CURSOR:
                native = XC_hand2;
                break;
            case LUM_RESIZE_EW_CURSOR:
                native = XC_sb_h_double_arrow;
                break;
            case LUM_RESIZE_NS_CURSOR:
                native = XC_sb_v_double_arrow;
                break;
            case LUM_RESIZE_ALL_CURSOR:
                native = XC_fleur;
                break;
            default:
                _lumInputError(LUM_CURSOR_UNAVAILABLE,
                                "X11: Standard cursor shape unavailable");
                return LUM_FALSE;
        }

        cursor->x11.handle = XCreateFontCursor(_lum.x11.display, native);
        if (!cursor->x11.handle)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Failed to create standard cursor");
            return LUM_FALSE;
        }
    }

    return LUM_TRUE;
}

void _lumDestroyCursorX11(_LUMcursor* cursor)
{
    if (cursor->x11.handle)
        XFreeCursor(_lum.x11.display, cursor->x11.handle);
}

void _lumSetCursorX11(_LUMwindow* window, _LUMcursor* cursor)
{
    if (window->cursorMode == LUM_CURSOR_NORMAL)
    {
        updateCursorImage(window);
        XFlush(_lum.x11.display);
    }
}

void _lumSetClipboardStringX11(const char* string)
{
    char* copy = _lum_strdup(string);
    _lum_free(_lum.x11.clipboardString);
    _lum.x11.clipboardString = copy;

    XSetSelectionOwner(_lum.x11.display,
                       _lum.x11.CLIPBOARD,
                       _lum.x11.helperWindowHandle,
                       CurrentTime);

    if (XGetSelectionOwner(_lum.x11.display, _lum.x11.CLIPBOARD) !=
        _lum.x11.helperWindowHandle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Failed to become owner of clipboard selection");
    }
}

const char* _lumGetClipboardStringX11(void)
{
    return getSelectionString(_lum.x11.CLIPBOARD);
}

EGLenum _lumGetEGLPlatformX11(EGLint** attribs)
{
    if (_lum.egl.ANGLE_platform_angle)
    {
        int type = 0;

        if (_lum.egl.ANGLE_platform_angle_opengl)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_OPENGL)
                type = EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
        }

        if (_lum.egl.ANGLE_platform_angle_vulkan)
        {
            if (_lum.hints.init.angleType == LUM_ANGLE_PLATFORM_TYPE_VULKAN)
                type = EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
        }

        if (type)
        {
            *attribs = _lum_calloc(5, sizeof(EGLint));
            (*attribs)[0] = EGL_PLATFORM_ANGLE_TYPE_ANGLE;
            (*attribs)[1] = type;
            (*attribs)[2] = EGL_PLATFORM_ANGLE_NATIVE_PLATFORM_TYPE_ANGLE;
            (*attribs)[3] = EGL_PLATFORM_X11_EXT;
            (*attribs)[4] = EGL_NONE;
            return EGL_PLATFORM_ANGLE_ANGLE;
        }
    }

    if (_lum.egl.EXT_platform_base && _lum.egl.EXT_platform_x11)
        return EGL_PLATFORM_X11_EXT;

    return 0;
}

EGLNativeDisplayType _lumGetEGLNativeDisplayX11(void)
{
    return _lum.x11.display;
}

EGLNativeWindowType _lumGetEGLNativeWindowX11(_LUMwindow* window)
{
    if (_lum.egl.platform)
        return &window->x11.handle;
    else
        return (EGLNativeWindowType) window->x11.handle;
}

void _lumGetRequiredInstanceExtensionsX11(char** extensions)
{
    if (!_lum.vk.KHR_surface)
        return;

    if (!_lum.vk.KHR_xcb_surface || !_lum.x11.x11xcb.handle)
    {
        if (!_lum.vk.KHR_xlib_surface)
            return;
    }

    extensions[0] = "VK_KHR_surface";

    // NOTE: VK_KHR_xcb_surface is preferred due to some early ICDs exposing but
    //       not correctly implementing VK_KHR_xlib_surface
    if (_lum.vk.KHR_xcb_surface && _lum.x11.x11xcb.handle)
        extensions[1] = "VK_KHR_xcb_surface";
    else
        extensions[1] = "VK_KHR_xlib_surface";
}

int _lumGetPhysicalDevicePresentationSupportX11(VkInstance instance,
                                                 VkPhysicalDevice device,
                                                 uint32_t queuefamily)
{
    VisualID visualID = XVisualIDFromVisual(DefaultVisual(_lum.x11.display,
                                                          _lum.x11.screen));

    if (_lum.vk.KHR_xcb_surface && _lum.x11.x11xcb.handle)
    {
        PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR
            vkGetPhysicalDeviceXcbPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXcbPresentationSupportKHR)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return LUM_FALSE;
        }

        xcb_connection_t* connection = XGetXCBConnection(_lum.x11.display);
        if (!connection)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Failed to retrieve XCB connection");
            return LUM_FALSE;
        }

        return vkGetPhysicalDeviceXcbPresentationSupportKHR(device,
                                                            queuefamily,
                                                            connection,
                                                            visualID);
    }
    else
    {
        PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR
            vkGetPhysicalDeviceXlibPresentationSupportKHR =
            (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)
            vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
        if (!vkGetPhysicalDeviceXlibPresentationSupportKHR)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return LUM_FALSE;
        }

        return vkGetPhysicalDeviceXlibPresentationSupportKHR(device,
                                                             queuefamily,
                                                             _lum.x11.display,
                                                             visualID);
    }
}

VkResult _lumCreateWindowSurfaceX11(VkInstance instance,
                                     _LUMwindow* window,
                                     const VkAllocationCallbacks* allocator,
                                     VkSurfaceKHR* surface)
{
    if (_lum.vk.KHR_xcb_surface && _lum.x11.x11xcb.handle)
    {
        VkResult err;
        VkXcbSurfaceCreateInfoKHR sci;
        PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;

        xcb_connection_t* connection = XGetXCBConnection(_lum.x11.display);
        if (!connection)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Failed to retrieve XCB connection");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)
            vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
        if (!vkCreateXcbSurfaceKHR)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xcb_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        sci.connection = connection;
        sci.window = window->x11.handle;

        err = vkCreateXcbSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Failed to create Vulkan XCB surface: %s",
                            _lumGetVulkanResultString(err));
        }

        return err;
    }
    else
    {
        VkResult err;
        VkXlibSurfaceCreateInfoKHR sci;
        PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

        vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)
            vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
        if (!vkCreateXlibSurfaceKHR)
        {
            _lumInputError(LUM_API_UNAVAILABLE,
                            "X11: Vulkan instance missing VK_KHR_xlib_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        sci.dpy = _lum.x11.display;
        sci.window = window->x11.handle;

        err = vkCreateXlibSurfaceKHR(instance, &sci, allocator, surface);
        if (err)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "X11: Failed to create Vulkan X11 surface: %s",
                            _lumGetVulkanResultString(err));
        }

        return err;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                        Lumina native API                       //////
//////////////////////////////////////////////////////////////////////////

LUMAPI Display* lumGetX11Display(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return NULL;
    }

    return _lum.x11.display;
}

LUMAPI Window lumGetX11Window(LUMwindow* handle)
{
    _LUMwindow* window = (_LUMwindow*) handle;
    _LUM_REQUIRE_INIT_OR_RETURN(None);

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return None;
    }

    return window->x11.handle;
}

LUMAPI void lumSetX11SelectionString(const char* string)
{
    _LUM_REQUIRE_INIT();

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return;
    }

    _lum_free(_lum.x11.primarySelectionString);
    _lum.x11.primarySelectionString = _lum_strdup(string);

    XSetSelectionOwner(_lum.x11.display,
                       _lum.x11.PRIMARY,
                       _lum.x11.helperWindowHandle,
                       CurrentTime);

    if (XGetSelectionOwner(_lum.x11.display, _lum.x11.PRIMARY) !=
        _lum.x11.helperWindowHandle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "X11: Failed to become owner of primary selection");
    }
}

LUMAPI const char* lumGetX11SelectionString(void)
{
    _LUM_REQUIRE_INIT_OR_RETURN(NULL);

    if (_lum.platform.platformID != LUM_PLATFORM_X11)
    {
        _lumInputError(LUM_PLATFORM_UNAVAILABLE, "X11: Platform not initialized");
        return NULL;
    }

    return getSelectionString(_lum.x11.PRIMARY);
}

