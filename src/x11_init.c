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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <locale.h>
#include <unistd.h>


// Translate the X11 KeySyms for a key to a Lumina key code
// NOTE: This is only used as a fallback, in case the XKB method fails
//       It is layout-dependent and will fail partially on most non-US layouts
//
static int translateKeySyms(const KeySym* keysyms, int width)
{
    if (width > 1)
    {
        switch (keysyms[1])
        {
            case XK_KP_0:           return LUM_KEY_KP_0;
            case XK_KP_1:           return LUM_KEY_KP_1;
            case XK_KP_2:           return LUM_KEY_KP_2;
            case XK_KP_3:           return LUM_KEY_KP_3;
            case XK_KP_4:           return LUM_KEY_KP_4;
            case XK_KP_5:           return LUM_KEY_KP_5;
            case XK_KP_6:           return LUM_KEY_KP_6;
            case XK_KP_7:           return LUM_KEY_KP_7;
            case XK_KP_8:           return LUM_KEY_KP_8;
            case XK_KP_9:           return LUM_KEY_KP_9;
            case XK_KP_Separator:
            case XK_KP_Decimal:     return LUM_KEY_KP_DECIMAL;
            case XK_KP_Equal:       return LUM_KEY_KP_EQUAL;
            case XK_KP_Enter:       return LUM_KEY_KP_ENTER;
            default:                break;
        }
    }

    switch (keysyms[0])
    {
        case XK_Escape:         return LUM_KEY_ESCAPE;
        case XK_Tab:            return LUM_KEY_TAB;
        case XK_Shift_L:        return LUM_KEY_LEFT_SHIFT;
        case XK_Shift_R:        return LUM_KEY_RIGHT_SHIFT;
        case XK_Control_L:      return LUM_KEY_LEFT_CONTROL;
        case XK_Control_R:      return LUM_KEY_RIGHT_CONTROL;
        case XK_Meta_L:
        case XK_Alt_L:          return LUM_KEY_LEFT_ALT;
        case XK_Mode_switch: // Mapped to Alt_R on many keyboards
        case XK_ISO_Level3_Shift: // AltGr on at least some machines
        case XK_Meta_R:
        case XK_Alt_R:          return LUM_KEY_RIGHT_ALT;
        case XK_Super_L:        return LUM_KEY_LEFT_SUPER;
        case XK_Super_R:        return LUM_KEY_RIGHT_SUPER;
        case XK_Menu:           return LUM_KEY_MENU;
        case XK_Num_Lock:       return LUM_KEY_NUM_LOCK;
        case XK_Caps_Lock:      return LUM_KEY_CAPS_LOCK;
        case XK_Print:          return LUM_KEY_PRINT_SCREEN;
        case XK_Scroll_Lock:    return LUM_KEY_SCROLL_LOCK;
        case XK_Pause:          return LUM_KEY_PAUSE;
        case XK_Delete:         return LUM_KEY_DELETE;
        case XK_BackSpace:      return LUM_KEY_BACKSPACE;
        case XK_Return:         return LUM_KEY_ENTER;
        case XK_Home:           return LUM_KEY_HOME;
        case XK_End:            return LUM_KEY_END;
        case XK_Page_Up:        return LUM_KEY_PAGE_UP;
        case XK_Page_Down:      return LUM_KEY_PAGE_DOWN;
        case XK_Insert:         return LUM_KEY_INSERT;
        case XK_Left:           return LUM_KEY_LEFT;
        case XK_Right:          return LUM_KEY_RIGHT;
        case XK_Down:           return LUM_KEY_DOWN;
        case XK_Up:             return LUM_KEY_UP;
        case XK_F1:             return LUM_KEY_F1;
        case XK_F2:             return LUM_KEY_F2;
        case XK_F3:             return LUM_KEY_F3;
        case XK_F4:             return LUM_KEY_F4;
        case XK_F5:             return LUM_KEY_F5;
        case XK_F6:             return LUM_KEY_F6;
        case XK_F7:             return LUM_KEY_F7;
        case XK_F8:             return LUM_KEY_F8;
        case XK_F9:             return LUM_KEY_F9;
        case XK_F10:            return LUM_KEY_F10;
        case XK_F11:            return LUM_KEY_F11;
        case XK_F12:            return LUM_KEY_F12;
        case XK_F13:            return LUM_KEY_F13;
        case XK_F14:            return LUM_KEY_F14;
        case XK_F15:            return LUM_KEY_F15;
        case XK_F16:            return LUM_KEY_F16;
        case XK_F17:            return LUM_KEY_F17;
        case XK_F18:            return LUM_KEY_F18;
        case XK_F19:            return LUM_KEY_F19;
        case XK_F20:            return LUM_KEY_F20;
        case XK_F21:            return LUM_KEY_F21;
        case XK_F22:            return LUM_KEY_F22;
        case XK_F23:            return LUM_KEY_F23;
        case XK_F24:            return LUM_KEY_F24;
        case XK_F25:            return LUM_KEY_F25;

        // Numeric keypad
        case XK_KP_Divide:      return LUM_KEY_KP_DIVIDE;
        case XK_KP_Multiply:    return LUM_KEY_KP_MULTIPLY;
        case XK_KP_Subtract:    return LUM_KEY_KP_SUBTRACT;
        case XK_KP_Add:         return LUM_KEY_KP_ADD;

        // These should have been detected in secondary keysym test above!
        case XK_KP_Insert:      return LUM_KEY_KP_0;
        case XK_KP_End:         return LUM_KEY_KP_1;
        case XK_KP_Down:        return LUM_KEY_KP_2;
        case XK_KP_Page_Down:   return LUM_KEY_KP_3;
        case XK_KP_Left:        return LUM_KEY_KP_4;
        case XK_KP_Right:       return LUM_KEY_KP_6;
        case XK_KP_Home:        return LUM_KEY_KP_7;
        case XK_KP_Up:          return LUM_KEY_KP_8;
        case XK_KP_Page_Up:     return LUM_KEY_KP_9;
        case XK_KP_Delete:      return LUM_KEY_KP_DECIMAL;
        case XK_KP_Equal:       return LUM_KEY_KP_EQUAL;
        case XK_KP_Enter:       return LUM_KEY_KP_ENTER;

        // Last resort: Check for printable keys (should not happen if the XKB
        // extension is available). This will give a layout dependent mapping
        // (which is wrong, and we may miss some keys, especially on non-US
        // keyboards), but it's better than nothing...
        case XK_a:              return LUM_KEY_A;
        case XK_b:              return LUM_KEY_B;
        case XK_c:              return LUM_KEY_C;
        case XK_d:              return LUM_KEY_D;
        case XK_e:              return LUM_KEY_E;
        case XK_f:              return LUM_KEY_F;
        case XK_g:              return LUM_KEY_G;
        case XK_h:              return LUM_KEY_H;
        case XK_i:              return LUM_KEY_I;
        case XK_j:              return LUM_KEY_J;
        case XK_k:              return LUM_KEY_K;
        case XK_l:              return LUM_KEY_L;
        case XK_m:              return LUM_KEY_M;
        case XK_n:              return LUM_KEY_N;
        case XK_o:              return LUM_KEY_O;
        case XK_p:              return LUM_KEY_P;
        case XK_q:              return LUM_KEY_Q;
        case XK_r:              return LUM_KEY_R;
        case XK_s:              return LUM_KEY_S;
        case XK_t:              return LUM_KEY_T;
        case XK_u:              return LUM_KEY_U;
        case XK_v:              return LUM_KEY_V;
        case XK_w:              return LUM_KEY_W;
        case XK_x:              return LUM_KEY_X;
        case XK_y:              return LUM_KEY_Y;
        case XK_z:              return LUM_KEY_Z;
        case XK_1:              return LUM_KEY_1;
        case XK_2:              return LUM_KEY_2;
        case XK_3:              return LUM_KEY_3;
        case XK_4:              return LUM_KEY_4;
        case XK_5:              return LUM_KEY_5;
        case XK_6:              return LUM_KEY_6;
        case XK_7:              return LUM_KEY_7;
        case XK_8:              return LUM_KEY_8;
        case XK_9:              return LUM_KEY_9;
        case XK_0:              return LUM_KEY_0;
        case XK_space:          return LUM_KEY_SPACE;
        case XK_minus:          return LUM_KEY_MINUS;
        case XK_equal:          return LUM_KEY_EQUAL;
        case XK_bracketleft:    return LUM_KEY_LEFT_BRACKET;
        case XK_bracketright:   return LUM_KEY_RIGHT_BRACKET;
        case XK_backslash:      return LUM_KEY_BACKSLASH;
        case XK_semicolon:      return LUM_KEY_SEMICOLON;
        case XK_apostrophe:     return LUM_KEY_APOSTROPHE;
        case XK_grave:          return LUM_KEY_GRAVE_ACCENT;
        case XK_comma:          return LUM_KEY_COMMA;
        case XK_period:         return LUM_KEY_PERIOD;
        case XK_slash:          return LUM_KEY_SLASH;
        case XK_less:           return LUM_KEY_WORLD_1; // At least in some layouts...
        default:                break;
    }

    // No matching translation was found
    return LUM_KEY_UNKNOWN;
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancodeMin, scancodeMax;

    memset(_lum.x11.keycodes, -1, sizeof(_lum.x11.keycodes));
    memset(_lum.x11.scancodes, -1, sizeof(_lum.x11.scancodes));

    if (_lum.x11.xkb.available)
    {
        // Use XKB to determine physical key locations independently of the
        // current keyboard layout

        XkbDescPtr desc = XkbGetMap(_lum.x11.display, 0, XkbUseCoreKbd);
        XkbGetNames(_lum.x11.display, XkbKeyNamesMask | XkbKeyAliasesMask, desc);

        scancodeMin = desc->min_key_code;
        scancodeMax = desc->max_key_code;

        const struct
        {
            int key;
            char* name;
        } keymap[] =
        {
            { LUM_KEY_GRAVE_ACCENT, "TLDE" },
            { LUM_KEY_1, "AE01" },
            { LUM_KEY_2, "AE02" },
            { LUM_KEY_3, "AE03" },
            { LUM_KEY_4, "AE04" },
            { LUM_KEY_5, "AE05" },
            { LUM_KEY_6, "AE06" },
            { LUM_KEY_7, "AE07" },
            { LUM_KEY_8, "AE08" },
            { LUM_KEY_9, "AE09" },
            { LUM_KEY_0, "AE10" },
            { LUM_KEY_MINUS, "AE11" },
            { LUM_KEY_EQUAL, "AE12" },
            { LUM_KEY_Q, "AD01" },
            { LUM_KEY_W, "AD02" },
            { LUM_KEY_E, "AD03" },
            { LUM_KEY_R, "AD04" },
            { LUM_KEY_T, "AD05" },
            { LUM_KEY_Y, "AD06" },
            { LUM_KEY_U, "AD07" },
            { LUM_KEY_I, "AD08" },
            { LUM_KEY_O, "AD09" },
            { LUM_KEY_P, "AD10" },
            { LUM_KEY_LEFT_BRACKET, "AD11" },
            { LUM_KEY_RIGHT_BRACKET, "AD12" },
            { LUM_KEY_A, "AC01" },
            { LUM_KEY_S, "AC02" },
            { LUM_KEY_D, "AC03" },
            { LUM_KEY_F, "AC04" },
            { LUM_KEY_G, "AC05" },
            { LUM_KEY_H, "AC06" },
            { LUM_KEY_J, "AC07" },
            { LUM_KEY_K, "AC08" },
            { LUM_KEY_L, "AC09" },
            { LUM_KEY_SEMICOLON, "AC10" },
            { LUM_KEY_APOSTROPHE, "AC11" },
            { LUM_KEY_Z, "AB01" },
            { LUM_KEY_X, "AB02" },
            { LUM_KEY_C, "AB03" },
            { LUM_KEY_V, "AB04" },
            { LUM_KEY_B, "AB05" },
            { LUM_KEY_N, "AB06" },
            { LUM_KEY_M, "AB07" },
            { LUM_KEY_COMMA, "AB08" },
            { LUM_KEY_PERIOD, "AB09" },
            { LUM_KEY_SLASH, "AB10" },
            { LUM_KEY_BACKSLASH, "BKSL" },
            { LUM_KEY_WORLD_1, "LSGT" },
            { LUM_KEY_SPACE, "SPCE" },
            { LUM_KEY_ESCAPE, "ESC" },
            { LUM_KEY_ENTER, "RTRN" },
            { LUM_KEY_TAB, "TAB" },
            { LUM_KEY_BACKSPACE, "BKSP" },
            { LUM_KEY_INSERT, "INS" },
            { LUM_KEY_DELETE, "DELE" },
            { LUM_KEY_RIGHT, "RGHT" },
            { LUM_KEY_LEFT, "LEFT" },
            { LUM_KEY_DOWN, "DOWN" },
            { LUM_KEY_UP, "UP" },
            { LUM_KEY_PAGE_UP, "PGUP" },
            { LUM_KEY_PAGE_DOWN, "PGDN" },
            { LUM_KEY_HOME, "HOME" },
            { LUM_KEY_END, "END" },
            { LUM_KEY_CAPS_LOCK, "CAPS" },
            { LUM_KEY_SCROLL_LOCK, "SCLK" },
            { LUM_KEY_NUM_LOCK, "NMLK" },
            { LUM_KEY_PRINT_SCREEN, "PRSC" },
            { LUM_KEY_PAUSE, "PAUS" },
            { LUM_KEY_F1, "FK01" },
            { LUM_KEY_F2, "FK02" },
            { LUM_KEY_F3, "FK03" },
            { LUM_KEY_F4, "FK04" },
            { LUM_KEY_F5, "FK05" },
            { LUM_KEY_F6, "FK06" },
            { LUM_KEY_F7, "FK07" },
            { LUM_KEY_F8, "FK08" },
            { LUM_KEY_F9, "FK09" },
            { LUM_KEY_F10, "FK10" },
            { LUM_KEY_F11, "FK11" },
            { LUM_KEY_F12, "FK12" },
            { LUM_KEY_F13, "FK13" },
            { LUM_KEY_F14, "FK14" },
            { LUM_KEY_F15, "FK15" },
            { LUM_KEY_F16, "FK16" },
            { LUM_KEY_F17, "FK17" },
            { LUM_KEY_F18, "FK18" },
            { LUM_KEY_F19, "FK19" },
            { LUM_KEY_F20, "FK20" },
            { LUM_KEY_F21, "FK21" },
            { LUM_KEY_F22, "FK22" },
            { LUM_KEY_F23, "FK23" },
            { LUM_KEY_F24, "FK24" },
            { LUM_KEY_F25, "FK25" },
            { LUM_KEY_KP_0, "KP0" },
            { LUM_KEY_KP_1, "KP1" },
            { LUM_KEY_KP_2, "KP2" },
            { LUM_KEY_KP_3, "KP3" },
            { LUM_KEY_KP_4, "KP4" },
            { LUM_KEY_KP_5, "KP5" },
            { LUM_KEY_KP_6, "KP6" },
            { LUM_KEY_KP_7, "KP7" },
            { LUM_KEY_KP_8, "KP8" },
            { LUM_KEY_KP_9, "KP9" },
            { LUM_KEY_KP_DECIMAL, "KPDL" },
            { LUM_KEY_KP_DIVIDE, "KPDV" },
            { LUM_KEY_KP_MULTIPLY, "KPMU" },
            { LUM_KEY_KP_SUBTRACT, "KPSU" },
            { LUM_KEY_KP_ADD, "KPAD" },
            { LUM_KEY_KP_ENTER, "KPEN" },
            { LUM_KEY_KP_EQUAL, "KPEQ" },
            { LUM_KEY_LEFT_SHIFT, "LFSH" },
            { LUM_KEY_LEFT_CONTROL, "LCTL" },
            { LUM_KEY_LEFT_ALT, "LALT" },
            { LUM_KEY_LEFT_SUPER, "LWIN" },
            { LUM_KEY_RIGHT_SHIFT, "RTSH" },
            { LUM_KEY_RIGHT_CONTROL, "RCTL" },
            { LUM_KEY_RIGHT_ALT, "RALT" },
            { LUM_KEY_RIGHT_ALT, "LVL3" },
            { LUM_KEY_RIGHT_ALT, "MDSW" },
            { LUM_KEY_RIGHT_SUPER, "RWIN" },
            { LUM_KEY_MENU, "MENU" }
        };

        // Find the X11 key code -> Lumina key code mapping
        for (int scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
        {
            int key = LUM_KEY_UNKNOWN;

            // Map the key name to a Lumina key code. Note: We use the US
            // keyboard layout. Because function keys aren't mapped correctly
            // when using traditional KeySym translations, they are mapped
            // here instead.
            for (int i = 0;  i < sizeof(keymap) / sizeof(keymap[0]);  i++)
            {
                if (strncmp(desc->names->keys[scancode].name,
                            keymap[i].name,
                            XkbKeyNameLength) == 0)
                {
                    key = keymap[i].key;
                    break;
                }
            }

            // Fall back to key aliases in case the key name did not match
            for (int i = 0;  i < desc->names->num_key_aliases;  i++)
            {
                if (key != LUM_KEY_UNKNOWN)
                    break;

                if (strncmp(desc->names->key_aliases[i].real,
                            desc->names->keys[scancode].name,
                            XkbKeyNameLength) != 0)
                {
                    continue;
                }

                for (int j = 0;  j < sizeof(keymap) / sizeof(keymap[0]);  j++)
                {
                    if (strncmp(desc->names->key_aliases[i].alias,
                                keymap[j].name,
                                XkbKeyNameLength) == 0)
                    {
                        key = keymap[j].key;
                        break;
                    }
                }
            }

            _lum.x11.keycodes[scancode] = key;
        }

        XkbFreeNames(desc, XkbKeyNamesMask, True);
        XkbFreeKeyboard(desc, 0, True);
    }
    else
        XDisplayKeycodes(_lum.x11.display, &scancodeMin, &scancodeMax);

    int width;
    KeySym* keysyms = XGetKeyboardMapping(_lum.x11.display,
                                          scancodeMin,
                                          scancodeMax - scancodeMin + 1,
                                          &width);

    for (int scancode = scancodeMin;  scancode <= scancodeMax;  scancode++)
    {
        // Translate the un-translated key codes using traditional X11 KeySym
        // lookups
        if (_lum.x11.keycodes[scancode] < 0)
        {
            const size_t base = (scancode - scancodeMin) * width;
            _lum.x11.keycodes[scancode] = translateKeySyms(&keysyms[base], width);
        }

        // Store the reverse translation for faster key name lookup
        if (_lum.x11.keycodes[scancode] > 0)
            _lum.x11.scancodes[_lum.x11.keycodes[scancode]] = scancode;
    }

    XFree(keysyms);
}

// Check whether the IM has a usable style
//
static LUMbool hasUsableInputMethodStyle(void)
{
    LUMbool found = LUM_FALSE;
    XIMStyles* styles = NULL;

    if (XGetIMValues(_lum.x11.im, XNQueryInputStyle, &styles, NULL) != NULL)
        return LUM_FALSE;

    for (unsigned int i = 0;  i < styles->count_styles;  i++)
    {
        if (styles->supported_styles[i] == (XIMPreeditNothing | XIMStatusNothing))
        {
            found = LUM_TRUE;
            break;
        }
    }

    XFree(styles);
    return found;
}

static void inputMethodDestroyCallback(XIM im, XPointer clientData, XPointer callData)
{
    _lum.x11.im = NULL;
}

static void inputMethodInstantiateCallback(Display* display,
                                           XPointer clientData,
                                           XPointer callData)
{
    if (_lum.x11.im)
        return;

    _lum.x11.im = XOpenIM(_lum.x11.display, 0, NULL, NULL);
    if (_lum.x11.im)
    {
        if (!hasUsableInputMethodStyle())
        {
            XCloseIM(_lum.x11.im);
            _lum.x11.im = NULL;
        }
    }

    if (_lum.x11.im)
    {
        XIMCallback callback;
        callback.callback = (XIMProc) inputMethodDestroyCallback;
        callback.client_data = NULL;
        XSetIMValues(_lum.x11.im, XNDestroyCallback, &callback, NULL);

        for (_LUMwindow* window = _lum.windowListHead;  window;  window = window->next)
            _lumCreateInputContextX11(window);
    }
}

// Return the atom ID only if it is listed in the specified array
//
static Atom getAtomIfSupported(Atom* supportedAtoms,
                               unsigned long atomCount,
                               const char* atomName)
{
    const Atom atom = XInternAtom(_lum.x11.display, atomName, False);

    for (unsigned long i = 0;  i < atomCount;  i++)
    {
        if (supportedAtoms[i] == atom)
            return atom;
    }

    return None;
}

// Check whether the running window manager is EWMH-compliant
//
static void detectEWMH(void)
{
    // First we read the _NET_SUPPORTING_WM_CHECK property on the root window

    Window* windowFromRoot = NULL;
    if (!_lumGetWindowPropertyX11(_lum.x11.root,
                                   _lum.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromRoot))
    {
        return;
    }

    _lumGrabErrorHandlerX11();

    // If it exists, it should be the XID of a top-level window
    // Then we look for the same property on that window

    Window* windowFromChild = NULL;
    if (!_lumGetWindowPropertyX11(*windowFromRoot,
                                   _lum.x11.NET_SUPPORTING_WM_CHECK,
                                   XA_WINDOW,
                                   (unsigned char**) &windowFromChild))
    {
        XFree(windowFromRoot);
        return;
    }

    _lumReleaseErrorHandlerX11();

    // If the property exists, it should contain the XID of the window

    if (*windowFromRoot != *windowFromChild)
    {
        XFree(windowFromRoot);
        XFree(windowFromChild);
        return;
    }

    XFree(windowFromRoot);
    XFree(windowFromChild);

    // We are now fairly sure that an EWMH-compliant WM is currently running
    // We can now start querying the WM about what features it supports by
    // looking in the _NET_SUPPORTED property on the root window
    // It should contain a list of supported EWMH protocol and state atoms

    Atom* supportedAtoms = NULL;
    const unsigned long atomCount =
        _lumGetWindowPropertyX11(_lum.x11.root,
                                  _lum.x11.NET_SUPPORTED,
                                  XA_ATOM,
                                  (unsigned char**) &supportedAtoms);

    // See which of the atoms we support that are supported by the WM

    _lum.x11.NET_WM_STATE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE");
    _lum.x11.NET_WM_STATE_ABOVE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_ABOVE");
    _lum.x11.NET_WM_STATE_FULLSCREEN =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_FULLSCREEN");
    _lum.x11.NET_WM_STATE_MAXIMIZED_VERT =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_VERT");
    _lum.x11.NET_WM_STATE_MAXIMIZED_HORZ =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_MAXIMIZED_HORZ");
    _lum.x11.NET_WM_STATE_DEMANDS_ATTENTION =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_STATE_DEMANDS_ATTENTION");
    _lum.x11.NET_WM_FULLSCREEN_MONITORS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_FULLSCREEN_MONITORS");
    _lum.x11.NET_WM_WINDOW_TYPE =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE");
    _lum.x11.NET_WM_WINDOW_TYPE_NORMAL =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WM_WINDOW_TYPE_NORMAL");
    _lum.x11.NET_WORKAREA =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_WORKAREA");
    _lum.x11.NET_CURRENT_DESKTOP =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_CURRENT_DESKTOP");
    _lum.x11.NET_ACTIVE_WINDOW =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_ACTIVE_WINDOW");
    _lum.x11.NET_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_FRAME_EXTENTS");
    _lum.x11.NET_REQUEST_FRAME_EXTENTS =
        getAtomIfSupported(supportedAtoms, atomCount, "_NET_REQUEST_FRAME_EXTENTS");

    if (supportedAtoms)
        XFree(supportedAtoms);
}

// Look for and initialize supported X11 extensions
//
static LUMbool initExtensions(void)
{
#if defined(__OpenBSD__)
    _lum.x11.vidmode.handle = _lumPlatformLoadModule("libXxf86vm.so");
#else
    _lum.x11.vidmode.handle = _lumPlatformLoadModule("libXxf86vm.so.1");
#endif
    if (_lum.x11.vidmode.handle)
    {
        _lum.x11.vidmode.QueryExtension = (PFN_XF86VidModeQueryExtension)
            _lumPlatformGetModuleSymbol(_lum.x11.vidmode.handle, "XF86VidModeQueryExtension");
        _lum.x11.vidmode.GetGammaRamp = (PFN_XF86VidModeGetGammaRamp)
            _lumPlatformGetModuleSymbol(_lum.x11.vidmode.handle, "XF86VidModeGetGammaRamp");
        _lum.x11.vidmode.SetGammaRamp = (PFN_XF86VidModeSetGammaRamp)
            _lumPlatformGetModuleSymbol(_lum.x11.vidmode.handle, "XF86VidModeSetGammaRamp");
        _lum.x11.vidmode.GetGammaRampSize = (PFN_XF86VidModeGetGammaRampSize)
            _lumPlatformGetModuleSymbol(_lum.x11.vidmode.handle, "XF86VidModeGetGammaRampSize");

        _lum.x11.vidmode.available =
            XF86VidModeQueryExtension(_lum.x11.display,
                                      &_lum.x11.vidmode.eventBase,
                                      &_lum.x11.vidmode.errorBase);
    }

#if defined(__CYGWIN__)
    _lum.x11.xi.handle = _lumPlatformLoadModule("libXi-6.so");
#elif defined(__OpenBSD__)
    _lum.x11.xi.handle = _lumPlatformLoadModule("libXi.so");
#else
    _lum.x11.xi.handle = _lumPlatformLoadModule("libXi.so.6");
#endif
    if (_lum.x11.xi.handle)
    {
        _lum.x11.xi.QueryVersion = (PFN_XIQueryVersion)
            _lumPlatformGetModuleSymbol(_lum.x11.xi.handle, "XIQueryVersion");
        _lum.x11.xi.SelectEvents = (PFN_XISelectEvents)
            _lumPlatformGetModuleSymbol(_lum.x11.xi.handle, "XISelectEvents");

        if (XQueryExtension(_lum.x11.display,
                            "XInputExtension",
                            &_lum.x11.xi.majorOpcode,
                            &_lum.x11.xi.eventBase,
                            &_lum.x11.xi.errorBase))
        {
            _lum.x11.xi.major = 2;
            _lum.x11.xi.minor = 0;

            if (XIQueryVersion(_lum.x11.display,
                               &_lum.x11.xi.major,
                               &_lum.x11.xi.minor) == Success)
            {
                _lum.x11.xi.available = LUM_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _lum.x11.randr.handle = _lumPlatformLoadModule("libXrandr-2.so");
#elif defined(__OpenBSD__)
    _lum.x11.randr.handle = _lumPlatformLoadModule("libXrandr.so");
#else
    _lum.x11.randr.handle = _lumPlatformLoadModule("libXrandr.so.2");
#endif
    if (_lum.x11.randr.handle)
    {
        _lum.x11.randr.AllocGamma = (PFN_XRRAllocGamma)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRAllocGamma");
        _lum.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRFreeGamma");
        _lum.x11.randr.FreeCrtcInfo = (PFN_XRRFreeCrtcInfo)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRFreeCrtcInfo");
        _lum.x11.randr.FreeGamma = (PFN_XRRFreeGamma)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRFreeGamma");
        _lum.x11.randr.FreeOutputInfo = (PFN_XRRFreeOutputInfo)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRFreeOutputInfo");
        _lum.x11.randr.FreeScreenResources = (PFN_XRRFreeScreenResources)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRFreeScreenResources");
        _lum.x11.randr.GetCrtcGamma = (PFN_XRRGetCrtcGamma)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetCrtcGamma");
        _lum.x11.randr.GetCrtcGammaSize = (PFN_XRRGetCrtcGammaSize)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetCrtcGammaSize");
        _lum.x11.randr.GetCrtcInfo = (PFN_XRRGetCrtcInfo)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetCrtcInfo");
        _lum.x11.randr.GetOutputInfo = (PFN_XRRGetOutputInfo)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetOutputInfo");
        _lum.x11.randr.GetOutputPrimary = (PFN_XRRGetOutputPrimary)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetOutputPrimary");
        _lum.x11.randr.GetScreenResourcesCurrent = (PFN_XRRGetScreenResourcesCurrent)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRGetScreenResourcesCurrent");
        _lum.x11.randr.QueryExtension = (PFN_XRRQueryExtension)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRQueryExtension");
        _lum.x11.randr.QueryVersion = (PFN_XRRQueryVersion)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRQueryVersion");
        _lum.x11.randr.SelectInput = (PFN_XRRSelectInput)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRSelectInput");
        _lum.x11.randr.SetCrtcConfig = (PFN_XRRSetCrtcConfig)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRSetCrtcConfig");
        _lum.x11.randr.SetCrtcGamma = (PFN_XRRSetCrtcGamma)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRSetCrtcGamma");
        _lum.x11.randr.UpdateConfiguration = (PFN_XRRUpdateConfiguration)
            _lumPlatformGetModuleSymbol(_lum.x11.randr.handle, "XRRUpdateConfiguration");

        if (XRRQueryExtension(_lum.x11.display,
                              &_lum.x11.randr.eventBase,
                              &_lum.x11.randr.errorBase))
        {
            if (XRRQueryVersion(_lum.x11.display,
                                &_lum.x11.randr.major,
                                &_lum.x11.randr.minor))
            {
                // The Lumina RandR path requires at least version 1.3
                if (_lum.x11.randr.major > 1 || _lum.x11.randr.minor >= 3)
                    _lum.x11.randr.available = LUM_TRUE;
            }
            else
            {
                _lumInputError(LUM_PLATFORM_ERROR,
                                "X11: Failed to query RandR version");
            }
        }
    }

    if (_lum.x11.randr.available)
    {
        XRRScreenResources* sr = XRRGetScreenResourcesCurrent(_lum.x11.display,
                                                              _lum.x11.root);

        if (!sr->ncrtc || !XRRGetCrtcGammaSize(_lum.x11.display, sr->crtcs[0]))
        {
            // This is likely an older Nvidia driver with broken gamma support
            // Flag it as useless and fall back to xf86vm gamma, if available
            _lum.x11.randr.gammaBroken = LUM_TRUE;
        }

        if (!sr->ncrtc)
        {
            // A system without CRTCs is likely a system with broken RandR
            // Disable the RandR monitor path and fall back to core functions
            _lum.x11.randr.monitorBroken = LUM_TRUE;
        }

        XRRFreeScreenResources(sr);
    }

    if (_lum.x11.randr.available && !_lum.x11.randr.monitorBroken)
    {
        XRRSelectInput(_lum.x11.display, _lum.x11.root,
                       RROutputChangeNotifyMask);
    }

#if defined(__CYGWIN__)
    _lum.x11.xcursor.handle = _lumPlatformLoadModule("libXcursor-1.so");
#elif defined(__OpenBSD__)
    _lum.x11.xcursor.handle = _lumPlatformLoadModule("libXcursor.so");
#else
    _lum.x11.xcursor.handle = _lumPlatformLoadModule("libXcursor.so.1");
#endif
    if (_lum.x11.xcursor.handle)
    {
        _lum.x11.xcursor.ImageCreate = (PFN_XcursorImageCreate)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorImageCreate");
        _lum.x11.xcursor.ImageDestroy = (PFN_XcursorImageDestroy)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorImageDestroy");
        _lum.x11.xcursor.ImageLoadCursor = (PFN_XcursorImageLoadCursor)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorImageLoadCursor");
        _lum.x11.xcursor.GetTheme = (PFN_XcursorGetTheme)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorGetTheme");
        _lum.x11.xcursor.GetDefaultSize = (PFN_XcursorGetDefaultSize)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorGetDefaultSize");
        _lum.x11.xcursor.LibraryLoadImage = (PFN_XcursorLibraryLoadImage)
            _lumPlatformGetModuleSymbol(_lum.x11.xcursor.handle, "XcursorLibraryLoadImage");
    }

#if defined(__CYGWIN__)
    _lum.x11.xinerama.handle = _lumPlatformLoadModule("libXinerama-1.so");
#elif defined(__OpenBSD__)
    _lum.x11.xinerama.handle = _lumPlatformLoadModule("libXinerama.so");
#else
    _lum.x11.xinerama.handle = _lumPlatformLoadModule("libXinerama.so.1");
#endif
    if (_lum.x11.xinerama.handle)
    {
        _lum.x11.xinerama.IsActive = (PFN_XineramaIsActive)
            _lumPlatformGetModuleSymbol(_lum.x11.xinerama.handle, "XineramaIsActive");
        _lum.x11.xinerama.QueryExtension = (PFN_XineramaQueryExtension)
            _lumPlatformGetModuleSymbol(_lum.x11.xinerama.handle, "XineramaQueryExtension");
        _lum.x11.xinerama.QueryScreens = (PFN_XineramaQueryScreens)
            _lumPlatformGetModuleSymbol(_lum.x11.xinerama.handle, "XineramaQueryScreens");

        if (XineramaQueryExtension(_lum.x11.display,
                                   &_lum.x11.xinerama.major,
                                   &_lum.x11.xinerama.minor))
        {
            if (XineramaIsActive(_lum.x11.display))
                _lum.x11.xinerama.available = LUM_TRUE;
        }
    }

    _lum.x11.xkb.major = 1;
    _lum.x11.xkb.minor = 0;
    _lum.x11.xkb.available =
        XkbQueryExtension(_lum.x11.display,
                          &_lum.x11.xkb.majorOpcode,
                          &_lum.x11.xkb.eventBase,
                          &_lum.x11.xkb.errorBase,
                          &_lum.x11.xkb.major,
                          &_lum.x11.xkb.minor);

    if (_lum.x11.xkb.available)
    {
        Bool supported;

        if (XkbSetDetectableAutoRepeat(_lum.x11.display, True, &supported))
        {
            if (supported)
                _lum.x11.xkb.detectable = LUM_TRUE;
        }

        XkbStateRec state;
        if (XkbGetState(_lum.x11.display, XkbUseCoreKbd, &state) == Success)
            _lum.x11.xkb.group = (unsigned int)state.group;

        XkbSelectEventDetails(_lum.x11.display, XkbUseCoreKbd, XkbStateNotify,
                              XkbGroupStateMask, XkbGroupStateMask);
    }

    if (_lum.hints.init.x11.xcbVulkanSurface)
    {
#if defined(__CYGWIN__)
        _lum.x11.x11xcb.handle = _lumPlatformLoadModule("libX11-xcb-1.so");
#elif defined(__OpenBSD__)
        _lum.x11.x11xcb.handle = _lumPlatformLoadModule("libX11-xcb.so");
#else
        _lum.x11.x11xcb.handle = _lumPlatformLoadModule("libX11-xcb.so.1");
#endif
    }

    if (_lum.x11.x11xcb.handle)
    {
        _lum.x11.x11xcb.GetXCBConnection = (PFN_XGetXCBConnection)
            _lumPlatformGetModuleSymbol(_lum.x11.x11xcb.handle, "XGetXCBConnection");
    }

#if defined(__CYGWIN__)
    _lum.x11.xrender.handle = _lumPlatformLoadModule("libXrender-1.so");
#elif defined(__OpenBSD__)
    _lum.x11.xrender.handle = _lumPlatformLoadModule("libXrender.so");
#else
    _lum.x11.xrender.handle = _lumPlatformLoadModule("libXrender.so.1");
#endif
    if (_lum.x11.xrender.handle)
    {
        _lum.x11.xrender.QueryExtension = (PFN_XRenderQueryExtension)
            _lumPlatformGetModuleSymbol(_lum.x11.xrender.handle, "XRenderQueryExtension");
        _lum.x11.xrender.QueryVersion = (PFN_XRenderQueryVersion)
            _lumPlatformGetModuleSymbol(_lum.x11.xrender.handle, "XRenderQueryVersion");
        _lum.x11.xrender.FindVisualFormat = (PFN_XRenderFindVisualFormat)
            _lumPlatformGetModuleSymbol(_lum.x11.xrender.handle, "XRenderFindVisualFormat");

        if (XRenderQueryExtension(_lum.x11.display,
                                  &_lum.x11.xrender.errorBase,
                                  &_lum.x11.xrender.eventBase))
        {
            if (XRenderQueryVersion(_lum.x11.display,
                                    &_lum.x11.xrender.major,
                                    &_lum.x11.xrender.minor))
            {
                _lum.x11.xrender.available = LUM_TRUE;
            }
        }
    }

#if defined(__CYGWIN__)
    _lum.x11.xshape.handle = _lumPlatformLoadModule("libXext-6.so");
#elif defined(__OpenBSD__)
    _lum.x11.xshape.handle = _lumPlatformLoadModule("libXext.so");
#else
    _lum.x11.xshape.handle = _lumPlatformLoadModule("libXext.so.6");
#endif
    if (_lum.x11.xshape.handle)
    {
        _lum.x11.xshape.QueryExtension = (PFN_XShapeQueryExtension)
            _lumPlatformGetModuleSymbol(_lum.x11.xshape.handle, "XShapeQueryExtension");
        _lum.x11.xshape.ShapeCombineRegion = (PFN_XShapeCombineRegion)
            _lumPlatformGetModuleSymbol(_lum.x11.xshape.handle, "XShapeCombineRegion");
        _lum.x11.xshape.QueryVersion = (PFN_XShapeQueryVersion)
            _lumPlatformGetModuleSymbol(_lum.x11.xshape.handle, "XShapeQueryVersion");
        _lum.x11.xshape.ShapeCombineMask = (PFN_XShapeCombineMask)
            _lumPlatformGetModuleSymbol(_lum.x11.xshape.handle, "XShapeCombineMask");

        if (XShapeQueryExtension(_lum.x11.display,
            &_lum.x11.xshape.errorBase,
            &_lum.x11.xshape.eventBase))
        {
            if (XShapeQueryVersion(_lum.x11.display,
                &_lum.x11.xshape.major,
                &_lum.x11.xshape.minor))
            {
                _lum.x11.xshape.available = LUM_TRUE;
            }
        }
    }

    // Update the key code LUT
    // FIXME: We should listen to XkbMapNotify events to track changes to
    // the keyboard mapping.
    createKeyTables();

    // String format atoms
    _lum.x11.NULL_ = XInternAtom(_lum.x11.display, "NULL", False);
    _lum.x11.UTF8_STRING = XInternAtom(_lum.x11.display, "UTF8_STRING", False);
    _lum.x11.ATOM_PAIR = XInternAtom(_lum.x11.display, "ATOM_PAIR", False);

    // Custom selection property atom
    _lum.x11.LUM_SELECTION =
        XInternAtom(_lum.x11.display, "LUM_SELECTION", False);

    // ICCCM standard clipboard atoms
    _lum.x11.TARGETS = XInternAtom(_lum.x11.display, "TARGETS", False);
    _lum.x11.MULTIPLE = XInternAtom(_lum.x11.display, "MULTIPLE", False);
    _lum.x11.PRIMARY = XInternAtom(_lum.x11.display, "PRIMARY", False);
    _lum.x11.INCR = XInternAtom(_lum.x11.display, "INCR", False);
    _lum.x11.CLIPBOARD = XInternAtom(_lum.x11.display, "CLIPBOARD", False);

    // Clipboard manager atoms
    _lum.x11.CLIPBOARD_MANAGER =
        XInternAtom(_lum.x11.display, "CLIPBOARD_MANAGER", False);
    _lum.x11.SAVE_TARGETS =
        XInternAtom(_lum.x11.display, "SAVE_TARGETS", False);

    // Xdnd (drag and drop) atoms
    _lum.x11.XdndAware = XInternAtom(_lum.x11.display, "XdndAware", False);
    _lum.x11.XdndEnter = XInternAtom(_lum.x11.display, "XdndEnter", False);
    _lum.x11.XdndPosition = XInternAtom(_lum.x11.display, "XdndPosition", False);
    _lum.x11.XdndStatus = XInternAtom(_lum.x11.display, "XdndStatus", False);
    _lum.x11.XdndActionCopy = XInternAtom(_lum.x11.display, "XdndActionCopy", False);
    _lum.x11.XdndDrop = XInternAtom(_lum.x11.display, "XdndDrop", False);
    _lum.x11.XdndFinished = XInternAtom(_lum.x11.display, "XdndFinished", False);
    _lum.x11.XdndSelection = XInternAtom(_lum.x11.display, "XdndSelection", False);
    _lum.x11.XdndTypeList = XInternAtom(_lum.x11.display, "XdndTypeList", False);
    _lum.x11.text_uri_list = XInternAtom(_lum.x11.display, "text/uri-list", False);

    // ICCCM, EWMH and Motif window property atoms
    // These can be set safely even without WM support
    // The EWMH atoms that require WM support are handled in detectEWMH
    _lum.x11.WM_PROTOCOLS =
        XInternAtom(_lum.x11.display, "WM_PROTOCOLS", False);
    _lum.x11.WM_STATE =
        XInternAtom(_lum.x11.display, "WM_STATE", False);
    _lum.x11.WM_DELETE_WINDOW =
        XInternAtom(_lum.x11.display, "WM_DELETE_WINDOW", False);
    _lum.x11.NET_SUPPORTED =
        XInternAtom(_lum.x11.display, "_NET_SUPPORTED", False);
    _lum.x11.NET_SUPPORTING_WM_CHECK =
        XInternAtom(_lum.x11.display, "_NET_SUPPORTING_WM_CHECK", False);
    _lum.x11.NET_WM_ICON =
        XInternAtom(_lum.x11.display, "_NET_WM_ICON", False);
    _lum.x11.NET_WM_PING =
        XInternAtom(_lum.x11.display, "_NET_WM_PING", False);
    _lum.x11.NET_WM_PID =
        XInternAtom(_lum.x11.display, "_NET_WM_PID", False);
    _lum.x11.NET_WM_NAME =
        XInternAtom(_lum.x11.display, "_NET_WM_NAME", False);
    _lum.x11.NET_WM_ICON_NAME =
        XInternAtom(_lum.x11.display, "_NET_WM_ICON_NAME", False);
    _lum.x11.NET_WM_BYPASS_COMPOSITOR =
        XInternAtom(_lum.x11.display, "_NET_WM_BYPASS_COMPOSITOR", False);
    _lum.x11.NET_WM_WINDOW_OPACITY =
        XInternAtom(_lum.x11.display, "_NET_WM_WINDOW_OPACITY", False);
    _lum.x11.MOTIF_WM_HINTS =
        XInternAtom(_lum.x11.display, "_MOTIF_WM_HINTS", False);

    // The compositing manager selection name contains the screen number
    {
        char name[32];
        snprintf(name, sizeof(name), "_NET_WM_CM_S%u", _lum.x11.screen);
        _lum.x11.NET_WM_CM_Sx = XInternAtom(_lum.x11.display, name, False);
    }

    // Detect whether an EWMH-conformant window manager is running
    detectEWMH();

    return LUM_TRUE;
}

// Retrieve system content scale via folklore heuristics
//
static void getSystemContentScale(float* xscale, float* yscale)
{
    // Start by assuming the default X11 DPI
    // NOTE: Some desktop environments (KDE) may remove the Xft.dpi field when it
    //       would be set to 96, so assume that is the case if we cannot find it
    float xdpi = 96.f, ydpi = 96.f;

    // NOTE: Basing the scale on Xft.dpi where available should provide the most
    //       consistent user experience (matches Qt, Gtk, etc), although not
    //       always the most accurate one
    char* rms = XResourceManagerString(_lum.x11.display);
    if (rms)
    {
        XrmDatabase db = XrmGetStringDatabase(rms);
        if (db)
        {
            XrmValue value;
            char* type = NULL;

            if (XrmGetResource(db, "Xft.dpi", "Xft.Dpi", &type, &value))
            {
                if (type && strcmp(type, "String") == 0)
                    xdpi = ydpi = atof(value.addr);
            }

            XrmDestroyDatabase(db);
        }
    }

    *xscale = xdpi / 96.f;
    *yscale = ydpi / 96.f;
}

// Create a blank cursor for hidden and disabled cursor modes
//
static Cursor createHiddenCursor(void)
{
    unsigned char pixels[16 * 16 * 4] = { 0 };
    LUMimage image = { 16, 16, pixels };
    return _lumCreateNativeCursorX11(&image, 0, 0);
}

// Create a helper window for IPC
//
static Window createHelperWindow(void)
{
    XSetWindowAttributes wa;
    wa.event_mask = PropertyChangeMask;

    return XCreateWindow(_lum.x11.display, _lum.x11.root,
                         0, 0, 1, 1, 0, 0,
                         InputOnly,
                         DefaultVisual(_lum.x11.display, _lum.x11.screen),
                         CWEventMask, &wa);
}

// X error handler
//
static int errorHandler(Display *display, XErrorEvent* event)
{
    if (_lum.x11.display != display)
        return 0;

    _lum.x11.errorCode = event->error_code;
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Sets the X error handler callback
//
void _lumGrabErrorHandlerX11(void)
{
    _lum.x11.errorCode = Success;
    XSetErrorHandler(errorHandler);
}

// Clears the X error handler callback
//
void _lumReleaseErrorHandlerX11(void)
{
    // Synchronize to make sure all commands are processed
    XSync(_lum.x11.display, False);
    XSetErrorHandler(NULL);
}

// Reports the specified error, appending information about the last X error
//
void _lumInputErrorX11(int error, const char* message)
{
    char buffer[_LUM_MESSAGE_SIZE];
    XGetErrorText(_lum.x11.display, _lum.x11.errorCode,
                  buffer, sizeof(buffer));

    _lumInputError(error, "%s: %s", message, buffer);
}

// Creates a native cursor object from the specified image and hotspot
//
Cursor _lumCreateNativeCursorX11(const LUMimage* image, int xhot, int yhot)
{
    Cursor cursor;

    if (!_lum.x11.xcursor.handle)
        return None;

    XcursorImage* native = XcursorImageCreate(image->width, image->height);
    if (native == NULL)
        return None;

    native->xhot = xhot;
    native->yhot = yhot;

    unsigned char* source = (unsigned char*) image->pixels;
    XcursorPixel* target = native->pixels;

    for (int i = 0;  i < image->width * image->height;  i++, target++, source += 4)
    {
        unsigned int alpha = source[3];

        *target = (alpha << 24) |
                  ((unsigned char) ((source[0] * alpha) / 255) << 16) |
                  ((unsigned char) ((source[1] * alpha) / 255) <<  8) |
                  ((unsigned char) ((source[2] * alpha) / 255) <<  0);
    }

    cursor = XcursorImageLoadCursor(_lum.x11.display, native);
    XcursorImageDestroy(native);

    return cursor;
}


//////////////////////////////////////////////////////////////////////////
//////                       Lumina platform API                      //////
//////////////////////////////////////////////////////////////////////////

LUMbool _lumConnectX11(int platformID, _LUMplatform* platform)
{
    const _LUMplatform x11 =
    {
        LUM_PLATFORM_X11,
        _lumInitX11,
        _lumTerminateX11,
        _lumGetCursorPosX11,
        _lumSetCursorPosX11,
        _lumSetCursorModeX11,
        _lumSetRawMouseMotionX11,
        _lumRawMouseMotionSupportedX11,
        _lumCreateCursorX11,
        _lumCreateStandardCursorX11,
        _lumDestroyCursorX11,
        _lumSetCursorX11,
        _lumGetScancodeNameX11,
        _lumGetKeyScancodeX11,
        _lumSetClipboardStringX11,
        _lumGetClipboardStringX11,
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
        _lumFreeMonitorX11,
        _lumGetMonitorPosX11,
        _lumGetMonitorContentScaleX11,
        _lumGetMonitorWorkareaX11,
        _lumGetVideoModesX11,
        _lumGetVideoModeX11,
        _lumGetGammaRampX11,
        _lumSetGammaRampX11,
        _lumCreateWindowX11,
        _lumDestroyWindowX11,
        _lumSetWindowTitleX11,
        _lumSetWindowIconX11,
        _lumGetWindowPosX11,
        _lumSetWindowPosX11,
        _lumGetWindowSizeX11,
        _lumSetWindowSizeX11,
        _lumSetWindowSizeLimitsX11,
        _lumSetWindowAspectRatioX11,
        _lumGetFramebufferSizeX11,
        _lumGetWindowFrameSizeX11,
        _lumGetWindowContentScaleX11,
        _lumIconifyWindowX11,
        _lumRestoreWindowX11,
        _lumMaximizeWindowX11,
        _lumShowWindowX11,
        _lumHideWindowX11,
        _lumRequestWindowAttentionX11,
        _lumFocusWindowX11,
        _lumSetWindowMonitorX11,
        _lumWindowFocusedX11,
        _lumWindowIconifiedX11,
        _lumWindowVisibleX11,
        _lumWindowMaximizedX11,
        _lumWindowHoveredX11,
        _lumFramebufferTransparentX11,
        _lumGetWindowOpacityX11,
        _lumSetWindowResizableX11,
        _lumSetWindowDecoratedX11,
        _lumSetWindowFloatingX11,
        _lumSetWindowOpacityX11,
        _lumSetWindowMousePassthroughX11,
        _lumPollEventsX11,
        _lumWaitEventsX11,
        _lumWaitEventsTimeoutX11,
        _lumPostEmptyEventX11,

        // Hazel
        _lumSetWindowTitlebarNull,

        _lumGetEGLPlatformX11,
        _lumGetEGLNativeDisplayX11,
        _lumGetEGLNativeWindowX11,
        _lumGetRequiredInstanceExtensionsX11,
        _lumGetPhysicalDevicePresentationSupportX11,
        _lumCreateWindowSurfaceX11,
    };

    // HACK: If the application has left the locale as "C" then both wide
    //       character text input and explicit UTF-8 input via XIM will break
    //       This sets the CTYPE part of the current locale from the environment
    //       in the hope that it is set to something more sane than "C"
    if (strcmp(setlocale(LC_CTYPE, NULL), "C") == 0)
        setlocale(LC_CTYPE, "");

#if defined(__CYGWIN__)
    void* module = _lumPlatformLoadModule("libX11-6.so");
#elif defined(__OpenBSD__)
    void* module = _lumPlatformLoadModule("libX11.so");
#else
    void* module = _lumPlatformLoadModule("libX11.so.6");
#endif
    if (!module)
    {
        if (platformID == LUM_PLATFORM_X11)
            _lumInputError(LUM_PLATFORM_ERROR, "X11: Failed to load Xlib");

        return LUM_FALSE;
    }

    PFN_XInitThreads XInitThreads = (PFN_XInitThreads)
        _lumPlatformGetModuleSymbol(module, "XInitThreads");
    PFN_XrmInitialize XrmInitialize = (PFN_XrmInitialize)
        _lumPlatformGetModuleSymbol(module, "XrmInitialize");
    PFN_XOpenDisplay XOpenDisplay = (PFN_XOpenDisplay)
        _lumPlatformGetModuleSymbol(module, "XOpenDisplay");
    if (!XInitThreads || !XrmInitialize || !XOpenDisplay)
    {
        if (platformID == LUM_PLATFORM_X11)
            _lumInputError(LUM_PLATFORM_ERROR, "X11: Failed to load Xlib entry point");

        _lumPlatformFreeModule(module);
        return LUM_FALSE;
    }

    XInitThreads();
    XrmInitialize();

    Display* display = XOpenDisplay(NULL);
    if (!display)
    {
        if (platformID == LUM_PLATFORM_X11)
        {
            const char* name = getenv("DISPLAY");
            if (name)
            {
                _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                                "X11: Failed to open display %s", name);
            }
            else
            {
                _lumInputError(LUM_PLATFORM_UNAVAILABLE,
                                "X11: The DISPLAY environment variable is missing");
            }
        }

        _lumPlatformFreeModule(module);
        return LUM_FALSE;
    }

    _lum.x11.display = display;
    _lum.x11.xlib.handle = module;

    *platform = x11;
    return LUM_TRUE;
}

int _lumInitX11(void)
{
    _lum.x11.xlib.AllocClassHint = (PFN_XAllocClassHint)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XAllocClassHint");
    _lum.x11.xlib.AllocSizeHints = (PFN_XAllocSizeHints)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XAllocSizeHints");
    _lum.x11.xlib.AllocWMHints = (PFN_XAllocWMHints)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XAllocWMHints");
    _lum.x11.xlib.ChangeProperty = (PFN_XChangeProperty)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XChangeProperty");
    _lum.x11.xlib.ChangeWindowAttributes = (PFN_XChangeWindowAttributes)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XChangeWindowAttributes");
    _lum.x11.xlib.CheckIfEvent = (PFN_XCheckIfEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCheckIfEvent");
    _lum.x11.xlib.CheckTypedWindowEvent = (PFN_XCheckTypedWindowEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCheckTypedWindowEvent");
    _lum.x11.xlib.CloseDisplay = (PFN_XCloseDisplay)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCloseDisplay");
    _lum.x11.xlib.CloseIM = (PFN_XCloseIM)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCloseIM");
    _lum.x11.xlib.ConvertSelection = (PFN_XConvertSelection)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XConvertSelection");
    _lum.x11.xlib.CreateColormap = (PFN_XCreateColormap)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCreateColormap");
    _lum.x11.xlib.CreateFontCursor = (PFN_XCreateFontCursor)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCreateFontCursor");
    _lum.x11.xlib.CreateIC = (PFN_XCreateIC)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCreateIC");
    _lum.x11.xlib.CreateRegion = (PFN_XCreateRegion)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCreateRegion");
    _lum.x11.xlib.CreateWindow = (PFN_XCreateWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XCreateWindow");
    _lum.x11.xlib.DefineCursor = (PFN_XDefineCursor)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDefineCursor");
    _lum.x11.xlib.DeleteContext = (PFN_XDeleteContext)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDeleteContext");
    _lum.x11.xlib.DeleteProperty = (PFN_XDeleteProperty)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDeleteProperty");
    _lum.x11.xlib.DestroyIC = (PFN_XDestroyIC)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDestroyIC");
    _lum.x11.xlib.DestroyRegion = (PFN_XDestroyRegion)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDestroyRegion");
    _lum.x11.xlib.DestroyWindow = (PFN_XDestroyWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDestroyWindow");
    _lum.x11.xlib.DisplayKeycodes = (PFN_XDisplayKeycodes)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XDisplayKeycodes");
    _lum.x11.xlib.EventsQueued = (PFN_XEventsQueued)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XEventsQueued");
    _lum.x11.xlib.FilterEvent = (PFN_XFilterEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFilterEvent");
    _lum.x11.xlib.FindContext = (PFN_XFindContext)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFindContext");
    _lum.x11.xlib.Flush = (PFN_XFlush)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFlush");
    _lum.x11.xlib.Free = (PFN_XFree)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFree");
    _lum.x11.xlib.FreeColormap = (PFN_XFreeColormap)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFreeColormap");
    _lum.x11.xlib.FreeCursor = (PFN_XFreeCursor)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFreeCursor");
    _lum.x11.xlib.FreeEventData = (PFN_XFreeEventData)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XFreeEventData");
    _lum.x11.xlib.GetErrorText = (PFN_XGetErrorText)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetErrorText");
    _lum.x11.xlib.GetEventData = (PFN_XGetEventData)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetEventData");
    _lum.x11.xlib.GetICValues = (PFN_XGetICValues)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetICValues");
    _lum.x11.xlib.GetIMValues = (PFN_XGetIMValues)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetIMValues");
    _lum.x11.xlib.GetInputFocus = (PFN_XGetInputFocus)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetInputFocus");
    _lum.x11.xlib.GetKeyboardMapping = (PFN_XGetKeyboardMapping)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetKeyboardMapping");
    _lum.x11.xlib.GetScreenSaver = (PFN_XGetScreenSaver)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetScreenSaver");
    _lum.x11.xlib.GetSelectionOwner = (PFN_XGetSelectionOwner)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetSelectionOwner");
    _lum.x11.xlib.GetVisualInfo = (PFN_XGetVisualInfo)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetVisualInfo");
    _lum.x11.xlib.GetWMNormalHints = (PFN_XGetWMNormalHints)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetWMNormalHints");
    _lum.x11.xlib.GetWindowAttributes = (PFN_XGetWindowAttributes)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetWindowAttributes");
    _lum.x11.xlib.GetWindowProperty = (PFN_XGetWindowProperty)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGetWindowProperty");
    _lum.x11.xlib.GrabPointer = (PFN_XGrabPointer)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XGrabPointer");
    _lum.x11.xlib.IconifyWindow = (PFN_XIconifyWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XIconifyWindow");
    _lum.x11.xlib.InternAtom = (PFN_XInternAtom)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XInternAtom");
    _lum.x11.xlib.LookupString = (PFN_XLookupString)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XLookupString");
    _lum.x11.xlib.MapRaised = (PFN_XMapRaised)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XMapRaised");
    _lum.x11.xlib.MapWindow = (PFN_XMapWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XMapWindow");
    _lum.x11.xlib.MoveResizeWindow = (PFN_XMoveResizeWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XMoveResizeWindow");
    _lum.x11.xlib.MoveWindow = (PFN_XMoveWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XMoveWindow");
    _lum.x11.xlib.NextEvent = (PFN_XNextEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XNextEvent");
    _lum.x11.xlib.OpenIM = (PFN_XOpenIM)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XOpenIM");
    _lum.x11.xlib.PeekEvent = (PFN_XPeekEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XPeekEvent");
    _lum.x11.xlib.Pending = (PFN_XPending)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XPending");
    _lum.x11.xlib.QueryExtension = (PFN_XQueryExtension)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XQueryExtension");
    _lum.x11.xlib.QueryPointer = (PFN_XQueryPointer)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XQueryPointer");
    _lum.x11.xlib.RaiseWindow = (PFN_XRaiseWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XRaiseWindow");
    _lum.x11.xlib.RegisterIMInstantiateCallback = (PFN_XRegisterIMInstantiateCallback)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XRegisterIMInstantiateCallback");
    _lum.x11.xlib.ResizeWindow = (PFN_XResizeWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XResizeWindow");
    _lum.x11.xlib.ResourceManagerString = (PFN_XResourceManagerString)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XResourceManagerString");
    _lum.x11.xlib.SaveContext = (PFN_XSaveContext)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSaveContext");
    _lum.x11.xlib.SelectInput = (PFN_XSelectInput)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSelectInput");
    _lum.x11.xlib.SendEvent = (PFN_XSendEvent)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSendEvent");
    _lum.x11.xlib.SetClassHint = (PFN_XSetClassHint)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetClassHint");
    _lum.x11.xlib.SetErrorHandler = (PFN_XSetErrorHandler)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetErrorHandler");
    _lum.x11.xlib.SetICFocus = (PFN_XSetICFocus)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetICFocus");
    _lum.x11.xlib.SetIMValues = (PFN_XSetIMValues)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetIMValues");
    _lum.x11.xlib.SetInputFocus = (PFN_XSetInputFocus)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetInputFocus");
    _lum.x11.xlib.SetLocaleModifiers = (PFN_XSetLocaleModifiers)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetLocaleModifiers");
    _lum.x11.xlib.SetScreenSaver = (PFN_XSetScreenSaver)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetScreenSaver");
    _lum.x11.xlib.SetSelectionOwner = (PFN_XSetSelectionOwner)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetSelectionOwner");
    _lum.x11.xlib.SetWMHints = (PFN_XSetWMHints)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetWMHints");
    _lum.x11.xlib.SetWMNormalHints = (PFN_XSetWMNormalHints)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetWMNormalHints");
    _lum.x11.xlib.SetWMProtocols = (PFN_XSetWMProtocols)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSetWMProtocols");
    _lum.x11.xlib.SupportsLocale = (PFN_XSupportsLocale)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSupportsLocale");
    _lum.x11.xlib.Sync = (PFN_XSync)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XSync");
    _lum.x11.xlib.TranslateCoordinates = (PFN_XTranslateCoordinates)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XTranslateCoordinates");
    _lum.x11.xlib.UndefineCursor = (PFN_XUndefineCursor)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XUndefineCursor");
    _lum.x11.xlib.UngrabPointer = (PFN_XUngrabPointer)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XUngrabPointer");
    _lum.x11.xlib.UnmapWindow = (PFN_XUnmapWindow)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XUnmapWindow");
    _lum.x11.xlib.UnsetICFocus = (PFN_XUnsetICFocus)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XUnsetICFocus");
    _lum.x11.xlib.VisualIDFromVisual = (PFN_XVisualIDFromVisual)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XVisualIDFromVisual");
    _lum.x11.xlib.WarpPointer = (PFN_XWarpPointer)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XWarpPointer");
    _lum.x11.xkb.FreeKeyboard = (PFN_XkbFreeKeyboard)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbFreeKeyboard");
    _lum.x11.xkb.FreeNames = (PFN_XkbFreeNames)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbFreeNames");
    _lum.x11.xkb.GetMap = (PFN_XkbGetMap)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbGetMap");
    _lum.x11.xkb.GetNames = (PFN_XkbGetNames)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbGetNames");
    _lum.x11.xkb.GetState = (PFN_XkbGetState)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbGetState");
    _lum.x11.xkb.KeycodeToKeysym = (PFN_XkbKeycodeToKeysym)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbKeycodeToKeysym");
    _lum.x11.xkb.QueryExtension = (PFN_XkbQueryExtension)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbQueryExtension");
    _lum.x11.xkb.SelectEventDetails = (PFN_XkbSelectEventDetails)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbSelectEventDetails");
    _lum.x11.xkb.SetDetectableAutoRepeat = (PFN_XkbSetDetectableAutoRepeat)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XkbSetDetectableAutoRepeat");
    _lum.x11.xrm.DestroyDatabase = (PFN_XrmDestroyDatabase)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XrmDestroyDatabase");
    _lum.x11.xrm.GetResource = (PFN_XrmGetResource)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XrmGetResource");
    _lum.x11.xrm.GetStringDatabase = (PFN_XrmGetStringDatabase)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XrmGetStringDatabase");
    _lum.x11.xrm.UniqueQuark = (PFN_XrmUniqueQuark)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XrmUniqueQuark");
    _lum.x11.xlib.UnregisterIMInstantiateCallback = (PFN_XUnregisterIMInstantiateCallback)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "XUnregisterIMInstantiateCallback");
    _lum.x11.xlib.utf8LookupString = (PFN_Xutf8LookupString)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "Xutf8LookupString");
    _lum.x11.xlib.utf8SetWMProperties = (PFN_Xutf8SetWMProperties)
        _lumPlatformGetModuleSymbol(_lum.x11.xlib.handle, "Xutf8SetWMProperties");

    if (_lum.x11.xlib.utf8LookupString && _lum.x11.xlib.utf8SetWMProperties)
        _lum.x11.xlib.utf8 = LUM_TRUE;

    _lum.x11.screen = DefaultScreen(_lum.x11.display);
    _lum.x11.root = RootWindow(_lum.x11.display, _lum.x11.screen);
    _lum.x11.context = XUniqueContext();

    getSystemContentScale(&_lum.x11.contentScaleX, &_lum.x11.contentScaleY);

    if (!initExtensions())
        return LUM_FALSE;

    _lum.x11.helperWindowHandle = createHelperWindow();
    _lum.x11.hiddenCursorHandle = createHiddenCursor();

    if (XSupportsLocale() && _lum.x11.xlib.utf8)
    {
        XSetLocaleModifiers("");

        // If an IM is already present our callback will be called right away
        XRegisterIMInstantiateCallback(_lum.x11.display,
                                       NULL, NULL, NULL,
                                       inputMethodInstantiateCallback,
                                       NULL);
    }

    _lumPollMonitorsX11();
    return LUM_TRUE;
}

void _lumTerminateX11(void)
{
    if (_lum.x11.helperWindowHandle)
    {
        if (XGetSelectionOwner(_lum.x11.display, _lum.x11.CLIPBOARD) ==
            _lum.x11.helperWindowHandle)
        {
            _lumPushSelectionToManagerX11();
        }

        XDestroyWindow(_lum.x11.display, _lum.x11.helperWindowHandle);
        _lum.x11.helperWindowHandle = None;
    }

    if (_lum.x11.hiddenCursorHandle)
    {
        XFreeCursor(_lum.x11.display, _lum.x11.hiddenCursorHandle);
        _lum.x11.hiddenCursorHandle = (Cursor) 0;
    }

    _lum_free(_lum.x11.primarySelectionString);
    _lum_free(_lum.x11.clipboardString);

    XUnregisterIMInstantiateCallback(_lum.x11.display,
                                     NULL, NULL, NULL,
                                     inputMethodInstantiateCallback,
                                     NULL);

    if (_lum.x11.im)
    {
        XCloseIM(_lum.x11.im);
        _lum.x11.im = NULL;
    }

    if (_lum.x11.display)
    {
        XCloseDisplay(_lum.x11.display);
        _lum.x11.display = NULL;
    }

    if (_lum.x11.x11xcb.handle)
    {
        _lumPlatformFreeModule(_lum.x11.x11xcb.handle);
        _lum.x11.x11xcb.handle = NULL;
    }

    if (_lum.x11.xcursor.handle)
    {
        _lumPlatformFreeModule(_lum.x11.xcursor.handle);
        _lum.x11.xcursor.handle = NULL;
    }

    if (_lum.x11.randr.handle)
    {
        _lumPlatformFreeModule(_lum.x11.randr.handle);
        _lum.x11.randr.handle = NULL;
    }

    if (_lum.x11.xinerama.handle)
    {
        _lumPlatformFreeModule(_lum.x11.xinerama.handle);
        _lum.x11.xinerama.handle = NULL;
    }

    if (_lum.x11.xrender.handle)
    {
        _lumPlatformFreeModule(_lum.x11.xrender.handle);
        _lum.x11.xrender.handle = NULL;
    }

    if (_lum.x11.vidmode.handle)
    {
        _lumPlatformFreeModule(_lum.x11.vidmode.handle);
        _lum.x11.vidmode.handle = NULL;
    }

    if (_lum.x11.xi.handle)
    {
        _lumPlatformFreeModule(_lum.x11.xi.handle);
        _lum.x11.xi.handle = NULL;
    }

    // NOTE: These need to be unloaded after XCloseDisplay, as they register
    //       cleanup callbacks that get called by that function
    _lumTerminateEGL();
    _lumTerminateGLX();

    if (_lum.x11.xlib.handle)
    {
        _lumPlatformFreeModule(_lum.x11.xlib.handle);
        _lum.x11.xlib.handle = NULL;
    }
}

