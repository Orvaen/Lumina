//========================================================================
// Lumina Wayland - fork of GLFW 3.4 (www.glfw.org)
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <time.h>

#include "wayland-client-protocol.h"
#include "wayland-xdg-shell-client-protocol.h"
#include "wayland-xdg-decoration-client-protocol.h"
#include "wayland-viewporter-client-protocol.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol.h"

#include "wayland-client-protocol-code.h"
#include "wayland-xdg-shell-client-protocol-code.h"
#include "wayland-xdg-decoration-client-protocol-code.h"
#include "wayland-viewporter-client-protocol-code.h"
#include "wayland-relative-pointer-unstable-v1-client-protocol-code.h"
#include "wayland-pointer-constraints-unstable-v1-client-protocol-code.h"
#include "wayland-idle-inhibit-unstable-v1-client-protocol-code.h"


static inline int min(int n1, int n2)
{
    return n1 < n2 ? n1 : n2;
}

static _LUMwindow* findWindowFromDecorationSurface(struct wl_surface* surface,
                                                    int* which)
{
    int focus;
    _LUMwindow* window = _lum.windowListHead;
    if (!which)
        which = &focus;
    while (window)
    {
        if (surface == window->wl.decorations.top.surface)
        {
            *which = topDecoration;
            break;
        }
        if (surface == window->wl.decorations.left.surface)
        {
            *which = leftDecoration;
            break;
        }
        if (surface == window->wl.decorations.right.surface)
        {
            *which = rightDecoration;
            break;
        }
        if (surface == window->wl.decorations.bottom.surface)
        {
            *which = bottomDecoration;
            break;
        }
        window = window->next;
    }
    return window;
}

static void pointerHandleEnter(void* data,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface,
                               wl_fixed_t sx,
                               wl_fixed_t sy)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    int focus = 0;
    _LUMwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, &focus);
        if (!window)
            return;
    }

    window->wl.decorations.focus = focus;
    _lum.wl.serial = serial;
    _lum.wl.pointerEnterSerial = serial;
    _lum.wl.pointerFocus = window;

    window->wl.hovered = LUM_TRUE;

    _lumSetCursorWayland(window, window->wl.currentCursor);
    _lumInputCursorEnter(window, LUM_TRUE);
}

static void pointerHandleLeave(void* data,
                               struct wl_pointer* pointer,
                               uint32_t serial,
                               struct wl_surface* surface)
{
    _LUMwindow* window = _lum.wl.pointerFocus;

    if (!window)
        return;

    window->wl.hovered = LUM_FALSE;

    _lum.wl.serial = serial;
    _lum.wl.pointerFocus = NULL;
    _lumInputCursorEnter(window, LUM_FALSE);
    _lum.wl.cursorPreviousName = NULL;
}

static void setCursor(_LUMwindow* window, const char* name)
{
    struct wl_buffer* buffer;
    struct wl_cursor* cursor;
    struct wl_cursor_image* image;
    struct wl_surface* surface = _lum.wl.cursorSurface;
    struct wl_cursor_theme* theme = _lum.wl.cursorTheme;
    int scale = 1;

    if (window->wl.scale > 1 && _lum.wl.cursorThemeHiDPI)
    {
        // We only support up to scale=2 for now, since libwayland-cursor
        // requires us to load a different theme for each size.
        scale = 2;
        theme = _lum.wl.cursorThemeHiDPI;
    }

    cursor = wl_cursor_theme_get_cursor(theme, name);
    if (!cursor)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Standard cursor not found");
        return;
    }
    // TODO: handle animated cursors too.
    image = cursor->images[0];

    if (!image)
        return;

    buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
        return;
    wl_pointer_set_cursor(_lum.wl.pointer, _lum.wl.pointerEnterSerial,
                          surface,
                          image->hotspot_x / scale,
                          image->hotspot_y / scale);
    wl_surface_set_buffer_scale(surface, scale);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0,
                      image->width, image->height);
    wl_surface_commit(surface);
    _lum.wl.cursorPreviousName = name;
}

static void pointerHandleMotion(void* data,
                                struct wl_pointer* pointer,
                                uint32_t time,
                                wl_fixed_t sx,
                                wl_fixed_t sy)
{
    _LUMwindow* window = _lum.wl.pointerFocus;
    const char* cursorName = NULL;
    double x, y;

    if (!window)
        return;

    if (window->cursorMode == LUM_CURSOR_DISABLED)
        return;
    x = wl_fixed_to_double(sx);
    y = wl_fixed_to_double(sy);
    window->wl.cursorPosX = x;
    window->wl.cursorPosY = y;

    switch (window->wl.decorations.focus)
    {
        case mainWindow:
            _lumInputCursorPos(window, x, y);
            _lum.wl.cursorPreviousName = NULL;
            return;
        case topDecoration:
            if (y < _LUM_DECORATION_WIDTH)
                cursorName = "n-resize";
            else
                cursorName = "left_ptr";
            break;
        case leftDecoration:
            if (y < _LUM_DECORATION_WIDTH)
                cursorName = "nw-resize";
            else
                cursorName = "w-resize";
            break;
        case rightDecoration:
            if (y < _LUM_DECORATION_WIDTH)
                cursorName = "ne-resize";
            else
                cursorName = "e-resize";
            break;
        case bottomDecoration:
            if (x < _LUM_DECORATION_WIDTH)
                cursorName = "sw-resize";
            else if (x > window->wl.width + _LUM_DECORATION_WIDTH)
                cursorName = "se-resize";
            else
                cursorName = "s-resize";
            break;
        default:
            assert(0);
    }
    if (_lum.wl.cursorPreviousName != cursorName)
        setCursor(window, cursorName);
}

static void pointerHandleButton(void* data,
                                struct wl_pointer* pointer,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t button,
                                uint32_t state)
{
    _LUMwindow* window = _lum.wl.pointerFocus;
    int lumButton;
    uint32_t edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;

    if (!window)
        return;
    if (button == BTN_LEFT)
    {
        switch (window->wl.decorations.focus)
        {
            case mainWindow:
                break;
            case topDecoration:
                if (window->wl.cursorPosY < _LUM_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                else
                    xdg_toplevel_move(window->wl.xdg.toplevel, _lum.wl.seat, serial);
                break;
            case leftDecoration:
                if (window->wl.cursorPosY < _LUM_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                break;
            case rightDecoration:
                if (window->wl.cursorPosY < _LUM_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                break;
            case bottomDecoration:
                if (window->wl.cursorPosX < _LUM_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                else if (window->wl.cursorPosX > window->wl.width + _LUM_DECORATION_WIDTH)
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                else
                    edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                break;
            default:
                assert(0);
        }
        if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE)
        {
            xdg_toplevel_resize(window->wl.xdg.toplevel, _lum.wl.seat,
                                serial, edges);
            return;
        }
    }
    else if (button == BTN_RIGHT)
    {
        if (window->wl.decorations.focus != mainWindow && window->wl.xdg.toplevel)
        {
            xdg_toplevel_show_window_menu(window->wl.xdg.toplevel,
                                          _lum.wl.seat, serial,
                                          window->wl.cursorPosX,
                                          window->wl.cursorPosY);
            return;
        }
    }

    // Don’t pass the button to the user if it was related to a decoration.
    if (window->wl.decorations.focus != mainWindow)
        return;

    _lum.wl.serial = serial;

    /* Makes left, right and middle 0, 1 and 2. Overall order follows evdev
     * codes. */
    lumButton = button - BTN_LEFT;

    _lumInputMouseClick(window,
                         lumButton,
                         state == WL_POINTER_BUTTON_STATE_PRESSED
                                ? LUM_PRESS
                                : LUM_RELEASE,
                         _lum.wl.xkb.modifiers);
}

static void pointerHandleAxis(void* data,
                              struct wl_pointer* pointer,
                              uint32_t time,
                              uint32_t axis,
                              wl_fixed_t value)
{
    _LUMwindow* window = _lum.wl.pointerFocus;
    double x = 0.0, y = 0.0;
    // Wayland scroll events are in pointer motion coordinate space (think two
    // finger scroll).  The factor 10 is commonly used to convert to "scroll
    // step means 1.0.
    const double scrollFactor = 1.0 / 10.0;

    if (!window)
        return;

    assert(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ||
           axis == WL_POINTER_AXIS_VERTICAL_SCROLL);

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        x = -wl_fixed_to_double(value) * scrollFactor;
    else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        y = -wl_fixed_to_double(value) * scrollFactor;

    _lumInputScroll(window, x, y);
}

static const struct wl_pointer_listener pointerListener = {
    pointerHandleEnter,
    pointerHandleLeave,
    pointerHandleMotion,
    pointerHandleButton,
    pointerHandleAxis,
};

static void keyboardHandleKeymap(void* data,
                                 struct wl_keyboard* keyboard,
                                 uint32_t format,
                                 int fd,
                                 uint32_t size)
{
    struct xkb_keymap* keymap;
    struct xkb_state* state;
    struct xkb_compose_table* composeTable;
    struct xkb_compose_state* composeState;

    char* mapStr;
    const char* locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        close(fd);
        return;
    }

    mapStr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapStr == MAP_FAILED) {
        close(fd);
        return;
    }

    keymap = xkb_keymap_new_from_string(_lum.wl.xkb.context,
                                        mapStr,
                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                        0);
    munmap(mapStr, size);
    close(fd);

    if (!keymap)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    // Look up the preferred locale, falling back to "C" as default.
    locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    composeTable =
        xkb_compose_table_new_from_locale(_lum.wl.xkb.context, locale,
                                          XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (composeTable)
    {
        composeState =
            xkb_compose_state_new(composeTable, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(composeTable);
        if (composeState)
            _lum.wl.xkb.composeState = composeState;
        else
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Failed to create XKB compose state");
    }
    else
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to create XKB compose table");
    }

    xkb_keymap_unref(_lum.wl.xkb.keymap);
    xkb_state_unref(_lum.wl.xkb.state);
    _lum.wl.xkb.keymap = keymap;
    _lum.wl.xkb.state = state;

    _lum.wl.xkb.controlMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Control");
    _lum.wl.xkb.altMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Mod1");
    _lum.wl.xkb.shiftMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Shift");
    _lum.wl.xkb.superMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Mod4");
    _lum.wl.xkb.capsLockMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Lock");
    _lum.wl.xkb.numLockMask =
        1 << xkb_keymap_mod_get_index(_lum.wl.xkb.keymap, "Mod2");
}

static void keyboardHandleEnter(void* data,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface,
                                struct wl_array* keys)
{
    // Happens in the case we just destroyed the surface.
    if (!surface)
        return;

    _LUMwindow* window = wl_surface_get_user_data(surface);
    if (!window)
    {
        window = findWindowFromDecorationSurface(surface, NULL);
        if (!window)
            return;
    }

    _lum.wl.serial = serial;
    _lum.wl.keyboardFocus = window;
    _lumInputWindowFocus(window, LUM_TRUE);
}

static void keyboardHandleLeave(void* data,
                                struct wl_keyboard* keyboard,
                                uint32_t serial,
                                struct wl_surface* surface)
{
    _LUMwindow* window = _lum.wl.keyboardFocus;

    if (!window)
        return;

    struct itimerspec timer = {};
    timerfd_settime(_lum.wl.timerfd, 0, &timer, NULL);

    _lum.wl.serial = serial;
    _lum.wl.keyboardFocus = NULL;
    _lumInputWindowFocus(window, LUM_FALSE);
}

static int translateKey(uint32_t scancode)
{
    if (scancode < sizeof(_lum.wl.keycodes) / sizeof(_lum.wl.keycodes[0]))
        return _lum.wl.keycodes[scancode];

    return LUM_KEY_UNKNOWN;
}

static xkb_keysym_t composeSymbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !_lum.wl.xkb.composeState)
        return sym;
    if (xkb_compose_state_feed(_lum.wl.xkb.composeState, sym)
            != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;
    switch (xkb_compose_state_get_status(_lum.wl.xkb.composeState))
    {
        case XKB_COMPOSE_COMPOSED:
            return xkb_compose_state_get_one_sym(_lum.wl.xkb.composeState);
        case XKB_COMPOSE_COMPOSING:
        case XKB_COMPOSE_CANCELLED:
            return XKB_KEY_NoSymbol;
        case XKB_COMPOSE_NOTHING:
        default:
            return sym;
    }
}

LUMbool _lumInputTextWayland(_LUMwindow* window, uint32_t scancode)
{
    const xkb_keysym_t* keysyms;
    const xkb_keycode_t keycode = scancode + 8;

    if (xkb_state_key_get_syms(_lum.wl.xkb.state, keycode, &keysyms) == 1)
    {
        const xkb_keysym_t keysym = composeSymbol(keysyms[0]);
        const uint32_t codepoint = _lumKeySym2Unicode(keysym);
        if (codepoint != LUM_INVALID_CODEPOINT)
        {
            const int mods = _lum.wl.xkb.modifiers;
            const int plain = !(mods & (LUM_MOD_CONTROL | LUM_MOD_ALT));
            _lumInputChar(window, codepoint, mods, plain);
        }
    }

    return xkb_keymap_key_repeats(_lum.wl.xkb.keymap, keycode);
}

static void keyboardHandleKey(void* data,
                              struct wl_keyboard* keyboard,
                              uint32_t serial,
                              uint32_t time,
                              uint32_t scancode,
                              uint32_t state)
{
    _LUMwindow* window = _lum.wl.keyboardFocus;
    if (!window)
        return;

    const int key = translateKey(scancode);
    const int action =
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? LUM_PRESS : LUM_RELEASE;

    _lum.wl.serial = serial;
    _lumInputKey(window, key, scancode, action, _lum.wl.xkb.modifiers);

    struct itimerspec timer = {};

    if (action == LUM_PRESS)
    {
        const LUMbool shouldRepeat = _lumInputTextWayland(window, scancode);

        if (shouldRepeat && _lum.wl.keyboardRepeatRate > 0)
        {
            _lum.wl.keyboardLastKey = key;
            _lum.wl.keyboardLastScancode = scancode;
            if (_lum.wl.keyboardRepeatRate > 1)
                timer.it_interval.tv_nsec = 1000000000 / _lum.wl.keyboardRepeatRate;
            else
                timer.it_interval.tv_sec = 1;

            timer.it_value.tv_sec = _lum.wl.keyboardRepeatDelay / 1000;
            timer.it_value.tv_nsec = (_lum.wl.keyboardRepeatDelay % 1000) * 1000000;
        }
    }

    timerfd_settime(_lum.wl.timerfd, 0, &timer, NULL);
}

static void keyboardHandleModifiers(void* data,
                                    struct wl_keyboard* keyboard,
                                    uint32_t serial,
                                    uint32_t modsDepressed,
                                    uint32_t modsLatched,
                                    uint32_t modsLocked,
                                    uint32_t group)
{
    _lum.wl.serial = serial;

    if (!_lum.wl.xkb.keymap)
        return;

    xkb_state_update_mask(_lum.wl.xkb.state,
                          modsDepressed,
                          modsLatched,
                          modsLocked,
                          0,
                          0,
                          group);

    const xkb_mod_mask_t mask =
        xkb_state_serialize_mods(_lum.wl.xkb.state,
                                 XKB_STATE_MODS_DEPRESSED |
                                 XKB_STATE_LAYOUT_DEPRESSED |
                                 XKB_STATE_MODS_LATCHED |
                                 XKB_STATE_LAYOUT_LATCHED);

    unsigned int mods = 0;

    if (mask & _lum.wl.xkb.controlMask)
        mods |= LUM_MOD_CONTROL;
    if (mask & _lum.wl.xkb.altMask)
        mods |= LUM_MOD_ALT;
    if (mask & _lum.wl.xkb.shiftMask)
        mods |= LUM_MOD_SHIFT;
    if (mask & _lum.wl.xkb.superMask)
        mods |= LUM_MOD_SUPER;
    if (mask & _lum.wl.xkb.capsLockMask)
        mods |= LUM_MOD_CAPS_LOCK;
    if (mask & _lum.wl.xkb.numLockMask)
        mods |= LUM_MOD_NUM_LOCK;

    _lum.wl.xkb.modifiers = mods;
}

#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
static void keyboardHandleRepeatInfo(void* data,
                                     struct wl_keyboard* keyboard,
                                     int32_t rate,
                                     int32_t delay)
{
    if (keyboard != _lum.wl.keyboard)
        return;

    _lum.wl.keyboardRepeatRate = rate;
    _lum.wl.keyboardRepeatDelay = delay;
}
#endif

static const struct wl_keyboard_listener keyboardListener = {
    keyboardHandleKeymap,
    keyboardHandleEnter,
    keyboardHandleLeave,
    keyboardHandleKey,
    keyboardHandleModifiers,
#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    keyboardHandleRepeatInfo,
#endif
};

static void seatHandleCapabilities(void* data,
                                   struct wl_seat* seat,
                                   enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !_lum.wl.pointer)
    {
        _lum.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(_lum.wl.pointer, &pointerListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && _lum.wl.pointer)
    {
        wl_pointer_destroy(_lum.wl.pointer);
        _lum.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !_lum.wl.keyboard)
    {
        _lum.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(_lum.wl.keyboard, &keyboardListener, NULL);
    }
    else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && _lum.wl.keyboard)
    {
        wl_keyboard_destroy(_lum.wl.keyboard);
        _lum.wl.keyboard = NULL;
    }
}

static void seatHandleName(void* data,
                           struct wl_seat* seat,
                           const char* name)
{
}

static const struct wl_seat_listener seatListener = {
    seatHandleCapabilities,
    seatHandleName,
};

static void dataOfferHandleOffer(void* data,
                                 struct wl_data_offer* dataOffer,
                                 const char* mimeType)
{
}

static const struct wl_data_offer_listener dataOfferListener = {
    dataOfferHandleOffer,
};

static void dataDeviceHandleDataOffer(void* data,
                                      struct wl_data_device* dataDevice,
                                      struct wl_data_offer* id)
{
    if (_lum.wl.dataOffer)
        wl_data_offer_destroy(_lum.wl.dataOffer);

    _lum.wl.dataOffer = id;
    wl_data_offer_add_listener(_lum.wl.dataOffer, &dataOfferListener, NULL);
}

static void dataDeviceHandleEnter(void* data,
                                  struct wl_data_device* dataDevice,
                                  uint32_t serial,
                                  struct wl_surface *surface,
                                  wl_fixed_t x,
                                  wl_fixed_t y,
                                  struct wl_data_offer *id)
{
}

static void dataDeviceHandleLeave(void* data,
                                  struct wl_data_device* dataDevice)
{
}

static void dataDeviceHandleMotion(void* data,
                                   struct wl_data_device* dataDevice,
                                   uint32_t time,
                                   wl_fixed_t x,
                                   wl_fixed_t y)
{
}

static void dataDeviceHandleDrop(void* data,
                                 struct wl_data_device* dataDevice)
{
}

static void dataDeviceHandleSelection(void* data,
                                      struct wl_data_device* dataDevice,
                                      struct wl_data_offer* id)
{
}

static const struct wl_data_device_listener dataDeviceListener = {
    dataDeviceHandleDataOffer,
    dataDeviceHandleEnter,
    dataDeviceHandleLeave,
    dataDeviceHandleMotion,
    dataDeviceHandleDrop,
    dataDeviceHandleSelection,
};

static void wmBaseHandlePing(void* data,
                             struct xdg_wm_base* wmBase,
                             uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

static const struct xdg_wm_base_listener wmBaseListener = {
    wmBaseHandlePing
};

static void registryHandleGlobal(void* data,
                                 struct wl_registry* registry,
                                 uint32_t name,
                                 const char* interface,
                                 uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        _lum.wl.compositorVersion = min(3, version);
        _lum.wl.compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface,
                             _lum.wl.compositorVersion);
    }
    else if (strcmp(interface, "wl_subcompositor") == 0)
    {
        _lum.wl.subcompositor =
            wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shm") == 0)
    {
        _lum.wl.shm =
            wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        _lumAddOutputWayland(name, version);
    }
    else if (strcmp(interface, "wl_seat") == 0)
    {
        if (!_lum.wl.seat)
        {
            _lum.wl.seatVersion = min(4, version);
            _lum.wl.seat =
                wl_registry_bind(registry, name, &wl_seat_interface,
                                 _lum.wl.seatVersion);
            wl_seat_add_listener(_lum.wl.seat, &seatListener, NULL);
        }
    }
    else if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        if (!_lum.wl.dataDeviceManager)
        {
            _lum.wl.dataDeviceManager =
                wl_registry_bind(registry, name,
                                 &wl_data_device_manager_interface, 1);
        }
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        _lum.wl.wmBase =
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(_lum.wl.wmBase, &wmBaseListener, NULL);
    }
    else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0)
    {
        _lum.wl.decorationManager =
            wl_registry_bind(registry, name,
                             &zxdg_decoration_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "wp_viewporter") == 0)
    {
        _lum.wl.viewporter =
            wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
    else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0)
    {
        _lum.wl.relativePointerManager =
            wl_registry_bind(registry, name,
                             &zwp_relative_pointer_manager_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0)
    {
        _lum.wl.pointerConstraints =
            wl_registry_bind(registry, name,
                             &zwp_pointer_constraints_v1_interface,
                             1);
    }
    else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0)
    {
        _lum.wl.idleInhibitManager =
            wl_registry_bind(registry, name,
                             &zwp_idle_inhibit_manager_v1_interface,
                             1);
    }
}

static void registryHandleGlobalRemove(void *data,
                                       struct wl_registry *registry,
                                       uint32_t name)
{
    _LUMmonitor* monitor;

    for (int i = 0; i < _lum.monitorCount; ++i)
    {
        monitor = _lum.monitors[i];
        if (monitor->wl.name == name)
        {
            _lumInputMonitor(monitor, LUM_DISCONNECTED, 0);
            return;
        }
    }
}


static const struct wl_registry_listener registryListener = {
    registryHandleGlobal,
    registryHandleGlobalRemove
};

// Create key code translation tables
//
static void createKeyTables(void)
{
    memset(_lum.wl.keycodes, -1, sizeof(_lum.wl.keycodes));
    memset(_lum.wl.scancodes, -1, sizeof(_lum.wl.scancodes));

    _lum.wl.keycodes[KEY_GRAVE]      = LUM_KEY_GRAVE_ACCENT;
    _lum.wl.keycodes[KEY_1]          = LUM_KEY_1;
    _lum.wl.keycodes[KEY_2]          = LUM_KEY_2;
    _lum.wl.keycodes[KEY_3]          = LUM_KEY_3;
    _lum.wl.keycodes[KEY_4]          = LUM_KEY_4;
    _lum.wl.keycodes[KEY_5]          = LUM_KEY_5;
    _lum.wl.keycodes[KEY_6]          = LUM_KEY_6;
    _lum.wl.keycodes[KEY_7]          = LUM_KEY_7;
    _lum.wl.keycodes[KEY_8]          = LUM_KEY_8;
    _lum.wl.keycodes[KEY_9]          = LUM_KEY_9;
    _lum.wl.keycodes[KEY_0]          = LUM_KEY_0;
    _lum.wl.keycodes[KEY_SPACE]      = LUM_KEY_SPACE;
    _lum.wl.keycodes[KEY_MINUS]      = LUM_KEY_MINUS;
    _lum.wl.keycodes[KEY_EQUAL]      = LUM_KEY_EQUAL;
    _lum.wl.keycodes[KEY_Q]          = LUM_KEY_Q;
    _lum.wl.keycodes[KEY_W]          = LUM_KEY_W;
    _lum.wl.keycodes[KEY_E]          = LUM_KEY_E;
    _lum.wl.keycodes[KEY_R]          = LUM_KEY_R;
    _lum.wl.keycodes[KEY_T]          = LUM_KEY_T;
    _lum.wl.keycodes[KEY_Y]          = LUM_KEY_Y;
    _lum.wl.keycodes[KEY_U]          = LUM_KEY_U;
    _lum.wl.keycodes[KEY_I]          = LUM_KEY_I;
    _lum.wl.keycodes[KEY_O]          = LUM_KEY_O;
    _lum.wl.keycodes[KEY_P]          = LUM_KEY_P;
    _lum.wl.keycodes[KEY_LEFTBRACE]  = LUM_KEY_LEFT_BRACKET;
    _lum.wl.keycodes[KEY_RIGHTBRACE] = LUM_KEY_RIGHT_BRACKET;
    _lum.wl.keycodes[KEY_A]          = LUM_KEY_A;
    _lum.wl.keycodes[KEY_S]          = LUM_KEY_S;
    _lum.wl.keycodes[KEY_D]          = LUM_KEY_D;
    _lum.wl.keycodes[KEY_F]          = LUM_KEY_F;
    _lum.wl.keycodes[KEY_G]          = LUM_KEY_G;
    _lum.wl.keycodes[KEY_H]          = LUM_KEY_H;
    _lum.wl.keycodes[KEY_J]          = LUM_KEY_J;
    _lum.wl.keycodes[KEY_K]          = LUM_KEY_K;
    _lum.wl.keycodes[KEY_L]          = LUM_KEY_L;
    _lum.wl.keycodes[KEY_SEMICOLON]  = LUM_KEY_SEMICOLON;
    _lum.wl.keycodes[KEY_APOSTROPHE] = LUM_KEY_APOSTROPHE;
    _lum.wl.keycodes[KEY_Z]          = LUM_KEY_Z;
    _lum.wl.keycodes[KEY_X]          = LUM_KEY_X;
    _lum.wl.keycodes[KEY_C]          = LUM_KEY_C;
    _lum.wl.keycodes[KEY_V]          = LUM_KEY_V;
    _lum.wl.keycodes[KEY_B]          = LUM_KEY_B;
    _lum.wl.keycodes[KEY_N]          = LUM_KEY_N;
    _lum.wl.keycodes[KEY_M]          = LUM_KEY_M;
    _lum.wl.keycodes[KEY_COMMA]      = LUM_KEY_COMMA;
    _lum.wl.keycodes[KEY_DOT]        = LUM_KEY_PERIOD;
    _lum.wl.keycodes[KEY_SLASH]      = LUM_KEY_SLASH;
    _lum.wl.keycodes[KEY_BACKSLASH]  = LUM_KEY_BACKSLASH;
    _lum.wl.keycodes[KEY_ESC]        = LUM_KEY_ESCAPE;
    _lum.wl.keycodes[KEY_TAB]        = LUM_KEY_TAB;
    _lum.wl.keycodes[KEY_LEFTSHIFT]  = LUM_KEY_LEFT_SHIFT;
    _lum.wl.keycodes[KEY_RIGHTSHIFT] = LUM_KEY_RIGHT_SHIFT;
    _lum.wl.keycodes[KEY_LEFTCTRL]   = LUM_KEY_LEFT_CONTROL;
    _lum.wl.keycodes[KEY_RIGHTCTRL]  = LUM_KEY_RIGHT_CONTROL;
    _lum.wl.keycodes[KEY_LEFTALT]    = LUM_KEY_LEFT_ALT;
    _lum.wl.keycodes[KEY_RIGHTALT]   = LUM_KEY_RIGHT_ALT;
    _lum.wl.keycodes[KEY_LEFTMETA]   = LUM_KEY_LEFT_SUPER;
    _lum.wl.keycodes[KEY_RIGHTMETA]  = LUM_KEY_RIGHT_SUPER;
    _lum.wl.keycodes[KEY_COMPOSE]    = LUM_KEY_MENU;
    _lum.wl.keycodes[KEY_NUMLOCK]    = LUM_KEY_NUM_LOCK;
    _lum.wl.keycodes[KEY_CAPSLOCK]   = LUM_KEY_CAPS_LOCK;
    _lum.wl.keycodes[KEY_PRINT]      = LUM_KEY_PRINT_SCREEN;
    _lum.wl.keycodes[KEY_SCROLLLOCK] = LUM_KEY_SCROLL_LOCK;
    _lum.wl.keycodes[KEY_PAUSE]      = LUM_KEY_PAUSE;
    _lum.wl.keycodes[KEY_DELETE]     = LUM_KEY_DELETE;
    _lum.wl.keycodes[KEY_BACKSPACE]  = LUM_KEY_BACKSPACE;
    _lum.wl.keycodes[KEY_ENTER]      = LUM_KEY_ENTER;
    _lum.wl.keycodes[KEY_HOME]       = LUM_KEY_HOME;
    _lum.wl.keycodes[KEY_END]        = LUM_KEY_END;
    _lum.wl.keycodes[KEY_PAGEUP]     = LUM_KEY_PAGE_UP;
    _lum.wl.keycodes[KEY_PAGEDOWN]   = LUM_KEY_PAGE_DOWN;
    _lum.wl.keycodes[KEY_INSERT]     = LUM_KEY_INSERT;
    _lum.wl.keycodes[KEY_LEFT]       = LUM_KEY_LEFT;
    _lum.wl.keycodes[KEY_RIGHT]      = LUM_KEY_RIGHT;
    _lum.wl.keycodes[KEY_DOWN]       = LUM_KEY_DOWN;
    _lum.wl.keycodes[KEY_UP]         = LUM_KEY_UP;
    _lum.wl.keycodes[KEY_F1]         = LUM_KEY_F1;
    _lum.wl.keycodes[KEY_F2]         = LUM_KEY_F2;
    _lum.wl.keycodes[KEY_F3]         = LUM_KEY_F3;
    _lum.wl.keycodes[KEY_F4]         = LUM_KEY_F4;
    _lum.wl.keycodes[KEY_F5]         = LUM_KEY_F5;
    _lum.wl.keycodes[KEY_F6]         = LUM_KEY_F6;
    _lum.wl.keycodes[KEY_F7]         = LUM_KEY_F7;
    _lum.wl.keycodes[KEY_F8]         = LUM_KEY_F8;
    _lum.wl.keycodes[KEY_F9]         = LUM_KEY_F9;
    _lum.wl.keycodes[KEY_F10]        = LUM_KEY_F10;
    _lum.wl.keycodes[KEY_F11]        = LUM_KEY_F11;
    _lum.wl.keycodes[KEY_F12]        = LUM_KEY_F12;
    _lum.wl.keycodes[KEY_F13]        = LUM_KEY_F13;
    _lum.wl.keycodes[KEY_F14]        = LUM_KEY_F14;
    _lum.wl.keycodes[KEY_F15]        = LUM_KEY_F15;
    _lum.wl.keycodes[KEY_F16]        = LUM_KEY_F16;
    _lum.wl.keycodes[KEY_F17]        = LUM_KEY_F17;
    _lum.wl.keycodes[KEY_F18]        = LUM_KEY_F18;
    _lum.wl.keycodes[KEY_F19]        = LUM_KEY_F19;
    _lum.wl.keycodes[KEY_F20]        = LUM_KEY_F20;
    _lum.wl.keycodes[KEY_F21]        = LUM_KEY_F21;
    _lum.wl.keycodes[KEY_F22]        = LUM_KEY_F22;
    _lum.wl.keycodes[KEY_F23]        = LUM_KEY_F23;
    _lum.wl.keycodes[KEY_F24]        = LUM_KEY_F24;
    _lum.wl.keycodes[KEY_KPSLASH]    = LUM_KEY_KP_DIVIDE;
    _lum.wl.keycodes[KEY_KPASTERISK] = LUM_KEY_KP_MULTIPLY;
    _lum.wl.keycodes[KEY_KPMINUS]    = LUM_KEY_KP_SUBTRACT;
    _lum.wl.keycodes[KEY_KPPLUS]     = LUM_KEY_KP_ADD;
    _lum.wl.keycodes[KEY_KP0]        = LUM_KEY_KP_0;
    _lum.wl.keycodes[KEY_KP1]        = LUM_KEY_KP_1;
    _lum.wl.keycodes[KEY_KP2]        = LUM_KEY_KP_2;
    _lum.wl.keycodes[KEY_KP3]        = LUM_KEY_KP_3;
    _lum.wl.keycodes[KEY_KP4]        = LUM_KEY_KP_4;
    _lum.wl.keycodes[KEY_KP5]        = LUM_KEY_KP_5;
    _lum.wl.keycodes[KEY_KP6]        = LUM_KEY_KP_6;
    _lum.wl.keycodes[KEY_KP7]        = LUM_KEY_KP_7;
    _lum.wl.keycodes[KEY_KP8]        = LUM_KEY_KP_8;
    _lum.wl.keycodes[KEY_KP9]        = LUM_KEY_KP_9;
    _lum.wl.keycodes[KEY_KPDOT]      = LUM_KEY_KP_DECIMAL;
    _lum.wl.keycodes[KEY_KPEQUAL]    = LUM_KEY_KP_EQUAL;
    _lum.wl.keycodes[KEY_KPENTER]    = LUM_KEY_KP_ENTER;
    _lum.wl.keycodes[KEY_102ND]      = LUM_KEY_WORLD_2;

    for (int scancode = 0;  scancode < 256;  scancode++)
    {
        if (_lum.wl.keycodes[scancode] > 0)
            _lum.wl.scancodes[_lum.wl.keycodes[scancode]] = scancode;
    }
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumConnectWayland(int platformID, _LUMplatform* platform)
{
    const _LUMplatform wayland =
    {
        LUM_PLATFORM_WAYLAND,
        _lumInitWayland,
        _lumTerminateWayland,
        _lumGetCursorPosWayland,
        _lumSetCursorPosWayland,
        _lumSetCursorModeWayland,
        _lumSetRawMouseMotionWayland,
        _lumRawMouseMotionSupportedWayland,
        _lumCreateCursorWayland,
        _lumCreateStandardCursorWayland,
        _lumDestroyCursorWayland,
        _lumSetCursorWayland,
        _lumGetScancodeNameWayland,
        _lumGetKeyScancodeWayland,
        _lumSetClipboardStringWayland,
        _lumGetClipboardStringWayland,
#if defined(__linux__)
        _lumInitJoysticksLinux,
        _lumTerminateJoysticksLinux,
        _lumPollJoystickLinux,
        _lumGetMappingNameLinux,
        _lumUpdateGamepadGUIDLinux,
#else
        _lumInitJoysticksNull,
        _lumTerminateJoysticksNull,
        _lumPollJoystickNull,
        _lumGetMappingNameNull,
        _lumUpdateGamepadGUIDNull,
#endif
        _lumFreeMonitorWayland,
        _lumGetMonitorPosWayland,
        _lumGetMonitorContentScaleWayland,
        _lumGetMonitorWorkareaWayland,
        _lumGetVideoModesWayland,
        _lumGetVideoModeWayland,
        _lumGetGammaRampWayland,
        _lumSetGammaRampWayland,
        _lumCreateWindowWayland,
        _lumDestroyWindowWayland,
        _lumSetWindowTitleWayland,
        _lumSetWindowIconWayland,
        _lumGetWindowPosWayland,
        _lumSetWindowPosWayland,
        _lumGetWindowSizeWayland,
        _lumSetWindowSizeWayland,
        _lumSetWindowSizeLimitsWayland,
        _lumSetWindowAspectRatioWayland,
        _lumGetFramebufferSizeWayland,
        _lumGetWindowFrameSizeWayland,
        _lumGetWindowContentScaleWayland,
        _lumIconifyWindowWayland,
        _lumRestoreWindowWayland,
        _lumMaximizeWindowWayland,
        _lumShowWindowWayland,
        _lumHideWindowWayland,
        _lumRequestWindowAttentionWayland,
        _lumFocusWindowWayland,
        _lumSetWindowMonitorWayland,
        _lumWindowFocusedWayland,
        _lumWindowIconifiedWayland,
        _lumWindowVisibleWayland,
        _lumWindowMaximizedWayland,
        _lumWindowHoveredWayland,
        _lumFramebufferTransparentWayland,
        _lumGetWindowOpacityWayland,
        _lumSetWindowResizableWayland,
        _lumSetWindowDecoratedWayland,
        _lumSetWindowFloatingWayland,
        _lumSetWindowOpacityWayland,
        _lumSetWindowMousePassthroughWayland,
        _lumPollEventsWayland,
        _lumWaitEventsWayland,
        _lumWaitEventsTimeoutWayland,
        _lumPostEmptyEventWayland,
        _lumGetEGLPlatformWayland,
        _lumGetEGLNativeDisplayWayland,
        _lumGetEGLNativeWindowWayland,
        _lumGetRequiredInstanceExtensionsWayland,
        _lumGetPhysicalDevicePresentationSupportWayland,
        _lumCreateWindowSurfaceWayland,
    };

    void* module = _lumPlatformLoadModule("libwayland-client.so.0");
    if (!module)
    {
        if (platformID == LUM_PLATFORM_WAYLAND)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Failed to open libwayland-client");
        }

        return LUM_FALSE;
    }

    PFN_wl_display_connect wl_display_connect = (PFN_wl_display_connect)
        _lumPlatformGetModuleSymbol(module, "wl_display_connect");
    if (!wl_display_connect)
    {
        if (platformID == LUM_PLATFORM_WAYLAND)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Failed to load libwayland-client entry point");
        }

        _lumPlatformFreeModule(module);
        return LUM_FALSE;
    }

    struct wl_display* display = wl_display_connect(NULL);
    if (!display)
    {
        if (platformID == LUM_PLATFORM_WAYLAND)
            _lumInputError(LUM_PLATFORM_ERROR, "Wayland: Failed to connect to display");

        _lumPlatformFreeModule(module);
        return LUM_FALSE;
    }

    _lum.wl.display = display;
    _lum.wl.client.handle = module;

    *platform = wayland;
    return LUM_TRUE;
}

int _lumInitWayland(void)
{
    const char *cursorTheme;
    const char *cursorSizeStr;
    char *cursorSizeEnd;
    long cursorSizeLong;
    int cursorSize;

    _lum.wl.client.display_flush = (PFN_wl_display_flush)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_flush");
    _lum.wl.client.display_cancel_read = (PFN_wl_display_cancel_read)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_cancel_read");
    _lum.wl.client.display_dispatch_pending = (PFN_wl_display_dispatch_pending)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_dispatch_pending");
    _lum.wl.client.display_read_events = (PFN_wl_display_read_events)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_read_events");
    _lum.wl.client.display_disconnect = (PFN_wl_display_disconnect)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_disconnect");
    _lum.wl.client.display_roundtrip = (PFN_wl_display_roundtrip)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_roundtrip");
    _lum.wl.client.display_get_fd = (PFN_wl_display_get_fd)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_get_fd");
    _lum.wl.client.display_prepare_read = (PFN_wl_display_prepare_read)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_display_prepare_read");
    _lum.wl.client.proxy_marshal = (PFN_wl_proxy_marshal)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_marshal");
    _lum.wl.client.proxy_add_listener = (PFN_wl_proxy_add_listener)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_add_listener");
    _lum.wl.client.proxy_destroy = (PFN_wl_proxy_destroy)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_destroy");
    _lum.wl.client.proxy_marshal_constructor = (PFN_wl_proxy_marshal_constructor)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_marshal_constructor");
    _lum.wl.client.proxy_marshal_constructor_versioned = (PFN_wl_proxy_marshal_constructor_versioned)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_marshal_constructor_versioned");
    _lum.wl.client.proxy_get_user_data = (PFN_wl_proxy_get_user_data)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_get_user_data");
    _lum.wl.client.proxy_set_user_data = (PFN_wl_proxy_set_user_data)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_set_user_data");
    _lum.wl.client.proxy_get_version = (PFN_wl_proxy_get_version)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_get_version");
    _lum.wl.client.proxy_marshal_flags = (PFN_wl_proxy_marshal_flags)
        _lumPlatformGetModuleSymbol(_lum.wl.client.handle, "wl_proxy_marshal_flags");

    if (!_lum.wl.client.display_flush ||
        !_lum.wl.client.display_cancel_read ||
        !_lum.wl.client.display_dispatch_pending ||
        !_lum.wl.client.display_read_events ||
        !_lum.wl.client.display_disconnect ||
        !_lum.wl.client.display_roundtrip ||
        !_lum.wl.client.display_get_fd ||
        !_lum.wl.client.display_prepare_read ||
        !_lum.wl.client.proxy_marshal ||
        !_lum.wl.client.proxy_add_listener ||
        !_lum.wl.client.proxy_destroy ||
        !_lum.wl.client.proxy_marshal_constructor ||
        !_lum.wl.client.proxy_marshal_constructor_versioned ||
        !_lum.wl.client.proxy_get_user_data ||
        !_lum.wl.client.proxy_set_user_data)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to load libwayland-client entry point");
        return LUM_FALSE;
    }

    _lum.wl.cursor.handle = _lumPlatformLoadModule("libwayland-cursor.so.0");
    if (!_lum.wl.cursor.handle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to open libwayland-cursor");
        return LUM_FALSE;
    }

    _lum.wl.cursor.theme_load = (PFN_wl_cursor_theme_load)
        _lumPlatformGetModuleSymbol(_lum.wl.cursor.handle, "wl_cursor_theme_load");
    _lum.wl.cursor.theme_destroy = (PFN_wl_cursor_theme_destroy)
        _lumPlatformGetModuleSymbol(_lum.wl.cursor.handle, "wl_cursor_theme_destroy");
    _lum.wl.cursor.theme_get_cursor = (PFN_wl_cursor_theme_get_cursor)
        _lumPlatformGetModuleSymbol(_lum.wl.cursor.handle, "wl_cursor_theme_get_cursor");
    _lum.wl.cursor.image_get_buffer = (PFN_wl_cursor_image_get_buffer)
        _lumPlatformGetModuleSymbol(_lum.wl.cursor.handle, "wl_cursor_image_get_buffer");

    _lum.wl.egl.handle = _lumPlatformLoadModule("libwayland-egl.so.1");
    if (!_lum.wl.egl.handle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to open libwayland-egl");
        return LUM_FALSE;
    }

    _lum.wl.egl.window_create = (PFN_wl_egl_window_create)
        _lumPlatformGetModuleSymbol(_lum.wl.egl.handle, "wl_egl_window_create");
    _lum.wl.egl.window_destroy = (PFN_wl_egl_window_destroy)
        _lumPlatformGetModuleSymbol(_lum.wl.egl.handle, "wl_egl_window_destroy");
    _lum.wl.egl.window_resize = (PFN_wl_egl_window_resize)
        _lumPlatformGetModuleSymbol(_lum.wl.egl.handle, "wl_egl_window_resize");

    _lum.wl.xkb.handle = _lumPlatformLoadModule("libxkbcommon.so.0");
    if (!_lum.wl.xkb.handle)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to open libxkbcommon");
        return LUM_FALSE;
    }

    _lum.wl.xkb.context_new = (PFN_xkb_context_new)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_context_new");
    _lum.wl.xkb.context_unref = (PFN_xkb_context_unref)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_context_unref");
    _lum.wl.xkb.keymap_new_from_string = (PFN_xkb_keymap_new_from_string)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_keymap_new_from_string");
    _lum.wl.xkb.keymap_unref = (PFN_xkb_keymap_unref)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_keymap_unref");
    _lum.wl.xkb.keymap_mod_get_index = (PFN_xkb_keymap_mod_get_index)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_keymap_mod_get_index");
    _lum.wl.xkb.keymap_key_repeats = (PFN_xkb_keymap_key_repeats)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_keymap_key_repeats");
    _lum.wl.xkb.keymap_key_get_syms_by_level = (PFN_xkb_keymap_key_get_syms_by_level)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_keymap_key_get_syms_by_level");
    _lum.wl.xkb.state_new = (PFN_xkb_state_new)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_new");
    _lum.wl.xkb.state_unref = (PFN_xkb_state_unref)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_unref");
    _lum.wl.xkb.state_key_get_syms = (PFN_xkb_state_key_get_syms)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_key_get_syms");
    _lum.wl.xkb.state_update_mask = (PFN_xkb_state_update_mask)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_update_mask");
    _lum.wl.xkb.state_serialize_mods = (PFN_xkb_state_serialize_mods)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_serialize_mods");
    _lum.wl.xkb.state_key_get_layout = (PFN_xkb_state_key_get_layout)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_state_key_get_layout");
    _lum.wl.xkb.compose_table_new_from_locale = (PFN_xkb_compose_table_new_from_locale)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_table_new_from_locale");
    _lum.wl.xkb.compose_table_unref = (PFN_xkb_compose_table_unref)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_table_unref");
    _lum.wl.xkb.compose_state_new = (PFN_xkb_compose_state_new)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_state_new");
    _lum.wl.xkb.compose_state_unref = (PFN_xkb_compose_state_unref)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_state_unref");
    _lum.wl.xkb.compose_state_feed = (PFN_xkb_compose_state_feed)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_state_feed");
    _lum.wl.xkb.compose_state_get_status = (PFN_xkb_compose_state_get_status)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_state_get_status");
    _lum.wl.xkb.compose_state_get_one_sym = (PFN_xkb_compose_state_get_one_sym)
        _lumPlatformGetModuleSymbol(_lum.wl.xkb.handle, "xkb_compose_state_get_one_sym");

    _lum.wl.registry = wl_display_get_registry(_lum.wl.display);
    wl_registry_add_listener(_lum.wl.registry, &registryListener, NULL);

    createKeyTables();

    _lum.wl.xkb.context = xkb_context_new(0);
    if (!_lum.wl.xkb.context)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to initialize xkb context");
        return LUM_FALSE;
    }

    // Sync so we got all registry objects
    wl_display_roundtrip(_lum.wl.display);

    // Sync so we got all initial output events
    wl_display_roundtrip(_lum.wl.display);

    _lum.wl.timerfd = -1;
    if (_lum.wl.seatVersion >= 4)
        _lum.wl.timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);

    if (!_lum.wl.wmBase)
    {
        _lumInputError(LUM_PLATFORM_ERROR,
                        "Wayland: Failed to find xdg-shell in your compositor");
        return LUM_FALSE;
    }

    if (_lum.wl.pointer && _lum.wl.shm)
    {
        cursorTheme = getenv("XCURSOR_THEME");
        cursorSizeStr = getenv("XCURSOR_SIZE");
        cursorSize = 32;
        if (cursorSizeStr)
        {
            errno = 0;
            cursorSizeLong = strtol(cursorSizeStr, &cursorSizeEnd, 10);
            if (!*cursorSizeEnd && !errno && cursorSizeLong > 0 && cursorSizeLong <= INT_MAX)
                cursorSize = (int)cursorSizeLong;
        }
        _lum.wl.cursorTheme =
            wl_cursor_theme_load(cursorTheme, cursorSize, _lum.wl.shm);
        if (!_lum.wl.cursorTheme)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Unable to load default cursor theme");
            return LUM_FALSE;
        }
        // If this happens to be NULL, we just fallback to the scale=1 version.
        _lum.wl.cursorThemeHiDPI =
            wl_cursor_theme_load(cursorTheme, 2 * cursorSize, _lum.wl.shm);
        _lum.wl.cursorSurface =
            wl_compositor_create_surface(_lum.wl.compositor);
        _lum.wl.cursorTimerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    }

    if (_lum.wl.seat && _lum.wl.dataDeviceManager)
    {
        _lum.wl.dataDevice =
            wl_data_device_manager_get_data_device(_lum.wl.dataDeviceManager,
                                                   _lum.wl.seat);
        wl_data_device_add_listener(_lum.wl.dataDevice, &dataDeviceListener, NULL);
        _lum.wl.clipboardString = _lum_calloc(4096, 1);
        if (!_lum.wl.clipboardString)
        {
            _lumInputError(LUM_PLATFORM_ERROR,
                            "Wayland: Unable to allocate clipboard memory");
            return LUM_FALSE;
        }
        _lum.wl.clipboardSize = 4096;
    }

    return LUM_TRUE;
}

void _lumTerminateWayland(void)
{
    _lumTerminateEGL();
    if (_lum.wl.egl.handle)
    {
        _lumPlatformFreeModule(_lum.wl.egl.handle);
        _lum.wl.egl.handle = NULL;
    }

    if (_lum.wl.xkb.composeState)
        xkb_compose_state_unref(_lum.wl.xkb.composeState);
    if (_lum.wl.xkb.keymap)
        xkb_keymap_unref(_lum.wl.xkb.keymap);
    if (_lum.wl.xkb.state)
        xkb_state_unref(_lum.wl.xkb.state);
    if (_lum.wl.xkb.context)
        xkb_context_unref(_lum.wl.xkb.context);
    if (_lum.wl.xkb.handle)
    {
        _lumPlatformFreeModule(_lum.wl.xkb.handle);
        _lum.wl.xkb.handle = NULL;
    }

    if (_lum.wl.cursorTheme)
        wl_cursor_theme_destroy(_lum.wl.cursorTheme);
    if (_lum.wl.cursorThemeHiDPI)
        wl_cursor_theme_destroy(_lum.wl.cursorThemeHiDPI);
    if (_lum.wl.cursor.handle)
    {
        _lumPlatformFreeModule(_lum.wl.cursor.handle);
        _lum.wl.cursor.handle = NULL;
    }

    if (_lum.wl.cursorSurface)
        wl_surface_destroy(_lum.wl.cursorSurface);
    if (_lum.wl.subcompositor)
        wl_subcompositor_destroy(_lum.wl.subcompositor);
    if (_lum.wl.compositor)
        wl_compositor_destroy(_lum.wl.compositor);
    if (_lum.wl.shm)
        wl_shm_destroy(_lum.wl.shm);
    if (_lum.wl.viewporter)
        wp_viewporter_destroy(_lum.wl.viewporter);
    if (_lum.wl.decorationManager)
        zxdg_decoration_manager_v1_destroy(_lum.wl.decorationManager);
    if (_lum.wl.wmBase)
        xdg_wm_base_destroy(_lum.wl.wmBase);
    if (_lum.wl.dataSource)
        wl_data_source_destroy(_lum.wl.dataSource);
    if (_lum.wl.dataDevice)
        wl_data_device_destroy(_lum.wl.dataDevice);
    if (_lum.wl.dataOffer)
        wl_data_offer_destroy(_lum.wl.dataOffer);
    if (_lum.wl.dataDeviceManager)
        wl_data_device_manager_destroy(_lum.wl.dataDeviceManager);
    if (_lum.wl.pointer)
        wl_pointer_destroy(_lum.wl.pointer);
    if (_lum.wl.keyboard)
        wl_keyboard_destroy(_lum.wl.keyboard);
    if (_lum.wl.seat)
        wl_seat_destroy(_lum.wl.seat);
    if (_lum.wl.relativePointerManager)
        zwp_relative_pointer_manager_v1_destroy(_lum.wl.relativePointerManager);
    if (_lum.wl.pointerConstraints)
        zwp_pointer_constraints_v1_destroy(_lum.wl.pointerConstraints);
    if (_lum.wl.idleInhibitManager)
        zwp_idle_inhibit_manager_v1_destroy(_lum.wl.idleInhibitManager);
    if (_lum.wl.registry)
        wl_registry_destroy(_lum.wl.registry);
    if (_lum.wl.display)
    {
        wl_display_flush(_lum.wl.display);
        wl_display_disconnect(_lum.wl.display);
    }

    if (_lum.wl.timerfd >= 0)
        close(_lum.wl.timerfd);
    if (_lum.wl.cursorTimerfd >= 0)
        close(_lum.wl.cursorTimerfd);

    if (_lum.wl.clipboardString)
        _lum_free(_lum.wl.clipboardString);
    if (_lum.wl.clipboardSendString)
        _lum_free(_lum.wl.clipboardSendString);
}

